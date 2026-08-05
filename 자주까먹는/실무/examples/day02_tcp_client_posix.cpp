#include <arpa/inet.h>  // inet_pton과 sockaddr_in을 제공한다.
#include <cerrno>       // errno를 제공한다.
#include <cstring>      // strerror를 제공한다.
#include <iostream>     // 사용자 입력과 echo 출력을 제공한다.
#include <string>       // 입력 메시지를 소유한다.
#include <sys/socket.h> // socket/connect/send/recv를 선언한다.
#include <unistd.h>     // close를 선언한다.

int main() {                                         // blocking echo client를 시작한다.
    const int socket_fd = ::socket(AF_INET, SOCK_STREAM, 0); // IPv4 TCP socket을 만든다.
    if (socket_fd < 0) { std::cerr << std::strerror(errno) << '\n'; return 1; } // 생성 실패를 처리한다.
    sockaddr_in server{};                            // 서버 주소 구조체를 0으로 초기화한다.
    server.sin_family = AF_INET;                     // IPv4 주소임을 지정한다.
    server.sin_port = htons(8585);                   // port를 network byte order로 저장한다.
    if (::inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) != 1) { ::close(socket_fd); return 1; } // 문자열 IP를 binary로 변환한다.
    if (::connect(socket_fd, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) { // TCP handshake를 요청한다.
        std::cerr << std::strerror(errno) << '\n'; ::close(socket_fd); return 1; // 실패 시 fd를 정리한다.
    }                                                // 연결된 byte stream이 준비됐다.
    const std::string message = "hello";            // 전송할 다섯 byte를 소유한다.
    std::size_t sent = 0;                            // 부분 send를 추적한다.
    while (sent < message.size()) {                  // 모든 byte가 kernel에 수락될 때까지 반복한다.
        const ssize_t n = ::send(socket_fd, message.data() + sent, message.size() - sent, MSG_NOSIGNAL); // 남은 범위를 보낸다.
        if (n > 0) { sent += static_cast<std::size_t>(n); continue; } // 성공량만큼 전진한다.
        if (n < 0 && errno == EINTR) continue;       // signal interruption만 재시도한다.
        ::close(socket_fd); return 1;                // 그 밖의 실패에서는 연결을 정리한다.
    }                                                // 전체 message 전송이 끝났다.
    std::string echoed(message.size(), '\0');       // 정확히 같은 길이의 echo 저장소를 만든다.
    std::size_t received = 0;                        // 부분 recv를 추적한다.
    while (received < echoed.size()) {               // 기대한 길이가 모두 올 때까지 반복한다.
        const ssize_t n = ::recv(socket_fd, echoed.data() + received, echoed.size() - received, 0); // 남은 범위를 읽는다.
        if (n > 0) { received += static_cast<std::size_t>(n); continue; } // 수신량만큼 전진한다.
        if (n < 0 && errno == EINTR) continue;       // signal interruption을 재시도한다.
        ::close(socket_fd); return 1;                // EOF/오류는 불완전 echo 실패다.
    }                                                // 전체 echo 수신이 끝났다.
    std::cout << echoed << '\n';                    // hello를 출력한다.
    ::close(socket_fd);                              // connected socket을 반환한다.
    return echoed == message ? 0 : 1;               // 내용까지 자체 검증한다.
}                                                    // client를 끝낸다.
