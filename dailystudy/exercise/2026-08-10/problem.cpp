// <algorithm>은 범위에서 조건을 만족하는 원소 수를 세는 std::ranges::count_if를 제공한다.
#include <algorithm>
// <iostream>은 표준 출력 객체를 제공한다.
#include <iostream>
// <vector>는 int 원소를 소유하는 동적 배열을 제공한다.
#include <vector>

// 템플릿 인자 R은 begin/end를 제공하는 범위 타입으로 추론된다.
template <class R>
[[nodiscard]] int count_even(const R& values) {
    // const R&는 lvalue에 바인딩된 비소유 참조이며 호출 동안 원본이 살아 있어야 한다.
    const auto count{std::ranges::count_if(values, [](int value) {
        return value % 2 == 0; // 나머지 연산자 %와 비교 연산자 ==로 짝수를 판정한다.
    })};
    return static_cast<int>(count); // ranges 결과 타입을 예제의 기본 타입 int로 명시 변환한다.
}

int main() {
    const std::vector<int> values{1, 2, 4, 7, 8}; // const 소유 객체의 수명은 main 끝까지다.
    const int answer{count_even(values)}; // values는 lvalue이고 const 참조에 복사 없이 바인딩된다.
    std::cout << answer << '\n'; // 함수 호출 결과를 출력한다.
    return answer == 3 ? 0 : 1; // 조건 연산자로 성공 코드 0 또는 실패 코드 1을 고른다.
}
