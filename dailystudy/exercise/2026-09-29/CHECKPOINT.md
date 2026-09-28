# 2026-09-29 CHECKPOINT — `resize_and_overwrite`와 Booth 최소 회전

코드나 README를 보지 않고 먼저 답한 뒤 실제 식과 작은 문자열을 손으로 추적한다. “문자열을 빠르게 만든다”, “후보를 건너뛴다”처럼 이름만 말하지 말고 타입·값 범주·소유권·수명·전제조건·복잡도를 함께 설명해야 통과다.

## 1. 초보자 기초 문법과 객체 설계

- [ ] 1. `std::size_t dropped{};`의 타입, 부호, 빈 중괄호 초기화 결과를 말한다.
- [ ] 2. `char separator_{}`에서 기본 타입 `char`와 기본 멤버 초기화의 역할을 말한다.
- [ ] 3. `struct NormalizationResult`와 `class LogKeyNormalizer`의 기본 접근 지정자 차이는?
- [ ] 4. `public:` API와 `private:` 문자열 owner가 보호하는 불변식은 무엇인가?
- [ ] 5. `final`이 상속을 통한 수명/정책 변경을 어떻게 막는가?
- [ ] 6. 생성자에 반환형이 없는 이유를 말하고, 두 인자 `LogKeyNormalizer`의 copy-list 초기화와 한 인자 `SerialNumber` 변환 중 `explicit`이 막는 식을 각각 쓴다.
- [ ] 7. 값 매개변수 `std::string raw`를 쓰면 lvalue/rvalue 호출자에서 각각 어떤 복사·이동 기회가 생기는가?
- [ ] 8. 멤버 초기화 목록과 생성자 본문 대입의 시점 차이를 설명한다.
- [ ] 9. 멤버 초기화의 실제 순서는 초기화 목록과 선언 순서 중 무엇을 따르는가?
- [ ] 10. `normalize() const`의 반환형, 매개변수 목록, 함수 `const`를 구분한다.
- [ ] 11. `[[nodiscard]]`가 반환값 무시를 언어상 금지하는지, 진단을 권고하는지 답한다.
- [ ] 12. `std::string`의 문자·traits·allocator 템플릿 역할을 초보자 말로 각각 설명한다.
- [ ] 12-A. `using CharacterCount = std::size_t;`가 새 타입을 만드는지, overload에서 원래 타입과 구별되는지 답한다.
- [ ] 13. 참조와 포인터의 null 가능성, 재지정, 소유권, 접근 문법 차이를 말한다.
- [ ] 14. `[this]`가 객체의 참조 캡처가 아니라 비소유 `this` 포인터 값의 복사 캡처라는 뜻과, 객체를 복사하거나 수명을 늘리지 않는 이유는?
- [ ] 15. `for` 초기화·조건·증가와 두 `if` 분기가 한 문자에 적용되는 순서를 적는다.

## 2. 값 범주·복사/이동·수명·복사 생략

- [ ] 16. 이름 있는 `raw`, `raw_`, `normalized`, `normalizer`, `result` 식의 값 범주는?
- [ ] 17. `std::string{"API v2/West!!"}`, lambda 식, `NormalizationResult{...}`의 값 범주는?
- [ ] 18. `std::move(raw)`가 가리키는 객체와 결과 값 범주를 말한다.
- [ ] 19. `std::move` 자체가 문자 byte를 이동하는지, 실제 이동은 어느 생성자가 수행하는지 구분한다.
- [ ] 20. 이동 뒤 원본 string에 허용되는 연산과 의존하면 안 되는 상태를 말한다.
- [ ] 21. 문자열 이동이 항상 heap pointer 하나만 복사한다고 단정할 수 없는 이유는?
- [ ] 22. callback `char* output`의 owner와 유효 기간을 말한다.
- [ ] 23. `[this]`와 `output`을 callback 밖에 저장하면 각각 어떤 dangling 위험이 생기는가?
- [ ] 24. `return NormalizationResult{std::move(normalized), dropped};`에서 바깥 결과와 안쪽 string의 구성을 구분한다.
- [ ] 25. `return doubled.substr(start,n);`의 prvalue가 지역 doubled 수명과 독립적인 이유는?
- [ ] 26. 보장된 prvalue 직접 구성과 이름 있는 지역 변수의 선택적 NRVO 차이를 설명한다.

## 3. `resize_and_overwrite` 호출 계약

- [ ] 27. 첫 인자 `maximum_size`가 최종 문자열 크기가 아니라 쓰기 상한인 이유는?
- [ ] 28. callback 둘째 인자가 `capacity()`가 아니라 전달한 `n`과 같은 값이라는 계약을 말한다.
- [ ] 29. 호출 전 크기 `o`, 요청 `n`, `k=min(o,n)`일 때 어느 버퍼 구간만 기존 접두사와 같은가?
- [ ] 30. `[p+k,p+n)`의 문자를 쓰기 전에 읽으면 안 되는 이유는?
- [ ] 31. callback 반환형이 만족해야 할 integer-like 조건과 값 범위는?
- [ ] 32. `written > writable_size`를 반환하면 왜 정상적인 오류 반환이 아니라 전제조건 위반인가?
- [ ] 33. `[output,output+written)` 중 한 칸을 쓰지 않은 채 포함하면 어떤 계약을 어기는가?
- [ ] 34. callback을 `noexcept`로 표시해도 본문에서 던지는 함수를 부르면 어떤 문제가 생기는가?
- [ ] 35. callback 안에서 목적 문자열의 `push_back`, `resize`, `size`를 다시 호출하면 왜 안 되는가?
- [ ] 36. 성공 뒤 문자열 내용·size, 원본 raw_, callback 포인터의 상태를 각각 말한다.
- [ ] 37. 멤버 함수의 반환형 `void`와 callback의 `std::size_t` 반환 용도를 구분한다.
- [ ] 38. 호출 전 포인터·참조·iterator를 호출 뒤 다시 얻어야 하는 이유는?
- [ ] 39. 일반 길이 초과·할당 실패와 callback 계약 위반의 실패 성격을 비교하고, 오늘 `n=raw_.size()`에서는 길이 초과가 불가능한 이유를 말한다.
- [ ] 40. callback 자체의 `O(n)`과 멤버 전체에 별도 표준 복잡도·할당 횟수·SSO 보장이 없다는 사실을 구분한다.

## 4. STL 호출 계약을 실제 식으로 설명하기

아래 각 식마다 반드시 여섯 부분으로 답한다.

1. 수신 객체의 정확한 타입과 호출 전 상태
2. 선택된 시그니처·overload·template 인자
3. 각 매개변수 식의 타입·값 범주·소유권 의미·허용값
4. 반환형·반환값 의미·실제 사용/폐기 여부
5. 호출 뒤 수신 객체와 각 인자의 상태 변화
6. 전제조건·후조건·복잡도·할당·무효화·수명·오류/예외·thread 보장

- [ ] 41. `std::string{"API v2/West!!"}`
- [ ] 42. `std::move(raw)`와 이어지는 string 이동 생성자
- [ ] 43. `raw_.size()`
- [ ] 44. `std::string normalized{}`
- [ ] 45. `normalized.resize_and_overwrite(maximum_size, callback)`
- [ ] 46. callback 안의 `raw_[index]`
- [ ] 47. `std::move(normalized)`와 결과 멤버 이동 생성
- [ ] 48. `std::cout << "key=" << result.value << ... << '\n'`
- [ ] 49. `std::string doubled{text}`
- [ ] 50. `doubled += text`
- [ ] 51. `doubled[first + matched]`와 `doubled[second + matched]`
- [ ] 52. `doubled.substr(start, n)`
- [ ] 53. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`
- [ ] 54. `std::cin >> input`
- [ ] 55. `std::cout << answer << '\n'`

## 5. 실제 Expression 해석

- [ ] 56. `output[written] = current; ++written;`에서 저장 주소와 증가 시점을 설명한다.
- [ ] 57. 모든 반복 시작에서 `written <= index <= writable_size`가 성립함을 귀납적으로 보인다.
- [ ] 58. `maximum_size - actual_size`가 unsigned underflow를 일으키지 않는 근거는?
- [ ] 59. `(value >= 'a' && value <= 'z') || ...`의 short-circuit 순서를 설명한다.
- [ ] 60. `first += matched + 1U`에서 각 피연산자 타입과 최대값을 계산한다.
- [ ] 61. `first + matched < 2*n`과 `second + matched < 2*n`을 루프 조건으로 증명한다.
- [ ] 62. `first < second ? first : second`의 조건 분기와 반환 값 범주는?
- [ ] 63. `if (!(std::cin >> input))`에서 추출 반환 참조에 `basic_ios::operator!()`가 직접 선택되는 과정과 `operator bool` 뒤 built-in `!`이 아닌 이유를 말한다.

## 6. Booth 불변식과 정확성 증명

- [ ] 64. 길이 `n` 문자열의 회전을 0-based 시작 위치 함수로 정의한다.
- [ ] 65. `s+s`에서 모든 길이 `n` 회전이 연속 구간 하나가 되는 이유는?
- [ ] 66. `first`, `second`, `matched`의 루프 불변식을 정확히 쓴다.
- [ ] 67. 첫 불일치에서 큰 후보 하나만이 아니라 `matched+1`개 시작점을 제거할 수 있음을 증명한다.
- [ ] 68. 갱신 후보가 상대 후보와 같을 때 한 칸 더 이동해야 하는 이유는?
- [ ] 69. 후보 포인터에 modulo를 적용하면 제거 후보가 부활하는 반례를 설명한다.
- [ ] 70. 모든 문자가 같을 때 `matched==n`으로 종료하고 출력이 올바른 이유는?
- [ ] 71. `ababab`처럼 최소 회전 시작점이 여러 개인 경우 어느 위치를 골라도 문자열이 같은 이유는?
- [ ] 72. 각 mismatch의 문자 비교를 제거 구간에 상각해 전체 `O(n)`을 유도한다.
- [ ] 73. doubled·answer·input의 peak 저장을 따져 추가 공간 `O(n)`을 설명한다.
- [ ] 74. 모든 회전 생성·정렬이 왜 최소 `O(n^2)` 문자 저장/비교 병목을 만드는가?

## 7. 손 추적과 실행 검증

- [ ] A. `API v2/West!!`의 각 문자를 보존/치환/삭제로 분류하고 `API-v2-West`, 제거 2를 계산한다.
- [ ] B. `SN-20 26-A`에서 `2026`, 제거 6을 손으로 계산한 뒤 실행한다.
- [ ] C. 공식 `acab`에서 네 회전을 적고 `abac`가 최소임을 확인한다.
- [ ] D. `a`, `aaaaaa`, `bcabca`, `aaba`, `aaaaabaaaa`의 기대 출력을 먼저 계산한다.
- [ ] E. 알파벳 `{a,b,c}`의 짧은 모든 문자열을 brute-force 회전 오라클과 비교한다.
- [ ] F. 고정 seed 무작위 문자열을 Booth 결과와 모든 회전 최솟값으로 대조한다.
- [ ] G. 길이 1,000,000의 같은 문자·교대 주기·마지막 유일 최소 문자 입력에서 종료와 출력 길이를 확인한다.
- [ ] H. CMake C++23 빌드, CTest, 엄격 경고 구문 검사, 전체 표준 라이브러리 감사를 통과시킨다.

## 통과 기준

- callback 버퍼의 owner·유효 범위·실제 길이 commit과 네 전제조건을 정확히 말한다.
- lvalue/prvalue/xvalue, 이동 생성, callback 비소유 수명, 결과 소유권, 복사 생략을 실제 식으로 설명한다.
- 모든 최초 표준 라이브러리 호출을 여섯 계약 항목으로 설명한다.
- Booth의 후보/공통 접두사 불변식과 `matched+1` 점프의 안전성을 증명한다.
- 전체 시간 `O(n)`, 추가 공간 `O(n)`을 유도하고 세 실행 파일·CTest·감사를 통과한다.
