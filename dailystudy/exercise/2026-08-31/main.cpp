#include <cassert>      // assert 매크로로 학습 예제의 후조건을 검증한다.
#include <format>       // std::format의 타입 안전한 자리표시자 기반 문자열 생성을 선언한다.
#include <iostream>     // std::cout과 스트림 삽입 연산자를 선언한다.
#include <string>       // 문자를 소유하는 std::string을 선언한다.
#include <string_view>  // 문자열을 소유하지 않고 빌려 보는 std::string_view를 선언한다.
#include <utility>      // std::move로 이름 있는 객체를 xvalue로 바꾼다.
#include <vector>       // 연속 동적 배열 std::vector를 선언한다.

// struct는 기본 접근이 public이라 아래 데이터 멤버를 호출자가 직접 초기화할 수 있다.
// 실무의 입력 DTO처럼 단순히 값을 운반하고, 불변식은 아래 class가 검사·표현한다.
struct DeploymentEvent {
    // int는 여기서 서비스 식별자를 담는 기본 정수 타입이고 {}는 0으로 값 초기화한다.
    int service_id{};
    // string_view는 문자열을 소유하지 않는다. 오늘은 정적 수명의 문자열 리터럴만 가리킨다.
    std::string_view service_name{};
    // 음수가 아닌 밀리초라는 도메인 규칙은 formatter가 확인한다.
    int latency_ms{};
};

// class는 기본 접근이 private이다. 표현 정책과 환경 문자열을 객체 안에 감춘다.
class DeploymentFormatter {
public:
    // 생성자에는 반환형이 없다. explicit은 string 하나가 formatter로 암시 변환되는 것을 막는다.
    // environment는 값 매개변수라 호출자가 lvalue를 주면 복사하고 rvalue를 주면 이동해 이 함수가 소유한다.
    explicit DeploymentFormatter(std::string environment)
        // std::move(environment)는 이름 있는 lvalue 매개변수를 xvalue로 바꾼다.
        // string 이동 생성자는 보통 버퍼 소유권을 environment_로 넘긴다. 원본은 유효하지만 값은 미지정이다.
        : environment_{std::move(environment)} {}

    // [[nodiscard]]는 반환 진단 문자열을 무시하면 컴파일러가 경고할 수 있게 한다.
    // const 참조 매개변수는 event를 복사·소유하지 않고 함수 호출 동안 읽기만 한다.
    [[nodiscard]] std::string render(const DeploymentEvent& event) const {
        // assert는 <cassert>의 매크로이며 NDEBUG 빌드에서는 식 전체가 제거될 수 있어 외부 입력 검증 수단이 아니다.
        assert(event.service_id > 0);
        assert(event.latency_ms >= 0);

        // std::format의 대표 형태는 template<class... Args> string format(format_string<Args...>, Args&&...)이다.
        // 첫 인자는 컴파일 시간에 검사되는 문자열 리터럴 lvalue이고, 뒤 네 인자는 각각 const string lvalue,
        // string_view lvalue, int lvalue, int lvalue다. 여기서는 읽기만 하며 소유권을 옮기지 않는다.
        // 반환형 string은 완성된 문자를 새로 소유하고, 호출 뒤 formatter와 event는 바뀌지 않는다.
        // 시간·추가 공간은 결과 문자 수에 선형이고 할당 실패는 bad_alloc, 형식 오류는 format_error가 될 수 있다.
        return std::format("[{}] service={} id={} latency={}ms",
                           environment_, event.service_name, event.service_id, event.latency_ms);
    }

private:
    // private 멤버는 클래스 외부에서 직접 바꿀 수 없다. formatter가 환경 이름의 수명을 소유한다.
    std::string environment_;
};

// 이 서비스는 여러 이벤트를 한 번에 표현한다. 입력과 표현을 분리하면 로그·HTTP 응답 등 출력 정책을 교체하기 쉽다.
class DeploymentReportService {
public:
    // formatter 값 매개변수는 소유권 경계를 명시하고, 멤버 초기화 목록이 저장 객체를 직접 만든다.
    explicit DeploymentReportService(DeploymentFormatter formatter)
        // formatter는 이름 있는 lvalue이므로 move가 xvalue로 바꾸고 formatter_의 이동 생성자를 선택하게 한다.
        : formatter_{std::move(formatter)} {}

    // vector의 const 참조는 호출자가 가진 이벤트 저장소를 빌려 읽으며 수명을 연장하지 않는다.
    [[nodiscard]] std::vector<std::string>
    render_all(const std::vector<DeploymentEvent>& events) const {
        // 빈 vector 직접 초기화는 저장 원소가 없고 capacity도 구현 정의의 빈 상태로 시작한다.
        std::vector<std::string> lines{};

        // size()의 대표 형태는 size_type size() const noexcept다. 인자 없이 원소 수를 반환하고 events는 유지된다.
        // reserve(count)는 최소 count 용량을 요청하고 size는 0으로 유지한다. 재할당 시 기존 포인터·참조·반복자는
        // 무효화되지만 지금은 원소가 없다. O(count) 이하 할당, 실패 시 예외와 강한 보장을 기대할 수 있다.
        lines.reserve(events.size());

        // 범위 for는 events.begin()/end()가 정한 각 원소를 const 참조로 바인딩해 복사하지 않는다.
        for (const DeploymentEvent& event : events) {
            // render(event)는 const 참조 하나를 받고 소유 string prvalue를 반환한다. push_back(T&&) 오버로드가
            // 그 임시 문자열을 vector 끝으로 이동한다. 성공하면 size가 1 증가하며 capacity 초과 재할당은 기존
            // lines 원소의 포인터·참조·반복자를 모두 무효화한다. 분할 상환 O(1), 할당 실패 시 예외가 가능하다.
            lines.push_back(formatter_.render(event));
        }

        // lines는 지역 lvalue지만 값 반환 시 NRVO 또는 암시적 이동 대상이다. 반환 vector가 문자열 소유권을 이어받는다.
        return lines;
    }

private:
    // formatter_는 서비스가 소유하므로 서비스 수명 동안 환경 문자열도 유효하다.
    DeploymentFormatter formatter_;
};

int main() {
    // string 리터럴로 string 임시(prvalue)를 만들고 explicit 생성자를 직접 초기화로 올바르게 호출한다.
    DeploymentReportService service{DeploymentFormatter{std::string{"prod"}}};

    // initializer_list 생성자는 세 DTO를 vector가 값으로 소유하게 한다. string_view는 정적 문자열 리터럴을 본다.
    const std::vector<DeploymentEvent> events{
        DeploymentEvent{.service_id = 7, .service_name = "billing", .latency_ms = 42},
        DeploymentEvent{.service_id = 8, .service_name = "search", .latency_ms = 17},
        DeploymentEvent{.service_id = 9, .service_name = "checkout", .latency_ms = 63},
    };

    // render_all 호출의 vector<string> prvalue가 lines를 직접 초기화한다. 복사 생략 또는 이동으로 소유권이 정착한다.
    const std::vector<std::string> lines{service.render_all(events)};

    // size()는 lines를 바꾸지 않고 size_type 3을 반환한다. assert가 참이면 실행 상태는 바뀌지 않는다.
    assert(lines.size() == 3U);

    // 각 line은 const string lvalue 참조라 출력 중 복사되지 않으며 vector가 반복 전체에서 수명을 보장한다.
    for (const std::string& line : lines) {
        // operator<<는 cout과 line을 소유하지 않고 문자를 출력한 뒤 ostream&를 반환해 '\n' 삽입을 연쇄한다.
        // cout의 출력 위치·버퍼가 변하고 line은 유지된다. 시간은 문자 수에 선형이며 I/O 실패는 stream 상태에 남는다.
        std::cout << line << '\n';
    }

    // 0은 운영체제에 정상 종료를 알리는 int prvalue다. 지역 객체는 역순으로 파괴된다.
    return 0;
}
