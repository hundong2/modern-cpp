#include <iostream>
#include <vector>
#include <array>
#include <numeric>
#include <span>
#include <concepts>
#include <ranges>   // [C++20] 컨테이너를 우아하게 다루기 위한 ranges 헤더 추가

// [기본 문법] constexpr: 빌드 타임에 확정되는 상수. 실행 시간(Runtime) 비용이 0입니다.
// string_view: 문자열을 복사하지 않고 포인터와 길이만으로 읽게 해주는 가벼운 참조 타입.
constexpr std::string_view LOG_PREFIX = "[StatAnalyzer] ";

// [C++20 Concept] T는 반드시 정수이거나 실수여야 한다는 엄격한 제약 조건
template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T>
class StatAnalyzer {
public:
    // [핵심 개선] C++20 std::ranges::contiguous_range 적용
    // vector, array, C-배열 등 '메모리가 연속된 컨테이너'를 사용자가 그대로 던지면 받아냅니다.
    // [기본 문법] explicit: 의도치 않은 암시적 형변환을 막아 버그를 차단합니다.
    // [기본 문법] noexcept: 메모리 할당이나 예외가 절대 터지지 않음을 보장합니다.
    template <std::ranges::contiguous_range R>
    explicit StatAnalyzer(R&& range) noexcept 
        // range에서 메모리 시작 주소(data)와 개수(size)만 뽑아내어 data_(span)를 초기화합니다. (복사 0)
        : data_{std::ranges::data(range), std::ranges::size(range)} {}
    
    // [기본 문법] [[nodiscard]]: 계산 결과를 호출자가 변수에 안 담고 버리면 경고를 띄웁니다.
    // [기본 문법] const: 이 함수는 멤버 변수를 절대 수정하지 않음을(Read-Only) 보장합니다.
    [[nodiscard]] T getSum() const noexcept {
        return std::accumulate(data_.begin(), data_.end(), T{0});
    }

private:
    // 원본 데이터의 생명주기를 건드리지 않고, '읽기 전용'으로 바라만 보는 뷰(View)
    std::span<const T> data_; 
};

// ---------------------------------------------------------
// ★ C++20 추론 가이드 (Deduction Guide) ★
// 사용자가 던진 컨테이너(R)의 '실제 알맹이 타입(range_value_t)'을 뽑아내서 T로 강제 추론시킵니다.
// ---------------------------------------------------------
template <std::ranges::contiguous_range R>
StatAnalyzer(R&&) -> StatAnalyzer<std::ranges::range_value_t<R>>;


int main() {
    // 1. std::vector 사용
    std::vector<int> v_data = {1, 2, 3, 4, 5};
    
    // [완벽해진 호출부] 사용자는 귀찮게 span을 감쌀 필요가 없습니다. 그냥 던지면 끝입니다.
    // 추론 가이드가 vector<int>를 보고 StatAnalyzer<int>로 정확히 추론합니다.
    StatAnalyzer analyzer1{v_data}; 
    std::cout << LOG_PREFIX << "벡터 합: " << analyzer1.getSum() << '\n';

    // 2. std::array 사용
    std::array<double, 3> a_data = {1.1, 2.2, 3.3};
    StatAnalyzer analyzer2{a_data}; 
    std::cout << LOG_PREFIX << "어레이 합: " << analyzer2.getSum() << '\n';

    // 3. C-스타일 날것의 배열 사용
    float c_data[] = {10.0f, 20.0f};
    StatAnalyzer analyzer3{c_data};
    std::cout << LOG_PREFIX << "C배열 합: " << analyzer3.getSum() << '\n';

    return 0;
}