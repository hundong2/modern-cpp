#include <algorithm>    // 범위에서 값을 찾는 std::ranges::find를 사용한다.
#include <cassert>      // assert로 예상 결과를 실행 중 검증한다.
#include <concepts>     // concept 반환형 조건 std::same_as를 사용한다.
#include <expected>     // 성공값 또는 오류 문자열을 담는 expected를 사용한다.
#include <iostream>     // cout과 cerr 표준 스트림에 결과를 출력한다.
#include <span>         // 컨테이너를 소유하지 않고 바라보는 span을 사용한다.
#include <string>       // 문자 버퍼를 소유하는 string을 사용한다.
#include <string_view>  // 문자를 복사하지 않는 읽기 전용 view를 사용한다.
#include <utility>      // 이동 가능성을 표시하는 std::move를 사용한다.
#include <vector>       // 명령과 로그를 동적 연속 배열로 저장한다.

enum class CommandKind {
    // enum class는 열거자 이름을 타입 안에 가두고 정수와의 암시적 변환을 막는다.
    // enum class는 정수로 암시 변환되지 않아 다른 숫자와 실수로 섞이는 일을 막는다.
    start,
    stop,
    status,
};

struct Command {
    // struct 멤버는 기본 public이며 string 멤버가 대상 이름을 소유한다.
    CommandKind kind{};
    std::string target;
};

struct RouteReport {
    int success_count{};  // {}로 성공 횟수를 0으로 값 초기화한다.
    int failure_count{};  // 실패 횟수도 독립된 정수 상태로 저장한다.
};

template <typename Logger>
concept CommandLogger = requires(Logger logger, std::string_view message) {
    // 실행 코드가 아니라 컴파일 시 "이 식이 유효하고 반환형이 void인가"를 검사한다.
    { logger.write(message) } -> std::same_as<void>;
};

class RouteScope {
public:
    // 생성자는 반환형이 없다. explicit은 int가 RouteScope로 저절로 변환되는 것을 막는다.
    // 초기화 목록은 참조 멤버를 매개변수가 가리키는 원본에 바인딩한다.
    explicit RouteScope(int& active_routes) : active_routes_{active_routes} { // &는 원본 카운터에 바인딩한다.
        // 전위 ++는 원본 카운터를 즉시 1 증가시킨 뒤 증가된 값을 나타낸다.
        ++active_routes_;
    }

    // RAII 객체의 중복 감소를 막기 위해 복사 생성과 대입을 컴파일 단계에서 금지한다.
    RouteScope(const RouteScope&) = delete;
    RouteScope& operator=(const RouteScope&) = delete;

    // 소멸자는 반환형이 없고 모든 반환 경로에서 자동 호출되어 카운터를 복원한다.
    ~RouteScope() {
        --active_routes_;
    }

private:
    // private 참조 멤버는 원본을 소유하지 않으므로 원본이 더 오래 살아야 한다.
    int& active_routes_;
};

class MemoryLogger {
public:
    // string_view는 빌린 문자이고 emplace_back은 vector 안에 소유 string을 직접 만든다.
    void write(std::string_view message) {
        // 표준 호출 계약: messages_가 수신 vector<string>이고 emplace_back(message)은 string_view를 string 생성자에 전달한다.
        // 반환값은 새 원소 string&지만 여기서는 버린다. 성공하면 size가 1 늘며 필요 시 재할당되어 기존 관찰자가 무효화된다.
        messages_.emplace_back(message);
    }

    // 반환 span은 vector를 소유하지 않는다. Logger가 살아 있고 재할당되지 않는 동안만 유효하다.
    [[nodiscard]] std::span<const std::string> messages() const {
        return messages_;
    }

private:
    std::vector<std::string> messages_;
};

std::expected<CommandKind, std::string> parse_kind(std::string_view word) {
    // 각 if는 문자열을 비교하고 참이면 해당 enum 값을 즉시 반환한다.
    if (word == "start") {
        return CommandKind::start;
    }
    if (word == "stop") {
        return CommandKind::stop;
    }
    if (word == "status") {
        return CommandKind::status;
    }

    // unexpected(error)는 소유 string 오류값 하나를 입력받아 unexpected<string> prvalue를 반환한다.
    // 그 반환값이 expected의 실패 상태를 직접 초기화하며 성공 CommandKind는 생성하지 않는다.
    return std::unexpected("unknown command kind: " + std::string(word));
}

std::expected<Command, std::string> parse_command(std::string_view line) {
    // find(' ')는 찾을 문자 하나를 입력받고 line을 바꾸지 않으며 첫 위치 size_t 또는 실패 npos를 반환한다.
    const std::size_t space = line.find(' ');
    if (space == std::string_view::npos) {
        return std::unexpected("expected format: <start|stop|status> <target>");
    }

    // substr(pos,count)는 위치와 선택 길이를 입력받아 비소유 string_view를 반환한다. count 생략 시 끝까지 선택한다.
    // empty()는 인자 없이 길이가 0인지 bool을 반환하며 target_text를 수정하지 않는다.
    const std::string_view kind_text = line.substr(0, space);
    const std::string_view target_text = line.substr(space + 1);
    if (target_text.empty()) {
        return std::unexpected("target is empty");
    }

    const auto kind = parse_kind(kind_text); // 반환 prvalue로 const 지역 객체를 초기화한다.
    if (!kind) {
        // error()는 실패 expected 안의 string 참조를 반환한다. 성공 상태에서 호출하면 전제조건을 어기므로 먼저 !kind를 검사한다.
        return std::unexpected(kind.error());
    }

    // string(target_text)는 문자를 복사해 소유하는 prvalue이고 Command를 직접 초기화한다.
    return Command{*kind, std::string(target_text)};
}

std::string_view to_text(CommandKind kind) {
    // switch는 enum 값을 비교해 일치하는 case로 분기한다. 각 return이 함수를 끝낸다.
    switch (kind) {
    case CommandKind::start:
        return "start";
    case CommandKind::stop:
        return "stop";
    case CommandKind::status:
        return "status";
    }

    return "unknown";
}

bool is_allowed_target(std::string_view target) {
    // constexpr 배열은 컴파일 시간에도 사용할 수 있는 고정 허용 목록이다.
    constexpr std::string_view allowed_targets[] = {"api", "database", "cache"};
    // ranges::find(range,value)는 허용 목록과 찾을 target을 입력받아 첫 일치 반복자 또는 끝 반복자를 반환하고 배열은 바꾸지 않는다.
    // ranges::end(range)는 같은 배열의 끝 센티널을 반환한다. 두 결과 비교가 bool 허용 여부가 되며 시간은 원소 수에 선형이다.
    return std::ranges::find(allowed_targets, target) != std::ranges::end(allowed_targets);
}

template <CommandLogger Logger>
class CommandRouter {
public:
    // 참조 멤버는 logger와 카운터를 소유하지 않으므로 두 객체가 router보다 오래 살아야 한다.
    CommandRouter(Logger& logger, int& active_routes)
        : logger_{logger}, active_routes_{active_routes} {}

    std::expected<RouteReport, std::string> route(std::span<const std::string_view> lines) {
        // RouteScope 지역 객체가 조기 반환에서도 활성 경로 수를 원래 값으로 복구한다.
        RouteScope scope{active_routes_};
        RouteReport report{};

        for (std::size_t index = 0; index < lines.size(); ++index) {
            const auto parsed = parse_command(lines[index]);
            if (!parsed) {
                ++report.failure_count;
                // to_string은 1부터 시작할 정수 하나를 입력받아 새 string을 반환하고, error()는 실패 문자열 참조를 반환한다.
                return std::unexpected(
                    "line " + std::to_string(index + 1) + ": " + parsed.error());
            }

            if (!is_allowed_target(parsed->target)) {
                // ->는 expected 내부 Command의 public 멤버를 참조한다.
                ++report.failure_count;
                return std::unexpected(
                    "line " + std::to_string(index + 1) + ": target is not allowed: " +
                    parsed->target);
            }

            ++report.success_count;
            // 가상 호출이 아니라 템플릿으로 Logger 타입이 확정되어 컴파일러가 인라인할 여지가 크다.
            logger_.write(
                std::string(to_text(parsed->kind)) + " routed to " + parsed->target);
        }

        return report;
    }

private:
    // private 멤버는 외부 코드가 직접 접근할 수 없는 구현 세부사항이다.
    Logger& logger_;
    int& active_routes_;
};

void run_tests() {
    // 중괄호 블록은 테스트마다 지역 객체 수명을 분리한다.
    {
        const auto parsed = parse_command("start api");
        // has_value()는 인자 없이 성공값 존재 여부를 bool로 반환하고 expected 상태는 바꾸지 않는다.
        assert(parsed.has_value());
        assert(parsed->kind == CommandKind::start);
        assert(parsed->target == "api");
    }

    {
        const auto parsed = parse_command("restart api");
        assert(!parsed.has_value());
    }

    {
        MemoryLogger logger;
        int active_routes = 0;
        CommandRouter router{logger, active_routes};

        const std::vector<std::string_view> commands = {
            "start api",
            "status database",
            "stop cache",
        };

        const auto report = router.route(commands);
        assert(report.has_value());
        assert(report->success_count == 3);
        assert(report->failure_count == 0);
        assert(logger.messages().size() == 3);
        assert(active_routes == 0);
    }

    {
        MemoryLogger logger;
        int active_routes = 0;
        CommandRouter router{logger, active_routes};

        const std::vector<std::string_view> commands = {
            "start api",
            "pause api",
            "stop cache",
        };

        const auto report = router.route(commands);
        assert(!report.has_value());
        assert(active_routes == 0);
    }
}

int main() {
    // main은 프로그램 진입점이고 0은 정상, 0이 아닌 값은 실패 종료를 뜻한다.
    run_tests();

    MemoryLogger logger;
    int active_routes = 0;
    CommandRouter router{logger, active_routes};

    const std::vector<std::string_view> batch = {
        "start api",
        "status database",
        "stop cache",
    };

    const auto report = router.route(batch);
    if (!report) {
        std::cerr << "[ERROR] " << report.error() << '\n';
        return 1;
    }

    std::cout << "[REPORT]\n";
    std::cout << "  successful commands: " << report->success_count << '\n';
    std::cout << "  failed commands    : " << report->failure_count << '\n';
    std::cout << "  active routes      : " << active_routes << '\n';

    for (const std::string& message : logger.messages()) {
        // const 참조로 각 로그 문자열의 버퍼 복사를 피한다.
        std::cout << "[LOG] " << message << '\n';
    }

    std::cout << "[TESTS] router exercise passed\n";
    return 0;
}
