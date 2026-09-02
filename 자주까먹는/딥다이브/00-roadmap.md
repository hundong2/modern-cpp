# 0단계: 학습 로드맵과 진단

> **한 줄 요약:** 동시성 최적화는 “빠른 문법 찾기”가 아니라 정확성 증명 → 병목 측정 → 가장 작은 변경 → 재측정의 반복이다.

- 선행 지식: 없음
- 초보자 우선: **세 가지 문제를 구별하기**와 **12주 과정**
- 실무 목표: correctness, progress, performance를 섞지 않고 각각 검증한다.

## 세 가지 문제를 구별하기

| 질문 | 분야 | 실패 예 |
|---|---|---|
| 결과가 항상 올바른가? | 정확성(correctness) | 데이터 레이스, use-after-free, 깨진 불변식 |
| 언젠가 앞으로 진행하는가? | 진행 보장(progress) | deadlock, livelock, starvation |
| 요구 시간/자원 안에 끝나는가? | 성능(performance) | lock contention, cache miss, oversubscription |

빠르지만 데이터 레이스가 있는 코드는 최적화된 코드가 아니다. 정확하지만 99번째 백분위 지연이 요구를 넘으면 성능 설계를 다시 해야 한다. 세 문제의 검사 도구도 다르다.

```mermaid
flowchart LR
    A["1. 불변식 작성<br/>무엇이 항상 참인가"] --> B["2. happens-before 증명<br/>누가 누구의 쓰기를 보는가"]
    B --> C["3. 진행성 점검<br/>교착·기아·취소"]
    C --> D["4. 기준선 측정<br/>처리량·지연·CPU"]
    D --> E["5. 한 병목 변경<br/>임계 구역·배치·샤딩"]
    E --> F["6. 재측정·회귀 검사"]
    F --> D
```

## 시작 전 자가 진단

다음 질문에 말로 답하지 못해도 괜찮다. 각 항목은 뒤 문서의 종료 조건이다.

1. `int counter`를 두 스레드가 `++`하면 왜 단순한 값 손실을 넘어 Undefined Behavior(정의되지 않은 동작)인가?
2. `mutex.unlock()` 이전의 쓰기를 다음 `mutex.lock()` 성공 뒤에서 볼 수 있는 이유는 무엇인가?
3. `lock_guard`, `unique_lock`, `scoped_lock`의 소유 상태와 생성자 인자가 어떻게 다른가?
4. semaphore의 permit과 mutex의 ownership은 무엇이 다른가?
5. `memory_order_relaxed`가 원자성은 보장하지만 메시지 publish에는 부족할 수 있는 이유는 무엇인가?
6. x86-64에서 acquire load가 평범한 load처럼 보여도 컴파일러 재배치는 왜 제한되는가?
7. `extern "C"`가 구조체 ABI와 예외 전달을 자동으로 안전하게 만들지 않는 이유는 무엇인가?
8. 평균 지연은 좋아졌지만 p99가 나빠진 최적화를 배포해도 되는가?

## 12주 실무 과정

각 주는 “읽기 → 실행 → 고장 내기 → 측정 → 짧은 보고서”로 끝낸다.

| 주 | 읽기/실습 | 산출물 |
|---:|---|---|
| 1 | 프로세스, 스레드, 스택, 가상 메모리 | 한 요청이 CPU까지 가는 계층 그림 |
| 2 | 컴파일 파이프라인, `-O0/-O2`, as-if rule | 같은 코드의 두 어셈블리 비교 |
| 3 | data race, sequenced-before, happens-before | 실패 코드의 관계 그래프 |
| 4 | mutex와 RAII guard | 수동 unlock을 guard로 리팩터링 |
| 5 | 교착, lock hierarchy, `scoped_lock` | lock order 문서와 회귀 테스트 |
| 6 | condition variable, semaphore, spinning | bounded queue의 대기 정책 비교 |
| 7 | atomic RMW와 CAS | 카운터와 publish 패턴 분리 |
| 8 | acquire/release/seq_cst | 각 메모리 순서의 필요조건 증명 |
| 9 | thread/jthread, stop, 예외와 수명 | 종료 가능한 worker 구현 |
| 10 | ABI, `extern "C"`, C#/Rust FFI | opaque handle API 빌드 |
| 11 | contention, false sharing, sharding | benchmark 원자료와 그래프 |
| 12 | profiler, sanitizer, 회귀 기준 | 최적화 제안서와 재현 명령 |

## 매 실험의 기록 양식

```text
가설: mutex 경합보다 같은 cache line의 쓰기 경쟁이 병목이다.
환경: CPU/코어 수, OS, 컴파일러 버전, 빌드 옵션, 전원 정책.
입력: 스레드 수, 반복 횟수, 데이터 크기.
기준선: median/p95/p99, throughput, CPU time, context switches.
변경: 카운터를 cache line별로 분리했다. 그 외에는 바꾸지 않았다.
결과: 원자료 위치와 반복별 수치.
판정: 지지/기각/불충분.
정확성: 테스트, ThreadSanitizer 결과, 불변식 검사.
다음 단계: 실제 부하에서 같은 경향인지 검증.
```

## 최적화 전문가의 습관

- 나노초 숫자 하나보다 분포와 오차를 본다.
- 소스 한 줄과 어셈블리 한 줄이 대응한다고 가정하지 않는다.
- “lock-free”를 “항상 더 빠름”으로 번역하지 않는다.
- `volatile`을 스레드 동기화로 쓰지 않는다.
- 임계 구역에서 I/O, 콜백, 메모리 할당을 가능한 한 분리한다.
- 공유을 제거하는 설계(message passing, partitioning, immutable snapshot)를 잠금 미세 조정보다 먼저 검토한다.
- 프로덕션에서는 처리량뿐 아니라 tail latency, 공정성, 복구 가능성, 관측 가능성을 함께 본다.

## 완료 기준

- [ ] 데이터 레이스와 논리 레이스를 구별한다.
- [ ] 하나의 동기화 관계를 Mermaid 또는 종이에 그릴 수 있다.
- [ ] 벤치마크에 환경, 워밍업, 반복 수, 분포를 기록한다.
- [ ] 다음 문서인 [소스에서 CPU까지](./01-machine-compiler-memory.md)로 이동할 준비가 됐다.
