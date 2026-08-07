#include <iostream>
#include <variant>
#include <string>

// ==========================================================================
// [C++17/20] 현재의 흑마법: overloaded 관용구
// 여러 개의 람다 클로저를 다중 상속받아 하나의 오버로딩 셋(Overload Set)으로 만듭니다.
// ==========================================================================
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// C++20부터는 아래의 추론 가이드를 컴파일러가 자동 생성하므로 생략 가능합니다.
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// 테스트용 데이터 타입 (0: int, 1: double, 2: std::string)
using MyData = std::variant<int, double, std::string>;

// ==========================================================================
// [C++11 방식] 태그를 수동으로 검사하거나 boost::variant를 쓰던 고통의 시절
// ==========================================================================
void Process_CPP11(const MyData& data) {
    // 런타임에 index를 검사하고 일일이 강제 캐스팅(std::get)을 해야 했습니다.
    // 캐시 분기 예측(Branch Prediction)에 매우 불리하며, 개발자 실수가 잦았습니다.
    if (data.index() == 0) {
        std::cout << "[C++11] 정수: " << std::get<0>(data) << '\n';
    } else if (data.index() == 1) {
        std::cout << "[C++11] 실수: " << std::get<1>(data) << '\n';
    } else {
        std::cout << "[C++11] 문자열: " << std::get<2>(data) << '\n';
    }
}

// ==========================================================================
// [C++17 ~ 23 방식] std::visit과 overloaded 흑마법의 완성
// ==========================================================================
void Process_CPP17_23(const MyData& data) {
    // 컴파일러가 내부적으로 O(1) 함수 포인터 배열(Dispatch Table)을 템플릿으로 생성합니다.
    std::visit(overloaded {
        [](int i) { std::cout << "[C++17] 정수: " << i << '\n'; },
        [](double d) { std::cout << "[C++17] 실수: " << d << '\n'; },
        [](const std::string& s) { std::cout << "[C++17] 문자열: " << s << '\n'; }
    }, data);
}

// ==========================================================================
// [C++26 미래] 언어 차원의 패턴 매칭 (Pattern Matching - P2688 제안 기반)
// 템플릿 흑마법 소멸. 언어 고유 문법으로 처리합니다.
// ==========================================================================
void Process_CPP26_Future(const MyData& data) {
#if defined(__cpp_pattern_matching) // C++26 컴파일러에서 활성화될 매크로
    // C++26의 match 구문 (WG21 제안 문서 기반 개념적 문법)
    match (data) {
        // 타입을 검사하고 즉시 바인딩(i, d, s)합니다.
        case int i => std::cout << "[C++26] 정수: " << i << '\n';
        case double d => std::cout << "[C++26] 실수: " << d << '\n';
        case std::string s => std::cout << "[C++26] 문자열: " << s << '\n';
    };
#else
    std::cout << "[C++26] 아직 컴파일러가 패턴 매칭을 지원하지 않습니다.\n";
#endif
}

int main() {
    MyData var1 = 42;
    MyData var2 = 3.14;

    std::cout << "--- C++ 다형성 최적화의 역사 ---\n";
    Process_CPP11(var1);
    Process_CPP17_23(var2);
    Process_CPP26_Future(var1);

    return 0;
}