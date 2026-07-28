#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

template<class T>
const T& larger(const T& left, const T& right) {
    return left < right ? right : left;
}

template<class... Ts>
auto sum(Ts... values) {
    // C++17 fold expression. 0을 초기값으로 둬 빈 pack도 처리한다.
    return (0 + ... + values);
}

template<class Callable, class Argument>
auto call_twice(Callable&& callable, const Argument& argument) {
    // 이름 붙은 callable을 두 번 호출한다. rvalue 인자를 두 번 forward하면 첫 호출에서
    // 이미 이동될 수 있으므로 이 교육용 함수는 읽기 전용 argument를 받는다.
    std::invoke(callable, argument);
    return std::invoke(callable, argument);
}

struct Doubler {
    int operator()(int value) const noexcept {
        return value * 2;
    }
};

int main() {
    std::cout << "larger = " << larger(3, 7) << '\n';
    std::cout << "sum = " << sum(1, 2, 3, 4) << '\n';
    std::cout << "empty sum = " << sum() << '\n';

    Doubler function_object;
    std::cout << "functor = " << std::invoke(function_object, 4) << '\n';

    int calls = 0;
    auto lambda = [&calls](int value) {
        ++calls; // 참조 capture라서 바깥 객체를 수정
        return value + calls;
    };

    const int result = call_twice(lambda, 10);
    std::cout << "result = " << result << ", calls = " << calls << '\n';

    static_assert(std::is_invocable_v<Doubler, int>);
    static_assert(std::is_same_v<
        std::invoke_result_t<Doubler, int>,
        int>);
}
