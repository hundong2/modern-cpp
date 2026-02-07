// C++20 Ranges 예제
#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <string>

namespace rg = std::ranges;
namespace vw = std::views;

int main() {
    std::cout << "=== C++20 Ranges 예제 ===" << std::endl << std::endl;
    
    // 1. 기본 Range 사용
    std::cout << "1. 기본 Range:" << std::endl;
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    std::cout << "원본: ";
    for (auto n : numbers) std::cout << n << " ";
    std::cout << std::endl;
    
    // 2. Views - filter: 짝수만 필터링
    std::cout << "\n2. filter (짝수만):" << std::endl;
    auto even_numbers = numbers | vw::filter([](int n) { return n % 2 == 0; });
    for (auto n : even_numbers) std::cout << n << " ";
    std::cout << std::endl;
    
    // 3. Views - transform: 각 요소를 제곱
    std::cout << "\n3. transform (제곱):" << std::endl;
    auto squared = numbers | vw::transform([](int n) { return n * n; });
    for (auto n : squared) std::cout << n << " ";
    std::cout << std::endl;
    
    // 4. Views 파이프라인: 필터링 + 변환 + 제한
    std::cout << "\n4. 파이프라인 (홀수 필터 -> 제곱 -> 처음 3개):" << std::endl;
    auto pipeline = numbers 
        | vw::filter([](int n) { return n % 2 == 1; })  // 홀수만
        | vw::transform([](int n) { return n * n; })    // 제곱
        | vw::take(3);                                   // 처음 3개
    
    for (auto n : pipeline) std::cout << n << " ";
    std::cout << std::endl;
    
    // 5. Views - reverse: 역순
    std::cout << "\n5. reverse (역순):" << std::endl;
    for (auto n : numbers | vw::reverse) std::cout << n << " ";
    std::cout << std::endl;
    
    // 6. Views - drop과 take
    std::cout << "\n6. drop(3) + take(4) (3개 건너뛰고 4개 가져오기):" << std::endl;
    auto dropped_taken = numbers | vw::drop(3) | vw::take(4);
    for (auto n : dropped_taken) std::cout << n << " ";
    std::cout << std::endl;
    
    // 7. iota: 범위 생성
    std::cout << "\n7. iota (1부터 10까지 생성):" << std::endl;
    for (auto n : vw::iota(1, 11)) std::cout << n << " ";
    std::cout << std::endl;
    
    // 8. 복잡한 파이프라인 예제
    std::cout << "\n8. 복잡한 파이프라인 (1-20에서 5의 배수만 -> 제곱 -> 역순):" << std::endl;
    auto complex_pipeline = vw::iota(1, 21)
        | vw::filter([](int n) { return n % 5 == 0; })
        | vw::transform([](int n) { return n * n; })
        | vw::reverse;
    
    for (auto n : complex_pipeline) std::cout << n << " ";
    std::cout << std::endl;
    
    // 9. 문자열과 함께 사용
    std::cout << "\n9. 문자열 범위:" << std::endl;
    std::vector<std::string> words = {"apple", "banana", "cherry", "date", "elderberry"};
    
    std::cout << "길이가 6 이상인 단어들 (대문자 변환):" << std::endl;
    auto long_words = words 
        | vw::filter([](const auto& s) { return s.length() >= 6; })
        | vw::transform([](const auto& s) {
            std::string upper;
            for (char c : s) upper += std::toupper(c);
            return upper;
        });
    
    for (const auto& word : long_words) {
        std::cout << word << " ";
    }
    std::cout << std::endl;
    
    // 10. ranges 알고리즘
    std::cout << "\n10. ranges 알고리즘:" << std::endl;
    std::vector<int> data = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    
    // sort
    rg::sort(data);
    std::cout << "정렬 후: ";
    for (auto n : data) std::cout << n << " ";
    std::cout << std::endl;
    
    // find
    auto it = rg::find(data, 7);
    if (it != data.end()) {
        std::cout << "7을 찾음: " << *it << std::endl;
    }
    
    return 0;
}
