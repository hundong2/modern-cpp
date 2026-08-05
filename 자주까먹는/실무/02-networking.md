# 2장: TCP 소켓과 이벤트 기반 I/O

## Day 2: TCP 연결

서버의 수명은 `socket → bind → listen → accept → read/write → close`, 클라이언트는 `socket → connect → read/write → close`다. listening socket은 새 연결을 받는 문지기이고, `accept`가 돌려준 connected socket은 특정 연결의 바이트 스트림이다. 하나를 분리해야 다음 연결을 받으면서 기존 연결과 통신할 수 있다.

[TCP](wiki/tcp-stream.md)는 메시지 경계가 없다. 원문 에코 코드는 입문용으로는 좋지만 실제 코드는 다음을 보완해야 한다.

- 모든 시스템 호출 반환값과 `errno` 검사
- `send_all` 또는 출력 큐를 통한 부분 전송 처리
- `SIGPIPE` 방지(Linux의 `MSG_NOSIGNAL` 등)
- 길이 제한, timeout, slow client에 대한 [backpressure](wiki/event-loop.md)
- `inet_ntop` 사용(`inet_ntoa`는 공유 정적 버퍼 문제)
- `SO_REUSEADDR`의 의미를 “모든 bind 오류 방지”로 오해하지 않기

실습: [서버](examples/day02_tcp_server_posix.cpp), [클라이언트](examples/day02_tcp_client_posix.cpp). POSIX/Linux 전용이다.

## Day 3: thread-per-client

연결마다 스레드를 만들면 블로킹 코드를 직선적으로 작성할 수 있다. 그러나 [스레드](wiki/process-thread.md)의 스택·스케줄링·동기화 비용이 연결 수와 함께 증가한다. `detach()`는 “안전한 수명 관리”가 아니라 join 책임을 버리는 동작이므로, 프로세스 종료·서버 객체 파괴·예외 시점의 수명을 더 어렵게 만들 수 있다. `std::jthread`(C++20), 명시적 join, 스레드 풀을 고려한다.

공유 접속자 수에는 [mutex 또는 atomic](wiki/concurrency.md)이 필요하다. 로그 여러 줄의 일관성과 카운터 원자성은 별도 문제다.

## Day 4: `select`

`select`는 매 루프마다 관심 `fd_set`의 복사본을 넘기며 호출 후 집합에는 준비된 fd만 남는다. 따라서 master set을 따로 유지하거나 매번 재구성한다. `maxfd + 1`과 `FD_SETSIZE` 범위를 지켜야 한다. 연결 목록을 순회하므로 규모가 커지면 비효율적이지만 작은 이식성 도구에는 여전히 유용하다.

실습: [day04_select_model.cpp](examples/day04_select_model.cpp)

## Day 5: `epoll`

Linux `epoll`은 커널에 관심 집합을 유지한다. `O_NONBLOCK` 소켓에서 `accept`/`read`를 반복하고 `EAGAIN`은 “지금 더 없음”으로 처리한다. [LT/ET 차이](wiki/event-loop.md)를 이해하기 전에는 LT가 안전한 출발점이다. ET에서는 이벤트 하나마다 반드시 drain해야 한다.

실습: [day05_epoll_model.cpp](examples/day05_epoll_model.cpp)

정정: `epoll`을 “무조건 O(1)”로, IOCP를 “진정한 zero-copy”로 부르면 안 된다. 통지 모델, 준비 이벤트 수, 버퍼 복사, 드라이버·API 선택은 독립적인 축이다. [zero-copy 용어](wiki/zero-copy.md)를 참고한다.

## Day 6: Reactor와 프레이밍

[Reactor](wiki/event-loop.md)는 준비 이벤트를 demultiplex하고 handler에 전달한다. 네트워크 코어는 read-ready를 받고 바이트를 수신 버퍼에 축적한다. 패킷 parser는 [길이 접두사 프레임](wiki/tcp-stream.md)을 “헤더가 충분한가 → 길이가 유효한가 → body가 충분한가” 순서로 반복 추출한다.

구조체를 `#pragma pack(1)`로 싸서 그대로 보내는 방식은 ABI/정렬/endian/버전 문제를 만든다. 교육용 wire header도 정수 필드를 명시적으로 encode/decode하고 최대 body 길이를 검증한다. [직렬화 위키](wiki/serialization.md)를 참고한다.

실습: [day06_packet_framing.cpp](examples/day06_packet_framing.cpp)

## 장애 상황 연습

1. 클라이언트가 헤더 1바이트만 보내고 멈춘다.
2. 길이 필드에 4GiB를 적는다.
3. `send`가 절반만 성공한다.
4. 한 번의 `read`에 패킷 2.5개가 들어온다.
5. 처리 속도보다 수신 속도가 빠르다.

각 경우에 블로킹, 메모리 증가, 연결 종료 정책, 출력 큐 상한이 어떻게 동작해야 하는지 적어 본다.
