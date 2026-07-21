#include <cassert>   // assert로 예상 결과를 실행 중 검증한다.
#include <iomanip>   // 소수점 출력 형식을 조정하는 setprecision을 사용한다.
#include <iostream>  // cout 표준 출력 스트림을 사용한다.
#include <optional>  // 값 또는 값 없음을 나타내는 optional을 사용한다.
#include <span>      // vector 원소를 소유하지 않고 읽는 span을 사용한다.
#include <vector>    // 여러 부하 값을 연속 메모리에 소유한다.

std::optional<std::size_t> first_index_at_least(
    // size_t는 음수가 없는 크기 타입이고 optional은 인덱스 부재를 별도 상태로 표현한다.
    std::span<const int> values,
    int threshold) {

    for (std::size_t index = 0; index < values.size(); ++index) {
        // for는 index를 0부터 시작해 크기보다 작은 동안 1씩 증가시킨다.
        // values[index]는 컨테이너 안 int를 가리키는 lvalue다. 여기서는 읽기만 한다.
        if (values[index] >= threshold) {
            return index;
        }
    }

    return std::nullopt;
}

std::optional<double> average(std::span<const int> values) {
    // span은 원소를 소유하지 않는 읽기 전용 뷰이므로 원본이 호출 중 살아 있어야 한다.
    if (values.empty()) {
        // empty는 원소 수가 0인지 bool로 반환하고 true면 조기 반환한다.
        return std::nullopt;
    }

    int total = 0; // 자동 저장 기간 지역 변수로, 보통 스택 또는 최적화 후 레지스터에 놓인다.
    for (int value : values) { // int는 작으므로 참조 대신 값 복사가 단순하고 저렴하다.
        total += value;
    }

    // 명시적 형 변환으로 정수 나눗셈이 아닌 부동소수점 나눗셈을 선택한다.
    return static_cast<double>(total) / static_cast<double>(values.size());
}

void run_tests() {
    // assert 조건은 디버그 빌드에서 거짓이면 프로그램을 중단해 실패를 드러낸다.
    {
        const std::vector<int> loads = {22, 48, 65, 81, 44};
        const auto index = first_index_at_least(loads, 80);
        assert(index.has_value());
        assert(*index == 3);
    }

    {
        const std::vector<int> loads = {10, 20, 30};
        const auto index = first_index_at_least(loads, 90);
        assert(!index.has_value());
    }

    {
        const std::vector<int> loads = {10, 20, 30};
        const auto result = average(loads);
        assert(result.has_value());
        assert(*result == 20.0);
    }

    {
        const std::vector<int> loads;
        const auto result = average(loads);
        assert(!result.has_value());
    }
}

int main() {
    // main은 프로그램 진입점이고 중괄호 초기화는 축소 변환을 방지한다.
    run_tests();

    const std::vector<int> loads = {22, 48, 65, 81, 44};
    const int warning_threshold = 80;

    const auto first_warning = first_index_at_least(loads, warning_threshold);
    if (first_warning) { // optional의 explicit bool 변환으로 값 존재 태그를 검사한다.
        std::cout << "first warning index: " << *first_warning << '\n';
    } else {
        std::cout << "no warning sample\n";
    }

    const auto mean = average(loads);
    if (mean) {
        // fixed와 setprecision(1)은 이후 부동소수점 출력을 소수 첫째 자리로 맞춘다.
        std::cout << "average load: " << std::fixed << std::setprecision(1) << *mean << '\n';
    }

    std::cout << "[TESTS] problem exercise passed\n";
    return 0;
}
