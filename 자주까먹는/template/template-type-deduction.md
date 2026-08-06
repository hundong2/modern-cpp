# C++23 템플릿 타입 추론: 처음부터 이해하는 가이드

> **한 줄 요약:** 컴파일러는 함수 인수의 타입을 템플릿 매개변수 자리에 맞춰 보는
> “패턴 맞추기”로 `T`를 결정합니다.

- 선행 지식: **없어도 됩니다. C++를 잊었다면 0절부터 다시 시작합니다.**
- 첫날 읽을 부분: **0절만** 읽고 복사와 참조 예제를 손으로 따라 합니다.
- 둘째 학습 단계: 1~4절의 `T`, `T&`, `const T&`
- 나중에 읽을 부분: 5절 이후의 `auto`, `decltype`, 전달 참조와 concept
- 기준 표준: C++23

여기서 **템플릿(template)** 은 `자주까먹는/template/`이라는 폴더 이름이 아니라,
여러 타입에 재사용할 C++ 코드의 틀을 뜻합니다.

표준 문맥에서 이 과정은 template argument deduction(템플릿 인자 추론)이라고
부릅니다. 이 문서에서는 초보자가 이해하기 쉬운 “템플릿 타입 추론”이라는 표현도
같은 뜻으로 사용합니다.

> **중요:** 이 문서를 한 번에 끝까지 이해할 필요가 없습니다. 8절의 `T&&`나
> 12절의 CTAD를 읽다가 막히는 것은 정상입니다. 먼저 0~4절만 이해하면 일반적인
> 함수 템플릿의 대부분을 읽기 시작할 수 있습니다.


## 0. C++를 다 잊었다면 여기서 다시 시작

템플릿을 이해하려면 먼저 “타입”, “변수”, “함수”, “매개변수”, “복사”, “참조”가
무엇인지 알아야 합니다. 지금은 템플릿을 잠시 잊고 아주 작은 C++ 프로그램부터
다시 시작합니다.

### 0.1 C++ 코드는 언제 실행될까?

C++ 소스 코드를 작성했다고 바로 CPU가 그 글자를 읽는 것은 아닙니다. 먼저 컴파일러가
코드를 검사하고 기계가 실행할 수 있는 파일로 바꿉니다.

```mermaid
flowchart LR
    SOURCE["1. 사람이 C++ 소스 작성"] -->|컴파일| OBJECT["2. 목적 코드 생성"]
    OBJECT -->|링크| PROGRAM["3. 실행 파일 생성"]
    PROGRAM -->|실행| RESULT["4. CPU가 명령 수행"]
```

시간 순서는 “소스 작성 → 컴파일 → 링크 → 실행”입니다.

- **컴파일 시간:** 타입이 맞는지, 문법이 맞는지, 템플릿의 `T`가 무엇인지 결정합니다.
- **실행 시간:** 완성된 프로그램이 실제 값을 읽고 계산하고 화면에 출력합니다.

템플릿 타입 추론은 대부분 **실행하기 전인 컴파일 시간**에 일어납니다. 실행 중에
`T`라는 상자를 열어 타입을 고르는 것이 아닙니다.

### 0.2 가장 작은 프로그램 읽기

```cpp
#include <iostream>

int main() {
    std::cout << "안녕하세요\n";
    return 0;
}
```

한 줄씩 읽으면 다음과 같습니다.

- `#include <iostream>`: 화면 출력 기능의 선언을 가져옵니다.
- `int main()`: 프로그램이 시작되는 특별한 함수입니다.
- `{`와 `}`: 함수에 속한 코드 범위를 표시합니다.
- `std::cout << ...`: 글자를 표준 출력에 보냅니다.
- `\n`: 줄을 바꿉니다.
- `;`: 하나의 명령문이 끝났다는 표시입니다.
- `return 0`: 프로그램이 정상적으로 끝났음을 운영체제에 알립니다.

지금은 `std::`의 정확한 의미를 몰라도 됩니다. 표준 라이브러리에서 제공하는 이름에는
대체로 `std::`가 붙는다고만 기억합니다.

### 0.3 값, 타입, 객체, 변수

다음 한 줄을 봅니다.

```cpp
int age{20};
```

이 한 줄에는 네 가지 개념이 있습니다.

```text
int       age       {20};
^^^       ^^^        ^^
타입      변수 이름   처음 저장할 값
```

- **값(value):** `20`처럼 프로그램이 다루는 실제 정보입니다.
- **타입(type):** `int`처럼 그 정보를 어떻게 저장하고 사용할지 정하는 규칙입니다.
- **객체(object):** 실행 중에 값을 담기 위해 마련된 실제 저장 공간입니다.
- **변수(variable):** 그 객체를 코드에서 다시 찾을 수 있게 붙인 이름입니다.

상자에 비유하면 다음과 같습니다.

```text
타입 int  = “정수를 담는 규격의 상자”
객체      = 실행 중 메모리에 실제로 만들어진 상자
변수 age  = 그 상자에 붙인 이름표
값 20     = 상자 안의 내용
```

`age`를 바꾸면 같은 객체 안의 값이 바뀝니다.

```cpp
int age{20};  // age 객체를 만들고 20을 저장합니다.
age = 21;     // 같은 age 객체의 값을 21로 바꿉니다.
```

`=`는 여기에서 오른쪽 값을 왼쪽 객체에 대입한다는 뜻입니다. 수학의 “항상 같다”와
조금 다릅니다.

### 0.4 자주 보는 기본 타입

```cpp
int count{3};                    // 정수
double temperature{23.5};        // 소수점이 있는 실수
bool completed{false};           // true 또는 false
char grade{'A'};                 // 문자 하나
std::string title{"C++ 공부"};   // 여러 문자를 소유하는 문자열
```

타입이 다른 이유는 저장할 값의 모양과 허용할 연산이 다르기 때문입니다.

```cpp
int result{10 + 20};             // 정수 덧셈
std::string word{"C++"};
word += "23";                    // 문자열 뒤에 글자 추가
```

컴파일러는 타입을 보고 `+`가 정수 덧셈인지 문자열 연산인지 판단합니다. 템플릿의
`T`를 알아야 하는 이유도 결국 “어떤 크기의 객체를 만들고 어떤 연산을 허용할지”
결정하기 위해서입니다.

### 0.5 함수는 이름 붙인 작업 묶음

함수는 입력을 받아 정해진 작업을 하고 필요하면 결과를 돌려주는 코드 묶음입니다.

```cpp
int add(int left, int right) {
    int result{left + right};
    return result;
}
```

조각별로 읽습니다.

```text
int       add       (int left, int right)
^^^       ^^^        ^^^^^^^^  ^^^^^^^^^
반환 타입  함수 이름   첫 입력칸  둘째 입력칸
```

- 첫 번째 `int`: 함수가 최종 결과로 돌려주는 타입입니다.
- `add`: 함수를 호출할 때 사용할 이름입니다.
- `int left`, `int right`: 함수가 받을 입력칸입니다.
- `return result`: 계산한 값을 호출한 위치로 돌려줍니다.

함수를 사용하는 것을 **호출(call)** 이라고 합니다.

```cpp
int answer{add(10, 20)};
```

실행 순서를 풀면 다음과 같습니다.

```text
1. add 함수에 10과 20을 전달
2. 함수 안에서 left는 10, right는 20
3. result에 30 저장
4. return으로 30을 돌려줌
5. answer에 30 저장
```

### 0.6 매개변수와 인수는 다른 말

초보자가 가장 자주 혼동하는 두 단어입니다.

```cpp
int add(int left, int right) {  // left와 right는 매개변수
    return left + right;
}

int answer{add(10, 20)};        // 10과 20은 인수
```

- **매개변수(parameter):** 함수를 만들 때 선언한 입력칸 `left`, `right`
- **인수(argument):** 실제 호출할 때 입력칸에 넣는 값 `10`, `20`

비유하면 매개변수는 신청서의 빈칸이고, 인수는 그 빈칸에 실제로 적는 내용입니다.

```text
함수 선언의 빈칸: add(int left, int right)
실제 호출 내용:   add(10,       20)
                      ↓          ↓
                    left       right
```

“컴파일러가 함수 **인수**를 보고 템플릿 **매개변수** `T`를 추론한다”는 문장은
결국 실제로 넣은 `10`을 보고 입력칸의 타입을 정한다는 뜻입니다.

### 0.7 값으로 전달하면 복사본을 만든다

다음 함수는 `int number`처럼 `&` 없이 값을 받습니다.

```cpp
void add_one_copy(int number) {
    number = number + 1;
}
```

`void`는 돌려주는 결과 값이 없다는 뜻입니다.

```cpp
int main() {
    int original{10};
    add_one_copy(original);
    std::cout << original << '\n';  // 10
}
```

왜 `11`이 아니라 `10`일까요? `add_one_copy`의 `number`는 `original`과 다른
새로운 객체이기 때문입니다.

```text
main 안 original 상자:       [10]
                               │ 값을 복사
                               ▼
함수 안 number 상자:         [10] → [11]

함수가 끝나면 number 상자는 사라짐
original 상자는 계속 [10]
```

이것을 pass by value(값으로 전달)라고 합니다.

### 0.8 `&` 참조로 받으면 원본을 사용한다

원본을 바꾸고 싶다면 reference(참조)를 사용할 수 있습니다.

```cpp
void add_one_original(int& number) {
    number = number + 1;
}
```

`int& number`는 새로운 `int` 값을 복사해 담는 상자라기보다, 호출자가 가진 `int`
객체에 붙인 또 다른 이름이라고 먼저 이해하면 됩니다.

```cpp
int main() {
    int original{10};
    add_one_original(original);
    std::cout << original << '\n';  // 11
}
```

```text
original ─────┐
              ▼
            [10] → [11]
              ▲
number ───────┘

두 이름이 같은 int 객체를 가리킴
```

함수가 끝나면 `number`라는 참조 이름은 없어지지만 원본 `original` 객체는 계속
존재합니다.

### 0.9 `const`는 “이 길로는 바꾸지 않겠다”

읽기만 하고 원본을 바꾸지 않을 함수는 `const` 참조를 자주 사용합니다.

```cpp
void print_number(const int& number) {
    std::cout << number << '\n';
}
```

- `&`: 원본을 복사하지 않고 봅니다.
- `const`: 이 `number`라는 접근 경로로 원본을 바꾸지 않겠다고 약속합니다.

다음 코드는 컴파일되지 않습니다.

```cpp
void print_number(const int& number) {
    number = 100;  // 오류: const 참조로 값을 변경하려고 했습니다.
}
```

`const`는 “이 메모리가 우주가 끝날 때까지 절대 변하지 않는다”는 뜻이 아닙니다.
현재 이름과 접근 경로로 수정하지 못하게 타입 시스템이 검사한다는 뜻에 가깝습니다.

### 0.10 복사, 수정 참조, 읽기 참조를 한 표로 비교

```cpp
void copy_value(int value);          // 복사본을 받음
void edit_original(int& value);      // 원본을 받아 수정할 수 있음
void read_original(const int& value); // 원본을 복사 없이 읽기만 함
```

| 선언 | 새 `int` 값 복사 | 원본 수정 가능 | 숫자 `42` 직접 전달 |
|---|---:|---:|---:|
| `int value` | 예 | 아니요 | 가능 |
| `int& value` | 아니요 | 예 | 불가능 |
| `const int& value` | 아니요 | 아니요 | 가능 |

나중에 `int` 자리에 `T`를 놓아도 이 세 가지 의미는 그대로입니다.

```cpp
void copy_value(int value);
template <typename T>
void copy_value(T value);
```

```cpp
void edit_original(int& value);
template <typename T>
void edit_original(T& value);
```

```cpp
void read_original(const int& value);
template <typename T>
void read_original(const T& value);
```

템플릿은 갑자기 완전히 다른 문법이 되는 것이 아닙니다. 이미 아는 함수에서 구체적인
타입 `int`를 “나중에 컴파일러가 정할 빈칸 `T`”로 바꾼 것입니다.

### 0.11 첫 템플릿을 정말 천천히 실행해 보기

일반 함수를 먼저 봅니다.

```cpp
void print_value(int value) {
    std::cout << value << '\n';
}
```

이 함수는 `int`만 받습니다. 이제 `int` 자리를 `T`로 바꿉니다.

```cpp
template <typename T>
void print_value(T value) {
    std::cout << value << '\n';
}
```

호출합니다.

```cpp
print_value(10);
```

컴파일러 입장에서 순서대로 적으면 다음과 같습니다.

```text
1. 호출 이름은 print_value
2. 실제 인수는 10
3. 10의 타입은 int
4. 함수 입력칸의 타입 자리는 T
5. 따라서 이번 호출에서 T = int
6. print_value<int>(int value) 형태의 함수를 사용할 수 있음
7. 컴파일 완료 후 실행할 때 value에 10을 복사하고 출력
```

다른 타입으로 호출합니다.

```cpp
print_value(3.14);
```

이번에는 `3.14`의 타입이 `double`이므로 `T = double`입니다. 컴파일러는 필요한
`double` 버전의 함수도 준비합니다.

### 0.12 템플릿을 만났을 때 사용하는 초보자 해독법

다음 코드를 바로 이해하려고 애쓰지 않습니다.

```cpp
template <typename T>
void show(const T& value);
```

먼저 `T`를 익숙한 `int`로 바꿔 적습니다.

```cpp
void show(const int& value);
```

이제 읽을 수 있습니다.

```text
show는 int 원본을 복사하지 않고 const 참조로 읽는 함수
```

그다음 `T=std::string`도 넣어 봅니다.

```cpp
void show(const std::string& value);
```

결론:

```text
show는 어떤 T가 오더라도 그 원본을 복사하지 않고 읽으려는 함수
```

복잡한 템플릿을 만나면 항상 다음 순서를 사용합니다.

1. `template <...>` 줄에서 타입 이름 `T`를 찾습니다.
2. `T`를 일단 `int`로 모두 바꿔 씁니다.
3. `&`와 `const`가 어디에 남는지 봅니다.
4. 실제 호출 인수의 타입으로 다시 바꿔 봅니다.

### 0.13 용어 번역 카드

| 영어/용어 | 이 문서에서의 쉬운 뜻 |
|---|---|
| type | 값을 저장하고 사용할 규칙 |
| object | 실행 중 값을 담는 실제 저장 공간 |
| variable | 객체를 찾기 위한 이름 |
| function | 이름 붙인 작업 묶음 |
| parameter | 함수를 만들 때 선언한 입력칸 |
| argument | 함수를 호출할 때 실제로 넣는 값 |
| template | 타입 일부를 빈칸으로 둔 코드 틀 |
| deduction | 실제 인수를 보고 빈칸을 알아내는 것 |
| instantiation | 알아낸 타입으로 필요한 함수/클래스를 만드는 과정 |
| reference | 원본 객체를 복사하지 않고 가리키는 별명 같은 기능 |
| const | 이 접근 경로로 수정하지 않겠다는 타입 규칙 |

### 0.14 첫날 확인 문제

다음 코드를 봅니다.

```cpp
void change_copy(int number) {
    number = 99;
}

void change_original(int& number) {
    number = 77;
}

int main() {
    int value{10};
    change_copy(value);
    change_original(value);
}
```

질문:

1. `change_copy` 호출 뒤 `value`는 얼마인가요?
2. `change_original` 호출 뒤 `value`는 얼마인가요?
3. `number`는 매개변수인가요, 인수인가요?
4. 호출 위치의 `value`는 매개변수인가요, 인수인가요?

<details>
<summary>정답</summary>

1. `change_copy`는 복사본만 99로 바꾸므로 원본 `value`는 여전히 10입니다.
2. `change_original`은 참조로 원본을 바꾸므로 `value`는 77입니다.
3. 함수 선언에 있는 `number`는 매개변수입니다.
4. 호출할 때 실제로 전달한 `value`는 인수입니다.

</details>

여기까지 이해했다면 오늘은 충분합니다. 다음에 1~4절을 읽으면서 `int`가 `T`로
바뀌었을 때 무엇이 달라지는지만 확인합니다. `T&&`, `decltype`, CTAD는 아직 건너뜁니다.

## 1. 타입부터 아주 짧게 이해하기

타입은 어떤 값을 어떻게 저장하고 어떤 연산을 허용할지 정하는 규칙입니다.

```cpp
int count{3};                 // count의 타입은 int입니다.
double ratio{0.5};            // ratio의 타입은 double입니다.
std::string name{"C++"};      // name의 타입은 std::string입니다.
```

`int` 객체에는 정수 값이 직접 들어갑니다. `std::string` 객체는 문자열의 길이와
저장 위치 같은 정보를 관리하며, 긴 문자는 별도의 힙 메모리에 저장될 수 있습니다.
타입을 알면 컴파일러는 객체 크기, 정렬 조건, 생성·복사·파괴 방법을 결정할 수 있습니다.

### 타입에 붙는 세 가지 중요한 정보

```cpp
int value{10};          // 수정 가능한 int 객체
const int limit{20};    // 이 이름을 통해 수정할 수 없는 const int 객체
int& alias{value};      // value를 새로 복사하지 않고 가리키는 int 참조
```

- `const`: 이 접근 경로를 통한 변경을 금지합니다.
- `&`: 원본 객체를 복사하지 않고 참조합니다.
- `&&`: 임시 값 또는 소유권 전달과 관련된 rvalue reference(오른값 참조)입니다.

템플릿 타입 추론이 어려워 보이는 이유는 컴파일러가 `T`만 고르는 것이 아니라
`const`, `&`, `&&`가 `T`에 포함되는지 함수 매개변수 쪽에 남는지도 결정하기 때문입니다.

## 2. 템플릿이 없을 때 생기는 반복

두 값을 비교해 더 큰 값을 반환하는 함수를 정수와 실수용으로 따로 작성하면 코드가
반복됩니다.

```cpp
int larger_int(int left, int right) {
    return left < right ? right : left;
}

double larger_double(double left, double right) {
    return left < right ? right : left;
}
```

연산 구조는 같은데 타입만 다릅니다. 함수 템플릿은 달라지는 타입을 빈칸으로 둡니다.

```cpp
template <typename T>
T larger(T left, T right) {
    return left < right ? right : left;
}
```

문법을 한 조각씩 읽으면 다음과 같습니다.

```text
template <typename T>
^^^^^^^^  ^^^^^^^^ ^
템플릿 시작  타입 자리  그 자리의 이름
```

- `template <...>`: 뒤의 선언이 템플릿이라는 표시입니다.
- `typename T`: `T`가 값을 담는 변수가 아니라 타입을 받을 이름이라는 선언입니다.
- 이 위치에서는 `template <class T>`도 같은 뜻으로 사용할 수 있습니다.
- `T left`: 추론된 실제 타입으로 함수 매개변수 `left`를 선언합니다.
- 첫 줄의 `T larger`: 추론된 타입을 반환 타입에도 사용합니다.

`T`는 실행 중에 값을 담는 변수가 아닙니다. 컴파일 중에 실제 타입으로 치환될
**타입 매개변수**입니다.

```cpp
larger(10, 20);      // T를 int로 추론합니다.
larger(1.5, 2.5);    // T를 double로 추론합니다.
```

컴파일러는 개념적으로 다음과 같은 별도 함수를 필요할 때 만듭니다.

```cpp
int larger<int>(int left, int right);
double larger<double>(double left, double right);
```

이를 template instantiation(템플릿 인스턴스화)이라고 합니다. Java나 C#의 모든
제네릭과 동일한 런타임 상자 하나를 공유한다고 생각하면 안 됩니다. C++ 컴파일러는
사용된 타입 조합별 코드를 만들 수 있으며, 최적화 과정에서 중복을 합치거나 함수를
인라인할 수도 있습니다.

## 3. 타입 추론은 “패턴 맞추기”

다음 함수의 매개변수 패턴은 `T`입니다.

```cpp
template <typename T>
void inspect(T value);
```

`inspect(42)`를 호출하면 인수 타입 `int`를 매개변수 패턴 `T`에 맞춥니다.

```text
매개변수 패턴: T
실제 인수 타입: int
결론: T = int
```

참조 매개변수라면 패턴에 `&`가 이미 들어 있습니다.

```cpp
template <typename T>
void inspect(T& value);
```

```text
매개변수 패턴: T&
실제 인수: int 변수
결론: T = int
최종 매개변수 타입: int&
```

핵심은 **먼저 함수 매개변수 선언 전체를 보고, 그 패턴에서 `T` 부분이 무엇이어야
실제 인수를 받을 수 있는지 맞춰 보는 것**입니다.

## 4. 가장 먼저 외울 세 가지 규칙

다음 세 규칙만 이해해도 대부분의 일반 함수 템플릿을 읽을 수 있습니다.

### 규칙 1: 값으로 받는 `T`

```cpp
template <typename T>
void by_value(T value);
```

값 매개변수는 함수 안에 새 객체를 만듭니다. 원본의 top-level `const`와 참조는
새 객체 타입에 포함되지 않습니다.

```cpp
int number{10};
const int limit{20};
int& alias{number};

by_value(number);  // T = int
by_value(limit);   // T = int: 원본의 top-level const가 제거됩니다.
by_value(alias);   // T = int: 참조 자체가 아니라 int 값을 복사합니다.
```

여기서 top-level `const`는 객체 자체에 붙은 가장 바깥쪽 `const`입니다.

```cpp
const int value{1};          // int 객체 자체가 const: top-level const
const int* pointer{&value};  // 포인터가 가리키는 int가 const: low-level const
```

`const int*`를 값으로 전달하면 포인터 자체는 복사되지만 가리키는 `int`의 `const`는
유지됩니다.

```cpp
template <typename T>
void read_pointer(T pointer);

const int value{1};
const int* pointer{&value};
read_pointer(pointer);  // T = const int*
```

포인터 복사본으로도 `const int`를 수정하면 안 되기 때문입니다.

### 규칙 2: 수정 가능한 참조로 받는 `T&`

```cpp
template <typename T>
void by_reference(T& value);
```

새로운 `T` 객체를 복사하지 않고 호출자의 원본을 참조합니다.

```cpp
int number{10};
const int limit{20};

by_reference(number);  // T = int, 최종 타입은 int&
by_reference(limit);   // T = const int, 최종 타입은 const int&
```

두 번째 호출에서는 원본을 수정하면 안 되므로 `const`가 `T` 안으로 들어갑니다.
즉, `T&`라고 적었다고 항상 수정 가능한 것은 아닙니다. 추론된 `T`가 `const int`라면
최종 타입은 `const int&`입니다.

리터럴 같은 임시 값은 일반 `T&`에 연결할 수 없습니다.

```cpp
by_reference(42);  // 오류: 수정 가능한 int&는 임시 값에 연결할 수 없습니다.
```

### 규칙 3: 읽기 전용 참조로 받는 `const T&`

```cpp
template <typename T>
void by_const_reference(const T& value);
```

복사 없이 읽을 때 가장 자주 보는 형태입니다.

```cpp
int number{10};
const int limit{20};

by_const_reference(number);  // T = int, 최종 타입은 const int&
by_const_reference(limit);   // T = int, 최종 타입은 const int&
by_const_reference(42);      // T = int, 임시 값도 안전하게 참조합니다.
```

`const`가 매개변수 패턴에 이미 있으므로 일반적인 경우 `T`는 `int`로 추론됩니다.
임시 값은 이 `const` 참조가 존재하는 동안 수명이 연장되어 함수 본문에서 안전하게
읽을 수 있습니다. 그러나 그 참조나 포인터를 함수 밖에 저장하면 다시 수명 문제가
생길 수 있습니다.

### 첫 번째 요약표

| 매개변수 | `int` 변수 | `const int` 변수 | `42` 임시 값 |
|---|---|---|---|
| `T` | `T=int` | `T=int` | `T=int` |
| `T&` | `T=int` | `T=const int` | 호출 불가 |
| `const T&` | `T=int` | `T=int` | `T=int` |

---

> **여기서 잠깐 멈춰도 됩니다.** 0~4절을 이해했다면 `T`, `T&`, `const T&`가
> 나오는 대부분의 기초 함수를 읽을 준비가 된 것입니다. 아래부터는 두 번째 학습
> 단계입니다.

## 5. 값, lvalue, rvalue를 이름 유무로 먼저 구분하기

정확한 값 범주(value category)는 더 세밀하지만, 처음에는 다음 질문으로 시작합니다.

> 이 식이 가리키는 객체를 다음 줄에서도 같은 이름으로 다시 찾을 수 있는가?

```cpp
int number{10};

number;          // lvalue: 이름으로 다시 찾을 수 있는 객체
42;              // prvalue: 계산으로 만들어진 임시 값
number + 1;      // prvalue: 계산 결과 임시 값
std::move(number); // xvalue: 아직 객체는 있지만 자원을 이동해도 된다고 표시한 식
```

- lvalue(왼값): 지속되는 객체의 위치를 나타내는 식
- prvalue(순수 오른값): 값을 계산하거나 임시 객체를 만드는 식
- xvalue(소멸 예정 값): 자원을 재사용해도 된다고 표현한 객체
- rvalue(오른값): prvalue와 xvalue를 묶어 부르는 범주

중요한 함정은 **이름이 있는 변수는 선언 타입이 `T&&`여도 그 이름을 식으로 사용하면
lvalue**라는 것입니다.

```cpp
int&& temporary_reference = 42;

// temporary_reference의 선언 타입은 int&&입니다.
// 하지만 temporary_reference라는 이름을 사용한 식은 lvalue입니다.
```

## 6. `auto`도 거의 같은 규칙을 사용합니다

`auto`는 변수 선언 위치에서 템플릿의 `T`와 비슷하게 추론됩니다.

### 값 `auto`

```cpp
int number{10};
const int limit{20};
const int& reference{limit};

auto a = number;     // int
auto b = limit;      // int: top-level const 제거
auto c = reference;  // int: 참조와 top-level const 제거
```

### 참조 `auto&`

```cpp
auto& a = number;  // int&
auto& b = limit;   // const int&
```

### 읽기 전용 참조 `const auto&`

```cpp
const auto& a = number;  // const int&
const auto& b = 42;      // const int&, 임시 값 수명 연장
```

### 중괄호 초기화의 특별 규칙

```cpp
auto one{1};        // int
auto many = {1, 2}; // std::initializer_list<int>
```

`auto`의 중괄호 규칙과 함수 템플릿 추론 규칙은 완전히 같지 않습니다.

```cpp
template <typename T>
void consume(T value);

consume({1, 2});  // 보통 T를 추론할 수 없어 오류입니다.
```

함수가 `std::initializer_list<T>`를 명시적으로 받는다면 원소 타입을 추론할 수 있습니다.

```cpp
template <typename T>
void consume_list(std::initializer_list<T> values);

consume_list({1, 2});  // T = int
```

## 7. `decltype`은 `auto`와 목적이 다릅니다

`auto`는 새 변수 타입을 편리하게 정합니다. `decltype`은 **식의 선언 타입 또는
값 범주까지 조사**할 때 사용합니다.

### 괄호 없는 이름

```cpp
const int limit{20};
const int& reference{limit};

decltype(limit) a = 1;      // const int
decltype(reference) b = a;  // const int&
```

괄호 없는 변수 이름에는 그 변수의 선언 타입을 그대로 반환하는 특별 규칙이 있습니다.

### 괄호로 감싼 식

```cpp
int number{10};

decltype((number)) a = number;            // int&: (number)는 lvalue 식
decltype(std::move(number)) b = 20;        // int&&: std::move 결과는 xvalue
decltype(number + 1) c = number + 1;       // int: 덧셈 결과는 prvalue
```

가장 유명한 함정은 괄호 하나의 차이입니다.

```cpp
decltype(number)   // int
decltype((number)) // int&
```

### 기억법

- `decltype(name)`: “이 이름을 어떻게 선언했지?”
- `decltype((expression))`: “이 식의 값 범주가 무엇이지?”

---

> **8절부터는 고급 참고편입니다.** 라이브러리 구현이나 generic wrapper를 작성할
> 때 필요한 내용입니다. 일반적인 업무 함수를 사용하는 단계라면 지금 이해하지
> 못해도 괜찮습니다.

## 8. `T&&`가 특별해지는 전달 참조

다음 조건을 모두 만족하는 `T&&`를 forwarding reference(전달 참조)라고 합니다.

1. 함수 템플릿의 추론 대상 `T`에 직접 `&&`가 붙어 있습니다.
2. `T`가 `const T`처럼 다른 형태로 고정되지 않았습니다.

```cpp
template <typename T>
void relay(T&& value);
```

전달 참조는 lvalue와 rvalue를 모두 받을 수 있습니다.

```cpp
int number{10};
const int limit{20};

relay(number);           // T = int&, 최종 타입은 int&
relay(limit);            // T = const int&, 최종 타입은 const int&
relay(42);               // T = int, 최종 타입은 int&&
relay(std::move(number)); // T = int, 최종 타입은 int&&
```

왜 `T`가 `int&`가 될 수 있을까요? 전달 참조에는 lvalue를 받으면 `T` 자체를 lvalue
reference로 추론하는 특별 규칙이 있습니다. 그다음 reference collapsing(참조 축약)
규칙이 적용됩니다.

### 참조 축약표

| 조합 | 결과 |
|---|---|
| `T& &` | `T&` |
| `T& &&` | `T&` |
| `T&& &` | `T&` |
| `T&& &&` | `T&&` |

**`&`가 하나라도 섞이면 최종 결과는 `&`이고, 둘 다 `&&`일 때만 `&&`**라고
기억하면 됩니다.

### `std::forward`가 필요한 이유

함수 안에서 `value`는 이름이 있으므로 항상 lvalue 식입니다.

```cpp
template <typename T>
void relay(T&& value) {
    target(value);  // 원래 rvalue였어도 여기서는 lvalue로 전달합니다.
}
```

호출자가 전달한 값 범주를 보존하려면 다음처럼 작성합니다.

```cpp
template <typename T>
void relay(T&& value) {
    target(std::forward<T>(value));
}
```

- 원래 lvalue였다면 `std::forward<T>`도 lvalue로 전달합니다.
- 원래 rvalue였다면 rvalue로 다시 전달합니다.
- 이 패턴을 perfect forwarding(완벽 전달)이라고 부릅니다.

`std::move`는 무조건 rvalue로 바꾸겠다는 표시이고, `std::forward`는 템플릿이
호출자의 원래 값 범주를 보존하는 조건부 변환입니다.

### 전달 참조가 아닌 `const T&&`

```cpp
template <typename T>
void inspect(const T&& value);
```

`const T&&`는 전달 참조가 아니라 일반 rvalue reference입니다. lvalue를 받는 특별
추론 규칙이 적용되지 않으며, 이동은 대개 원본을 수정해야 하므로 `const` rvalue는
실무에서 활용 범위가 좁습니다.

## 9. 두 인수가 서로 다른 타입이면 어떻게 될까?

하나의 `T`를 두 위치에서 추론하면 결과가 같아야 합니다.

```cpp
template <typename T>
T add(T left, T right);

add(1, 2);    // T = int
add(1, 2.0);  // 오류: 첫 인수는 int, 둘째 인수는 double을 요구합니다.
```

함수 인수의 암시적 변환을 먼저 수행해 적당한 `T`를 골라 주리라고 기대하면 안 됩니다.
해결 방법은 의도를 명확히 선택하는 것입니다.

### 서로 다른 타입을 허용

```cpp
template <typename Left, typename Right>
auto add(Left left, Right right) {
    return left + right;
}
```

```cpp
add(1, 2.0);  // Left = int, Right = double, 반환 타입은 double
```

### 호출자가 타입을 명시

```cpp
add<double>(1, 2.0);  // 첫 번째 1을 double 매개변수로 변환합니다.
```

타입을 명시하면 추론 편의는 줄지만 변환 의도가 코드에 드러납니다.

## 10. 반환 타입만 보고는 보통 `T`를 추론하지 못합니다

```cpp
template <typename T>
T make_default();

int value = make_default();  // 오류: 반환받을 변수 타입만으로 T를 추론하지 않습니다.
```

함수 템플릿 타입 추론은 기본적으로 함수 인수에서 이루어집니다.

```cpp
int value = make_default<int>();  // T를 명시하면 정상입니다.
```

이 규칙 덕분에 overload resolution(오버로드 결정)이 반환받는 주변 문맥에 지나치게
의존하지 않습니다.

## 11. 배열과 함수 이름은 값 전달에서 모양이 바뀔 수 있습니다

### 배열의 decay

```cpp
template <typename T>
void by_value(T value);

int numbers[3]{1, 2, 3};
by_value(numbers);  // T = int*: 배열이 첫 원소 포인터로 decay합니다.
```

배열 크기 3 정보가 사라집니다. 배열 자체와 크기를 보존하려면 참조 패턴을 사용합니다.

```cpp
template <typename T, std::size_t N>
void inspect_array(T (&array)[N]);

inspect_array(numbers);  // T = int, N = 3
```

여기서 `N`은 타입이 아니라 컴파일 시간 정수 값을 받는 non-type template
parameter(비타입 템플릿 매개변수)입니다.

### 함수 이름의 decay

함수 이름도 값 매개변수에 전달하면 함수 포인터로 변환될 수 있습니다. 정확한 함수
타입이나 overload를 보존해야 한다면 reference, 명시적 cast 또는 `std::invoke`
계열 도구가 필요할 수 있습니다.

## 12. 클래스 템플릿 타입 추론

C++17부터 많은 클래스 템플릿은 생성자 인수로 템플릿 인자를 추론할 수 있습니다.
이를 CTAD(Class Template Argument Deduction, 클래스 템플릿 인자 추론)라고 합니다.

```cpp
std::pair pair{1, 2.5};  // std::pair<int, double>
std::vector values{1, 2, 3};  // std::vector<int>
```

예전에는 다음처럼 템플릿 인자를 직접 적었습니다.

```cpp
std::pair<int, double> pair{1, 2.5};
std::vector<int> values{1, 2, 3};
```

### 문자열 리터럴 주의

```cpp
std::pair pair{1, "hello"};  // 두 번째 타입은 보통 const char*
```

문자열을 소유하고 싶다면 의도를 명시합니다.

```cpp
std::pair pair{1, std::string{"hello"}};  // std::pair<int, std::string>
```

추론되었다는 사실이 원하는 소유권까지 보장하지는 않습니다.

### 사용자 deduction guide

사용자 클래스는 생성자만으로 원하는 타입을 추론하기 어렵다면 deduction guide를
제공할 수 있습니다.

```cpp
template <typename T>
class Box {
public:
    Box(T value);
};

Box(const char*) -> Box<std::string>;
```

이제 문자열 리터럴로 `Box`를 만들 때 소유 `std::string`을 선택하도록 안내할 수
있습니다. deduction guide를 너무 많이 만들면 숨은 변환이 늘어나므로 타입과 소유권이
분명해지는 경우에만 사용합니다.

## 13. concept는 추론 후 잘못된 타입을 걸러 줍니다

이 프로젝트의 [`task_sorting.hpp`](./include/modern_cpp/domain/task_sorting.hpp)는
다음 형태를 사용합니다.

```cpp
template <typename Range>
concept SnapshotRandomAccessRange =
    std::ranges::random_access_range<Range> &&
    std::same_as<std::ranges::range_value_t<Range>, TaskSnapshot>;

template <SnapshotRandomAccessRange Range>
void sort_by_priority_then_title(Range& tasks);
```

호출 코드를 살펴봅니다.

```cpp
std::vector<TaskSnapshot> tasks;
sort_by_priority_then_title(tasks);
```

추론 과정은 다음 순서로 이해하면 됩니다.

```text
1. 매개변수 패턴은 Range&
2. 실제 인수는 std::vector<TaskSnapshot> 변수
3. Range = std::vector<TaskSnapshot>
4. 최종 매개변수 타입은 std::vector<TaskSnapshot>&
5. 그다음 concept 조건을 검사
6. vector는 random-access range이고 원소 값 타입도 TaskSnapshot이므로 통과
```

concept가 임의로 다른 타입을 찾아 주는 것은 아닙니다. 먼저 후보 타입을 추론한 뒤,
그 타입이 함수가 요구하는 기능을 제공하는지 검사하고 읽기 좋은 오류를 만드는
역할에 가깝습니다.

`std::list<TaskSnapshot>`은 원소 타입은 맞지만 random-access iterator를 제공하지
않으므로 이 정렬 함수의 concept를 통과하지 못합니다.

## 14. 추론 결과를 직접 확인하는 안전한 방법

### `static_assert`와 `std::same_as`

컴파일 시간에 예상 타입과 같은지 확인할 수 있습니다.

```cpp
int number{10};
const int limit{20};

auto a = limit;
auto& b = limit;

static_assert(std::same_as<decltype(a), int>);
static_assert(std::same_as<decltype(b), const int&>);
```

조건이 틀리면 실행하기 전에 컴파일 오류가 납니다. 학습 코드와 템플릿 단위 테스트에
가장 확실한 방법입니다.

### 일부러 불완전한 타입을 사용해 진단 보기

디버깅 중에만 다음처럼 선언하고 정의하지 않은 템플릿을 사용할 수 있습니다.

```cpp
template <typename>
struct show_type;

show_type<decltype(expression)> reveal;
```

컴파일러 오류에 `show_type<실제타입>`이 나타날 수 있습니다. 컴파일러별 진단 형식이
다르며 이 코드는 빌드 성공용 코드에 남기지 않습니다.

### `typeid(...).name()`의 한계

`typeid` 이름은 사람이 읽기 쉬운 형식이라는 보장이 없고 reference나 일부 `const`
정보가 기대와 다르게 보일 수 있습니다. 타입 추론 학습에는 `static_assert`와
`decltype` 조합이 더 정확합니다.

## 15. 복사와 메모리 비용을 함께 읽기

템플릿 추론은 컴파일 시간 규칙이지만, 추론된 최종 매개변수 타입은 런타임 비용을
바꿉니다.

### `T value`

```cpp
template <typename T>
void consume(T value);
```

- lvalue를 전달하면 보통 복사 생성이 필요합니다.
- rvalue를 전달하면 이동 생성 또는 복사 생략이 가능할 수 있습니다.
- `std::vector` 복사는 원소 배열을 위한 새 힙 메모리와 원소 복사가 필요할 수 있습니다.
- 작은 `int`, pointer, `string_view`는 값 복사가 더 단순할 수 있습니다.

### `const T& value`

```cpp
template <typename T>
void observe(const T& value);
```

- 원본 객체를 가리키는 reference만 전달하고 큰 객체의 깊은 복사를 피합니다.
- 함수가 값을 소유하지 않으므로 원본보다 오래 저장하면 안 됩니다.
- 작은 타입은 참조를 따라 메모리를 한 번 더 읽는 비용이 값 전달보다 불리할 수도
  있으므로 무조건 `const&`가 빠르다고 단정하지 않습니다.

### `T&& value`와 이동

전달 참조 자체는 소유권을 이동하지 않습니다. 실제로 `std::move` 또는 적절한
`std::forward` 결과를 이동 생성자에 전달할 때 자원이 이동합니다. 이동된
`std::string`이나 `std::vector`는 여전히 파괴할 수 있는 유효한 객체지만 내용은
일반적으로 지정되지 않은 상태이므로 값에 의존하면 안 됩니다.

## 16. 흔한 오류를 메시지와 연결하기

### “couldn’t deduce template parameter”

```cpp
template <typename T>
T make();

make();  // 인수에 T가 없어 추론할 정보가 없습니다.
```

해결: `make<int>()`처럼 명시하거나 추론할 함수 인수를 설계합니다.

### “deduced conflicting types”

```cpp
larger(1, 2.0);  // 같은 T 자리에 int와 double이 충돌합니다.
```

해결: 타입을 통일하거나 `Left`, `Right` 두 매개변수를 사용합니다.

### “cannot bind non-const lvalue reference to an rvalue”

```cpp
template <typename T>
void edit(T& value);

edit(42);
```

해결: 수정할 지속 객체를 전달하거나, 읽기만 한다면 `const T&`, 소유한다면 값
매개변수를 선택합니다.

### concept 또는 constraints 오류

추론 자체는 성공했지만 타입이 요구 기능을 제공하지 못한 경우입니다. 긴 오류의
마지막 줄만 보지 말고 `required expression`, `same_as`, `random_access_range`처럼
처음 실패한 요구 사항을 찾습니다.

## 17. 매개변수 선택표

| 의도 | 우선 고려할 형태 | 이유 |
|---|---|---|
| 작은 값을 내 함수가 복사해 사용 | `T value` | 소유와 수명이 단순함 |
| 큰 객체를 복사 없이 읽기 | `const T& value` | 비소유 읽기 계약 |
| 호출자의 객체를 수정 | `T& value` | 수정 사실이 호출부에 드러남 |
| null일 수도 있는 비소유 객체 | `T* value` | 선택적 관찰 |
| 함수가 독점 소유권을 받음 | `std::unique_ptr<T> value` | 소유권 이전을 타입으로 강제 |
| 실제 공유 소유권에 참여 | `std::shared_ptr<T> value` | 참조 계수 소유권 공유 |
| generic wrapper가 값 범주 보존 | `T&& value` + `forward<T>` | 완벽 전달 |
| 연속 배열을 복사 없이 관찰 | `std::span<const T>` | pointer와 길이를 함께 전달 |

템플릿이라는 이유만으로 `T&&`를 기본 선택하지 않습니다. 대부분의 업무 함수는 값,
`const T&`, `T&`만으로 의도를 더 쉽게 전달할 수 있습니다.

## 18. 단계별 연습

### 연습 1: `T`를 적어 보기

```cpp
template <typename T>
void inspect(T value);

const int number{10};
inspect(number);
```

질문:

1. `T`는 무엇인가요?
2. 함수 안 `value`를 수정할 수 있나요?
3. 원본 `number`가 같이 바뀌나요?

<details>
<summary>정답</summary>

`T`는 `int`입니다. 값 매개변수에서 top-level `const`가 제거되므로 함수 안의 복사본
`value`는 수정할 수 있습니다. 원본과 다른 객체이므로 원본 `number`는 바뀌지 않습니다.

</details>

### 연습 2: 참조와 `const`

```cpp
template <typename T>
void inspect(T& value);

const int number{10};
inspect(number);
```

<details>
<summary>정답</summary>

`T`는 `const int`, 최종 매개변수 타입은 `const int&`입니다. 복사는 없지만 이
참조를 통해 원본을 수정할 수 없습니다.

</details>

### 연습 3: 전달 참조

```cpp
template <typename T>
void relay(T&& value);

int number{10};
relay(number);
relay(20);
```

<details>
<summary>정답</summary>

첫 호출은 `T=int&`이고 참조 축약 후 매개변수는 `int&`입니다. 두 번째 호출은
`T=int`이고 매개변수는 `int&&`입니다.

</details>

### 연습 4: `decltype`

```cpp
int number{10};

using A = decltype(number);
using B = decltype((number));
using C = decltype(number + 1);
```

<details>
<summary>정답</summary>

`A=int`, `B=int&`, `C=int`입니다. 괄호 없는 이름은 선언 타입, 괄호로 감싼 lvalue
식은 lvalue reference, 덧셈 prvalue는 값 타입을 만듭니다.

</details>

## 19. 마지막 암기 카드

```text
T              : 값으로 받음. 참조와 바깥 const가 대체로 제거됨.
T&             : 원본을 참조. const 인수라면 T 안에 const가 들어감.
const T&       : 복사 없이 읽음. 일반적으로 T에는 바깥 const가 들어가지 않음.
T&&            : 추론되는 T에 직접 붙었다면 전달 참조일 수 있음.
auto           : 대체로 값 매개변수 T와 비슷함.
auto&          : 대체로 T&와 비슷함.
decltype(name) : 이름의 선언 타입.
decltype((식)) : 식의 값 범주까지 반영.
concept        : 추론된 타입이 요구 조건을 만족하는지 검사.
```

모든 규칙을 한 번에 외우기보다 다음 세 질문을 호출마다 적어 보는 것이 좋습니다.

1. 함수 매개변수의 전체 패턴이 `T`, `T&`, `const T&`, `T&&` 중 무엇인가?
2. 실제 인수는 수정 가능한 lvalue, const lvalue, rvalue 중 무엇인가?
3. 추론된 `T`를 패턴에 다시 넣었을 때 최종 매개변수 타입은 무엇인가?

이 세 단계를 종이에 적을 수 있으면 긴 오픈소스 템플릿 오류도 작은 패턴부터
분해해서 읽을 수 있습니다.
