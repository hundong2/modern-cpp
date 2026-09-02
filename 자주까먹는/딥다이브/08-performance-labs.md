# 8단계: 동시성 최적화 실습

> **한 줄 요약:** 최적화는 mutex를 무작정 없애는 일이 아니라 대기 원인을 분해하고, 공유를 줄이고, 정확성을 유지한 채 실제 workload의 분포를 개선하는 일이다.

- 선행 지식: [버전별 진화](./07-version-evolution.md)
- 초보자 우선: **측정 규칙**, **실습 1~3**
- 전문가 목표: throughput/tail latency, cache/NUMA, queueing/backpressure를 함께 최적화한다.

## 측정 전에 고정할 것

| 항목 | 기록 예 |
|---|---|
| hardware | CPU model, physical/logical core, socket/NUMA, RAM |
| software | OS/kernel, compiler/standard library, commit hash |
| build | Release, `-O2/-O3`, LTO 여부, sanitizer 비활성/활성 별도 |
| workload | thread 수, read/write 비율, key 분포, critical-section work |
| 실행 | warm-up, 반복 수, affinity/전원 정책, background load |
| 결과 | ops/s, median/p95/p99/max latency, CPU time, context switch |

Debug와 Release 결과를 섞지 않는다. Sanitizer build는 정확성 검사에는 좋지만 성능 숫자에는 instrumentation overhead가 있다.

## 잘못된 microbenchmark를 피하는 법

- 결과를 사용해 compiler가 전체 loop를 제거하지 못하게 한다.
- setup/allocation을 측정 구간에 넣을지 의도적으로 결정한다.
- clock 호출 비용이 작업보다 크지 않게 batch한다.
- 한 번이 아니라 여러 process/run의 분포를 남긴다.
- CPU frequency scaling과 thermal throttling을 관찰한다.
- synthetic workload 결과를 production latency의 직접 증거로 쓰지 않는다.
- baseline과 변경본의 correctness test가 동일하게 통과해야 한다.

## 실습 1: race를 mutex로 고치기

목표: 정확성 비용과 성능 비용을 분리한다.

1. 비원자 `counter++`를 여러 thread에서 실행하는 위험 코드는 실제 제품/일반 실행에 넣지 말고 ThreadSanitizer용 별도 fixture로 격리한다.
2. `mutex + lock_guard` 버전을 기준선으로 만든다.
3. 독립 합계라면 `atomic::fetch_add(relaxed)` 버전을 만든다.
4. thread-local counter 후 마지막 reduction 버전을 만든다.
5. 결과값, ops/s, cache traffic을 비교한다.

예상: 단순 카운터에는 atomic이 맞을 수 있지만, 고경합에서는 per-thread reduction이 공유 cache line 쓰기를 크게 줄일 수 있다. 복합 invariant에는 이 결론을 그대로 적용할 수 없다.

## 실습 2: 임계 구역 줄이기

기준선:

```text
lock → parse/allocate/serialize/I/O → state update → unlock
```

개선 후보:

```text
parse/allocate → lock → 짧은 state commit/snapshot → unlock → serialize/I/O
```

측정:

- lock hold time histogram
- waiter 수/대기 시간
- allocation 수
- p99 request latency

정확성 질문: prepare와 commit 사이 state가 변해도 되는가? 안 된다면 version check/retry 또는 다른 transaction 경계가 필요하다.

## 실습 3: lock sharding

하나의 map mutex를 N개 shard로 나눈다.

```text
shard_index = stable_hash(key) % shard_count
각 shard = mutex + local container
```

검토:

- key 분포가 치우치면 hot shard는 그대로다.
- 여러 key operation은 shard lock 순서를 정해야 한다.
- shard 수 증가가 memory와 cache locality를 해칠 수 있다.
- rehash/resize를 shard 내부에서 어떻게 처리하는지 확인한다.

Thread 수와 shard 수를 sweep하고 throughput/p99의 knee point를 찾는다.

## 실습 4: false sharing

두 thread가 인접한 atomic counter를 각각 갱신하는 버전과 cache line을 분리한 버전을 비교한다.

```cpp
struct alignas(std::hardware_destructive_interference_size) PaddedCounter {
    std::atomic<std::uint64_t> value{0};
};
```

확인:

- `sizeof`, `alignof`, member address 차이
- hardware counter의 cache-to-cache transfer/LLC miss
- 읽기만 하는 인접 데이터까지 지나치게 분리해 locality를 해치지 않았는가

## 실습 5: mutex vs shared_mutex

Read 비율 50/90/99%, 임계 구역 길이, thread 수를 바꾼다. 평균만 보지 말고 writer p99와 starvation 여부를 본다. Immutable snapshot + atomic pointer publish 대안도 비교하되 reclamation 비용을 포함한다.

## 실습 6: spinning vs blocking

Critical section hold time을 짧음/중간/길게 인위적으로 나누고 다음을 비교한다.

- 표준 mutex
- atomic wait/notify(C++20)
- 검증된 adaptive primitive

직접 spinlock은 oversubscription에서 lock owner가 schedule되지 못한 채 waiter들이 CPU를 태우는 최악 상황을 만든다. `pause/yield` 삽입만으로 correctness/fairness가 해결되지 않는다.

## 실습 7: bounded queue와 backpressure

Queue capacity를 1, 16, 256, unbounded로 바꾼다.

- producer latency
- consumer utilization
- queue residence time
- drop/reject 수
- shutdown 시간

큰 queue는 burst를 흡수하지만 overload latency를 메모리 안에 숨긴다. Little's Law의 직관 `동시 체류량 ≈ 도착률 × 체류시간`으로 queue 크기와 latency 관계를 생각한다.

## 실습 8: NUMA와 thread placement

Multi-socket 장비에서만 의미 있게 수행한다.

1. single shared queue의 remote memory traffic을 측정한다.
2. NUMA node별 queue/worker/data partition을 비교한다.
3. cross-node work stealing 비용과 imbalance를 함께 측정한다.
4. affinity를 제거했을 때 회복력/성능 차이를 확인한다.

NUMA 최적화는 topology가 다른 배포 환경에서 역효과가 날 수 있어 runtime discovery와 fallback이 필요하다.

## 성능 개선 우선순위

1. 불필요한 공유와 작업 자체 제거
2. 자료구조/알고리즘과 batching 개선
3. 임계 구역 밖으로 계산·I/O 이동
4. sharding/partitioning/immutable snapshot
5. 적절한 primitive와 대기 정책 선택
6. layout, false sharing, NUMA 조정
7. 증명과 benchmark가 가능한 경우에만 약한 memory order/lock-free

## Lock-free 도입 승인 질문

- 실제 profiler에서 기존 lock이 병목인가?
- 진행 보장 요구가 lock-free인가, 단순 latency 개선인가?
- ABA와 memory reclamation을 누가 검증했는가?
- target architecture와 compiler 조합의 test가 있는가?
- fallback과 관측 지표가 있는가?
- 유지보수자가 happens-before proof를 재검토할 수 있는가?

## 결과 보고서 예시

```text
변경: global mutex map → 32 shard
정확성: unit/invariant/stress/TSan 통과
workload: 16 workers, 95% read, Zipf key 분포
throughput: median +38% (5-run CI 포함)
p99: -21%; hot-key p99는 변화 없음
CPU: user +8%, context switches -44%
위험: multi-key transaction에 lock order 추가
판정: 일반 key workload 채택, hot-key 별도 partition 후속 실험
```

## 완료 기준

- [ ] 실험 환경과 원자료로 결과를 재현할 수 있다.
- [ ] 평균뿐 아니라 p95/p99, fairness, shutdown을 측정한다.
- [ ] 다음 문서인 [도구와 어셈블리](./09-tooling-and-assembly.md)로 이동한다.
