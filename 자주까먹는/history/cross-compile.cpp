#include <iostream>
#include <variant>    // C++17: 타입 안전 공용체
#include <vector>
#include <string>
#include <concepts>   // C++20: 템플릿 제약 조건
#include <ranges>     // C++20: 파이프라인 형태의 지연 평가
// #include <print>   // C++23: 더 빠르고 안전한 포맷팅 출력 (일부 컴파일러 지원)

// [C++20] Concepts: 템플릿 타입 T가 반드시 숫자(정수 또는 실수)여야 함을 컴파일 타임에 강제합니다.
// 예전의 SFINAE(std::enable_if)보다 컴파일 속도가 훨씬 빠르고 에러 메시지가 직관적입니다.
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// [C++17] std::variant: C 스타일의 union을 대체하는 타입 안전(Type-safe) 공용체입니다.
// 메모리는 내부 타입 중 가장 큰 크기 + 현재 타입을 기억하는 태그(index)만큼만 차지합니다.
using Element = std::variant<int, double, std::string>;

// [C++14] auto 반환 타입: 컴파일러가 반환 타입을 추론합니다.
auto processElements(const std::vector<Element>& elements) {
    std::vector<std::string> results;
    
    // [C++11] Range-based for loop: 포인터 연산 없이 안전하게 순회합니다.
    for (const auto& el : elements) {
        
        // [C++17] std::visit: variant 내부에 현재 들어있는 실제 타입에 따라 적절한 코드를 실행합니다.
        // 컴파일러 내부적으로는 vtable(가상 함수 테이블)이나 점프 테이블(Jump Table)로 최적화됩니다.
        std::visit([&results](auto&& arg) {
            
            // [C++11] decltype 및 type traits를 사용해 현재 타입을 추론합니다.
            using T = std::decay_t<decltype(arg)>;
            
            // [C++17] if constexpr: 컴파일 타임 분기문입니다.
            // 조건에 맞지 않는 블록은 컴파일러가 아예 어셈블리 코드로 생성하지 않으므로 런타임 오버헤드가 '0'입니다.
            if constexpr (Numeric<T>) { // C++20 Concept 적용
                results.push_back("Number: " + std::to_string(arg));
            } else if constexpr (std::is_same_v<T, std::string>) {
                results.push_back("String: " + arg);
            }
        }, el);
    }
    return results;
}

int main() {
    // [C++11] Uniform Initialization: 중괄호를 이용해 직관적으로 초기화합니다.
    std::vector<Element> data = {10, 3.14, "Hello C++ 23"};

    auto processed = processElements(data);

    // [C++20] Ranges & Views: 불필요한 메모리 복사 없이, 파이프라인(|)을 통해 데이터를 필터링합니다.
    // 지연 평가(Lazy Evaluation)를 사용하므로 실제 순회할 때만 연산이 수행됩니다.
    auto string_only = processed 
                     | std::views::filter([](const std::string& s) {
                         // [C++20] starts_with: 직관적인 문자열 검색
                         return s.starts_with("String"); 
                     });

    // [C++23] std::println의 개념적 도입 (여기서는 호환성을 위해 cout 사용)
    // C++23의 std::print는 <iostream>보다 파싱 오버헤드가 적어 컴파일 타임/런타임 모두 최적화됩니다.
    for (const auto& res : string_only) {
        std::cout << res << '\n'; 
    }

    return 0;
}