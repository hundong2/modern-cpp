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
    // enum class는 정수로 암시 변환되지 않아 다른 숫자와 실수로 섞이는 일을 막는다.
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
    // 실행 코드가 아니라 컴파일 시 "이 식이 유효하고 반환형이 void인가"를 검사한다.
    { logger.write(message) } -> std::same_as<void>;
};

class RouteScope {
public:
    explicit RouteScope(int& active_routes) : active_routes_{active_routes} { // &는 원본 카운터에 바인딩한다.
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

    // 문자열 연결 결과는 임시 std::string(prvalue)이며 오류 expected를 이동/직접 초기화한다.
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

    const auto kind = parse_kind(kind_text); // 반환 prvalue로 const 지역 객체를 초기화한다.
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
            // 가상 호출이 아니라 템플릿으로 Logger 타입이 확정되어 컴파일러가 인라인할 여지가 크다.
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
