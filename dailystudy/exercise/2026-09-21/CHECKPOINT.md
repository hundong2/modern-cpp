# 2026-09-21 스스로 설명하고 고쳐 보기

답을 보기 전에 실제 식의 타입·값 범주·수명과 호출 전후 상태를 종이에 적는다. “무엇을 한다”만 말하지 말고 매개변수, 반환값, 무효화, 오류까지 설명해야 통과다.

## C++ 기초 문법 확인

1. `struct Order`와 `class PricingPolicy`에서 접근 지정자를 생략했을 때 각각 기본 접근은 무엇인가?
2. `int subtotal{};`와 `int next_sequence_{1};`는 각각 어떤 초기값을 만드는가?
3. `explicit PricingPolicy(std::string, int)`의 `explicit`을 없애면 `PricingPolicy p = {name, 1000}` 같은 어떤 copy-list 암시 변환 경로가 생기는가?
4. 생성자 본문 대입보다 `: name_{...}, basis_points_{...}` 멤버 초기화 목록이 먼저 수행되는 이유를 설명하라.
5. `using QuoteFunction = Quote (*)(Order, const PricingPolicy&);`가 새 타입을 만드는가? `*`는 무엇을 뜻하는가?
6. `const PricingPolicy& policy`는 무엇을 소유하고 무엇을 금지하는가? 참조가 유효하려면 누가 더 오래 살아야 하는가?
7. `[[nodiscard]]`가 붙은 결과를 버렸을 때 컴파일러가 할 수 있는 일과 할 수 없는 일을 구분하라.
8. `if (seats <= 0)`, Digit DP의 `for`, `continue`가 각각 제어 흐름을 어떻게 바꾸는가?

## 실제 식, 값 범주와 객체 수명

9. 이름 있는 `policy`, `quote_with_policy`, `order.id`의 값 범주는 무엇인가?
10. `PricingPolicy{...}`, `Order{...}`, `Quote{...}`의 값 범주는 무엇인가?
11. `std::move(policy)`의 반환형과 값 범주는 무엇이며, 그 식 자체가 객체를 실제로 이동하는가?
12. `name_{std::move(name)}` 뒤 매개변수 name은 파괴되기 전 어떤 상태 계약을 가지는가?
13. `return Quote{std::move(order.id), policy.name(), ...};`에서 첫 문자열은 왜 이동되고 둘째 문자열은 왜 복사되는가?
14. 반환 Quote가 입력 Order와 bind-back 래퍼보다 오래 살아도 안전한 근거를 소유권으로 설명하라.
15. 같은 타입 prvalue 반환의 보장된 복사 생략과 이름 있는 지역 반환의 NRVO를 구분하라.
16. 함수 포인터 `quote_function`은 가리키는 함수 코드를 소유하는가? 포인터의 유효 기간은 왜 충분한가?

## `std::bind_back` 호출 계약

17. `std::bind_back(quote_function, std::move(policy))`의 `F`, `Args...`, 저장되는 decay 타입을 적어라.
18. 첫 인자 함수 포인터와 둘째 인자 policy는 각각 복사되는가, 이동되는가, 빌려지는가?
19. 반환형 이름을 코드에 직접 쓰지 않고 `auto`를 쓰는 이유는 무엇인가?
20. 이름 있는 non-const lvalue 래퍼를 호출할 때 저장 대상은 `T&`, `const T&`, `T&&`, `const T&&` 중 무엇으로 전달되는가?
21. const lvalue, non-const rvalue, const rvalue 래퍼라면 저장 상태의 전달 범주가 각각 어떻게 바뀌는가?
22. volatile 래퍼 호출이 지원되지 않는다는 것은 런타임 실패인가, 컴파일 실패인가?
23. `quote_with_policy(Order{...})`의 call-time 인자와 저장 bound 인자가 최종 함수 매개변수의 어느 위치로 가는가?
24. bind-back은 왜 임시 `ReservationLedger{}`의 참조 수명을 연장하는 것이 아닌가? 원본 임시가 파괴된 뒤 어떤 별도 subobject가 살아 있는가?
25. `problem.cpp`에서 첫 호출 뒤 두 번째 호출이 순번 2를 받는 이유를 래퍼의 저장 상태로 설명하라.
26. `const auto reserve_in_order = ...;`로 바꾸면 `ReservationLedger&` 매개변수와 왜 맞지 않는가?
27. bound 인자로 raw pointer나 `std::reference_wrapper`를 저장했다면 원본 수명은 연장되는가?
28. `bind_back` 생성 중과 래퍼 호출 중 각각 어떤 예외가 전파될 수 있는가?
29. 표준이 일반적인 무할당/상수 시간 보장을 하지 않을 때 문서에 어떤 표현을 피해야 하는가?
30. 같은 변경 가능한 래퍼를 여러 스레드가 동기화 없이 호출하면 어떤 문제가 생길 수 있는가?

## 문자열과 스트림 계약

31. `std::string{"standard"}`의 입력 배열 타입, 복사 범위, 반환 여부, 할당 실패를 설명하라.
32. string 이동 생성 뒤 원본의 문자 포인터·반복자를 재사용하지 않는 이유는 무엇인가?
33. `digits_.size()`의 매개변수, 반환형, 객체 상태 변화, 복잡도, 예외 여부를 말하라.
34. `digits_[static_cast<std::size_t>(position)]`의 전제조건과 반환형은 무엇인가?
35. operator[] 반환 참조를 함수 밖에 저장하지 않고 즉시 char 값으로 읽는 수명상 이점은 무엇인가?
36. `std::to_string(limit)`이 고르는 오버로드, 반환 소유권, 표준에 별도 복잡도 상한이 있는지와 가능한 오류를 설명하라.
37. `std::cin >> a >> b`에서 각 호출의 반환값은 어떻게 다음 호출의 수신자가 되는가?
38. `std::cout << answer << '\n'`의 정수와 char 오버로드, 반환값 사용 여부, 오류 표현을 설명하라.
39. `sync_with_stdio(false)`와 `cin.tie(nullptr)`가 소유권을 넘기는 호출이 아닌 이유를 말하라.

## Digit DP와 정확성

40. `F(x)`를 정의하고 `[a,b]`의 답이 `F(b)-F(a-1)`인 이유를 집합으로 설명하라.
41. `position`, `previous`, `started`, `tight` 각각이 기억하는 정보를 말하라.
42. `previous=10` sentinel이 필요한 이유와 실제 digit 0과의 차이를 설명하라.
43. 선행 0에서 `previous=0`으로 바꾸면 숫자 7 같은 입력을 어떻게 잘못 세는가?
44. `position == digits_.size()`에서 `started=false`여도 1을 반환해야 하는 이유는 무엇인가?
45. `tight=false` 상태만 memoize해도 충분한 이유는 무엇인가?
46. `next_tight = tight && digit == upper_digit`가 상한을 보존하는 이유를 두 경우로 나눠 설명하라.
47. `101`은 허용되고 `100`은 거부되는 전이 과정을 손으로 추적하라.
48. 상태 수와 상태당 전이를 곱해 시간 복잡도 `O(D*10^2)`를 유도하라.
49. 답을 32비트 int가 아니라 long long으로 저장해야 하는 최대 사례를 들어라.
50. 19자리에서 재귀 전이는 19번이고 활성 frame은 기저 위치를 포함해 최대 20개라 안전하다. 입력 길이가 200,000인 문제라면 무엇을 바꿀지 말하라.

## 직접 실행하고 증명하기

51. 실행 전에 `daily_main`과 `daily_problem`의 정확한 두 출력을 적고 CTest와 비교하라.
52. `0 10`, `11 11`, `98 102`, `101 101`의 답을 손으로 구한 뒤 실행하라.
53. `0..9999`의 각 수를 문자열로 직접 검사하는 완전탐색 oracle을 만들고 모든 prefix 답을 DP와 대조하라.
54. `0..10^18` 답이 0 하나와 길이 1..18의 `9^length` 합이라는 점으로 최대 범위 결과를 독립 검산하라.
55. `reserve_in_order`를 const로 만들어 컴파일 오류를 읽고, 대상 함수를 `const ReservationLedger&`로 바꾸는 것만으로 해결할 수 없는 이유를 상태 변경 관점에서 적어라.
56. 높은 경고와 `-Werror`, CMake Release build, CTest, 공용 문서 `-Scope all` 감사를 모두 통과한 뒤에만 완료 표시하라.
