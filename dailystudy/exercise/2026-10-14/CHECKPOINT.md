# 2026-10-14 학습 체크포인트

답을 바로 보지 말고 먼저 말과 코드로 설명한다. “그냥 된다” 대신 타입, 값 범주, 수명, 전제조건, 후조건을 붙인다.

## 1. 기초 문법 검증

1. `enum class DeliveryState : std::uint16_t`에서 `enum class`, 기저 타입, 열거자 명시 값의 역할을 각각 설명하라.
2. `DeliveryState state{};`와 `std::uint16_t state_code{};`가 어떤 값으로 초기화되는가?
3. `struct Delivery`와 `class DeliveryBatchEncoder`의 기본 접근 지정자는 어떻게 다른가?
4. `public:`과 `private:`가 컴파일러에게 강제하는 경계는 무엇인가?
5. `using Source = std::vector<Delivery>;`가 새 타입을 만드는가, 별칭만 만드는가?
6. `std::vector<Delivery>`에서 class template과 template 인자를 구분하라.
7. `explicit DeliveryBatchEncoder(Source)`가 막는 코드를 한 줄 작성하라.
8. `: deliveries_{...}`가 생성자 본문 대입보다 먼저 수행되는 이유를 설명하라.
9. `encode() const`의 뒤 `const`는 반환값이 const라는 뜻인가?
10. `const Delivery& delivery`에서 참조와 `const`가 각각 무엇을 보장하는가?
11. `const WireDelivery* const first`의 두 `const`를 각각 설명하라.
12. 참조와 포인터의 null 가능성, 재바인딩, 접근 문법, 수명 연장 여부를 비교하라.
13. `if (copied.empty())`가 `copied[0]`보다 먼저 있어야 하는 이유를 UB와 연결하라.
14. range-for가 숨기는 다섯 연산을 말하라.
15. `return 1;`과 `return 2;`가 어떤 검증 실패를 나타내는가?

## 2. 실제 식과 값 범주 해석

다음 각 식의 타입, lvalue/prvalue/xvalue, 소유권 변화를 말하라.

1. `seed`
2. `std::move(seed)`
3. `DeliveryBatchEncoder{std::move(seed)}`
4. `WireDelivery{.tracking_id = delivery.tracking_id, ...}`
5. `delivery.tracking_id`
6. `encode_state(delivery.state)`
7. `encoder.encode()`
8. `const Snapshot encoded{encoder.encode()}`
9. `const Snapshot copied{encoded}`
10. `copied[0]`
11. `&copied[0]`
12. `first->tracking_id`

추가 질문:

- `std::move` 자체가 vector 버퍼를 옮기는가? `seed -> deliveries`와 `deliveries -> deliveries_`에서 실제 이동을 수행하는 두 생성자를 각각 지목하라.
- `const` lvalue 문자열을 `push_back`용 DTO에 넣을 때 왜 문자열 복사가 선택되는가?
- `return result;`에서 NRVO가 없어도 어떤 언어 규칙이 복사를 줄일 수 있는가?
- `copied`가 `encoded`와 독립 소유임을 한쪽 수명 종료 상황으로 증명하라.
- 이동 후 `seed`에 대해 허용되는 연산과 가정하면 안 되는 값을 말하라.

## 3. `std::to_underlying` 호출 계약

`std::to_underlying(state)`를 보지 않고 다음 여섯 항목을 채워 보라.

1. 수신 객체의 정확한 타입과 호출 전 상태는 무엇인가?
2. 선택된 함수 template 특수화의 인자/반환 타입은 무엇인가?
3. `state`의 식 타입, 값 범주, 소유권 의미와 허용 값은 무엇인가?
4. 반환형·값 의미·실제 사용 위치는 무엇인가?
5. 호출 뒤 `state`와 외부 객체 상태는 어떻게 되는가?
6. 전제조건, 후조건, 복잡도, 할당, 무효화, 수명, 오류, 스레드 보장은 무엇인가?

역변환 검증:

- 외부 숫자 `25`를 `static_cast<DeliveryState>(25)`로 바꾸면 컴파일은 될 수 있다. 이것이 유효한 열거자라는 보장까지 주는가?
- wire code를 enum으로 올리는 함수에 `switch` 또는 허용 집합 검증이 필요한 이유를 설명하라.
- 열거자에 명시 값을 주지 않고 선언 순서를 바꾸었을 때 영속 데이터가 어떻게 깨질 수 있는가?

## 4. 표준 라이브러리 호출 계약 자가 감사

각 항목에서 매개변수, 반환값 사용 여부, 호출 후 상태, 무효화, 오류를 모두 답한다.

1. `std::move(deliveries)`와 뒤 vector 이동 생성자
2. `Snapshot result{}` 기본 생성
3. `deliveries_.size()`
4. `result.reserve(deliveries_.size())`
5. `result.push_back(WireDelivery{...})`
6. vector initializer-list 생성과 그 안의 string 리터럴 생성
7. `Snapshot copied{encoded}` 복사 생성
8. `copied.empty()`
9. `copied[0]`
10. `first->tracking_id.empty()`
11. const vector range-for의 `begin/end/*/++/!=`
12. `std::array<int,26> next{}`
13. `states_[clone] = states_[target]` 속 array 복사 대입
14. `std::ios::sync_with_stdio(false)`
15. `std::cin.tie(nullptr)`
16. `std::cin >> text`와 `basic_ios::operator!`
17. `text.size()`와 `text[index]`
18. `std::cout << answer << '\n'`

다음 위험을 바로 설명하지 못하면 해당 코드 위 계약 주석을 다시 읽는다.

- `reserve`가 size를 늘리지 않는다는 사실
- vector 재할당이 포인터·참조·반복자를 무효화하는 조건
- `operator[]`가 범위 검사를 하지 않는다는 전제조건
- string 복사 중 할당 실패와 원본 보존
- stream 부분 출력이 오류 때 자동 롤백되지 않는다는 사실
- `sync_with_stdio(false)` 뒤 같은 표준 stream의 병행 접근 위험

## 5. Suffix Automaton 핵심 알고리즘

1. 부분 문자열의 `endpos` 집합을 정의하라.
2. 같은 상태에 속한 문자열들이 왜 같은 `endpos` 집합을 갖는가?
3. `longest[v]`는 무엇인가?
4. `suffix_link[v]`는 무엇을 가리키는가?
5. 상태 `v`가 대표하는 길이 구간이 왜 `(longest[link[v]], longest[v]]`인가?
6. 위 구간의 원소 수가 왜 `longest[v]-longest[link[v]]`인가?
7. 서로 다른 상태의 길이/내용 기여가 왜 중복되지 않는가?
8. 새 문자 전이가 없는 suffix-link 조상에 current 전이를 추가하는 while 불변식을 말하라.
9. `cursor==0`일 때 current의 link가 root인 이유는 무엇인가?
10. `longest[cursor]+1==longest[target]`이면 clone이 필요 없는 이유는 무엇인가?
11. 길이 연속성이 깨졌을 때 target을 직접 줄이면 안 되는 이유는 무엇인가?
12. clone이 복사해야 할 필드와 바꿔야 할 필드를 각각 말하라.
13. 전이 redirect loop가 `next[symbol]==target`인 조상만 바꾸는 이유는 무엇인가?
14. current와 target의 suffix link를 모두 clone으로 바꾸는 이유는 무엇인가?
15. 모든 `n>=1`에서 상태 수가 최대 `2n`이고 `n>=2`에서는 최대 `2n-1`인 이유를 current와 clone 개수로 설명하라.
16. 고정 알파벳 배열 구현의 시간·공간 복잡도를 `|Sigma|`까지 포함해 쓰라.
17. `n=100000`에서 답이 32비트 signed 범위를 넘을 수 있음을 계산하라.
18. `0=absent`, `1=root` 설계가 zero initialization과 어떻게 맞물리는가?

## 6. 손으로 추적하기

문자열 `abaa`에 대해 각 문자를 추가한 뒤 다음을 표로 적는다.

| 단계 | 추가 문자 | current | clone 생성 여부 | current link | 지금까지 상태 수 |
|---|---:|---:|---:|---:|---:|
| 1 | `a` |  |  |  |  |
| 2 | `b` |  |  |  |  |
| 3 | `a` |  |  |  |  |
| 4 | `a` |  |  |  |  |

그 뒤 root를 제외한 모든 상태의 `longest[v]-longest[link[v]]`를 합해 8이 되는지 확인한다.

추가 입력 기대값:

- `a -> 1`
- `aaaa -> 4`
- `abcd -> 10`
- `ababa -> 9`
- `banana -> 15`

## 7. 직접 구현·변형

1. `icpc_problem.cpp`를 보지 않고 1-based 상태와 0 sentinel 버전을 다시 작성한다.
2. `std::array<int,26>` 대신 map 전이를 쓰고 알파벳 크기·메모리·상수 계수를 비교한다.
3. 각 상태의 occurrence 횟수를 세어 “k번 이상 등장하는 부분 문자열 수”로 확장한다.
4. 여러 문자열의 공통 부분 문자열 문제에서 상태를 어떻게 재사용할지 조사한다.
5. 작은 문자열 모든 부분 문자열 set 오라클을 작성해 1,000건을 차등 테스트한다.
6. 길이 100,000의 모두 같은 문자열과 무작위 문자열에서 상태 수·답·실행 시간을 기록한다.

## 8. 정답 요지로 자기 채점

- `to_underlying`은 enum을 수정하지 않는 `constexpr`, `noexcept`, O(1), 무할당 값 변환이다.
- `std::move`는 xvalue cast이고 실제 이동은 뒤 생성자/대입이 한다.
- vector `operator[]`는 범위 전제조건을 검사하지 않는다.
- SAM 각 상태의 새 부분 문자열 수는 `longest[v]-longest[link[v]]`다.
- clone은 `target`의 미래 전이/endpos 행동을 공유하면서 길이 구간만 분리한다.
- 고정 소문자 알파벳에서 구축과 합산은 `O(n)`, 저장은 `O(n)`이며 답은 64비트다.

위 문장을 외운 것이 아니라 코드의 실제 식과 불변식으로 다시 설명할 수 있어야 통과다.
