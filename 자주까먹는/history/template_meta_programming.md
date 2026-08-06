# template meta programming 

- [01. lambda](#01-lambda-내부-구조)  
- [02. naming hiding](#02-naming-hiding-using)  
- [03. variant visit example code](#03-variant_visit). 
- [04. inline]()

## 01. lambda 내부 구조 

- [lambda example](./template_meta_programming.cpp). 

```cpp
auto empty_visitor = overloaded {
    [](int i) { std::cout << "Int: " << i << '\n'; },
    [](double d) { std::cout << "Double: " << d << '\n'; }
};
```

```cpp
// 1. 첫 번째 람다를 컴파일러가 임의의 이름으로 구조체로 만듦 (이것이 첫 번째 부모!)
struct __CompilerGenerated_Lambda1 {
    inline void operator()(int i) const { 
        std::cout << "Int: " << i << '\n'; 
    }
};

// 2. 두 번째 람다 역시 임의의 이름의 구조체로 만듦 (이것이 두 번째 부모!)
struct __CompilerGenerated_Lambda2 {
    inline void operator()(double d) const { 
        std::cout << "Double: " << d << '\n'; 
    }
};

// 3. overloaded 객체는 이 두 개의 구조체를 "다중 상속" 받는 자식 클래스입니다!
struct overloaded : __CompilerGenerated_Lambda1, __CompilerGenerated_Lambda2 {
    // 부모들의 함수(operator())를 자식의 이름으로 쓸 수 있게 끌어옴
    using __CompilerGenerated_Lambda1::operator();
    using __CompilerGenerated_Lambda2::operator();
};

// 4. 최종적으로 생성되는 객체 (empty_visitor)
overloaded empty_visitor = overloaded{ __CompilerGenerated_Lambda1{}, __CompilerGenerated_Lambda2{} };
```

- `using`을 씀으로써 [naming hiding](#02-naming-hiding-using)을 통해 쉽게 부모 클래스의 이름으로 호출 가능 

## 02. Naming hiding (using)

- [naming hiding code](./naming_hiding.cpp). 

- using 이 없을 때 대참사 

```cpp
struct Parent_Int {
    void operator()(int i) { /* 정수 처리 */ }
};
struct Parent_Double {
    void operator()(double d) { /* 실수 처리 */ }
};
// 다중 상속을 받음
struct Child : Parent_Int, Parent_Double {
    // using 키워드가 없다고 가정해 봅시다.
};
Child my_child;
my_child(42); // 💥 컴파일 에러 발생! "ambiguous(모호함)"
```

- 컴파일러 : my_child가 42(int)를 넣고 operator()를 호출 했는데 Parent_Inteh operator()가 있고, Parent_Double도 operator()가 있다. -> 누굴 불러야지?
- 이때 프로그래머들이 명시적으로 my_child.Parent_Int::operator()(42); 라고 부모 이름을 적어야만 에러가 풀림. 

### using 사용 (오버로딩 셋 병합)

```cpp
struct Child : Parent_Int, Parent_Double {
    // "부모들의 operator()를 내(Child) 방으로 끌고 와서 하나의 세트로 묶어라!"
    using Parent_Int::operator();
    using Parent_Double::operator();
};

Child my_child;
my_child(42);   // ✅ 성공! 컴파일러가 자동으로 Parent_Int의 함수를 선택함.
my_child(3.14); // ✅ 성공! 컴파일러가 자동으로 Parent_Double의 함수를 선택함.
```

- 이것이 "자식의 이름으로 끌어온다"는 의미입니다. using을 선언하면 컴파일러는 여러 부모의 operator()들을 모아서 자식 클래스 안에 하나의 완벽한 오버로딩(Overloading) 목록을 만듭니다. 이제 사용자는 부모의 이름(__CompilerGenerated_...)을 전혀 몰라도, 그저 자식 객체(my_child)에 원하는 값을 던지기만 하면 컴파일러가 타입(int냐 double이냐)을 보고 알아서 알맞은 부모의 함수로 연결해 줍니다. 
- `오버로딩`이 핵심!!!

### 🦀 Rust (Enum과 Match - C++의 최종 진화형)

- Rust는 C++의 std::variant와 std::visit (+ overloaded 흑마법) 구조를 언어 차원으로 끌어올려 대체 불가능한 우위를 점했습니다. Rust는 '상속' 자체를 폐기하고, 타입 안전성이 완벽한 enum (Tagged Union)과 match를 사용합니다.

- [rust example](../rust/enum.rs). 

## 03. variant_visit 

- [variant visit example](./variant_visit.cpp). 

## 04. inline 

안녕하세요! C++의 창시자이자 하드웨어 바닥부터 컴파일러 최적화까지, 극한의 퍼포먼스를 설계하는 **오정세**입니다.

오늘 질문해주신 **`inline` 함수**는 초창기 C++부터 현대의 C++23에 이르기까지 그 의미와 역할이 크게 진화한 아주 매력적인 주제입니다. 하드웨어의 파이프라인부터 어셈블리, 그리고 타 언어와의 비교까지 당신이 실무에서 완벽히 통제할 수 있도록 깊고 직관적으로 설명해 드리겠습니다.

---

## 1. 컴파일러와 하드웨어 관점에서의 `inline`

함수를 호출(Call)한다는 것은 하드웨어 입장에서는 꽤 무거운 작업입니다.

1. **레지스터 백업:** 현재 실행 중인 상태를 스택(Stack) 메모리에 저장합니다.
2. **명령어 포인터(EIP/RIP) 점프:** 함수의 메모리 주소로 실행 흐름을 물리적으로 건너뜁니다(`call` 어셈블리 명령어).
3. **스택 프레임 생성:** 지역 변수를 위한 공간을 만듭니다.
4. **복귀(Return):** 함수가 끝나면 반환값을 레지스터에 넣고 원래 주소로 돌아옵니다(`ret` 명령어).

**`inline`의 탄생 목적**은 이 오버헤드를 없애는 것이었습니다. 컴파일러에게 "이 함수는 굳이 점프하지 말고, 함수를 호출한 그 자리에 코드 자체를 통째로 복사해 넣어라(Substitution)"라고 지시하는 힌트(Hint)입니다.

### 디버그(Debug) vs 릴리스(Release) 관점의 최적화

* **Debug 모드 (`-O0`)**: 컴파일러는 `inline` 키워드를 **무시**합니다. 디버깅을 위해 브레이크포인트를 걸고 스택 트레이스를 추적해야 하므로, 원본 함수 구조를 그대로 유지합니다.
* **Release 모드 (`-O2, -O3`)**: 현대의 컴파일러(GCC, Clang, MSVC)는 매우 똑똑합니다. `inline` 키워드가 없어도 자체적인 비용 분석(Cost Analysis)을 통해 득이 된다면 알아서 인라인화를 수행합니다. 반대로 `inline`을 적었어도 함수가 너무 크면(명령어 캐시를 낭비한다고 판단하면) 인라인화를 거부합니다.

---

## 2. C++ 역사에 따른 `inline`의 진화 (C++11 ~ C++23)

* **C++11 이전**: 순수하게 "최적화를 위한 힌트" 및 "헤더 파일에 함수 구현을 넣기 위한 용도"였습니다.
* **C++17 (혁명기)**: `inline` 변수(Inline Variables)가 도입되었습니다. 이전에는 클래스의 `static` 멤버 변수를 헤더에 선언하면, 반드시 `.cpp` 파일에서 초기화를 따로 해줘야 하는 골칫거리가 있었습니다(ODR 위반 문제). C++17부터는 `inline static int count = 0;` 처럼 헤더에서 바로 초기화가 가능해져, 헤더 온리(Header-only) 라이브러리 제작이 압도적으로 쉬워졌습니다.
* **C++20 / C++23**: **모듈(Modules)** 시스템의 도입으로 헤더 파일의 의존성이 줄어들고 있습니다. 모듈 내에서 선언된 함수는 기본적으로 외부로 유출될 때 ODR 문제가 발생하지 않도록 깔끔하게 관리됩니다. 즉, 이제 `inline`은 최적화 힌트보다는 "여러 번호(번역 단위)에 정의되어도 링커가 하나로 합쳐라(ODR-Linkage)"라는 의미가 훨씬 강해졌습니다.

---

## 3. C++ 전체 실행 가능한 예제 및 단계별 분석

현대적인 C++17 이상의 기능을 활용하여 `inline` 함수와 변수의 역할을 보여주는 완전한 예제입니다.

- [inline example code](./inline.cpp). 

---

## 4. 타 언어에서의 인라인 최적화 접근법

제가 C++을 만들었지만, 다른 훌륭한 언어들도 이 하드웨어적 병목을 각자의 방식으로 해결하고 있습니다.

### C# (JIT 컴파일러 기반)

C#은 코드가 실행되는 런타임에 JIT(Just-In-Time) 컴파일러가 기계어로 번역합니다. JIT 컴파일러에게 인라인을 강제하고 싶을 때 특성을 부여합니다.

```csharp
using System;
using System.Runtime.CompilerServices;

class Program {
    // MethodImplOptions.AggressiveInlining을 통해 JIT에게 강력한 인라인 힌트를 줍니다.
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    static int Add(int a, int b) {
        return a + b;
    }

    static void Main() {
        int result = Add(10, 20);
        Console.WriteLine(result);
    }
}

```

### Python (인터프리터 및 동적 타입)

Python은 동적 타입 인터프리터 언어이므로 C++같은 물리적인 인라인 개념이 존재하지 않습니다. 함수 호출 오버헤드가 C++에 비해 수십 배 큽니다.
이를 최적화하기 위해서는 **PyPy** 같은 JIT 기반 인터프리터를 사용하거나, 병목 구간만 **Cython**을 사용하여 C/C++로 컴파일(인라인 포함)하여 바인딩하는 방식을 주로 사용합니다.

### Rust (제로 비용 추상화의 계승자)

Rust는 C++의 사상을 많이 물려받아 LLVM 기반의 매우 강력한 인라인 최적화를 수행합니다. 특히 크로스 크레이트(Cross-Crate, 다른 라이브러리 간) 최적화를 위해 LTO(Link Time Optimization)와 더불어 직관적인 속성을 제공합니다.

```rust
// #[inline] : C++의 inline과 유사하게 크로스 크레이트 인라인 힌트를 줍니다.
// #[inline(always)] : C++의 __forceinline 처럼 컴파일러에게 무조건 인라인화를 강제합니다.
#[inline(always)]
fn add(a: i32, b: i32) -> i32 {
    a + b
}

fn main() {
    let result = add(10, 20); // 릴리스 타겟에서 컴파일 타임에 30으로 치환됨
    println!("{}", result);
}

```