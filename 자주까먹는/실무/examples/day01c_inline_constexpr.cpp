#include <array>     // 컴파일 타임 크기를 갖는 std::array를 제공한다.
#include <iostream>  // 계산 결과를 출력한다.

inline int runtime_square(int value) {             // inline의 핵심은 ODR 허용이며 펼침은 보장되지 않는다.
    return value * value;                          // 런타임 인자도 타입 안전하게 한 번 평가한다.
}                                                  // 작은 함수 정의를 끝낸다.

constexpr int factorial(int n) {                   // 상수 평가와 런타임 호출 모두 가능한 함수다.
    return n <= 1 ? 1 : n * factorial(n - 1);      // 종료 조건이 있는 재귀 상수 표현식이다.
}                                                  // C++14 이상 constexpr 규칙에 맞는다.

int main() {                                       // constexpr 차이를 관찰하는 진입점이다.
    constexpr int capacity = factorial(5);         // 상수 문맥이므로 컴파일 타임에 120이어야 한다.
    std::array<int, capacity> slots{};             // 타입 일부인 배열 크기에 상수를 사용한다.
    int runtime = 6;                               // 실행 중 바뀔 수 있는 일반 변수다.
    const int result = factorial(runtime);          // const지만 계산 시점은 런타임일 수 있다.
    std::cout << slots.size() << ' ' << result << ' ' << runtime_square(7) << '\n'; // 120,720,49를 출력한다.
    return 0;                                      // 정상 종료한다.
}                                                  // 지역 배열과 정수의 수명을 끝낸다.
