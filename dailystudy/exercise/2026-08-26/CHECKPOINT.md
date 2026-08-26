# 2026-08-26 이해 검증

자료를 닫고 먼저 답한다. 이름을 한글로 번역하는 데 그치지 말고 수신 객체·각 위치 인자·반환·호출 후 상태·수명·복잡도·실패 조건을 실제 식과 연결한다.

## 1. 기초 문법과 아키텍처

1. `struct RowSummary`와 `class MetricBatch`의 기본 접근 권한 차이와 오늘 역할에 각각 알맞은 이유를 설명한다.
2. `using ConstMetricView = ...`가 새 객체·상속 타입을 만드는지 답하고 템플릿 인자 각각의 뜻을 말한다.
3. 생성자에는 왜 반환형이 없으며 `explicit BatchAnalyzer(ConstMetricView)`가 막는 암시 변환은 무엇인가?
4. 멤버 초기화 목록 `samples_{std::move(samples)}`는 생성자 본문 대입과 무엇이 다른가?
5. `MetricBatch`와 `BatchAnalyzer` 중 누가 int 원소·버퍼·포인터·extent를 각각 소유하는가?
6. `rows * columns == samples.size()`를 mdspan이 검사하지 않는다면 어느 아키텍처 경계가 이 불변식을 책임져야 하는가?

## 2. 값 범주·이동·수명

1. `samples`, `std::move(samples)`, `batch.view()`, `ConstMetricView{...}`의 값 범주를 분류한다.
2. `std::move(samples)`와 vector 이동 생성자 중 실제 버퍼 소유권을 옮기는 것은 무엇인가?
3. 이동 뒤 원본 samples에 표준이 보장하는 상태와 보장하지 않는 내용을 구분한다.
4. mdspan 값 복사가 원소 수명, Batch 수명, vector capacity에 아무 보장도 더하지 않는 이유를 설명한다.
5. `BatchAnalyzer analyzer{batch.view()}`가 안전한 파괴 순서를 현재 지역 변수 선언 순서와 연결한다.
6. `summarize()`의 이름 있는 `result` 반환에서 NRVO와 복사 가능성을 설명한다.
7. `view_[row,column]`이 반환한 참조를 Batch 파괴 또는 vector 재할당 뒤 쓰면 왜 잘못인가?

## 3. 표준 라이브러리 호출 계약

각 식의 수신 객체 정확한 타입·호출 전 상태, 선택된 오버로드·템플릿 인자, 각 위치 인자의 타입·값 범주·복사/이동/참조/소유권, 반환형·사용 여부, 호출 후 상태, 복잡도·할당·무효화·수명·오류·스레드 보장을 자료 없이 설명한다.

1. `std::vector<int> samples{3, 4, 5, 1, 7, 2}`
2. `MetricBatch{2U, 3U, std::move(samples)}`
3. `samples_.data()`
4. `ConstMetricView{samples_.data(), rows_, columns_}`
5. `view_.extent(1)`
6. `view_[row, column]`
7. `std::vector<int>(side * side, 0)`
8. `grid.apply(CellUpdate{...})`
9. `heights.size()`와 `pending.empty()`
10. `pending.reserve(heights.size())`
11. `pending.back()`
12. `pending.pop_back()`
13. `pending.push_back(OpenBar{current_height, start})`
14. `std::max(best, area)`
15. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`
16. `std::cin >> count`의 반환값을 bool 문맥에서 검사하는 과정
17. `std::cin >> height`
18. `std::cout << largest_rectangle(heights) << '\n'`

추가 검증:

- `mdspan::operator[]`가 범위를 검사하지 않을 때 잘못된 인덱스는 어떤 전제조건을 어기는가?
- vector 재할당 전후 `data()` 포인터와 기존 mdspan의 유효성을 설명한다.
- `back()` 참조를 `pop_back()` 뒤 사용하면 왜 잘못이며 오늘 코드는 어떻게 피하는가?
- `reserve()`가 size를 바꾸지 않는다는 말과 capacity가 늘 수 있다는 말을 구분한다.
- 표준 vector·mdspan이 같은 원소의 동시 읽기/쓰기를 자동으로 데이터 경쟁에서 보호하는가?
- 스트림 추출 실패 때 대상 값, 스트림 상태, 뒤 연쇄 호출에 무엇이 보장되는가?

## 4. 단조 스택 대회 필수 지식

1. pending의 높이가 아래에서 위로 엄격히 증가한다는 불변식을 초기화·유지·종료로 증명한다.
2. `OpenBar::start`가 단순 삽입 위치가 아니라 현재 높이가 확장 가능한 최좌측인 이유를 예제로 말한다.
3. 더 낮은 `current_height`를 만난 순간 pop된 막대의 오른쪽 경계가 `index-1`로 확정되는 이유를 설명한다.
4. pop할 때 `start = closing.start`를 전달하지 않으면 어떤 직사각형을 놓치는지 반례를 든다.
5. 같은 높이를 하나만 유지해도 되는 이유와 더 이른 start를 보존해야 하는 이유를 말한다.
6. 끝에 높이 0 센티널을 한 번 처리하면 남은 모든 후보가 빠짐없이 계산되는 이유를 설명한다.
7. 중첩 while이 있어도 각 막대가 최대 한 번 push·pop되므로 `O(n)`인 상각 분석을 한다.
8. 높이 `1e9`, 막대 `1e5`에서 int가 넘치고 long long이 필요한 이유를 수치로 계산한다.
9. `2 1 4 5 1 3 3`에서 stack의 `(height,start)`와 best를 모든 인덱스에서 손으로 추적한다.
10. 단조 스택을 다음 작은 원소, 빗물, 주가 기간 문제로 바꿀 때 무엇을 스택에 저장하는지 비교한다.

## 5. 초보자 실기 검증

- [ ] `main.cpp` 출력 `12 5 10 7`을 실행 전에 예측하고 각 값의 근거를 적었다.
- [ ] `problem.cpp`를 보지 않고 3x3 연속 vector와 쓰기 mdspan을 다시 작성했다.
- [ ] mdspan을 만든 뒤 소유 vector 재할당을 유도하는 위험 예를 작성하고 실행하지 않고 수명 위반을 설명했다.
- [ ] 공개 예제 첫 케이스를 단조 스택으로 끝까지 손 추적해 최대 넓이 8을 얻었다.
- [ ] 증가·감소·동일 높이·0 높이·단일 막대의 답을 코드 없이 계산했다.
- [ ] 모든 C++의 표준 호출마다 인자 수와 주석 설명 수, 반환값 사용·무시 여부를 대조했다.
- [ ] 세 실행 파일을 높은 경고 수준으로 빌드하고 두 학습 실행 파일을 직접 실행했다.
- [ ] CTest 전체와 표준 라이브러리 문서 감사를 통과했다.

## 완료 기준

`mdspan`을 “2차원 배열처럼 쓴다”에서 멈추지 않고 비소유 포인터·extent·매핑, 범위 전제조건, 원본 수명과 재할당 무효화를 설명한다. 단조 스택은 코드를 보지 않고 증가 불변식, start 전달, pop 면적, 센티널, 정확성, `O(n)` 상각 분석을 다시 구현할 수 있어야 한다.
