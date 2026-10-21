# 2026-10-22 CHECKPOINT

자료를 닫고 먼저 말이나 종이에 답한다. “대충 안다”가 아니라 **타입, 값 범주, 소유자, 수명 끝, 전제조건, 반환값 사용, 복잡도**를 실제 식에 붙일 수 있어야 통과다.

## 1. 기초 문법

1. `struct`와 `class`의 기본 접근 지정자는 각각 무엇인가?
2. `public`과 `private`는 컴파일 시 어떤 접근을 허용하거나 막는가?
3. `int replicas{};`와 초기화하지 않은 지역 `int replicas;`의 차이는 무엇인가?
4. 중괄호 초기화가 narrowing을 막는 예를 하나 만든다.
5. 생성자에 반환형을 쓰지 않는 이유는 무엇인가?
6. `explicit ConfigRepository(...)`가 막는 암시 변환은 무엇인가?
7. 멤버 초기화 목록은 생성자 본문보다 언제 실행되는가?
8. 초기화 목록에 쓴 순서와 실제 멤버 초기화 순서가 다르면 어느 순서가 우선하는가?
9. `[[nodiscard]]`가 붙은 결과를 버리면 어떤 도움을 받을 수 있는가?
10. `find(...) const`의 마지막 `const`와 `const ConfigRepository& repository`의 앞 `const`를 각각 설명한다.
11. pointer와 reference의 null 가능성·재바인딩·소유권을 비교한다.
12. `using Value = long long;`은 새 독립 타입을 만드는가, 별칭만 만드는가?
13. range-for에서 `const ServiceConfig& config`로 받는 이유를 복사 비용과 변경 가능성 관점에서 설명한다.
14. `if (launch)`가 false이면 중괄호 안 `launch->service`가 평가되는가?
15. 함수의 값 매개변수 `std::vector<ServiceConfig> configs`는 호출자의 vector와 같은 객체인가?

## 2. `optional` 상태와 호출 계약

16. `std::optional<T>`는 `T*`와 달리 `T` 객체를 어디에 소유하는가?
17. engaged와 disengaged 상태를 각각 설명한다.
18. 빈 optional에 `value()`를 호출할 때와 `operator*`를 호출할 때의 실패 계약 차이는 무엇인가?
19. `std::nullopt`의 타입은 무엇이며 어떤 생성자를 선택하는가?
20. `has_value()`의 매개변수, 반환형, 상태 변화, 복잡도, 예외 보장을 말한다.
21. `operator bool()`이 `explicit`이어도 `if (launch)`에서 쓸 수 있는 이유는 무엇인가?
22. `operator->()`가 돌려주는 pointer는 언제까지 유효한가?
23. `value_or(-1)`은 값이 있는 경우에도 `-1` 식을 평가하는가? 반환값은 reference인가 값인가?
24. `optional` 자체는 heap 할당을 반드시 하는가? contained `std::string`도 반드시 무할당인가?
25. 같은 optional 객체를 한 thread가 바꾸는 동안 다른 thread가 `has_value()`를 호출해도 되는가?

## 3. `transform`을 식으로 증명하기

26. `std::optional::transform`이 C++ 어느 표준에 추가되었는가?
27. 수신 객체가 각각 `optional<T>&`, `const optional<T>&`, `optional<T>&&`, `const optional<T>&&`일 때 callable 인자의 cv/ref는 무엇인가?
28. empty receiver에서 callable 호출 횟수와 결과 상태는 무엇인가?
29. engaged receiver에서 callable 호출 횟수의 상한은 몇 번인가?
30. callable이 `U`를 반환하면 전체 반환형은 무엇인가?
31. callable이 `optional<U>`를 반환하면 `transform` 결과가 왜 중첩되는가?
32. 같은 callable이 이미 `optional<U>`를 반환하고 평탄화가 필요하면 어떤 연산을 써야 하는가?
33. `transform`의 `U`로 `void`나 reference 타입을 직접 쓸 수 없는 이유를 optional의 contained-type 조건으로 설명한다.
34. callable이나 `U` 생성자가 던진 예외는 어디로 가는가?
35. 결과 `U`가 `string_view`나 pointer라면 `optional<U>`가 생겼다는 사실만으로 pointee 수명이 연장되는가?

다음 실제 식을 한 토큰씩 해석한다.

```cpp
return std::move(found).transform([](ServiceConfig&& config) {
    return LaunchPlan{std::move(config.service), config.replicas};
});
```

36. 이름 있는 `found` 식의 값 범주는 무엇인가?
37. `std::move(found)`의 반환형과 값 범주는 무엇인가?
38. `std::move` 호출 자체가 contained `ServiceConfig`를 이동시키는가?
39. 위 식에서 선택되는 `transform` ref-qualified overload는 무엇인가?
40. lambda 매개변수가 `ServiceConfig&&`에 바인딩되는 원본 객체는 어디에 저장되어 있는가?
41. 이름 있는 rvalue-reference 매개변수 `config`를 식으로 쓸 때 값 범주는 무엇인가?
42. 그래서 `config.service`에 다시 `std::move`가 필요한 이유는 무엇인가?
43. 문자열 값/자원의 실제 이동은 `std::move` 함수와 string 이동 생성 중 어디서 일어나며, 왜 heap buffer 탈취까지 보장된다고 말하면 안 되는가?
44. 성공 뒤 원본 `found`는 반드시 empty가 되는가? 아니라면 contained 객체 상태는 어떻게 설명해야 하는가?
45. 결과 `LaunchPlan`이 repository보다 오래 살아도 안전한 이유는 무엇인가?
46. lambda가 실행되지 않는 missing 경로에서 `config.service` 식은 평가되는가?

## 4. 복사·이동·수명·RVO

47. `return config;`가 repository 원소를 가리키는 reference를 반환하지 않고 어떤 객체를 만드는가?
48. 이 복사 중 `std::string` 할당이 실패하면 원본 repository 상태는 보존되는가?
49. `repository.find(service)` prvalue로 지역 optional을 초기화할 때 C++17 보장 복사 생략을 설명한다.
50. 보장 복사 생략과 이름 있는 local 반환의 NRVO는 같은 의무 규칙인가?
51. `std::string_view service`가 소유하지 않는 것은 무엇인가?
52. 오늘 literal로 만든 string_view가 안전한 수명 근거는 무엇인가?
53. 함수가 임시 `std::string`에서 view를 받아 저장해 두었다면 어떤 문제가 생기는가?
54. vector 이동 생성 뒤 source vector에 할 수 있는 일과 가정하면 안 되는 일을 구분한다.
55. vector 이동 뒤 기존 원소 reference/iterator의 표준 보장을 allocator 없는 이동 생성과 연결해 설명한다.

## 5. Lazy Segment Tree 불변식

56. 구간 `[left,right]`를 담당하는 노드 합이 언제나 나타내야 하는 값은 무엇인가?
57. 자식 노드가 아직 과거 값이어도 완전 포함 질의가 부모 합을 바로 써도 되는 이유는 무엇인가?
58. 일부 포함 질의와 갱신 전에 `push`해야 하는 이유는 무엇인가?
59. `push` 뒤 부모 tag를 항등 상태로 되돌리지 않으면 어떤 중복 적용이 생기는가?
60. 합의 결합 항등원이 0인 이유는 무엇인가?
61. 닫힌 구간 길이가 `right-left+1`인 이유를 길이 1인 구간으로 검증한다.
62. 재귀 중간점을 `left + (right-left)/2`로 계산하는 습관의 장점은 무엇인가?
63. tree와 lazy 배열을 보통 `4*n` 크기로 잡는 이유는 무엇인가?
64. build가 `O(n)`이지 `O(n log n)`이 아닌 이유는 무엇인가?
65. 한 번의 연속 구간 update/query가 `O(log n)`인 근거를 표준 구간 분해로 설명한다.
66. 재귀 stack 공간이 `O(log n)`인 이유는 무엇인가?

## 6. 대입·덧셈 tag 합성

tag를 함수로 생각한다. 대입이 없으면 `f(v)=v+addition`, 대입이 있으면 `f(v)=assignment+addition`이다. 새 대입은 두 수치를 `assignment=x, addition=0`으로 덮고, 새 덧셈은 `addition`에 누적한다.

67. 항등 tag는 `has_assignment`와 `addition`이 각각 어떤 값인가?
68. 기존 `+5` 뒤 새 `=2`를 합성한 최종 함수는 무엇인가?
69. 기존 `=2` 뒤 새 `+5`를 합성한 최종 함수는 무엇인가?
70. 68번과 69번 결과가 다른 사실이 어떤 대수적 성질의 부재를 뜻하는가?
71. 새 대입이 기존 대입과 덧셈을 모두 버려야 하는 이유는 무엇인가?
72. pending 대입이 있을 때도 새 덧셈을 `addition`에 누적하고 push에서 대입 다음에 적용해야 하는 이유는 무엇인가?
73. “대입값 0”과 “대입 없음”을 숫자 하나로 함께 나타내면 왜 틀리는가?
74. 노드 길이가 `len`일 때 `=x`를 적용한 합과 `+x`를 적용한 합 공식을 각각 쓴다.
75. 자식에 여러 tag를 내릴 때 기존 자식 tag 다음에 부모 tag가 실행되어야 하는 시간 순서를 함수 합성으로 쓴다.
76. `apply_set` 뒤 `apply_add`와 `apply_add` 뒤 `apply_set`을 길이 3, 초기값 `[1,2,3]`으로 손 계산한다.

## 7. 실제 CSES 입력 추적

공식 예제 초기 배열은 `[2,3,1,1,5,3]`이다.

77. 첫 `3 3 5`의 합이 7인 과정을 적는다.
78. `1 2 4 2` 뒤 전체 배열을 적는다.
79. 다음 `3 3 5`의 합이 11인 과정을 적는다.
80. `2 2 4 5`가 이전 덧셈 효과를 해당 구간에서 어떻게 덮는가?
81. 마지막 `3 3 5`가 15인 과정을 적는다.
82. 1-based `[a,b]`를 0-based로 바꿀 때 어느 두 값에서 1을 빼는가?
83. `n=q=200000`, 값/갱신이 최대일 때 합에 `int`가 부족함을 수치로 보인다.
84. `long long` 범위 안이라는 근거를 최악 갱신 횟수와 구간 길이로 계산한다.

## 8. 표준 라이브러리 호출 계약 말하기

각 식에 대해 여섯 항목을 빠짐없이 말한다: (1) 정확한 수신 타입·호출 전 상태, (2) signature/overload/template 인자, (3) 각 인자의 타입·값 범주·소유 의미·허용값, (4) 반환형·의미·사용 여부, (5) 호출 뒤 상태, (6) 전제조건·후조건·복잡도·할당·무효화·수명·오류·thread.

85. `std::move(found)`
86. `found`를 xvalue로 만든 뒤의 `transform(lambda)`
87. `std::move(config.service)`와 뒤의 string 이동 생성
88. `repository.find(service)`가 성공하며 만드는 optional 값 생성자
89. `return std::nullopt`
90. `if (launch)`의 optional `operator bool`
91. `launch->service`의 optional `operator->`
92. `missing.has_value()`
93. `known.value_or(-1)`
94. vector initializer-list 생성자와 repository 안 vector 이동 생성자
95. range-for가 사용하는 begin/end/반복자 연산
96. `std::ios::sync_with_stdio(false)`
97. `std::cin.tie(nullptr)`
98. 첫 입력 연쇄 `std::cin >> n >> q`
99. segment tree vector 크기 생성자와 첫 `operator[]`
100. 첫 정답 출력 `std::cout << answer << '\n'`

## 9. 기계 실행 관점

101. optional의 존재 상태를 검사할 때 개념적으로 어떤 load와 조건 분기를 떠올릴 수 있는가?
102. 문자열 비교에는 어떤 길이/문자 load와 조기 분기가 있을 수 있는가?
103. lazy tree의 완전 포함/부분 포함/불포함 판정은 어떤 비교와 분기를 만드는가?
104. tag를 자식에 내릴 때 어떤 sum/tag store가 필요할 수 있는가?
105. 오늘 코드에 virtual 함수와 가상 간접 호출이 있는가?
106. lambda 호출이 반드시 실행 시 간접 호출로 남는다고 단정할 수 없는 이유는 무엇인가?
107. Debug와 Release에서 load/store/분기/재귀 호출 모양이 달라질 수 있는 이유는 무엇인가?
108. 소스 한 줄이 특정 어셈블리 한 줄과 언제나 일대일 대응한다고 말하면 왜 틀리는가?

## 10. 실기 통과 과제

1. `problem.cpp`의 lambda를 `score + 10`으로 바꾸고 기대 출력을 먼저 적은 뒤 CTest를 함께 수정한다.
2. empty optional에서는 lambda가 실행되지 않음을 부작용 counter로 증명하되, counter의 소유자와 thread 전제를 주석에 쓴다.
3. `main.cpp`의 `std::move(found)`를 제거해 `&` overload가 선택되게 바꾸고, lambda 매개변수와 복사/이동 차이를 고친다.
4. 길이 8 배열에 `+3`, `=0`, `+4`를 겹치는 구간으로 적용하며 루트와 두 자식의 sum/tag를 손으로 추적한다.
5. `push`를 일부 포함 query 전에 생략했을 때 틀리는 최소 반례를 만든다.
6. `apply_set`에서 구간 길이 곱을 빠뜨렸을 때 잡아내는 CTest를 추가한다.
7. `add -> assign`과 `assign -> add` 순서를 각각 검사하는 서로 다른 입력을 만든다.
8. `n<=8`, 무작위 명령을 생성해 단순 배열 oracle과 ICPC 실행 결과를 최소 1,000회 비교한다.
9. `n=q=200000` stress 입력으로 시간과 stack 깊이를 확인한다.
10. 공용 감사와 CTest를 모두 실행하고 build 산출물이 Git stage에 들어가지 않았음을 확인한다.

## 통과 기준

- 1~108 중 핵심 질문 16~46, 56~76, 85~100을 자료 없이 정확히 설명한다.
- “이동했다” 대신 **어느 식이 xvalue이고 어느 생성자가 실제 자원을 이전했는지** 말한다.
- 네 가지 tag 순서 사례를 함수 합성으로 증명하고 시간·공간 복잡도를 유도한다.
- 여섯 항목 호출 계약에서 반환값 사용 여부, 무효화/수명, 예외/UB, thread 조건을 생략하지 않는다.
- 엄격 경고 build, 모든 CTest, 무작위 oracle, 표준 라이브러리 전체 감사가 통과한다.
