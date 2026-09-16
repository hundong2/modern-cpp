# 2026-09-17 CHECKPOINT — 말로 설명하고 코드로 증명하기

정답을 먼저 보지 말고 [`main.cpp`](main.cpp), [`problem.cpp`](problem.cpp), [`icpc_problem.cpp`](icpc_problem.cpp)를 한 줄씩 가리키며 설명하자. 답을 모르면 컴파일 가능한 작은 변경을 해 보고 결과를 다시 기록한다.

## 1. 기초 문법

1. `<expected>`, `<charconv>`, `<string_view>`, `<utility>`를 각각 왜 포함하는가? `#include`가 없을 때 우연한 간접 포함에 기대면 무엇이 위험한가?
2. `int requested`와 `unsigned parsed`의 값 범위 차이는 무엇인가? `parsed{}`와 `std::string{"service"}`에서 중괄호 초기화는 각각 무엇을 보장하는가?
3. `parse_port(std::string_view text)`의 반환형과 매개변수 타입을 말하라. `text`가 소유하지 않는 문자 배열을 함수 밖에 저장하면 어떤 문제가 생기는가?
4. `const char* const first`의 두 `const`를 분리해서 설명하라. `Quota::label() const`의 함수 뒤 `const`와 반환형 `const std::string&`도 각각 설명하라.
5. `class Config`와 `struct`의 기본 접근은 무엇인가? `public`/`private`가 직접 멤버 수정을 막아도 이 공개 생성자만으로 포트 범위 불변식을 강제할 수 없는 이유는? 두 인자 생성자의 `explicit`은 어떤 초기화 문법을 막는가?
6. `using Port = unsigned`가 새 타입인가? `expected<Config,ParseError>`의 두 템플릿 인자와 `and_then(validate)`의 피호출 함수 반환형은 무엇인가?
7. 멤버 초기화 목록에서 `std::move(name)`의 `name` 식은 lvalue인가 xvalue인가? `port_`와 `name_`의 실제 초기화 순서를 누가 정하는가?
8. `if (candidate)`가 거짓일 때 `*candidate`를 평가해도 되는가? 조건 분기와 중괄호 블록이 수명·제어 흐름을 어떻게 지키는가?

## 2. 실제 식과 Modern C++ 수명

1. `std::expected<Config,ParseError>{std::in_place, parsed, std::string{"service"}}`에서 `parsed`는 어떤 값 범주이고 문자열 식은 어떤 값 범주인가? 결과 `expected`가 입력 `string_view`를 빌리는가?
2. `auto candidate{parse_port("8080")};`에서 리터럴 배열, 임시 `string_view`, 반환 `expected`, 내부 `Config`의 수명을 각각 말하라. 어디에 C++17 보장 복사 생략이 적용되는가? 이름 있는 지역 반환의 NRVO와 어떻게 다른가?
3. `active = std::move(*candidate);`에서 `candidate`, `*candidate`, `std::move(*candidate)`의 값 범주를 분리해 말하라. 무엇이 실제로 문자열 자원을 이전하며 이동 뒤 원본은 어떤 상태인가?
4. `good->label()`의 참조를 저장해 두었다가 `good`을 파괴하면 어떤 포인터/참조 수명 문제가 생기는가? `std::expected`의 오류 경로에서 `transform` 람다는 호출되는가?
5. "실패한 검증은 active를 바꾸지 않는다"와 "이동 대입이 모든 사용자 타입에서 예외 없이 원자적이다"의 차이를 말하라.
6. `from_chars`의 문자 로드/비교, `if`의 조건 분기, 문자열 이동의 저장소 갱신 관점을 설명하되 특정 CPU 명령이나 가상 호출을 단정하지 말아야 하는 이유는?

## 3. 표준 라이브러리 호출 계약 여섯 항목

각 식에 대해 **수신 객체/호출 전 상태 → 선택 시그니처·템플릿 인자 → 각 인자의 타입·값 범주·소유권·허용값 → 반환형·사용 여부 → 호출 뒤 상태 → 전제/후조건·복잡도·할당·무효화·수명·오류·스레드**를 빠짐없이 구술하라. 실제 답은 코드 바로 위 주석과 [공용 표준 라이브러리 문서](../standard-library/README.md)를 대조한다.

1. `text.empty()`, `text.data()`, `text.size()`: 길이가 0인 기본 생성 뷰의 `data()`로 포인터 산술을 바로 하면 무엇을 먼저 증명해야 하는가?
2. `std::from_chars(first, last, parsed)`: 범위를 벗어난 수, 접미 문자가 남는 수, 선행 공백은 각각 `ec`·`ptr`·`parsed`에 어떻게 반영되는가? 예외/할당 여부는?
3. `std::expected<Config,ParseError>{std::in_place,...}`와 `std::unexpected{ParseError::Invalid}`: 무엇이 실제 값을 소유하며 생성 실패는 어떻게 전파되는가?
4. `candidate`, `*candidate`, `rejected.error()`: 값/오류 상태의 각 관찰 전제조건과 잘못 접근할 때의 오류(UB 또는 예외)를 구별하라.
5. `.and_then(validate).transform(lambda)`: 수신 객체가 prvalue일 때 선택한 `&&` 오버로드, callable의 매개변수/반환형, 오류 전파와 성공 경로의 Quota 수명을 설명하라.
6. `std::cout << "quota=" << good->units()`: 스트림 삽입의 각 반환형, 출력 버퍼/실패 상태, 다른 스레드 출력과 뒤섞일 수 있는 부분을 설명하라.
7. ICPC 풀이의 `std::vector` 생성/첨자/삽입과 입력 스트림 `>>`: 범위 전제조건, 재할당 무효화, 값 복사, I/O 실패와 시간 복잡도를 실제 줄에 맞춰 설명하라.

## 4. 함수형 그래프 알고리즘 증명

1. 모든 정점의 나가는 간선이 하나이면 유한 그래프에서 따라가던 경로가 결국 사이클에 들어가는 이유를 증명하라.
2. 진입 차수 0인 정점부터 제거한 뒤 남은 정점이 정확히 사이클 정점인 이유는? 별도 컴포넌트의 사이클이 서로 합쳐질 수 있는가?
3. 제거 순서 역방향에서 `answer[next[v]]`가 이미 계산되었음을 불변식으로 쓰고, `answer[v] = answer[next[v]] + 1`을 증명하라.
4. `next = [2,4,3,1,4]`에서 각 정점의 정답을 직접 추적하라. 자기 루프 하나, 사이클 앞 긴 꼬리, 여러 사이클이 있을 때도 같은 규칙이 성립하는가?
5. 각 정점이 큐/배열/사이클 열거에서 몇 번 처리되는지 세어 `O(n)` 시간·공간을 유도하라. 재귀 DFS와 비교해 `n`이 클 때 스택 위험은 무엇인가?

## 5. 직접 해 보는 변경과 검증

- `parse_port("42x")`, `parse_port("0")`, `parse_port("65536")`, `parse_port("+42")`의 반환 상태를 예상하고 리터럴을 바꾸어 실행한다. 첫 유효 숫자 뒤 남은 문자를 확인하지 않으면 어떤 버그가 생기는가?
- `make_quota(51)`과 `make_quota(0)`의 결과를 예상한다. 람다에 카운터를 추가해 오류 경로에서 실제로 실행되지 않음을 검증한다.
- 세 정점 `1→2→3→3`의 정답을 계산하고 CTest에 한 사례를 추가해 본다. 기대값과 프로그램 결과를 비교한다.
- 마지막으로 [README의 검증 절](README.md#검증)을 재현하고 경고, 실패 테스트, 누락된 문서 링크가 없는지 확인한다.
