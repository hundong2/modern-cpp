#include <cassert>      // assert로 예제의 예상 결과를 실행 중 검증한다.
#include <charconv>     // 문자열을 정수로 바꾸는 std::from_chars를 선언한다.
#include <concepts>     // 템플릿 요구 조건에 쓰는 std::same_as를 제공한다.
#include <expected>     // 성공값 또는 오류값을 담는 std::expected를 제공한다.
#include <iostream>     // 표준 출력 cout과 오류 출력 cerr를 사용한다.
#include <span>         // 연속 메모리를 소유하지 않고 바라보는 std::span을 사용한다.
#include <string>       // 문자 버퍼를 소유하는 std::string을 사용한다.
#include <string_view>  // 문자열을 소유하지 않고 읽는 std::string_view를 사용한다.
#include <system_error> // from_chars의 오류 코드 std::errc를 확인한다.
#include <utility>      // std::move로 이동 후보인 xvalue를 만든다.
#include <vector>       // 크기가 변하는 연속 컨테이너 std::vector를 사용한다.

struct TemperatureReading {
    // struct의 멤버는 기본 public이다. 단순 데이터 묶음은 값처럼 복사·이동할 수 있다.
    int sensor_id{}; // {}는 0으로 값 초기화한다. 이 멤버 식은 이름이 있으므로 lvalue다.
    int celsius{};
};

struct Alert {
    int sensor_id{};      // 센서 번호를 0으로 값 초기화한다.
    int celsius{};        // 온도를 0으로 값 초기화한다.
    std::string message;  // 경고 문자의 저장 공간과 수명을 직접 소유한다.
};

// template은 Publisher라는 실제 타입을 사용하는 코드를 컴파일 시점에 만든다.
template <typename Publisher>
// concept는 publish(Alert) 호출이 가능하고 반환형이 void인지 검사하는 타입 조건이다.
concept AlertPublisher = requires(Publisher publisher, Alert alert) {
    { publisher.publish(alert) } -> std::same_as<void>;
};
//concept는 requires 키워드로 선언하며, requires 뒤에 오는 표현식이 모두 만족해야 한다.

class ProcessingSession {
public:
    // 생성자는 반환형이 없다. explicit은 int에서 세션으로의 암시적 변환을 막는다.
    // 멤버 초기화 목록은 참조 멤버를 생성 시점에 원본 카운터에 바인딩한다.
    explicit ProcessingSession(int& active_sessions) : active_sessions_{active_sessions} {
        // ++는 원본 정수 저장 위치를 읽고 1을 더해 다시 저장한다.
        ++active_sessions_;
    }

    // = delete는 복사 생성과 복사 대입을 호출하면 컴파일 오류가 나게 한다.
    ProcessingSession(const ProcessingSession&) = delete;
    ProcessingSession& operator=(const ProcessingSession&) = delete;

    // 소멸자는 반환형이 없고 객체 수명이 끝날 때 자동 호출되어 RAII 정리를 수행한다.
    ~ProcessingSession() {
        --active_sessions_;
    }

private:
    // class 상태는 기본 private이며, 참조 대상이 이 객체보다 오래 살아야 한다.
    int& active_sessions_;
};

class MemoryAlertPublisher {
public:
    // Alert를 값으로 받아 독립 객체를 만든 뒤 vector로 소유권을 이동한다.
    void publish(Alert alert) {
        alerts_.push_back(std::move(alert));
    }

    // [[nodiscard]]는 반환 뷰를 무시할 때 경고를 유도하고, 뒤 const는 멤버를 바꾸지 않는다.
    [[nodiscard]] std::span<const Alert> alerts() const {
        return alerts_;
    }

private:
    // vector가 Alert 원소의 메모리와 수명을 소유한다.
    std::vector<Alert> alerts_;
};

struct BatchReport {
    int parsed_count{};
    int alert_count{};
};

std::expected<int, std::string> parse_int(std::string_view text, std::string_view field_name) {
    // expected의 첫 타입 int는 성공값, 둘째 string은 오류값이다.
    // text는 원본 문자를 소유하지 않는 작은 뷰를 값으로 복사해 받는다(보통 포인터+길이 두 단어).
    if (text.empty()) {
        // unexpected 임시 객체는 prvalue이며 expected의 오류 저장소를 직접 초기화할 수 있다.
        return std::unexpected(std::string(field_name) + " is empty");
    }

    int value{};
    // const char*는 문자를 바꾸지 않는 주소다. 포인터 자체는 지역 변수에 복사된다.
    const char* first = text.data();
    const char* last = text.data() + text.size();
    // 구조적 바인딩은 from_chars가 반환한 결과 객체의 두 멤버에 읽기 좋은 이름을 붙인다.
    const auto [parsed_until, error] = std::from_chars(first, last, value);

    if (error != std::errc{} || parsed_until != last) {
        // ||는 왼쪽이 참이면 오른쪽 비교를 생략하는 단락 평가 논리합이다.
        return std::unexpected(std::string(field_name) + " must be an integer");
    }

    return value;
}

std::expected<TemperatureReading, std::string> parse_reading(std::string_view line) {
    // find는 ':' 위치를 반환하며 없으면 특별한 npos 값을 반환한다.
    const std::size_t separator = line.find(':');
    if (separator == std::string_view::npos) {
        return std::unexpected("expected format is sensor_id:celsius");
    }

    // 두 string_view는 원본 line의 일부를 빌릴 뿐 문자를 복사하거나 소유하지 않는다.
    const std::string_view sensor_text = line.substr(0, separator);
    const std::string_view celsius_text = line.substr(separator + 1);

    const auto sensor_id = parse_int(sensor_text, "sensor_id");
    if (!sensor_id) {
        // !는 expected의 성공 여부를 뒤집어 오류 상태인지 검사한다.
        return std::unexpected(sensor_id.error());
    }

    const auto celsius = parse_int(celsius_text, "celsius");
    if (!celsius) {
        return std::unexpected(celsius.error());
    }

    // *expected는 내부 int를 가리키는 lvalue이고 중괄호는 결과 구조체를 직접 초기화한다.
    return TemperatureReading{*sensor_id, *celsius};
}

template <AlertPublisher Publisher>
class ReadingService {
public:
    ReadingService(Publisher& publisher, int high_temperature_celsius, int& active_sessions)
        // 참조 멤버는 객체를 복사·소유하지 않고 기존 객체 주소를 보관한다.
        : publisher_{publisher},
          high_temperature_celsius_{high_temperature_celsius},
          active_sessions_{active_sessions} {}

    std::expected<BatchReport, std::string> process(std::span<const std::string_view> lines) {
        // 지역 RAII 객체는 정상 반환과 조기 반환 모두에서 소멸자가 호출된다.
        ProcessingSession session{active_sessions_};
        BatchReport report{};

        for (std::size_t index = 0; index < lines.size(); ++index) {
            // CPU 관점에서는 대략 인덱스 비교→조건 분기→증가로 반복되지만 실제 명령은 최적화에 따라 달라진다.
            const auto parsed = parse_reading(lines[index]);
            if (!parsed) {
                return std::unexpected(
                    "line " + std::to_string(index + 1) + ": " + parsed.error());
            }

            ++report.parsed_count;
            if (parsed->celsius >= high_temperature_celsius_) {
                // ->는 expected 안 객체의 멤버에 접근한다. 비교 결과로 경고 경로를 분기한다.
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
    // 중괄호 블록마다 지역 객체 수명을 짧게 제한해 각 검증 사례를 독립시킨다.
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
    // main은 운영체제가 호출하는 프로그램 진입 함수이며 int 종료 코드를 반환한다.
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
        // cerr는 오류용 표준 스트림이고 <<는 오른쪽 값을 차례로 스트림에 보낸다.
        std::cerr << "[ERROR] " << report.error() << '\n';
        return 1;
    }

    std::cout << "[REPORT]\n";
    std::cout << "  parsed readings: " << report->parsed_count << '\n';
    std::cout << "  alert count    : " << report->alert_count << '\n';
    std::cout << "  active sessions: " << active_sessions << '\n';

    for (const Alert& alert : publisher.alerts()) {
        // 범위 for의 const 참조는 string을 포함한 Alert 전체 복사를 피한다.
        std::cout << "[ALERT] sensor=" << alert.sensor_id
                  << ", celsius=" << alert.celsius
                  << ", message=" << alert.message << '\n';
    }

    std::cout << "[TESTS] architecture exercise passed\n";
    return 0;
}
