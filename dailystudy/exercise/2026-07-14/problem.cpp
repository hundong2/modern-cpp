#include <cassert>   // assert로 함수 결과가 예상과 같은지 검증한다.
#include <iostream>  // std::cout으로 결과를 출력한다.
#include <numeric>   // 범위의 합을 계산하는 std::accumulate를 사용한다.
#include <optional>  // 값 또는 값 없음을 나타내는 std::optional을 사용한다.
#include <span>      // vector를 복사하지 않고 읽는 std::span을 사용한다.
#include <vector>    // 정수들을 연속 저장하는 std::vector를 사용한다.

std::optional<std::size_t> first_window_at_least(
    // optional은 성공 인덱스가 있거나 값이 없다는 두 상태 중 하나를 표현한다.
    std::span<const int> readings, // 원소 복사 없이 시작 주소와 길이만 값으로 전달한다.
    std::size_t window_size,
    int threshold) {

    if (window_size == 0 || readings.size() < window_size) {
        // ||는 어느 조건 하나만 참이어도 참이고 왼쪽이 참이면 오른쪽을 계산하지 않는다.
        return std::nullopt; // 값이 없다는 상태를 optional 내부 태그로 표현한다.
    }

    // window_sum은 이름 있는 지역 객체이므로 lvalue다. 대입 시 이 객체의 저장 위치가 갱신된다.
    int window_sum = std::accumulate(
        // begin은 첫 원소, begin+window_size는 합산에서 제외되는 끝 위치다.
        readings.begin(),
        readings.begin() + static_cast<std::ptrdiff_t>(window_size),
        0);

    if (window_sum >= threshold) {
        return 0;
    }

    for (std::size_t right = window_size; right < readings.size(); ++right) {
        // for는 초기화 후 조건을 검사하고, 본문 뒤 ++right를 수행하며 반복한다.
        window_sum += readings[right]; // 배열 원소 식도 lvalue이며, 값을 레지스터로 load해 합산하는 형태가 일반적이다.
        window_sum -= readings[right - window_size];

        if (window_sum >= threshold) {
            return right + 1 - window_size;
        }
    }

    return std::nullopt;
}

void run_tests() {
    // 각 블록 안 const vector는 초기화 뒤 입력이 바뀌지 않음을 보장한다.
    {
        const std::vector<int> readings = {4, 1, 7, 3, 6, 2};
        // 함수 반환 optional은 prvalue다. auto 지역 객체 index를 직접 초기화해 불필요한 복사를 피할 수 있다.
        const auto index = first_window_at_least(readings, 3, 16);
        assert(index.has_value());
        assert(*index == 2); // 7 + 3 + 6
    }

    {
        const std::vector<int> readings = {1, 1, 1, 1};
        const auto index = first_window_at_least(readings, 2, 5);
        assert(!index.has_value());
    }

    {
        const std::vector<int> readings = {10, 1, 1};
        const auto index = first_window_at_least(readings, 1, 10);
        assert(index.has_value());
        assert(*index == 0);
    }

    {
        const std::vector<int> readings = {3, 4};
        const auto index = first_window_at_least(readings, 3, 7);
        assert(!index.has_value());
    }
}

int main() {
    // main은 프로그램 진입점이며 int 반환값 0은 정상 종료를 뜻한다.
    run_tests();

    const std::vector<int> readings = {4, 1, 7, 3, 6, 2};
    const auto index = first_window_at_least(readings, 3, 16);

    if (index) {
        // optional의 bool 변환으로 값 존재를 확인한 뒤 *로 내부 값에 접근한다.
        std::cout << "first matching window starts at index " << *index << '\n';
    } else {
        std::cout << "no matching window\n";
    }

    std::cout << "[TESTS] window exercise passed\n";
    return 0;
}
