#include <iostream>
#include <variant>

// [C++17] overloaded 관용구 (여러 람다를 하나로 병합)
// C++20부터는 추론 가이드가 자동 생성되므로 1줄만 작성하면 됩니다.
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// 분석을 위한 간단한 데이터 정의
using MyVariant = std::variant<int, double>;

// ---------------------------------------------------------
// [컴파일러가 내부적으로 생성하는 Dispatch Table의 개념적 구현]
// ---------------------------------------------------------
// std::visit은 내부적으로 아래와 같은 2차원(또는 1차원) 배열을 만듭니다.
// 템플릿과 constexpr을 사용해 프로그램 실행 전(Compile-time)에 완성됩니다.
//
// static constexpr void (*dispatch_table[])(const MyVariant&) = {
//     &invoke_int_lambda,     // index 0일 때의 함수 주소
//     &invoke_double_lambda   // index 1일 때의 함수 주소
// };
// 
// 런타임에는 그저 dispatch_table[variant.index()](variant) 로 호출합니다.
// ---------------------------------------------------------

int main() {
    MyVariant data = 3.14; // 내부에 index = 1, 값 = 3.14 가 저장됨

    // visitor 객체 생성 (캡처 없는 람다이므로 EBCO 적용, 크기 1바이트)
    auto visitor = overloaded {
        [](int i) { 
            std::cout << "[정수] " << i << '\n'; 
        },
        [](double d) { 
            std::cout << "[실수] " << d << '\n'; 
        }
    };

    // std::visit 실행
    // 1. data.index()를 확인 (현재 1)
    // 2. 내부적으로 생성된 Dispatch Table 배열의 1번 인덱스로 점프
    // 3. double을 받는 람다 실행
    std::visit(visitor, data);

    return 0;
}