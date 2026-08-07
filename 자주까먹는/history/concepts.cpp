#include <iostream>      // 표준 입출력 스트림을 사용하기 위한 헤더 (std::cout 등)
#include <type_traits>   // C++11 템플릿 메타프로그래밍(타입 특성 검사)을 위한 헤더
#include <concepts>      // C++20 Concept 기능(std::integral 등)을 사용하기 위한 헤더

// ==========================================
// [C++20 Concept 정의]
// ==========================================
// 'Numeric'이라는 이름의 컨셉(조건식)을 정의합니다. 
// T 타입이 정수형(integral)이거나 부동소수점형(floating_point)일 때만 true가 됩니다.
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// ==========================================
// 1. [C++11 방식] SFINAE (Substitution Failure Is Not An Error)
// ==========================================
// std::enable_if_t를 사용하여 조건이 참일 때만 이 함수 템플릿을 활성화합니다.
// 문법이 매우 기괴하고 복잡하여 에러 발생 시 수십 줄의 템플릿 에러를 뿜어냅니다.
template <typename T, 
          typename std::enable_if<std::is_integral<T>::value || 
                                  std::is_floating_point<T>::value, int>::type = 0>
T add_cpp11(T a, T b) {
    return a + b; // 두 값을 더해서 반환
}

// ==========================================
// 2. [C++17 방식] if constexpr
// ==========================================
// 일단 모든 타입을 다 받습니다(template <typename T>).
// 하지만 함수 내부에서 컴파일 타임(constexpr)에 분기를 태워, 조건에 안 맞으면 컴파일을 거부(static_assert)합니다.
template <typename T>
T add_cpp17(T a, T b) {
    // std::is_integral_v 등은 C++17부터 도입된 변수 템플릿으로, ::value를 생략하게 해줍니다.
    if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
        return a + b;
    } else {
        static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "Numeric type required!");
    }
}

// ==========================================
// 3. [C++20 방식 - Type 1] Requires Clause
// ==========================================
// 기존 템플릿 문법 뒤에 requires 절을 붙여 조건을 명시합니다.
// 복잡한 다중 조건을 걸 때 가독성이 좋습니다.
template <typename T>
    requires Numeric<T>  // T는 반드시 Numeric 조건을 만족해야 한다!
T add_cpp20_requires(T a, T b) {
    return a + b;
}

// ==========================================
// 4. [C++20 방식 - Type 2] Constrained Template Parameter (질문하신 문법!)
// ==========================================
// typename 키워드 자체를 Concept 이름으로 대체해버립니다.
// "이 템플릿은 아무 타입(typename)이나 받는게 아니라, Numeric 타입만 받는다"는 가장 직관적인 선언입니다.
template <Numeric T>
T add_cpp20_constrained(T a, T b) {
    return a + b;
}

// ==========================================
// 5. [C++20 방식 - Type 3] Abbreviated Function Template
// ==========================================
// 템플릿 선언부(template <...>) 자체를 없애고, 인자 타입에 'Concept auto'를 직접 적습니다.
// 람다(Lambda)나 간단한 일반 함수 작성 시 코드를 극도로 간결하게 만들어줍니다.
auto add_cpp20_auto(Numeric auto a, Numeric auto b) {
    return a + b;
}

int main() {
    // 각 시대별 문법이 동일한 기계어(어셈블리)로 컴파일되어 실행됩니다.
    std::cout << "C++11 SFINAE   : " << add_cpp11(10, 20) << '\n';
    std::cout << "C++17 constexpr: " << add_cpp17(10.5, 20.3) << '\n';
    std::cout << "C++20 requires : " << add_cpp20_requires(100, 200) << '\n';
    
    // 질문하신 제약된 템플릿 매개변수 문법의 실행
    std::cout << "C++20 template<Numeric T>: " << add_cpp20_constrained(3.14f, 2.71f) << '\n';
    
    std::cout << "C++20 auto     : " << add_cpp20_auto(1, 2) << '\n';

    // 만약 아래 코드를 주석 해제하면 어떻게 될까요?
    // std::string s1 = "Hello", s2 = "World";
    // add_cpp20_constrained(s1, s2);
    //
    // C++11에서는 수십 줄의 치명적인 "매칭되는 함수를 찾을 수 없음" 에러가 발생하지만,
    // C++20에서는 "std::string이 Numeric 제약을 만족하지 않음" 이라는 단 1~2줄의 깔끔한 에러를 보여줍니다.

    return 0;
}

// == execute: /modern-cpp/자주까먹는/build/history/concepts ==

// C++11 SFINAE   : 30
// C++17 constexpr: 30.8
// C++20 requires : 300
// C++20 template<Numeric T>: 5.85
// C++20 auto     : 3