# 2026-08-15 Modern C++ 학습 자료

## 오늘의 목표

- 읽기가 많은 공유 저장소에서 `std::shared_mutex`, `std::shared_lock`, `std::unique_lock`으로 읽기와 쓰기 임계 구역을 구분한다.
- 잠금 객체의 RAII 수명, 참조 기반 의존성 주입, 값 스냅샷 반환을 실제 실무형 코드에서 익힌다.
- 펜윅 트리의 불변식과 `index & -index`가 나타내는 구간을 이해하고 점 갱신·구간 합을 `O(log N)`에 처리한다.

## 생성 파일

- [`main.cpp`](main.cpp): 읽기-쓰기 잠금을 적용한 구성 저장소와 서비스 계층
- [`problem.cpp`](problem.cpp): `std::mutex`와 `std::scoped_lock`을 사용하는 실무형 메트릭 레지스트리
- [`icpc_problem.cpp`](icpc_problem.cpp): [BOJ 2042 구간 합 구하기](https://www.acmicpc.net/problem/2042) 제출 가능 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일과 CTest 검증 정의
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·동시성·알고리즘 이해를 증명하는 점검표

## Modern C++와 실무 아키텍처

`ThreadSafeConfigRepository`는 변경 가능한 해시 맵과 잠금을 한 객체에 캡슐화한다. 조회는 여러 스레드가 함께 획득할 수 있는 `std::shared_lock`을, 갱신은 단독 접근이 필요한 `std::unique_lock`을 쓴다. 잠금 객체가 범위를 벗어날 때 자동으로 해제되므로 예외나 조기 반환에도 잠금 누락을 줄인다. `ConfigService`는 저장소를 소유하지 않는 `IConfigRepository&`를 빌리므로 저장소가 서비스보다 오래 살아야 한다.

`find()`는 내부 원소의 참조를 잠금 밖으로 노출하지 않고 `std::optional<ConfigEntry>` 값 스냅샷을 반환한다. 이 복사는 비용이 조금 들지만 잠금 해제 뒤 다른 스레드가 컨테이너를 변경해도 호출자가 안전하게 읽을 수 있게 한다. 이름 있는 객체는 lvalue, `std::move(value)`는 xvalue, `ConfigEntry{...}`와 값 반환식은 prvalue이며 C++17 이후 결과 객체에 직접 만들어지는 복사 생략의 대상이다.

잠금 획득은 메모리 접근의 순서를 동기화하지만, 구체적인 로드·저장·비교·조건 분기·함수 호출과 운영체제 대기 방식은 CPU, ABI, 표준 라이브러리 구현, 컴파일러와 최적화 옵션에 따라 달라진다. 특정 어셈블리 명령 하나로 동작을 단정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 2042 구간 합 구하기](https://www.acmicpc.net/problem/2042), Baekjoon Online Judge
- 핵심 알고리즘: 펜윅 트리(Binary Indexed Tree), 공용 문서 [`../algorithm/fenwick-tree.md`](../algorithm/fenwick-tree.md)
- 대회 필수성: 값이 바뀌는 배열의 누적 합을 매 쿼리마다 다시 계산하지 않고 `O(log N)`에 갱신·질의하는 대표 자료구조다. 세그먼트 트리보다 구현이 짧아 합·빈도·순서 통계 문제에서 강력하다.
- 복잡도: 초기화와 전체 명령 처리 시간 `O((N+M+K) log N)`, 공간 `O(N)`
- 검증: 공식 예제, 한 원소 갱신, 음수 변경과 부분 구간을 CTest에서 출력 비교한다.

## 빌드와 실행

저장소 루트에서 다음을 실행한다.

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-08-15 -B build/2026-08-15 -G "MinGW Makefiles"
cmake --build build/2026-08-15
ctest --test-dir build/2026-08-15 --output-on-failure
```

## 직접 해보기

1. `main.cpp`의 `find()` 반환형을 내부 원소의 참조로 바꾸면 잠금 해제 뒤 어떤 수명·데이터 경쟁 문제가 생기는지 설명한다.
2. `problem.cpp`의 `std::scoped_lock`을 수동 `lock()`/`unlock()`으로 바꾸고 조기 반환을 넣어 RAII가 막아 주던 실수를 찾는다.
3. 펜윅 트리에서 `prefix_sum(right) - prefix_sum(left - 1)`가 닫힌 구간 `[left, right]`의 합인지 작은 배열로 손으로 검산한다.
