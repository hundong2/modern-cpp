// C++20 새로운 기능들 종합 예제
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <format>
#include <span>
#include <array>
#include <compare>

// 1. designated initializers (지정 초기화)
struct Person {
    std::string name;
    int age;
    std::string city;
};

// 2. 삼방향 비교 연산자 (spaceship operator)
struct Point {
    int x, y;
    
    // C++20: 자동으로 ==, !=, <, <=, >, >= 생성
    auto operator<=>(const Point& other) const = default;
};

// 3. consteval - 컴파일 타임 강제 실행
consteval int square_compile_time(int n) {
    return n * n;
}

// 4. constinit - 컴파일 타임 초기화
constinit int global_value = 100;

// 5. std::span 예제 함수
void print_span(std::span<const int> data) {
    std::cout << "Span 크기: " << data.size() << ", 값: ";
    for (auto val : data) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "=== C++20 새로운 기능들 종합 예제 ===" << std::endl << std::endl;
    
    // 1. Designated Initializers
    std::cout << "1. Designated Initializers:" << std::endl;
    Person p1 = {
        .name = "홍길동",
        .age = 30,
        .city = "서울"
    };
    std::cout << "이름: " << p1.name << ", 나이: " << p1.age 
              << ", 도시: " << p1.city << std::endl << std::endl;
    
    // 2. 삼방향 비교 연산자
    std::cout << "2. 삼방향 비교 연산자 (Spaceship <=>):" << std::endl;
    Point pt1{1, 2};
    Point pt2{1, 3};
    Point pt3{1, 2};
    
    std::cout << "pt1 == pt3: " << (pt1 == pt3 ? "true" : "false") << std::endl;
    std::cout << "pt1 < pt2: " << (pt1 < pt2 ? "true" : "false") << std::endl;
    std::cout << "pt1 != pt2: " << (pt1 != pt2 ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // 3. consteval
    std::cout << "3. consteval (컴파일 타임 실행):" << std::endl;
    constexpr int result = square_compile_time(10);  // 컴파일 타임에 계산
    std::cout << "10의 제곱 (컴파일 타임): " << result << std::endl << std::endl;
    
    // 4. std::format (C++20 포맷팅)
    std::cout << "4. std::format (문자열 포맷팅):" << std::endl;
    std::string name = "Alice";
    int score = 95;
    // 주의: std::format은 일부 컴파일러에서 아직 완전히 지원되지 않을 수 있음
    // 대안으로 iostream 사용
    std::cout << name << "의 점수: " << score << "점" << std::endl;
    std::cout << std::endl;
    
    // 5. std::span
    std::cout << "5. std::span (연속 메모리 뷰):" << std::endl;
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::array<int, 5> arr = {10, 20, 30, 40, 50};
    
    std::cout << "Vector span: ";
    print_span(vec);
    
    std::cout << "Array span: ";
    print_span(arr);
    
    std::cout << "Partial span (vec[1:4]): ";
    print_span(std::span(vec).subspan(1, 3));
    std::cout << std::endl;
    
    // 6. contains (C++20 for map/set)
    std::cout << "6. contains (컨테이너 멤버 함수):" << std::endl;
    std::map<std::string, int> scores = {
        {"Alice", 95},
        {"Bob", 87},
        {"Charlie", 92}
    };
    
    std::string search_name = "Bob";
    if (scores.contains(search_name)) {
        std::cout << search_name << "을(를) 찾았습니다! 점수: " 
                  << scores[search_name] << std::endl;
    }
    std::cout << std::endl;
    
    // 7. starts_with / ends_with (string_view)
    std::cout << "7. starts_with / ends_with:" << std::endl;
    std::string text = "Hello, World!";
    std::string_view sv(text);
    
    std::cout << "'" << text << "'은(는)" << std::endl;
    std::cout << "  'Hello'로 시작? " << (sv.starts_with("Hello") ? "Yes" : "No") << std::endl;
    std::cout << "  'World!'로 끝남? " << (sv.ends_with("World!") ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
    
    // 8. Range-based for with initializer
    std::cout << "8. Range-based for with initializer:" << std::endl;
    for (std::vector<int> nums = {1, 2, 3, 4, 5}; auto num : nums) {
        std::cout << num << " ";
    }
    std::cout << std::endl << std::endl;
    
    // 9. [[likely]] / [[unlikely]] 힌트
    std::cout << "9. 컴파일러 힌트 속성:" << std::endl;
    int value = 10;
    
    if (value > 0) [[likely]] {
        std::cout << "양수입니다 (likely 힌트 사용)" << std::endl;
    } else [[unlikely]] {
        std::cout << "0 이하입니다 (unlikely)" << std::endl;
    }
    std::cout << std::endl;
    
    // 10. constinit
    std::cout << "10. constinit (컴파일 타임 초기화):" << std::endl;
    std::cout << "Global value: " << global_value << std::endl;
    
    return 0;
}
