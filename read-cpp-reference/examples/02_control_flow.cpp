#include <iostream>
#include <vector>

bool is_even(int value) noexcept {
    return value % 2 == 0;
}

int main() {
    const std::vector<int> values{1, 2, 3, 4, 5};
    int sum = 0;

    // const auto&: 각 요소를 복사하지 않고 읽기 전용으로 참조한다.
    for (const auto& value : values) {
        if (!is_even(value)) {
            continue; // 홀수는 이번 반복의 나머지를 건너뛴다.
        }
        sum += value;
    }

    std::cout << "sum of evens = " << sum << '\n';

    switch (sum) {
    case 6:
        std::cout << "expected result\n";
        break; // 없으면 다음 case로 fall through한다.
    default:
        std::cout << "unexpected result\n";
        break;
    }

    // &&는 왼쪽이 false이면 오른쪽을 평가하지 않는다.
    int divisor = 0;
    if (divisor != 0 && (sum / divisor) > 1) {
        std::cout << "division result is large\n";
    }

    // 목록 초기화는 정보 손실 가능 narrowing을 막는다.
    int exact{42};
    // TRY_COMPILE_ERROR: double 3.14를 int로 narrowing.
    // int narrowed{3.14};
    std::cout << "exact = " << exact << '\n';
}
