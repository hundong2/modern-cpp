/*
[기초 문법부터 읽는 순서]
1. enum class는 start, stop, status처럼 가능한 값을 이름으로 제한합니다.
2. struct Command는 명령 종류와 대상 문자열을 한 객체에 묶습니다.
3. string은 문자를 소유하지만 string_view는 기존 문자열을 빌려 읽습니다.
4. expected<Command, string>은 파싱 성공 시 Command, 실패 시 오류 문자열을
   저장합니다. unexpected(...)로 실패 값을 만듭니다.
5. line.find(' ')와 substr은 공백 위치를 찾고 문자열의 일부를 잘라 봅니다.
6. switch와 case는 enum 값에 따라 실행할 코드를 선택합니다.
7. Logger&의 &는 logger를 복사하지 않고 원본 객체를 참조한다는 뜻입니다.
8. template/concept는 write(message)가 가능한 로거만 CommandRouter에 주입되게
   하며, 서비스 규칙과 출력 저장소를 분리합니다.
9. RouteScope의 생성자·소멸자는 조기 return이 있어도 횟수를 원상 복구하는
   RAII 패턴입니다.
*/

/*
Daily Modern C++ Exercise - 2026-07-15

Theme:
  Route tiny server commands while practicing architecture boundaries.

What to notice:
  - parse_command owns parsing and returns std::expected.
  - CommandRouter owns business rules.
  - MemoryLogger is an output adapter used by tests and the demo.
  - RouteScope uses RAII to keep active route counting correct.
*/

#include <algorithm>
#include <cassert>
#include <concepts>
#include <expected>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

enum class CommandKind {
    start,
    stop,
    status,
};

struct Command {
    CommandKind kind{};
    std::string target;
};

struct RouteReport {
    int success_count{};
    int failure_count{};
};

template <typename Logger>
concept CommandLogger = requires(Logger logger, std::string_view message) {
    { logger.write(message) } -> std::same_as<void>;
};

class RouteScope {
public:
    explicit RouteScope(int& active_routes) : active_routes_{active_routes} {
        ++active_routes_;
    }

    RouteScope(const RouteScope&) = delete;
    RouteScope& operator=(const RouteScope&) = delete;

    ~RouteScope() {
        --active_routes_;
    }

private:
    int& active_routes_;
};

class MemoryLogger {
public:
    void write(std::string_view message) {
        messages_.emplace_back(message);
    }

    [[nodiscard]] std::span<const std::string> messages() const {
        return messages_;
    }

private:
    std::vector<std::string> messages_;
};

std::expected<CommandKind, std::string> parse_kind(std::string_view word) {
    if (word == "start") {
        return CommandKind::start;
    }
    if (word == "stop") {
        return CommandKind::stop;
    }
    if (word == "status") {
        return CommandKind::status;
    }

    return std::unexpected("unknown command kind: " + std::string(word));
}

std::expected<Command, std::string> parse_command(std::string_view line) {
    const std::size_t space = line.find(' ');
    if (space == std::string_view::npos) {
        return std::unexpected("expected format: <start|stop|status> <target>");
    }

    const std::string_view kind_text = line.substr(0, space);
    const std::string_view target_text = line.substr(space + 1);
    if (target_text.empty()) {
        return std::unexpected("target is empty");
    }

    const auto kind = parse_kind(kind_text);
    if (!kind) {
        return std::unexpected(kind.error());
    }

    return Command{*kind, std::string(target_text)};
}

std::string_view to_text(CommandKind kind) {
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
    constexpr std::string_view allowed_targets[] = {"api", "database", "cache"};
    return std::ranges::find(allowed_targets, target) != std::ranges::end(allowed_targets);
}

template <CommandLogger Logger>
class CommandRouter {
public:
    CommandRouter(Logger& logger, int& active_routes)
        : logger_{logger}, active_routes_{active_routes} {}

    std::expected<RouteReport, std::string> route(std::span<const std::string_view> lines) {
        RouteScope scope{active_routes_};
        RouteReport report{};

        for (std::size_t index = 0; index < lines.size(); ++index) {
            const auto parsed = parse_command(lines[index]);
            if (!parsed) {
                ++report.failure_count;
                return std::unexpected(
                    "line " + std::to_string(index + 1) + ": " + parsed.error());
            }

            if (!is_allowed_target(parsed->target)) {
                ++report.failure_count;
                return std::unexpected(
                    "line " + std::to_string(index + 1) + ": target is not allowed: " +
                    parsed->target);
            }

            ++report.success_count;
            logger_.write(
                std::string(to_text(parsed->kind)) + " routed to " + parsed->target);
        }

        return report;
    }

private:
    Logger& logger_;
    int& active_routes_;
};

void run_tests() {
    {
        const auto parsed = parse_command("start api");
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
        std::cout << "[LOG] " << message << '\n';
    }

    std::cout << "[TESTS] router exercise passed\n";
    return 0;
}
