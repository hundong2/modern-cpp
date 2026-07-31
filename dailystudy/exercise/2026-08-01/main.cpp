// <iostream>은 표준 출력 객체 std::cout과 출력 연산자 <<를 제공합니다.
#include <iostream>
// <memory>는 단독 소유 스마트 포인터 unique_ptr와 생성 함수 make_unique를 제공합니다.
#include <memory>
// <string>은 문자 데이터를 직접 소유하는 std::string을 제공합니다.
#include <string>
// <string_view>는 문자 데이터를 소유하지 않고 빌려 읽는 std::string_view를 제공합니다.
#include <string_view>
// <utility>는 lvalue를 xvalue로 표현하는 std::move를 제공합니다.
#include <utility>

// using은 새 타입을 만드는 것이 아니라 기존 타입에 읽기 쉬운 별칭을 붙입니다.
using Text = std::string;

// enum class는 이름이 다른 영역으로 새지 않는 강한 열거형이며, Kind가 기본 타입 이름입니다.
enum class Kind { echo, quit, unknown };

// struct의 기본 접근은 public이므로 단순 결과 데이터를 공개 멤버로 묶기에 알맞습니다.
struct Command {
    Kind kind{Kind::unknown}; // 중괄호 초기화로 열거형 멤버의 기본 상태를 명시합니다.
    Text argument{};         // std::string 멤버가 문자를 소유하므로 원본 입력의 수명과 분리됩니다.
};

// 반환형은 Command이고 매개변수 const std::string&는 문자열을 복사하지 않고 읽기만 합니다.
[[nodiscard]] Command parse_command(const Text& input) {
    // string_view 생성자에 input lvalue가 바인딩됩니다. text는 input의 문자 버퍼를 빌립니다.
    const std::string_view text{input};
    // starts_with는 C++20 표준 라이브러리 함수이며 앞부분이 같은지 비교해 bool을 반환합니다.
    if (text.starts_with("echo ")) { // if는 비교 결과가 참일 때 이 조건 분기로 들어옵니다.
        // substr(5)는 뷰를 만들고, std::string 생성자는 그 범위의 문자를 복사하여 소유합니다.
        return Command{Kind::echo, Text{text.substr(5)}}; // prvalue는 반환 목적지에 직접 생성될 수 있습니다.
    }
    // == 연산자는 두 문자열 뷰의 길이와 문자를 비교합니다.
    if (text == "quit") {
        return Command{Kind::quit, {}}; // 빈 중괄호는 argument를 빈 문자열로 값 초기화합니다.
    }
    return Command{Kind::unknown, Text{text}};
}

// struct의 기본 public을 이용한 출력 포트입니다. 구현이 아닌 필요한 동작만 선언합니다.
struct OutputPort {
    virtual ~OutputPort() = default; // 기반 포인터로 삭제할 때 파생 소멸자까지 호출되게 합니다.
    // virtual은 동적 타입의 구현을 고르게 하며, 매개변수는 비소유 읽기 전용 뷰입니다.
    virtual void write(std::string_view message) const = 0;
};

// class의 기본 접근은 private입니다. public:으로 외부에 공개할 동작을 명시합니다.
class ConsoleOutput final : public OutputPort {
public:
    // void는 반환값이 없다는 뜻이고 override는 기반 함수와 서명이 맞는지 검사합니다.
    void write(std::string_view message) const override {
        std::cout << message << '\n'; // <<는 값을 스트림에 보내고 '\n'은 줄바꿈 문자입니다.
    }
};

class CommandService {
public:
    // 생성자에는 반환형이 없습니다. explicit은 unique_ptr에서 서비스로의 암시적 변환을 막습니다.
    // 올바른 사용은 CommandService service{std::move(output)}처럼 직접 초기화하는 것입니다.
    explicit CommandService(std::unique_ptr<OutputPort> output)
        // 멤버 초기화 목록은 생성자 본문 전에 output_을 이동 생성하여 소유권을 넘깁니다.
        : output_{std::move(output)} {}

    // 반환형 bool은 계속 실행할지를 뜻하고, const 참조 매개변수는 Command를 복사하지 않습니다.
    [[nodiscard]] bool execute(const Command& command) const {
        // switch는 열거형 값을 비교해 해당 case로 조건 분기합니다.
        switch (command.kind) {
        case Kind::echo:
            output_->write(command.argument); // ->로 포인터가 가리키는 객체의 가상 함수를 호출합니다.
            return true;
        case Kind::quit:
            output_->write("bye");
            return false;
        case Kind::unknown:
            output_->write("unknown: " + command.argument); // +는 새 소유 문자열 prvalue를 만듭니다.
            return true;
        }
        return false;
    }

private:
    // 템플릿 인자 OutputPort는 unique_ptr가 어떤 타입을 단독 소유하는지 지정합니다.
    std::unique_ptr<OutputPort> output_;
};

int main() { // int 반환값은 운영체제에 전달되는 프로그램 종료 상태입니다.
    Text input{"echo hello"}; // 기본 문자열 타입 변수이며 중괄호로 문자 데이터를 직접 초기화합니다.
    auto output{std::make_unique<ConsoleOutput>()}; // auto가 unique_ptr<ConsoleOutput> 타입을 추론합니다.
    CommandService service{std::move(output)}; // output lvalue를 xvalue로 바꾸어 소유권을 이동합니다.

    const Command command{parse_command(input)}; // 함수 호출 결과 prvalue로 const 객체를 초기화합니다.
    const bool keep_running{service.execute(command)}; // 이름 있는 command lvalue가 const 참조에 바인딩됩니다.

    // 포인터는 주소를 저장하며 nullptr과 비교할 수 있습니다. 이동 후 output은 비어 있어야 합니다.
    const ConsoleOutput* const observer{output.get()};
    if (observer == nullptr && keep_running) { // &&는 왼쪽이 참일 때만 오른쪽도 평가합니다.
        // int 반복 변수는 0, 1을 차례로 저장하며 ++ 연산자가 1씩 증가시킵니다.
        for (int check{0}; check < 2; ++check) {
            std::cout << "check " << check << '\n';
        }
    }

    // ?: 조건 연산자는 참이면 0, 거짓이면 1을 선택합니다.
    return keep_running ? 0 : 1;
}
