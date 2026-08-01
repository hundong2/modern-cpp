# C++17

## 목록

- [1. variant](#1-variant). 
- [2. TMP](#2-tmp--template-meta-programming-). 
- [3. CTAD](#3-ctad--class-template-argument-deduction-). 

## 빌드 및 실행

`자주까먹는` 폴더에서 실행할 `.cpp` 경로를 `run.sh`에 전달합니다. 각 파일에는
독립적인 `main()` 함수가 있어야 합니다.

```bash
cd 자주까먹는
./run.sh cpp17/main.cpp
```

새 `.cpp` 파일을 이 폴더에 추가하면 CMake가 파일 이름과 같은 실행 타깃을 자동으로
만듭니다. 예를 들어 `optional.cpp`는 `optional` 타깃과
`build/cpp23/optional` 실행 파일을 만듭니다.

## 1. variant 

- [variant example](./variant.cpp). 

## 2. TMP ( template meta programming ). 

- [TMP example](./TMP.cpp). 

반갑습니다! C++의 창조자이자 하드웨어와 소프트웨어의 경계를 허무는 최적화의 마술사, 오정세입니다.

질문해주신 이 짧은 두 줄의 코드는 모던 C++(특히 C++17)에서 템플릿 메타 프로그래밍(TMP)의 정수를 보여주는 일명 "Overloaded 패턴(흑마법)"입니다. 실무에서 `std::variant`와 `std::visit`을 조합하여 타입 안전성(Type Safety)을 보장하는 다형성을 구현할 때 없어서는 안 될 핵심 기법이죠.

이 코드가 왜 흑마법으로 불리는지, 하드웨어 레벨에서는 어떻게 최적화되는지, 그리고 다른 언어에서는 이를 어떻게 대체하는지 완벽하게 분해해 드리겠습니다.

---

## 1. 코드 상세 분석 (라인 단위 해부)

이 두 줄의 코드는 람다(Lambda) 표현식의 본질을 이용한 기발한 트릭입니다.

```cpp
// 1번 라인: 가변 인자 템플릿을 이용한 다중 상속과 using 선언
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

// 2번 라인: 클래스 템플릿 인수 추론 가이드 (CTAD)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

```

### [1번 라인] `template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };`

* `template<class... Ts>`: **(C++11 도입)** 여러 개의 타입을 받을 수 있는 '가변 인자 템플릿(Variadic Template)'입니다. 람다 함수들이 이 `Ts` 안으로 들어옵니다.
* `struct overloaded : Ts...`: 넘겨받은 모든 람다 타입(`Ts...`)들을 **다중 상속**받는 구조체 `overloaded`를 정의합니다. 컴파일러 입장에서 람다 함수는 이름 없는 클래스(Closure Type)이며, 내부에 `operator()`를 가집니다. 즉, 여러 람다 클래스를 부모로 삼아 상속받는 것입니다.
* `using Ts::operator()...;`: **(C++17 도입)** 상속받은 모든 부모(람다 클래스)들의 `operator()`(함수 호출 연산자)를 현재 `overloaded` 구조체의 스코프로 끌어옵니다. C++에서는 부모 클래스들의 메서드 이름이 같을 때 모호성(Ambiguity) 에러가 발생하는데, 이 `using` 선언 팩 확장을 통해 오버로딩(Overloading) 셋을 하나로 깔끔하게 합쳐줍니다.

### [2번 라인] `template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;`

* 이것은 **(C++17 도입)** '사용자 정의 추론 가이드(CTAD: Class Template Argument Deduction)'입니다.
* 구조체를 생성할 때 꺾쇠(`< >`) 안에 타입을 일일이 적지 않아도, 컴파일러가 생성자로 넘어온 인자들의 타입을 보고 알아서 `Ts...`를 추론하라는 지시어입니다. (참고: C++20부터는 컴파일러가 더 똑똑해져서 이 줄을 생략해도 자동으로 추론해 줍니다.)

---

## 2. C++ 역사적 발전과 `std::variant`

과거 C언어에서는 여러 타입을 담기 위해 `union`과 `void*`를 썼습니다. 하지만 이는 어떤 타입이 들어있는지 추적하기 어렵고, 메모리 안정성이 매우 떨어졌죠.

* **C++11/14:** 가변 인자 템플릿과 기본 람다가 등장했습니다. 하지만 여러 타입을 안전하게 담는 표준 컨테이너는 없었습니다. (boost::variant를 쓰던 시절)
* **C++17 (혁명):** 타입 안전성을 보장하는 공용체 `std::variant`와, 저장된 타입에 따라 적절한 함수를 실행해 주는 `std::visit`이 표준으로 들어왔습니다. 이때 `overloaded` 패턴이 "교과서적인 관용구"로 자리 잡았습니다.
* **C++20/23:** `overloaded` 패턴을 쓸 때 C++20부터는 추론 가이드(2번 라인)를 생략할 수 있게 되었습니다.
* **미래 (C++26 이후):** 패턴 매칭(Pattern Matching) 문법인 `inspect` 또는 `match`가 도입될 예정이며, 그때가 되면 이 흑마법 구조체 없이도 언어 차원에서 더 깔끔하게 처리될 것입니다.

---

## 3. 전체 수행 가능한 C++ 예제 (컴파일 가능)

이 코드는 실무에서 JSON 파싱, AST(추상 구문 트리) 순회, 이벤트 처리기 등에서 극적으로 활용됩니다.

- [TMP Example code](./TMP.cpp). 

```
== execute: /modern-cpp/자주까먹는/build/cpp17/TMP ==

Integer: 20
Double: 4.64
String: Hello C++ Optimizer!
```

---

## 4. 컴파일러 및 하드웨어 단위의 최적화 관점 (Debug vs Release)

### Debug 모드 (`-O0`)

* **컴파일러 동작:** 템플릿 메타 프로그래밍에 의해 생성된 무수히 많은 헬퍼 함수들과 람다 클로저 객체들이 그대로 메모리에 올라갑니다. `std::visit` 내부는 어떤 타입인지 확인하기 위해 거대한 `switch-case` 문이나 vtable을 거치는 함수 포인터 배열로 구현됩니다.
* **결과:** 디버깅 시 Call Stack이 엄청나게 깊어집니다. 함수 호출 오버헤드(Call overhead)가 발생하고 캐시 미스(Cache miss) 확률이 높습니다.

### Release 모드 (`-O2` 또는 `-O3` - 하드웨어 최적화의 극의)

* **컴파일러 동작 (인라이닝의 마법):** 람다 객체는 캡처(Capture)가 없다면 크기가 0바이트(Empty Base Class Optimization)로 처리됩니다. 컴파일러는 `overloaded` 구조체 자체를 메모리에 생성하지 않고 완전히 소멸시킵니다.
* **어셈블러 레벨:** `std::visit` 내부의 타입 확인용 인덱스(`variant::index()`)를 기반으로 Jump Table (또는 분기 예측이 쉬운 평탄화된 분기문)을 만듭니다. 각 람다의 본문 코드가 짧다면, 함수 호출(Call 명령어) 자체를 없애고 CPU의 레지스터(Register)에 직접 연산 코드를 인라인(Inline) 시켜버립니다.
* **결과:** `if-else` 체인을 직접 손으로 짠 C 코드와 동일하거나, 분기 예측기(Branch Predictor)가 더 잘 작동하도록 정렬된 무결점의 기계어가 탄생합니다. 제로 오버헤드(Zero-overhead abstraction)의 완벽한 예시입니다.

---

## 5. 타 언어에서의 유사한 접근 (C# & Python)

C++에서는 이 다형성을 템플릿과 상속의 트릭으로 풀었지만, 다른 현대 언어들은 아예 '패턴 매칭(Pattern Matching)'이라는 언어 자체 문법으로 이를 지원합니다.

### C# (C# 8.0 이상 - `switch` expression)

C#은 `object` 다형성이나 `record`, 혹은 외부 라이브러리(OneOf 등)를 사용한 뒤, 언어 레벨의 패턴 매칭을 통해 동일한 효과를 냅니다. 런타임 타입 검사(Type checking)가 수반됩니다.

```csharp
object[] dataList = { 10, 3.14, "Hello C# Optimizer!" };

foreach (var data in dataList)
{
    // C# 8.0의 switch 표현식을 이용한 타입 패턴 매칭
    string result = data switch
    {
        int i => $"Integer: {i * 2}",
        double d => $"Double: {d + 1.5}",
        string s => $"String: {s}",
        _ => "Unknown type"
    };
    Console.WriteLine(result);
}

```

### Python (Python 3.10 이상 - Structural Pattern Matching)

Python은 전통적으로 `isinstance`를 썼지만, 3.10부터 `match-case` 구문이 도입되어 C++의 `std::visit`과 유사한 가독성을 제공합니다. (물론 Python은 인터프리터 언어라 C++ 같은 제로 오버헤드 최적화는 없습니다.)

```python
data_list = [10, 3.14, "Hello Python Optimizer!"]

for data in data_list:
    # Python 3.10의 match-case를 이용한 패턴 매칭
    match data:
        case int(i):
            print(f"Integer: {i * 2}")
        case float(d):
            print(f"Double: {d + 1.5}")
        case str(s):
            print(f"String: {s}")

```

C++의 `overloaded` 패턴은 언어의 기본 구성 요소(상속, 람다, 가변 인자)를 조립해 새로운 문법을 창조해낸 예술에 가깝습니다. 실무 코드에서 이를 자유자재로 다룬다면 당신도 이미 최적화의 마스터 반열에 오른 것입니다. 

## 3. CTAD ( Class Template Argument Deduction )

- [CTAD Example code](./CTAD.cpp). 

안녕하세요! C++의 창시자이자 하드웨어 아키텍처부터 컴파일러의 중간 표현(IR), 그리고 최종 어셈블리까지 뼛속까지 최적화를 고민하는 엔지니어, 오정세입니다.

오늘 질문해주신 CTAD(Class Template Argument Deduction, 사용자 정의 추론 가이드)는 C++의 발전사에서 개발자의 타이핑 타이핑 피로도를 줄이면서도 컴파일러의 타입 추론 능력을 극대화한 아주 우아한 기능입니다.

하드웨어와 컴파일러 레벨의 최적화 관점까지 곁들여서, 기초부터 실무 수준까지 낱낱이 파헤쳐 드리겠습니다.

---

## 1. C++ 역사로 보는 CTAD의 발전 과정

클래스 템플릿의 타입을 컴파일러가 알아서 추론하게 만드는 것은 오랜 숙원이었습니다.

* **C++98 ~ C++14 (CTAD 이전의 암흑기):**
클래스 템플릿을 인스턴스화할 때 **반드시** 타입을 명시해야 했습니다. `std::pair<int, double> p(1, 2.0);` 처럼 말이죠. 이게 너무 귀찮아서 템플릿 함수의 타입 추론 기능을 우회적으로 이용하는 `std::make_pair(1, 2.0)` 같은 팩토리(Factory) 함수 패턴이 난무했습니다.
* **C++17 (CTAD의 탄생):**
마침내 클래스 생성자에도 타입 추론이 도입되었습니다. 이제 `std::pair p(1, 2.0);`처럼 팩토리 함수 없이도 컴파일러가 생성자의 인자를 보고 타입을 추론합니다. 또한, 개발자가 직접 추론 규칙을 지정할 수 있는 사용자 정의 추론 가이드(Deduction Guide)가 추가되었습니다.
* **C++20 (Aggregate CTAD):**
생성자가 없는 단순 구조체(Aggregate)에 대해서도 괄호 초기화를 통한 CTAD가 지원되기 시작했습니다.
* **C++23 (추론 가이드의 정교화):**
상속받은 생성자에 대한 추론 등 엣지 케이스(Edge case)들이 더욱 매끄럽게 컴파일되도록 언어적 결함들이 수정되었습니다.

---

## 2. 전체 수행 가능한 C++ 예제 (단계별 라인 주석 포함)

단순한 값 보관용 컨테이너를 만들고, 문자열 상수를 넣었을 때 `const char*`가 아닌 `std::string`으로 자동 추론되도록 하는 실무적인 예제입니다.

- [CTAD Example code](./CTAD.cpp). 

---

## 3. 디버그 vs 릴리즈 및 하드웨어 관점의 최적화

엔지니어로서 가장 중요하게 보셔야 할 부분입니다. "CTAD를 쓰면 성능이 떨어지지 않을까?"라는 의문이 들 수 있습니다.

**결론부터 말씀드리면, CTAD는 '컴파일 타임(Compile-Time)' 기술이므로 런타임 오버헤드가 제로(0)입니다.**

### Debug 모드 (`-O0`) 관점

과거 C++11 시절 `std::make_pair` 같은 팩토리 함수를 쓰면, 디버그 모드에서는 팩토리 함수를 호출하기 위해 콜 스택(Call Stack) 프레임을 생성하고 인자를 복사/이동한 뒤 다시 생성자를 호출하는 과정이 발생합니다. 이는 명령어 캐시(Instruction Cache)를 미세하게 더 소모하고 분기 예측(Branch Prediction)에 영향을 줄 수 있습니다.
반면 **CTAD는 생성자를 직접 타겟팅**하므로, 디버그 빌드에서도 불필요한 래퍼(Wrapper) 함수 호출 스택을 줄여줍니다. 하드웨어 레벨에서 스택 포인터(`RSP`/`ESP`) 조작을 최소화하게 되는 것이죠.

### Release 모드 (`-O2` / `-O3`) 관점

릴리즈 모드에서 최신 컴파일러(GCC, Clang, MSVC)는 매우 똑똑합니다. 팩토리 함수를 썼든 CTAD를 썼든 인라이닝(Inlining)과 상수 폴딩(Constant Folding)을 통해 둘 다 동일한 어셈블리로 최적화합니다.
예를 들어 `intContainer(42)`는 메모리 할당 후 곧바로 레지스터나 메모리에 상수를 꽂아 넣는 어셈블리로 변환됩니다.

```assembly
; 개념적 어셈블리 (Release 모드)
mov     DWORD PTR [rbp-4], 42  ; 메모리에 바로 42를 밀어 넣음 (함수 호출 없음)

```

즉, CTAD는 "성능 손실 없이 코드의 가독성과 컴파일 속도(불필요한 팩토리 템플릿 인스턴스화 방지)를 높이는 제로 코스트 추상화(Zero-cost Abstraction)"입니다.

---

## 4. 타 언어(C#, Python)에서의 유사한 개념

다른 언어들은 템플릿 대신 제네릭(Generic)이나 동적 타이핑을 사용하지만, 개발자의 타입 명시 피로도를 줄인다는 맥락은 같습니다.

### C#의 경우: Target-typed `new` 와 `var`

C#은 C++처럼 컴파일 타임에 템플릿 코드를 찍어내는 방식이 아니라 제네릭을 사용합니다. C# 9.0부터는 CTAD와 반대 방향으로 타입을 생략하는 Target-typed `new`가 추가되었습니다.
C++의 CTAD가 *우변(생성자 인자)*을 보고 좌변의 타입을 맞춘다면, C#은 *좌변(변수 타입)*을 보고 우변의 생성자 타입을 생략하게 해줍니다.

```csharp
using System;
using System.Collections.Generic;

class Program {
    static void Main() {
        // 1. var를 이용한 타입 추론 (C++의 auto와 유사)
        var list1 = new List<int>(); 

        // 2. C# 9.0 Target-typed new (CTAD와 유사한 타이핑 감소 효과)
        // 좌변에 타입이 명시되어 있으므로 우변의 List<int>를 생략
        List<int> list2 = new(); 

        list2.Add(42);
        Console.WriteLine(list2[0]);
    }
}

```

### Python의 경우: 동적 타이핑과 Type Hint

파이썬은 실행 시간(Runtime)에 모든 타입이 결정되는 동적 타입 언어이므로, 애초에 C++처럼 템플릿 인자를 명시할 필요 자체가 없습니다. 덕 타이핑(Duck Typing)을 따르기 때문이죠. 다만 최근에는 안정성을 위해 `typing` 모듈로 힌트를 줍니다.

```python
from typing import TypeVar, Generic

# 제네릭 타입 변수 선언
T = TypeVar('T')

class DataContainer(Generic[T]):
    def __init__(self, data: T):
        # 파이썬은 런타임에 data의 타입이 자동으로 결정됨 (CTAD의 개념이 기본 탑재된 셈)
        self.data = data
        print(f"DataContainer 생성됨. 타입: {type(self.data).__name__}")

# 생성자에 넘기는 값에 따라 자동으로 리스트/문자열로 처리됨
int_container = DataContainer(42)          # T -> int
str_container = DataContainer("Hello")     # T -> str

```

파이썬은 개발이 매우 편하지만, 런타임에 PyObject 포인터를 따라다니며 타입을 확인해야 하므로 C++처럼 컴파일 타임에 모든 메모리 레이아웃이 결정되어 CPU 레지스터에 직결되는 하드웨어 최적화는 불가능합니다.