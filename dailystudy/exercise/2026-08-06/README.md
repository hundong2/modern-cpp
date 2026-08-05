# 2026-08-06 Modern C++ 학습 자료

## 오늘의 주제: `std::function` 명령 파이프라인

서로 다른 람다를 `std::function<int(int)>`이라는 하나의 호출 계약으로 묶고, 애플리케이션 서비스가 실행 순서를 소유하도록 구성한다. `ResultPort`는 출력 정책을 추상화하고 `ConsoleResultAdapter`가 구체적인 표준 출력을 담당한다.

- `using Command = std::function<int(int)>`은 호출 가능한 객체의 형식을 통일한다.
- 값 캡처 람다는 필요한 상태를 자체 소유하여 원래 지역 변수의 수명과 분리한다.
- `unique_ptr<ResultPort>`는 포트 구현의 단독 소유권을 나타내며 `std::move`로 전달한다.
- `pipeline`은 lvalue, `CommandPipeline{...}`과 함수 반환값은 prvalue, `std::move(output)`은 xvalue다. 참조 바인딩과 복사·이동 여부는 이 값 범주와 객체 수명에 좌우된다.
- 반환 객체는 복사 생략으로 목적지에 직접 만들어질 수 있다. 로드·저장·비교·분기·함수 호출·가상 간접 호출의 실제 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.

## 오늘의 ICPC/OJ 문제

- 문제 ID/제목: BOJ 2252, **줄 세우기**
- 출처: [Baekjoon Online Judge](https://www.acmicpc.net/problem/2252)
- 핵심 알고리즘: Kahn 방식 위상 정렬(진입 차수 + 큐)
- 공용 문서: [`../algorithm/topological-sort.md`](../algorithm/topological-sort.md)
- 복잡도: 시간 `O(N + M)`, 공간 `O(N + M)`

진입 차수가 0인 학생은 아직 남은 선행 조건이 없으므로 안전하게 줄의 다음 위치에 놓을 수 있다. 그 학생의 간선을 제거하며 새로 진입 차수가 0이 된 학생을 큐에 추가하면 모든 비교 관계를 만족하는 순서를 얻는다.

## 빌드와 검증

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

CTest는 두 학습 실행 파일, 공식 예제 형태, 단일 사슬, 관계가 없는 정점들을 검증한다. ICPC 출력은 FIFO 큐와 번호 순회 때문에 테스트 입력마다 결정적이다.

## 직접 해보기

1. `main.cpp`에 `subtract` 명령을 추가하고 실행 순서에 따라 결과가 달라지는 이유를 설명한다.
2. `problem.cpp`의 값 캡처 람다를 참조 캡처로 바꾸고 저장된 람다보다 지역 변수가 먼저 파괴될 때 생기는 위험을 설명한다.
3. `icpc_problem.cpp`의 큐를 최소 힙으로 바꾸어 가능한 답 중 사전순으로 가장 작은 순서를 출력한다.
