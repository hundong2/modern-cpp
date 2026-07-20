/*
[기초 문법부터 읽는 순서]
1. #include는 표준 라이브러리 기능을 가져옵니다. <iostream>은 화면 출력,
   <vector>는 여러 값 저장, <string>은 문자열에 사용합니다.
2. struct는 관련 변수를 한 타입으로 묶습니다. int는 정수이며 멤버 뒤의 {}
   는 0으로 안전하게 초기화한다는 뜻입니다.
3. 함수 선언의 왼쪽은 반환 타입, 괄호 안은 매개변수입니다. const T&는
   원본을 복사하지 않고 읽기만 하겠다는 뜻입니다.
4. if는 조건 분기, for는 반복, return은 함수 결과를 호출한 곳에 돌려줍니다.
5. string_view와 span은 데이터를 소유하지 않고 빌려 봅니다. 따라서 원본이
   먼저 사라지면 안 됩니다.
6. expected<값, 오류>는 성공 값 또는 실패 이유 중 하나를 담습니다. !result는
   실패 여부를 검사하고, *result와 result->member는 성공 값을 읽습니다.
7. template과 concept는 publish 함수를 가진 타입만 서비스에 연결되도록
   컴파일 시점에 검사합니다.
8. 생성자에서 횟수를 늘리고 소멸자에서 줄이는 ProcessingSession은 스코프를
   벗어나면 자동 정리되는 RAII의 예입니다.
*/

/*
Daily Modern C++ Exercise - 2026-07-14

Theme:
  Build a tiny temperature alert pipeline while practicing architecture
  boundaries and Modern C++ syntax.

What to notice:
  - std::string_view and std::span borrow data without owning it.
  - std::expected returns either a value or an error.
  - A concept documents the required shape of an adapter.
  - RAII keeps cleanup reliable even when a function returns early.
*/

#include <cassert>
#include <charconv>
#include <concepts>
#include <expected>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

struct TemperatureReading {
    int sensor_id{};
    int celsius{};
};

struct Alert {
    int sensor_id{};
    int celsius{};
    std::string message;
};

template <typename Publisher>
concept AlertPublisher = requires(Publisher publisher, Alert alert) {
    { publisher.publish(alert) } -> std::same_as<void>;
};

class ProcessingSession {
public:
    explicit ProcessingSession(int& active_sessions) : active_sessions_{active_sessions} {
        ++active_sessions_;
    }

    ProcessingSession(const ProcessingSession&) = delete;
    ProcessingSession& operator=(const ProcessingSession&) = delete;

    ~ProcessingSession() {
        --active_sessions_;
    }

private:
    int& active_sessions_;
};

class MemoryAlertPublisher {
public:
    void publish(Alert alert) {
        alerts_.push_back(std::move(alert));
    }

    [[nodiscard]] std::span<const Alert> alerts() const {
        return alerts_;
    }

private:
    std::vector<Alert> alerts_;
};

struct BatchReport {
    int parsed_count{};
    int alert_count{};
};

std::expected<int, std::string> parse_int(std::string_view text, std::string_view field_name) {
    if (text.empty()) {
        return std::unexpected(std::string(field_name) + " is empty");
    }

    int value{};
    const char* first = text.data();
    const char* last = text.data() + text.size();
    const auto [parsed_until, error] = std::from_chars(first, last, value);

    if (error != std::errc{} || parsed_until != last) {
        return std::unexpected(std::string(field_name) + " must be an integer");
    }

    return value;
}

std::expected<TemperatureReading, std::string> parse_reading(std::string_view line) {
    const std::size_t separator = line.find(':');
    if (separator == std::string_view::npos) {
        return std::unexpected("expected format is sensor_id:celsius");
    }

    const std::string_view sensor_text = line.substr(0, separator);
    const std::string_view celsius_text = line.substr(separator + 1);

    const auto sensor_id = parse_int(sensor_text, "sensor_id");
    if (!sensor_id) {
        return std::unexpected(sensor_id.error());
    }

    const auto celsius = parse_int(celsius_text, "celsius");
    if (!celsius) {
        return std::unexpected(celsius.error());
    }

    return TemperatureReading{*sensor_id, *celsius};
}

template <AlertPublisher Publisher>
class ReadingService {
public:
    ReadingService(Publisher& publisher, int high_temperature_celsius, int& active_sessions)
        : publisher_{publisher},
          high_temperature_celsius_{high_temperature_celsius},
          active_sessions_{active_sessions} {}

    std::expected<BatchReport, std::string> process(std::span<const std::string_view> lines) {
        ProcessingSession session{active_sessions_};
        BatchReport report{};

        for (std::size_t index = 0; index < lines.size(); ++index) {
            const auto parsed = parse_reading(lines[index]);
            if (!parsed) {
                return std::unexpected(
                    "line " + std::to_string(index + 1) + ": " + parsed.error());
            }

            ++report.parsed_count;
            if (parsed->celsius >= high_temperature_celsius_) {
                ++report.alert_count;
                publisher_.publish(Alert{
                    parsed->sensor_id,
                    parsed->celsius,
                    "temperature is above the service threshold",
                });
            }
        }

        return report;
    }

private:
    Publisher& publisher_;
    int high_temperature_celsius_{};
    int& active_sessions_;
};

void run_tests() {
    {
        const auto parsed = parse_reading("101:84");
        assert(parsed.has_value());
        assert(parsed->sensor_id == 101);
        assert(parsed->celsius == 84);
    }

    {
        const auto parsed = parse_reading("broken-line");
        assert(!parsed.has_value());
    }

    {
        MemoryAlertPublisher publisher;
        int active_sessions = 0;
        ReadingService service{publisher, 80, active_sessions};

        const std::vector<std::string_view> lines = {
            "101:72",
            "102:81",
            "103:95",
        };

        const auto report = service.process(lines);
        assert(report.has_value());
        assert(report->parsed_count == 3);
        assert(report->alert_count == 2);
        assert(publisher.alerts().size() == 2);
        assert(active_sessions == 0);
    }

    {
        MemoryAlertPublisher publisher;
        int active_sessions = 0;
        ReadingService service{publisher, 80, active_sessions};

        const std::vector<std::string_view> lines = {
            "101:72",
            "bad-input",
            "103:95",
        };

        const auto report = service.process(lines);
        assert(!report.has_value());
        assert(active_sessions == 0);
    }
}

int main() {
    run_tests();

    MemoryAlertPublisher publisher;
    int active_sessions = 0;
    ReadingService service{publisher, 80, active_sessions};

    const std::vector<std::string_view> batch = {
        "201:73",
        "202:88",
        "203:91",
        "204:64",
    };

    const auto report = service.process(batch);
    if (!report) {
        std::cerr << "[ERROR] " << report.error() << '\n';
        return 1;
    }

    std::cout << "[REPORT]\n";
    std::cout << "  parsed readings: " << report->parsed_count << '\n';
    std::cout << "  alert count    : " << report->alert_count << '\n';
    std::cout << "  active sessions: " << active_sessions << '\n';

    for (const Alert& alert : publisher.alerts()) {
        std::cout << "[ALERT] sensor=" << alert.sensor_id
                  << ", celsius=" << alert.celsius
                  << ", message=" << alert.message << '\n';
    }

    std::cout << "[TESTS] architecture exercise passed\n";
    return 0;
}
