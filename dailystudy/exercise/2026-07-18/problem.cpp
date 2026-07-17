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
