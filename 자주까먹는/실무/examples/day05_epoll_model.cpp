#include <cerrno>      // non-blocking 오류 EAGAIN을 구분한다.
#include <fcntl.h>     // O_NONBLOCK 설정을 제공한다.
#include <iostream>    // drain 결과를 출력한다.
#include <sys/epoll.h> // epoll_create1/ctl/wait를 제공한다.
#include <unistd.h>    // pipe/read/write/close를 제공한다.

int main() {                                          // Linux pipe로 epoll readiness를 재현한다.
    int fds[2]{};                                     // 읽기/쓰기 pipe 끝을 받을 배열이다.
    if (::pipe2(fds, O_NONBLOCK | O_CLOEXEC) < 0) return 1; // non-blocking이고 exec에 상속되지 않는 pipe를 만든다.
    const int epoll_fd = ::epoll_create1(EPOLL_CLOEXEC); // kernel 관심 집합 handle을 만든다.
    if (epoll_fd < 0) return 1;                       // 생성 실패를 처리한다.
    epoll_event interest{};                           // 등록할 event 구조체를 0 초기화한다.
    interest.events = EPOLLIN;                        // 읽을 수 있는 상태(LT)를 감시한다.
    interest.data.fd = fds[0];                        // 반환 event에서 대상을 식별할 fd를 저장한다.
    if (::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fds[0], &interest) < 0) return 1; // 관심 집합에 읽기 fd를 추가한다.
    const char bytes[] = "abc";                      // 한 event에서 drain할 세 byte다.
    if (::write(fds[1], bytes, 3) != 3) return 1;    // pipe를 readable 상태로 만든다.
    epoll_event event{};                              // 준비 event 하나를 받을 저장소다.
    int count;                                        // EINTR 재시도 결과를 받을 변수다.
    do { count = ::epoll_wait(epoll_fd, &event, 1, -1); } while (count < 0 && errno == EINTR); // event까지 block한다.
    if (count != 1 || event.data.fd != fds[0]) return 1; // 예상한 fd의 event인지 확인한다.
    char buffer[8]{};                                 // non-blocking drain 결과를 받을 buffer다.
    std::size_t used = 0;                             // 실제 누적 byte 수를 추적한다.
    while (true) {                                    // ET에서도 필요한 EAGAIN까지 drain 패턴이다.
        const ssize_t n = ::read(fds[0], buffer + used, sizeof(buffer) - used); // 남은 공간에 읽는다.
        if (n > 0) { used += static_cast<std::size_t>(n); continue; } // 수신량만큼 전진하고 더 읽는다.
        if (n < 0 && errno == EINTR) continue;        // signal interruption을 재시도한다.
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) break; // 지금 읽을 byte가 없으면 drain 완료다.
        break;                                        // EOF 또는 다른 오류는 loop를 끝낸다.
    }                                                 // drain을 끝냈다.
    std::cout.write(buffer, static_cast<std::streamsize>(used)); // null terminator 가정 없이 받은 byte를 출력한다.
    std::cout << '\n';                               // 로그 줄을 끝낸다.
    ::close(epoll_fd);                                // epoll kernel 객체를 반환한다.
    ::close(fds[0]);                                  // pipe 읽기 끝을 닫는다.
    ::close(fds[1]);                                  // pipe 쓰기 끝을 닫는다.
    return used == 3 ? 0 : 1;                        // 세 byte drain을 자체 검증한다.
}                                                     // Linux 예제를 끝낸다.
