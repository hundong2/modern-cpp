#include <cassert>      // assert로 성공값과 오류값을 검증한다.
#include <expected>     // 성공 온도 또는 오류 열거값을 한 객체에 표현한다.
#include <iostream>     // cout으로 검증 결과를 출력한다.
#include <string_view>  // 정적 오류 문자를 복사 없이 바라보는 view를 사용한다.

enum class TemperatureError { below_absolute_zero, sensor_too_hot };
// enum class는 가능한 실패를 닫힌 집합으로 만들고 정수와 암시적으로 섞이지 않는다.

using Temperature = std::expected<double, TemperatureError>;
// using 별칭에서 double은 성공값, TemperatureError는 오류값 타입이다.

// 연습: 아래 규칙을 읽은 뒤 구현을 가리고 직접 다시 입력해 본다.
[[nodiscard]] Temperature validate_celsius(double value) {
    // 반환형 별칭 Temperature는 expected<double, TemperatureError>를 뜻한다.
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
    // constexpr 함수는 조건이 맞으면 컴파일 시간에도 결과를 구할 수 있다.
    switch (error) {
    case TemperatureError::below_absolute_zero: return "below absolute zero";
    case TemperatureError::sensor_too_hot: return "above sensor range";
    }
    return "unknown";
}

int main() {
    // main은 프로그램 진입점이며 끝까지 도달하면 정상 코드 0을 반환한다.
    // const auto는 반환 타입을 추론하되 초기화 뒤 결과 상태 변경을 막는다.
    const auto room = validate_celsius(21.5);
    const auto impossible = validate_celsius(-300.0);
    const auto overheated = validate_celsius(180.0);

    // &&는 단락 평가한다. room이 비었으면 오른쪽 *room을 실행하지 않아 잘못된 접근을 막는다.
    assert(room && *room == 21.5);
    assert(!impossible &&
           // &&는 왼쪽이 거짓이면 오른쪽 error 비교를 실행하지 않는 단락 평가다.
           impossible.error() == TemperatureError::below_absolute_zero);
    assert(!overheated &&
           overheated.error() == TemperatureError::sensor_too_hot);

    std::cout << "Valid temperature: " << *room << " C\n";
    std::cout << "Rejected: " << describe(impossible.error()) << '\n';
    std::cout << "All temperature checks passed.\n";
}
