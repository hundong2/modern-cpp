// <iostream>은 표준 출력 객체 std::cout와 << 출력 연산자를 제공한다.
#include <iostream>
// <string>은 문자 버퍼의 수명을 소유하는 std::string을 제공한다.
#include <string>
// <type_traits>는 템플릿 타입을 비교하는 std::is_same_v를 제공한다.
#include <type_traits>
// <utility>는 전달된 값의 값 범주를 보존하는 std::forward를 제공한다.
#include <utility>
// <variant>는 여러 타입 중 정확히 하나를 보관하는 std::variant와 std::visit을 제공한다.
#include <variant>

// struct는 기본 접근이 public이므로 단순 명령 데이터의 공개 필드를 표현하기 좋다.
struct Start { std::string job{}; }; // 중괄호 초기화로 빈 문자열을 기본값으로 만든다.
struct Stop { int code{}; };          // int는 정수 기본 타입이고 {}는 0으로 값 초기화한다.

// using은 새 타입을 만드는 것이 아니라 긴 템플릿 인스턴스에 읽기 쉬운 별칭을 붙인다.
using Command = std::variant<Start, Stop>; // 템플릿 인자 Start와 Stop 중 하나만 활성 상태이다.

// class의 기본 접근은 private이다. 공개 동작과 내부 상태를 분리해 애플리케이션 경계를 만든다.
class CommandHandler {
public:
    // 생성자에는 반환형이 없다. explicit은 bool 하나가 핸들러로 암시 변환되는 일을 막는다.
    explicit CommandHandler(bool enabled)
        : enabled_{enabled} {} // 멤버 초기화 목록은 본문 진입 전에 멤버를 직접 초기화한다.

    // const 참조는 Command를 복사하지 않고 읽으며, 뒤의 const는 객체 상태를 바꾸지 않는다는 계약이다.
    [[nodiscard]] std::string handle(const Command& command) const {
        // std::visit은 현재 활성 대안을 확인한 뒤 람다의 알맞은 호출 연산자를 선택한다.
        return std::visit([this](const auto& value) -> std::string {
            // decltype과 remove_cvref_t는 const 참조를 벗겨 실제 대안 타입을 얻는다.
            using T = std::remove_cvref_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Start>) { // 컴파일 시간 조건이라 Stop용 분기에서는 job을 보지 않는다.
                return enabled_ ? "start:" + value.job : "disabled"; // ?:는 조건에 따라 두 prvalue 중 하나를 만든다.
            } else {
                return "stop:" + std::to_string(value.code); // 표준 함수 호출로 정수를 소유 문자열로 바꾼다.
            }
        }, command); // 구현은 태그 로드·비교·조건 분기와 함수 호출이 될 수 있으나 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    }

private:
    bool enabled_{}; // private 멤버 변수는 정책 상태를 외부의 직접 변경에서 보호한다.
};

int main() { // 운영체제에 성공 여부를 int로 반환하는 진입 함수이다.
    CommandHandler handler{true}; // explicit 생성자는 이처럼 직접 초기화해야 한다.
    Command command{Start{"backup"}}; // Start 임시 객체는 prvalue이고 variant 내부에 직접 생성되어 복사 생략될 수 있다.
    const std::string first{handler.handle(command)}; // 이름 있는 command는 lvalue이며 결과 문자열의 수명은 first가 소유한다.
    command = Stop{7}; // Stop prvalue를 variant에 이동/복사하여 활성 대안을 바꾸고 이전 Start의 수명을 끝낸다.
    const std::string second{handler.handle(command)};
    std::cout << first << '\n' << second << '\n';
    return first == "start:backup" && second == "stop:7" ? 0 : 1; // &&는 왼쪽이 거짓이면 오른쪽 비교를 생략한다.
}
