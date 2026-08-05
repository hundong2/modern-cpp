# 5장: 성능과 게임 서버

## Day 17: false sharing

[CPU cache line](wiki/cpu-cache.md)을 공유하는 두 atomic을 서로 다른 코어가 계속 쓰면 coherence traffic이 늘어난다. `alignas`로 분리할 수 있지만 먼저 profiler로 병목을 확인한다. 벤치마크는 release build, 여러 회, CPU affinity/부하 환경을 기록해야 한다.

실습: [day17_false_sharing.cpp](examples/day17_false_sharing.cpp)

## Day 18: UDP/RUDP

UDP는 연결 없는 datagram 서비스이며 메시지 경계를 보존하지만 전달·순서·중복 방지를 보장하지 않는다. TCP의 head-of-line blocking이 실시간 상태 갱신에 불리할 수 있지만, TCP를 버린다고 자동으로 빨라지지는 않는다.

RUDP는 하나의 표준이 아니라 UDP 위에 sequence number, ACK/bitmask, retransmission timer, congestion control, fragmentation, anti-spoofing 등을 선택해 만든 설계 계열이다. 직접 만들기 전 QUIC/검증된 라이브러리를 평가한다.

실습: [day18_udp_model.cpp](examples/day18_udp_model.cpp)

## Day 19: 오브젝트 풀

풀은 allocation 빈도와 locality를 개선할 수 있지만 객체 수명, reset, 최대 capacity, pool보다 오래 사는 대여 객체를 어렵게 만든다. custom deleter가 pool의 raw `this`를 캡처하면 pool이 먼저 파괴될 때 dangling pointer가 된다. 공유 state를 `shared_ptr`로 두거나 pool 수명이 모든 handle보다 길다는 계약을 강제한다.

실습: [day19_object_pool.cpp](examples/day19_object_pool.cpp)

## Day 20: CAS와 lock-free

[CAS/ABA/메모리 회수](wiki/lock-free.md)를 모르면 raw pointer lock-free 자료구조를 배포하지 않는다. `compare_exchange_weak`는 spuriously fail할 수 있어 loop에서 사용한다. memory order는 증명 없이 완화하지 말고 우선 기본 sequential consistency로 정확성을 확보한다.

실습의 코드는 CAS 갱신 원리를 보여주는 모형이며 안전한 concurrent reclamation stack이 아니다: [day20_cas_stack_model.cpp](examples/day20_cas_stack_model.cpp)

## Day 21: 게임 루프

고정 tick accumulator는 렌더/벽시계 변동과 시뮬레이션 step을 분리한다. 입력 수집→고정 step 반복→snapshot 발행 순서를 유지한다. 한 프레임에서 최대 step을 제한해 [spiral of death](wiki/game-networking.md)를 방지한다.

실습: [day21_game_loop.cpp](examples/day21_game_loop.cpp)

## Day 22: AOI

모든 N개 엔티티 쌍의 거리 검사는 O(N²)이다. uniform grid는 위치를 cell coordinate로 나눠 주변 cell만 본다. 원문처럼 raw pointer를 저장할 때는 grid가 소유자가 아님을 명시하고, entity 파괴/이동 시 제거가 반드시 동기화되어야 한다. 실무에서는 stable handle+generation을 고려한다.

실습: [day22_grid_aoi.cpp](examples/day22_grid_aoi.cpp)

## Day 23: 추측항법과 보간

[Dead reckoning과 interpolation](wiki/game-networking.md)은 latency를 숨기는 client presentation 기법이다. 단순 `factor = speed * dt` lerp는 frame-rate에 따라 응답 곡선이 달라질 수 있다. 지수 감쇠 `1-exp(-lambda*dt)`는 시간에 더 일관된다. 큰 오차는 snap, 작은 오차는 smooth correction 같은 threshold가 필요하다.

권위 서버는 위치/충돌을 판정하고 클라이언트는 입력 sequence를 보내며 예측 후 server reconciliation을 수행한다. 치트 방지와 부드러운 화면은 별도 목표다.

실습: [day23_interpolation.cpp](examples/day23_interpolation.cpp)

## 최적화 판단 순서

1. 성능 목표와 실제 production trace를 정의한다.
2. profiler로 CPU, allocation, lock, syscall, network wait 중 병목을 찾는다.
3. 알고리즘/데이터 배치/불필요한 작업을 먼저 줄인다.
4. microbenchmark로 변경을 격리해 측정한다.
5. tail latency, 메모리, 정확성, 유지보수 비용까지 회귀 검사한다.
