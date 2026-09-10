# 2026-09-10 CHECKPOINT — `std::out_ptr`와 Manacher

체크 표시는 답을 소리 내어 설명하고 관련 식의 호출 전후 상태를 종이에 그린 뒤에만 한다. 표준 라이브러리 호출 문제는 최소한 **수신 객체의 정확한 타입·호출 전 상태, 선택된 signature/overload와 템플릿 인자, 각 인자의 타입·값 범주·소유권·허용값, 반환형·의미·사용 여부, 호출 뒤 상태, 전제/후조건·복잡도·할당·무효화·수명·오류·스레드 보장**을 포함해야 통과다.

## A. 기초 문법과 타입

- [ ] 1. `#include <memory>`, `<utility>`, `<iostream>`이 오늘 코드에서 각각 어떤 선언을 직접 제공하는가?
- [ ] 2. `int endpoint{}`와 초기화하지 않은 지역 `int endpoint;`의 초기 상태 차이를 설명하라.
- [ ] 3. `const int status{...}`에서 `const`, 타입, 중괄호 초기화가 각각 무엇을 뜻하는가?
- [ ] 4. `LegacyConnection*`와 `LegacyConnection**`를 상자와 슬롯 그림으로 구별하라.
- [ ] 5. `*output = new LegacyConnection{endpoint};`에서 첫 `*`, new 결과, 중괄호 초기화를 차례로 해석하라.
- [ ] 6. `delete connection;`이 객체 수명과 저장소에 각각 무엇을 하는가? null일 때도 답하라.
- [ ] 7. `struct LegacyConnection`과 `class Connection`의 기본 접근 차이를 말하라.
- [ ] 8. `public:` factory와 `private:` owner 분리가 어떤 불변식을 보호하는가?
- [ ] 9. `using ConnectionPtr = ...`가 새 타입을 만드는지 별칭을 만드는지 설명하고 두 템플릿 인자의 역할을 말하라.
- [ ] 10. `explicit Connection(ConnectionPtr handle)`에서 `explicit`을 제거하면 어떤 암시 변환 경계가 넓어지는가?
- [ ] 11. `: handle_{std::move(handle)}` 멤버 초기화 목록과 본문 안 대입의 객체 수명 차이를 설명하라.
- [ ] 12. `[[nodiscard]] static Connection open(int endpoint)`의 반환형, 매개변수, `static`, attribute 의미를 설명하라.
- [ ] 13. `if (output == nullptr || endpoint <= 0)`에서 `||` 단락 평가와 bool 조건 분기를 설명하라.
- [ ] 14. Manacher while의 세 조건에서 왜 문자열 인덱스 접근이 마지막이어야 하는가?

## B. lvalue/prvalue/xvalue, 복사·이동·수명

- [ ] 15. 생성자 본문의 `handle`은 타입이 move-only인데도 식으로는 왜 lvalue인가?
- [ ] 16. `std::move(handle)`의 결과 값 범주와 타입을 쓰고, move 자체가 포인터를 지우지 않는 이유를 설명하라.
- [ ] 17. 실제 raw pointer/deleter 이전은 어느 생성자가 수행하며 이동 뒤 원본은 어떤 상태인가?
- [ ] 18. `std::out_ptr(handle)` 반환 adapter의 값 범주와 수명 끝을 정확히 표시하라.
- [ ] 19. adapter가 빌린 `handle`과 C 함수가 받은 `LegacyConnection**` 중 무엇이 무엇보다 오래 살아야 하는가?
- [ ] 20. `return Connection{std::move(handle)};`이 같은 타입 prvalue 직접 구성인 이유와 NRVO의 차이를 말하라.
- [ ] 21. `const Connection connected{Connection::open(7)};`에서 중간 Connection 복사/이동이 필요한가?
- [ ] 22. `Connection` 복사가 금지되는 멤버 수준 이유와 double free를 막는 의미를 설명하라.
- [ ] 23. `operator->`가 반환한 raw pointer가 owner의 이동·소멸 뒤 댕글링이 되는 시점을 말하라.
- [ ] 24. 지역 객체 `connected`, `rejected`의 파괴 순서와 실제 C 해제 함수 호출 횟수를 예측하라.

## C. 실제 식 해석

- [ ] 25. `ConnectionPtr handle{};`에서 생성되는 정확한 특수화, 초기 포인터, deleter 상태를 쓰라.
- [ ] 26. `legacy_connect(endpoint, std::out_ptr(handle))`의 평가를 adapter 생성→변환→C 호출→상태 저장→adapter 소멸 순으로 적어라.
- [ ] 27. `const int status{legacy_connect(...)};`의 세미콜론 직전과 직후에 `handle` 상태가 어떻게 다른가?
- [ ] 28. C 함수가 오류 코드를 반환하면서도 non-null 포인터를 썼다면 wrapper가 누수를 막는 경로를 설명하라.
- [ ] 29. `if (legacy_connect(..., std::out_ptr(handle)) == 0 && handle)`가 성공해도 handle을 false로 볼 수 있는 이유는?
- [ ] 30. `return Connection{ConnectionPtr{}};`에서 임시 owner, 값 매개변수, 멤버 owner의 상태를 순서대로 표시하라.
- [ ] 31. `return handle_->endpoint;`에서 표준 `operator->`, 내장 `->`, int lvalue-to-rvalue 변환을 구분하라.
- [ ] 32. `std::cout << connected.endpoint() << ' ' << rejected.valid() << '\n';`의 각 피연산자 타입·값 범주와 stream 반환 사용을 적어라.
- [ ] 33. `const auto n_size{text.length()};`의 `auto` 추론 결과와 이후 int 변환이 오늘 제약에서 안전한 이유를 말하라.
- [ ] 34. `std::vector<int> odd(n_size, 0);`이 iterator-range 생성자가 아니라 fill 생성자를 선택하는 이유를 설명하라.
- [ ] 35. `std::min(odd[mirror], right-i+1)`이 반환 참조로 둘째 임시를 가리켜도 radius 복사가 안전한 수명 경계를 말하라.
- [ ] 36. `text.substr(best_start,best_length)`가 view가 아니라 독립 소유 값을 만드는 이유와 비용을 설명하라.

## D. 표준 라이브러리 호출 계약

- [ ] 37. `ConnectionPtr handle{}`의 목적 객체, overload, 인자 없음, 반환 없음, null 사후 상태, 복잡도·예외·소멸을 설명하라.
- [ ] 38. `std::move(handle)`의 템플릿 인자 추론, 인자 값 범주, 반환형, 상태 비변경, noexcept와 수명을 설명하라.
- [ ] 39. `std::out_ptr(handle)`의 `Pointer` 기본값과 실제 `P`, 반환 `out_ptr_t` 타입, owner reset 시점을 설명하라.
- [ ] 40. adapter의 `LegacyConnection**` 변환 반환값과 그 주소의 허용 수명, 두 변환 종류를 섞는 전제조건을 설명하라.
- [ ] 41. out_ptr adapter 소멸자의 null/non-null 분기, custom deleter 유지, 할당·예외·스레드 계약을 설명하라.
- [ ] 42. `out_ptr`와 `inout_ptr`를 기존 포인터 입력 여부와 해제 책임을 기준으로 비교하라.
- [ ] 43. `ConnectionPtr::operator bool`의 수신 상태, 인자, bool 반환 의미, 사후 상태와 동시성 계약을 설명하라.
- [ ] 44. `ConnectionPtr::operator->`의 non-null 전제, raw pointer 반환 수명, 빈 포인터 역참조의 미정의 동작을 설명하라.
- [ ] 45. `text.length()`의 수신 const 상태, 반환형·사용, O(1), 할당·무효화·오류 계약을 설명하라.
- [ ] 46. vector fill 생성자의 두 인자와 기본 allocator, 반환 없음, size/capacity, O(n), 실패 시 상태를 설명하라.
- [ ] 47. vector/string `operator[]`의 정확한 반환 참조, 범위 전제, O(1), 예외 없음과 미정의 동작을 설명하라.
- [ ] 48. `std::min` 두 인자의 참조 바인딩, 반환 참조, 동률 선택, O(1), 임시 수명을 설명하라.
- [ ] 49. `substr(pos,count)`의 pos/count 허용 범위, 반환 소유권, 원본 상태, O(count), 오류·할당을 설명하라.
- [ ] 50. `sync_with_stdio(false)`의 bool 인자/반환과 사용 여부, 첫 I/O 전 전제 및 C stdio 혼용 영향을 설명하라.
- [ ] 51. `std::cin.tie(nullptr)`의 pointer 인자, 이전 pointer 반환/무시, 이후 flush 상태와 수명을 설명하라.
- [ ] 52. `std::cin >> text`의 두 lvalue reference 인자, 반환 stream reference/무시, 실패 상태와 재할당 무효화를 설명하라.
- [ ] 53. string/int/bool 삽입과 `operator<<(std::ostream&, char)`의 overload, 반환 연쇄, 오류·복잡도·스레드 보장을 설명하라.

## E. Manacher 불변식과 증명

- [ ] 54. `odd[i]=k`가 나타내는 정확한 반열린 구간과 길이를 쓰라.
- [ ] 55. `even[i]=k`가 나타내는 중심 위치, 반열린 구간과 길이를 쓰라.
- [ ] 56. 홀수 radius 기본값이 1, 짝수 기본값이 0인 이유는?
- [ ] 57. 루프 시작의 `[left,right]` 불변식을 정확히 진술하라.
- [ ] 58. 홀수 mirror가 `left+right-i`, 짝수 mirror가 `left+right-i+1`인 이유를 그림으로 증명하라.
- [ ] 59. `min(radius[mirror], right-i+1)`에서 두 번째 상한이 반드시 필요한 반례를 만들어라.
- [ ] 60. 초기 radius 내부가 문자 비교 없이 팰린드롬이라고 결론 내릴 수 있는 이유를 대칭성으로 설명하라.
- [ ] 61. while 종료 뒤 radius가 과대도 과소도 아님을 첫 불일치/경계로 나누어 증명하라.
- [ ] 62. 두 pass를 합치면 모든 짝수·홀수 길이 팰린드롬을 빠짐없이 고려한다는 것을 보이라.
- [ ] 63. while의 성공 비교 총수가 pass마다 O(n)인 이유를 right의 단조 증가로 상각 분석하라.
- [ ] 64. 단순 중심 확장 `O(n^2)`, rolling hash+이분 탐색, Manacher의 시간·충돌·구현 난도를 비교하라.
- [ ] 65. tie에서 `>` 대신 `>=`를 쓰면 `babad` 결과가 어떻게 달라질 수 있고 문제 계약상 왜 둘 다 맞는가?

## F. 손으로 실행하기

- [ ] 66. `a`의 odd/even 배열과 최종 `[best_start,best_length]`를 적어라.
- [ ] 67. `abba`의 every-center even 값과 `[left,right]` 변화를 표로 만들라.
- [ ] 68. `aaaaa`의 odd 배열이 `1 2 3 2 1`인지 직접 확인하고 mirror 재사용 지점을 표시하라.
- [ ] 69. `cbbd`에서 odd 최장 1보다 even 최장 2가 갱신되는 문장을 찾으라.
- [ ] 70. `forgeeksskeegfor`의 정답 시작/길이를 계산하고 substr 인자를 적어라.

## G. 실기 검증

- [ ] 71. `main.cpp`를 빌드해 정확히 `7 0`이 출력되는지 확인했다.
- [ ] 72. `problem.cpp`를 가리고 다시 작성해 정확히 `42 0`이 출력되는지 확인했다.
- [ ] 73. CTest 9개가 모두 통과하고 출력 helper가 공백 차이도 잡는지 확인했다.
- [ ] 74. 1,000개 이상 작은 무작위 문자열을 독립 `O(n^2)` oracle과 비교했다.
- [ ] 75. 길이 1,000,000 동일/교대 문자열 stress에서 stack overflow 없이 제한 내 동작함을 확인했다.
- [ ] 76. 알고리즘 대표 문서의 C++20 예제를 따로 컴파일·실행했다.
- [ ] 77. `-Scope latest`와 `-Scope all` 표준 라이브러리 감사를 모두 통과했다.
- [ ] 78. README Mermaid, 모든 로컬 Markdown 링크, UTF-8, `git diff --check`를 확인했다.

## 최종 자기 설명

아래 문장을 자료를 보지 않고 완성한다.

> `std::out_ptr`는 ______ 수명 동안 ______ 출력 슬롯을 제공하고, 생성 시 스마트 포인터를 ______ 뒤 소멸 시 non-null 결과를 ______한다. 오늘 `handle` 식은 ______이고 `std::move(handle)` 결과는 ______이며 실제 소유권 이전은 ______가 수행한다. Manacher는 `[left,right]` ______ 불변식과 ______ 반지름을 재사용해 각 pass에서 right가 최대 ______번 전진하므로 시간 ______, 공간 ______이다.
