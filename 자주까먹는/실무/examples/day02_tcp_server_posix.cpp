#include <arpa/inet.h>  // sockaddr_in과 host/network byte 변환을 제공한다.
#include <cerrno>       // 시스템 호출 실패 원인 errno를 읽는다.
#include <cstdint>      // std::uint16_t 고정 폭 포트 타입을 제공한다.
#include <cstring>      // strerror로 errno를 사람이 읽게 바꾼다.
#include <iostream>     // 서버 상태와 오류를 출력한다.
#include <string>       // 수신 buffer를 소유한다.
#include <sys/socket.h> // socket/bind/listen/accept/send를 선언한다.
#include <unistd.h>     // POSIX close/read를 선언한다.

namespace {                                          // 이 번역 단위 안에서만 helper가 보이게 한다.
constexpr std::uint16_t port = 8585;                 // host byte order의 교육용 포트다.

bool send_all(int fd, const char* data, std::size_t size) { // blocking socket에서 전체 범위를 보낸다.
    std::size_t sent = 0;                            // 이미 전송한 byte 수를 추적한다.
    while (sent < size) {                            // 부분 전송이면 남은 범위를 반복한다.
        const ssize_t n = ::send(fd, data + sent, size - sent, MSG_NOSIGNAL); // SIGPIPE 없이 전송을 시도한다.
        if (n > 0) { sent += static_cast<std::size_t>(n); continue; } // 실제 전송량만큼 전진한다.
        if (n < 0 && errno == EINTR) continue;       // signal interruption은 같은 작업을 다시 시도한다.
        return false;                                // 다른 오류/0은 연결 처리 실패로 돌린다.
    }                                                // 요청한 모든 byte를 전송했다.
    return true;                                     // 호출자에게 성공을 알린다.
}                                                    // helper 정의를 끝낸다.
}                                                    // 익명 namespace를 끝낸다.

int main() {                                         // 단일 연결 blocking echo server를 시작한다.
    const int listener = ::socket(AF_INET, SOCK_STREAM, 0); // IPv4 TCP listening socket을 만든다.
    if (listener < 0) { std::cerr << std::strerror(errno) << '\n'; return 1; } // 생성 실패를 검사한다.
    int reuse = 1;                                   // setsockopt가 읽을 정수 true 값이다.
    if (::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) { // 재시작 bind 정책을 설정한다.
        std::cerr << std::strerror(errno) << '\n'; ::close(listener); return 1; // 오류 시 listener를 정리한다.
    }                                                // socket option 설정이 끝났다.
    sockaddr_in address{};                           // padding까지 0으로 초기화한 IPv4 주소다.
    address.sin_family = AF_INET;                    // 구조체가 IPv4 주소임을 지정한다.
    address.sin_addr.s_addr = htonl(INADDR_ANY);     // 모든 local interface에 bind한다.
    address.sin_port = htons(port);                  // 16비트 port를 network byte order로 바꾼다.
    if (::bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 || ::listen(listener, 16) < 0) { // 주소 결합과 대기열을 연다.
        std::cerr << std::strerror(errno) << '\n'; ::close(listener); return 1; // 실패 경로에서 fd를 닫는다.
    }                                                // 새 연결을 받을 준비가 됐다.
    const int client = ::accept(listener, nullptr, nullptr); // 한 연결이 올 때까지 block하고 connected fd를 얻는다.
    if (client < 0) { std::cerr << std::strerror(errno) << '\n'; ::close(listener); return 1; } // accept 실패를 처리한다.
    char buffer[4096];                               // 한 read chunk를 받을 고정 저장소다.
    while (true) {                                   // peer 종료 또는 오류까지 echo한다.
        const ssize_t n = ::read(client, buffer, sizeof(buffer)); // TCP stream에서 가능한 만큼 읽는다.
        if (n > 0) {                                 // 일부 byte를 정상 수신했다.
            if (!send_all(client, buffer, static_cast<std::size_t>(n))) break; // 같은 byte 범위를 모두 echo한다.
            continue;                                // 다음 stream byte를 기다린다.
        }                                            // 0 또는 오류 경로다.
        if (n < 0 && errno == EINTR) continue;       // signal로 중단된 read만 재시도한다.
        break;                                       // 0 orderly shutdown 또는 다른 오류로 연결을 끝낸다.
    }                                                // echo loop를 끝낸다.
    ::close(client);                                 // connected socket의 커널 자원을 반환한다.
    ::close(listener);                               // listening socket도 반환한다.
    return 0;                                        // 정상 종료한다.
}                                                    // 프로세스가 끝난다.
