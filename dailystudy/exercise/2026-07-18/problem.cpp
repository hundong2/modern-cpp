/*
[기초 문법부터 읽는 순서]
1. double은 소수점이 있는 수를 저장합니다.
2. enum class는 온도 오류를 두 가지 이름 있는 값으로 제한합니다.
3. using Temperature는 expected<double, TemperatureError>의 짧은 별명입니다.
4. validate_celsius는 정상 온도면 double을, 범위를 벗어나면 unexpected 오류를 반환합니다.
5. if는 위에서부터 조건을 검사하고 return을 만나면 함수 실행을 즉시 끝냅니다.
6. switch의 case는 오류 종류에 맞는 설명 문자열을 선택합니다.
7. `room && *room == 21.5`는 값이 존재하는지 먼저 확인한 뒤 && 오른쪽에서 값을 읽습니다.
   &&는 왼쪽이 거짓이면 오른쪽을 실행하지 않아 빈 expected 접근을 막습니다.
8. assert는 학습 예제의 예상 결과가 실제 결과와 같은지 자동 검증합니다.
*/

#include <cassert>
#include <expected>
#include <iostream>
#include <string_view>

enum class TemperatureError { below_absolute_zero, sensor_too_hot };

using Temperature = std::expected<double, TemperatureError>;

// Exercise: read the rules below, then hide this solution and retype it.
[[nodiscard]] Temperature validate_celsius(double value) {
    if (value < -273.15) {
        return std::unexpected(TemperatureError::below_absolute_zero);
    }
    if (value > 150.0) {
        return std::unexpected(TemperatureError::sensor_too_hot);
    }
    return value;
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

    assert(room && *room == 21.5);
    assert(!impossible &&
           impossible.error() == TemperatureError::below_absolute_zero);
    assert(!overheated &&
           overheated.error() == TemperatureError::sensor_too_hot);

    std::cout << "Valid temperature: " << *room << " C\n";
    std::cout << "Rejected: " << describe(impossible.error()) << '\n';
    std::cout << "All temperature checks passed.\n";
}
