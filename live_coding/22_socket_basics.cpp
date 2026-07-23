/*
Chapter 22. TCP 소켓, partial send/recv, RAII 기초

목표:
- 실무 C++ 네트워크 코드를 읽을 때 자주 보는 문제를 찾는다.
- 소켓은 C++ 표준 라이브러리가 아니라 OS API다. Windows는 WinSock, Linux/macOS는 POSIX socket을 쓴다.

리뷰 체크리스트:
- send가 요청한 바이트를 항상 전부 보낸다고 가정하고 있지 않은가?
- recv가 메시지 하나를 정확히 한 번에 준다고 가정하고 있지 않은가?
- socket close/freeaddrinfo/WSACleanup 같은 자원 해제가 누락되지 않았는가?
- blocking socket을 UI thread 또는 중요 worker에서 무제한 대기시키고 있지 않은가?
- 프로토콜 파싱에서 줄 끝, 길이, timeout, partial frame을 고려하고 있는가?
*/

#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
using SocketHandle = SOCKET;
const SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
using SocketHandle = int;
const SocketHandle kInvalidSocket = -1;
#endif

using namespace std;

class SocketRuntime {
public:
    SocketRuntime() {
#ifdef _WIN32
        WSADATA data{};
        int rc = WSAStartup(MAKEWORD(2, 2), &data); // Windows에서는 소켓 API 사용 전 WinSock 초기화가 필요하다.
        if (rc != 0) throw runtime_error("WSAStartup failed");
#endif
    }

    ~SocketRuntime() {
#ifdef _WIN32
        WSACleanup(); // 프로세스에서 WinSock 사용이 끝났음을 OS에 알린다.
#endif
    }
};

void closeSocket(SocketHandle handle) {
    if (handle == kInvalidSocket) return;
#ifdef _WIN32
    closesocket(handle); // Windows socket은 close가 아니라 closesocket으로 닫는다.
#else
    close(handle);       // POSIX에서는 파일 디스크립터와 같은 close를 사용한다.
#endif
}

class UniqueSocket {
public:
    UniqueSocket() = default;
    explicit UniqueSocket(SocketHandle handle) : handle(handle) {}

    UniqueSocket(const UniqueSocket&) = delete;
    UniqueSocket& operator=(const UniqueSocket&) = delete;

    UniqueSocket(UniqueSocket&& other) noexcept : handle(exchange(other.handle, kInvalidSocket)) {}

    UniqueSocket& operator=(UniqueSocket&& other) noexcept {
        if (this != &other) {
            reset();                                      // 기존 소켓을 먼저 닫아 descriptor leak을 막는다.
            handle = exchange(other.handle, kInvalidSocket); // ownership을 이동하고 원본은 invalid로 만든다.
        }
        return *this;
    }

    ~UniqueSocket() {
        reset(); // 예외가 나도 소켓이 닫히도록 RAII로 감싼다.
    }

    SocketHandle get() const {
        return handle;
    }

    bool valid() const {
        return handle != kInvalidSocket;
    }

    void reset(SocketHandle next = kInvalidSocket) {
        closeSocket(handle);
        handle = next;
    }

private:
    SocketHandle handle = kInvalidSocket;
};

UniqueSocket connectTcp(const string& host, const string& port) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;     // IPv4와 IPv6를 모두 허용한다.
    hints.ai_socktype = SOCK_STREAM; // TCP는 stream socket이다.

    addrinfo* rawResult = nullptr;
    int rc = getaddrinfo(host.c_str(), port.c_str(), &hints, &rawResult); // DNS와 주소 변환을 OS에 맡긴다.
    if (rc != 0) throw runtime_error("getaddrinfo failed");

    unique_ptr<addrinfo, decltype(&freeaddrinfo)> result(rawResult, freeaddrinfo); // unique_ptr은 단독 소유 RAII 포인터이며 custom deleter로 C API 자원을 닫는다.

    for (addrinfo* p = result.get(); p != nullptr; p = p->ai_next) {
        UniqueSocket candidate(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
        if (!candidate.valid()) continue;

        if (connect(candidate.get(), p->ai_addr, (int)p->ai_addrlen) == 0) {
            return candidate; // connect 성공 시 소켓 ownership을 호출자에게 이동한다.
        }
    }

    throw runtime_error("connect failed");
}

bool sendAll(SocketHandle socket, const string& data) {
    size_t sentTotal = 0;
    while (sentTotal < data.size()) {
        const char* current = data.data() + sentTotal; // 아직 보내지 못한 첫 바이트 위치다.
        size_t remaining = data.size() - sentTotal;

#ifdef _WIN32
        int sent = send(socket, current, (int)remaining, 0);
#else
        ssize_t sent = send(socket, current, remaining, 0);
#endif
        if (sent <= 0) return false; // send는 일부만 보내거나 실패할 수 있으므로 반드시 반환값을 확인한다.
        sentTotal += (size_t)sent;
    }
    return true;
}

string receiveSome(SocketHandle socket, size_t maxBytes) {
    vector<char> buffer(maxBytes); // recv용 임시 버퍼는 크기가 런타임에 정해지므로 vector<char>가 안전하다.
#ifdef _WIN32
    int received = recv(socket, buffer.data(), (int)buffer.size(), 0);
#else
    ssize_t received = recv(socket, buffer.data(), buffer.size(), 0);
#endif
    if (received <= 0) return {}; // 0은 정상 종료, 음수는 오류다. 실제 서비스에서는 errno/WSAGetLastError를 확인한다.
    return string(buffer.data(), buffer.data() + received); // recv는 문자열 끝의 '\0'을 보장하지 않는다.
}

string buildHttpGetRequest(const string& host, const string& path) {
    string request; // std::string은 길이를 따로 저장하므로 중간에 '\0'이 있어도 데이터로 보관할 수 있다.
    request += "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n"; // HTTP header의 끝은 빈 줄(CRLF CRLF)이다.
    return request;
}

string parseStatusLine(const string& response) {
    size_t end = response.find("\r\n");
    if (end == string::npos) return {}; // 첫 줄이 아직 다 도착하지 않았을 수 있다.
    return response.substr(0, end);
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string sample = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello";
    cout << "[status line] " << parseStatusLine(sample) << '\n';
    cout << "[request]\n" << buildHttpGetRequest("example.com", "/") << '\n';

    if (argc > 1 && string(argv[1]) == "--connect") {
        SocketRuntime runtime; // 실제 네트워크 호출을 할 때만 OS socket runtime을 초기화한다.
        UniqueSocket socket = connectTcp("example.com", "80");
        string request = buildHttpGetRequest("example.com", "/");
        if (sendAll(socket.get(), request)) {
            cout << "[received bytes]\n" << receiveSome(socket.get(), 1024) << '\n';
        }
    }

    return 0;
}
