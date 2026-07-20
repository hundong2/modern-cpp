#include <cassert>
#include <expected>
#include <iostream>
#include <string_view>

enum class TemperatureError { below_absolute_zero, sensor_too_hot };

using Temperature = std::expected<double, TemperatureError>;

// 연습: 아래 규칙을 읽은 뒤 구현을 가리고 직접 다시 입력해 본다.
[[nodiscard]] Temperature validate_celsius(double value) {
    // value는 함수 호출 시 복사되는 지역 lvalue다. double 비교는 보통 부동소수점 비교+조건 분기로 구현된다.
    if (value < -273.15) {
        return std::unexpected(TemperatureError::below_absolute_zero); // 오류 prvalue로 expected를 구성한다.
    }
    if (value > 150.0) {
        return std::unexpected(TemperatureError::sensor_too_hot);
    }
    return value; // double 값을 성공 저장소에 복사한다.
}

[[nodiscard]] constexpr std::string_view describe(TemperatureError error) {
    switch (error) {
    case TemperatureError::below_absolute_zero: return "below absolute zero";
    case TemperatureError::sensor_too_hot: return "above sensor range";
    }
    return "unknown";
}

int main() {
    const auto room = validate_celsius(21.5);
    const auto impossible = validate_celsius(-300.0);
    const auto overheated = validate_celsius(180.0);

    // &&는 단락 평가한다. room이 비었으면 오른쪽 *room을 실행하지 않아 잘못된 접근을 막는다.
    assert(room && *room == 21.5);
    assert(!impossible &&
           impossible.error() == TemperatureError::below_absolute_zero);
    assert(!overheated &&
           overheated.error() == TemperatureError::sensor_too_hot);

    std::cout << "Valid temperature: " << *room << " C\n";
    std::cout << "Rejected: " << describe(impossible.error()) << '\n';
    std::cout << "All temperature checks passed.\n";
}
