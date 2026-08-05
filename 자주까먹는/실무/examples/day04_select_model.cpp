#include <cerrno>       // EINTR를 구분한다.
#include <iostream>     // 준비 event 결과를 출력한다.
#include <sys/select.h> // fd_set과 select를 제공한다.
#include <unistd.h>     // pipe/read/write/close를 제공한다.

int main() {                                         // socket 대신 pipe로 select 핵심을 재현한다.
    int fds[2]{};                                    // fds[0]은 읽기, fds[1]은 쓰기 끝이 된다.
    if (::pipe(fds) < 0) return 1;                   // 커널 byte stream 한 쌍을 만든다.
    const char byte = 'X';                           // readiness를 만들 한 byte다.
    if (::write(fds[1], &byte, 1) != 1) return 1;   // 읽기 끝을 ready 상태로 만든다.
    fd_set reads;                                    // select가 수정할 관심 집합 복사본이다.
    FD_ZERO(&reads);                                 // 쓰레기 bit를 모두 지운다.
    FD_SET(fds[0], &reads);                          // pipe 읽기 fd를 감시 대상으로 넣는다.
    int ready;                                       // EINTR 재시도 결과를 받을 변수다.
    do { ready = ::select(fds[0] + 1, &reads, nullptr, nullptr, nullptr); } while (ready < 0 && errno == EINTR); // 준비까지 block한다.
    if (ready != 1 || !FD_ISSET(fds[0], &reads)) return 1; // 반환 수와 준비 bit를 모두 확인한다.
    char received = '\0';                           // 실제 read 결과를 받을 byte다.
    if (::read(fds[0], &received, 1) != 1) return 1; // readiness 뒤에도 read 반환값을 검사한다.
    std::cout << received << '\n';                  // X를 출력한다.
    ::close(fds[0]);                                 // pipe 읽기 커널 자원을 반환한다.
    ::close(fds[1]);                                 // pipe 쓰기 커널 자원을 반환한다.
    return received == 'X' ? 0 : 1;                 // select 모형을 자체 검증한다.
}                                                    // 프로그램을 끝낸다.
