#include <iostream>
#include <variant>
#include <string>
#include <vector>

// 1. 흑마법 구조체 정의 (여러 람다를 하나의 객체로 합침)
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// 2. 여러 타입을 가질 수 있는 variant 정의 (int, double, string 중 하나)
using MyData = std::variant<int, double, std::string>;

int main() {
    // 3. 다양한 타입의 데이터를 담은 벡터 생성
    std::vector<MyData> dataList = { 10, 3.14, "Hello C++ Optimizer!" };

    for (const auto& data : dataList) {
        // 4. std::visit과 overloaded 구조체를 결합하여 타입별로 분기 처리
        std::visit(overloaded {
            // int 타입일 때 실행될 람다
            [](int i) { std::cout << "Integer: " << i * 2 << '\n'; },
            
            // double 타입일 때 실행될 람다
            [](double d) { std::cout << "Double: " << d + 1.5 << '\n'; },
            
            // std::string 타입일 때 실행될 람다
            [](const std::string& s) { std::cout << "String: " << s << '\n'; }
        }, data); 
    }
    
    return 0;
}