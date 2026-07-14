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
