# 모던 C++ 실무 서버 프로그래밍 가이드

이 교재는 [공유 원문](https://share.gemini.google/xfEQ65qqiUOG)의 C++ 복습 과정 전체를 초보자가 직접 빌드하고 관찰할 수 있는 형태로 다시 설계한 실습서다. 원문의 1~23일차(문법, TCP, 동시성, `select`/`epoll`, Reactor, 패킷 조립, 스레드 풀, Boost.Asio, 코루틴, Protobuf, Redis, MySQL, CPU 캐시, UDP/RUDP, 오브젝트 풀, lock-free, 게임 루프, AOI, 보간)를 모두 다룬다.

> 먼저 읽을 사람: 포인터·객체 수명·운영체제·네트워크 용어가 낯설다면 [0장: 실행 모델](00-foundations.md)부터 읽는다. C++ 문법이 익숙하다면 [학습 지도](#학습-지도)에서 필요한 장으로 이동한다.

## 이 교재의 약속

- 실행 가능한 예제의 각 의미 있는 줄에는 그 줄의 목적·수명·비용·위험을 설명하는 주석이 있다.
- 원문의 교육용 단순화를 그대로 사실로 외우지 않도록 `정정` 상자를 둔다.
- [RAII](wiki/raii.md), [값 범주](wiki/value-categories.md), [TCP](wiki/tcp-stream.md)처럼 처음 등장하는 기술 용어는 위키 문서로 연결한다.
- 기본 예제는 C++17, 코루틴 예제는 C++20이다. Linux 전용 API와 외부 라이브러리 예제는 별도로 표시한다.

## 학습 지도

| 단계 | 읽을 문서 | 실습 | 도달 목표 |
|---|---|---|---|
| 0 | [컴퓨터와 C++ 실행 모델](00-foundations.md) | `example.cpp` | 소스→컴파일→프로세스, 스택·힙 이해 |
| 1 | [모던 C++ 언어와 자원 관리](01-modern-cpp.md) | `day01`~`day01c` | 템플릿, STL, 스마트 포인터, 이동 의미론 |
| 2 | [TCP 소켓과 스트림](02-networking.md) | `day02`~`day06` | 연결 수명, 부분 I/O, 멀티플렉싱, 패킷 조립 |
| 3 | [동시성과 비동기 실행](03-concurrency.md) | `day03`, `day08`~`day12` | mutex, pool, future, Asio, coroutine |
| 4 | [외부 시스템과 계층화](04-production.md) | `day07`, `day13`~`day16` | 직렬화, 캐시/DB, 계층 경계와 생명주기 |
| 5 | [성능과 게임 서버](05-performance-game.md) | `day17`~`day23` | 캐시, UDP, 풀, CAS, tick, AOI, 보간 |
| 검토 | [초보자 재검토·막힘 해결표](06-beginner-review.md) | 오류를 말로 설명 | 모르는 코드를 읽는 절차 정착 |
| 비교 | [C#/Python과 C++ 비교](compare.md) | 언어별 사고 비교 | GC 언어의 습관을 C++에 잘못 옮기지 않기 |

## 23일 완주 순서

1. Day 1: 템플릿, `map`/`list`, `shared_ptr` — [문서](01-modern-cpp.md#day-1-템플릿stl스마트-포인터) / [코드](examples/day01_templates_smart_ptr.cpp)
2. 특별편: 매크로, `inline`, `constexpr`, lvalue/rvalue — [문서](01-modern-cpp.md#특별편-매크로와-값-범주) / [코드](examples/day01b_value_move.cpp), [코드](examples/day01c_inline_constexpr.cpp)
3. Day 2: TCP 에코 서버/클라이언트 — [문서](02-networking.md#day-2-tcp-연결) / [서버](examples/day02_tcp_server_posix.cpp), [클라이언트](examples/day02_tcp_client_posix.cpp)
4. Day 3: thread-per-client와 동기화 — [문서](03-concurrency.md#day-3-스레드와-뮤텍스) / [코드](examples/day03_threaded_counter.cpp)
5. Day 4: `select` — [문서](02-networking.md#day-4-select) / [코드](examples/day04_select_model.cpp)
6. Day 5: `epoll` + non-blocking — [문서](02-networking.md#day-5-epoll) / [코드](examples/day05_epoll_model.cpp)
7. Day 6: Reactor와 TCP 패킷 조립 — [문서](02-networking.md#day-6-reactor와-프레이밍) / [코드](examples/day06_packet_framing.cpp)
8. Day 7: 네트워크/비즈니스 계층 분리 — [문서](04-production.md#day-7-계층화) / [코드](examples/day07_layered_server.cpp)
9. Day 8: 스레드 풀 — [문서](03-concurrency.md#day-8-스레드-풀) / [코드](examples/day08_thread_pool.cpp)
10. Day 9: `future` 반환값 — [문서](03-concurrency.md#day-9-future) / [코드](examples/day09_future.cpp)
11. Day 10: Boost.Asio 비동기 세션 — [문서](03-concurrency.md#day-10-11-boostasio와-strand) / [코드](examples/day10_asio_lifetime.cpp)
12. Day 11: 멀티스레드 Asio와 strand — [문서](03-concurrency.md#day-10-11-boostasio와-strand) / [코드](examples/day11_strand_model.cpp)
13. Day 12: C++20 코루틴 — [문서](03-concurrency.md#day-12-코루틴) / [코드](examples/day12_coroutine_model.cpp)
14. Day 13: Protobuf — [문서](04-production.md#day-13-protobuf) / [스키마](examples/day13_packet.proto), [코드](examples/day13_protobuf_model.cpp)
15. Day 14: Redis 캐시 — [문서](04-production.md#day-14-redis) / [코드](examples/day14_redis_model.cpp)
16. Day 15: MySQL 비동기 경계 — [문서](04-production.md#day-15-mysql) / [코드](examples/day15_mysql_model.cpp)
17. Day 16: MMORPG 4계층 조감도 — [문서](04-production.md#day-16-전체-아키텍처) / [코드](examples/day16_packet_pipeline.cpp)
18. Day 17: cache line과 false sharing — [문서](05-performance-game.md#day-17-false-sharing) / [코드](examples/day17_false_sharing.cpp)
19. Day 18: UDP/RUDP — [문서](05-performance-game.md#day-18-udprudp) / [코드](examples/day18_udp_model.cpp)
20. Day 19: 오브젝트 풀 — [문서](05-performance-game.md#day-19-오브젝트-풀) / [코드](examples/day19_object_pool.cpp)
21. Day 20: CAS와 lock-free — [문서](05-performance-game.md#day-20-cas와-lock-free) / [코드](examples/day20_cas_stack_model.cpp)
22. Day 21: game loop와 delta time — [문서](05-performance-game.md#day-21-게임-루프) / [코드](examples/day21_game_loop.cpp)
23. Day 22: grid AOI — [문서](05-performance-game.md#day-22-aoi) / [코드](examples/day22_grid_aoi.cpp)
24. Day 23: dead reckoning과 interpolation — [문서](05-performance-game.md#day-23-추측항법과-보간) / [코드](examples/day23_interpolation.cpp)

## 빌드와 첫 실행

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Windows에서 C++ compiler/generator가 설치되지 않아 `nmake` 또는 `CMAKE_CXX_COMPILER` 오류가 나면 코드 오류가 아니라 도구chain 부재다. Visual Studio의 **Desktop development with C++** workload나 Ninja+Clang/GCC를 설치해 명시적 generator를 선택한다. WSL Ubuntu에 `g++`와 CMake가 있다면 다음처럼 검증할 수 있다.

```powershell
wsl bash -lc 'cd /mnt/d/workspace/modern-cpp/자주까먹는/실무 && cmake -S . -B build-wsl -DBUILD_POSIX_NETWORK=ON && cmake --build build-wsl -j4 && ctest --test-dir build-wsl --output-on-failure'
```

Linux 전용 소켓 예제까지 빌드하려면 Linux/WSL에서 다음을 사용한다.

```bash
cmake -S . -B build -DBUILD_POSIX_NETWORK=ON
cmake --build build
```

외부 서비스가 필요한 Boost/Protobuf/Redis/MySQL 코드는 실제 SDK 호출과 동일한 생명주기 구조를 표준 C++ 모형으로 먼저 익히게 한다. 실제 라이브러리 설치·연결 명령은 [외부 시스템 장](04-production.md)에 있다.

## 원문에서 바로잡은 핵심

- `std::move`는 이동 자체가 아니라 [rvalue로 캐스팅](wiki/value-categories.md)할 뿐이다. 실제 이동 여부는 호출되는 생성자/대입 연산자가 정한다.
- 이동은 일반적으로 싸지만 언제나 `O(1)`이 아니며, C++의 이동을 OS의 [zero-copy](wiki/zero-copy.md)와 동일시하면 안 된다.
- `epoll` 또는 IOCP를 무조건 `O(1)`이라고 부르는 것은 과도한 단순화다. 준비된 이벤트 수, 등록 변경, 실제 I/O 비용이 남는다.
- TCP는 메시지가 아니라 [바이트 스트림](wiki/tcp-stream.md)이다. 한 번의 `write`가 한 번의 `read`와 대응하지 않는다.
- `write`/`send`는 일부만 처리할 수 있고, 오류와 `EINTR`/`EAGAIN`을 구분해야 한다.
- lock-free는 wait-free가 아니며, 단순 포인터 CAS 스택은 [ABA와 메모리 회수](wiki/lock-free.md) 문제 때문에 그대로 배포할 수 없다.

## 다음 행동

처음이라면 [0장](00-foundations.md)을 읽고 `example.cpp`를 실행한 뒤, 각 Day의 `관찰 질문`에 답한다. 답을 보기 전 [exercise.cpp](exercise.cpp)의 TODO를 먼저 해결해 보는 것이 가장 빠른 학습 방법이다.

처음 읽고 막혔다면 [초보자 재검토·막힘 해결표](06-beginner-review.md)에서 “컴파일과 실행의 차이”, “`*`/`&`가 문맥마다 다른 이유”, “동기·비동기·blocking의 차이”부터 보완한다.
