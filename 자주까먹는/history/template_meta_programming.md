# template meta programming 

- [01. lambda](#01-lambda-내부-구조)  
- [02. naming hiding](#02-naming-hiding-using)  
- [03. variant visit example code](#03-variant_visit). 

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