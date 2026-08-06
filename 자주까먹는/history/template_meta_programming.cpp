#include <iostream>
#include <variant>
#include <string>

// C++17 기준 overloaded 관용구 (C++20부터는 2번째 줄 생략 가능)
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using MDyata = std::variant<int, double>;

int main() {
    MDyata data = 42;
    MDyata data2 = 3.14;

    // 1. 상태가 없는 빈 람다들 (EBCO 적용됨)
    auto empty_visitor = overloaded {
        [](int i) { std::cout << "Int: " << i << '\n'; },
        [](double d) { std::cout << "Double: " << d << '\n'; }
    };

    // 2. 외부 상태를 캡처하는 람다들
    int count = 10;       // 4바이트 정수
    double factor = 3.14; // 8바이트 실수

    auto capture_visitor = overloaded {
        // count를 '값'으로 캡처 (람다 내부에 int 멤버 생성)
        [count](int i) { std::cout << "Int: " << i + count << '\n'; },
        // factor를 '값'으로 캡처 (람다 내부에 double 멤버 생성)
        [factor](double d) { std::cout << "Double: " << d * factor << '\n'; }
    };

    // std::visit은 visitor 객체를 내부적으로 '값 복사(Pass by value)' 하여 사용합니다.
    std::visit(empty_visitor, data);
    std::visit(capture_visitor, data);
    std::visit(empty_visitor, data2);
    std::visit(capture_visitor, data2);

    // 하드웨어 레벨의 메모리 크기 확인
    std::cout << "empty_visitor size: " << sizeof(empty_visitor) << " bytes\n"; 
    std::cout << "capture_visitor size: " << sizeof(capture_visitor) << " bytes\n";

    // Int: 42
    // Int: 52
    // Double: 3.14
    // Double: 9.8596
    // empty_visitor size: 1 bytes
    // capture_visitor size: 16 bytes
    return 0;
}