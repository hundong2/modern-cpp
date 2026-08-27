// <cstddef>는 컨테이너 크기와 줄 수에 쓰는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 비소유 출력 인터페이스 std::ostream과 최종 출력 객체 std::cout을 선언한다.
#include <iostream>
// <sstream>은 메모리 안에 문자를 소유해 테스트 가능한 출력 포트가 되는 std::ostringstream를 선언한다.
#include <sstream>
// <string>은 로그 필드와 완성된 스냅샷을 소유하는 std::string을 선언한다.
#include <string>
// <syncstream>은 여러 스레드의 조각난 출력을 레코드 단위로 내보내는 std::osyncstream을 선언한다.
#include <syncstream>
// <thread>는 소멸 시 합류하는 RAII 스레드 소유자 std::jthread를 선언한다.
#include <thread>
// <utility>는 lvalue를 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>
// <vector>는 실행 중 개수가 정해지는 이벤트와 스레드 소유자를 연속 저장한다.
#include <vector>

// struct는 기본 접근이 public이라 입력 데이터 묶음처럼 불변식을 거의 갖지 않는 DTO에 알맞다.
struct AuditEvent {
    // std::string은 서비스 이름의 문자 버퍼와 수명을 소유한다.
    std::string service;
    // int는 오늘 예제 범위의 작은 상태 코드를 값으로 저장하는 기본 정수 타입이다.
    int code{};
};

// class는 기본 접근이 private라 출력 포트라는 구현 의존성을 감추는 데 알맞다.
class ConcurrentAuditLog {
public:
    // 생성자에는 반환형이 없다. explicit은 ostream 하나가 AuditLog로 암시 변환되는 것을 막는다.
    // output은 lvalue 참조로 빌릴 뿐 소유하지 않으므로 로그 객체보다 오래 살아야 한다.
    explicit ConcurrentAuditLog(std::ostream& output) noexcept
        // 멤버 초기화 목록은 같은 ostream 객체를 가리키는 참조 멤버를 바인딩한다.
        : output_{output} {}

    // void 반환형은 성공 레코드를 따로 돌려주지 않음을 뜻하고, const는 참조 대상을 제외한 자기 상태를 바꾸지 않는다.
    void write(const AuditEvent& event) const {
        // 대표 생성은 osyncstream(ostream&)이다. output_은 ostream lvalue이며 소유권을 넘기지 않는다.
        // record는 output_.rdbuf()를 감싼 동기 버퍼를 소유하고, 생성자는 반환값이 없다.
        // 같은 최종 streambuf를 감싼 osyncstream끼리는 emit이 섞이지 않는다. 생성 중 할당 실패는 예외가 될 수 있다.
        std::osyncstream record{output_};

        // operator<<(value)는 왼쪽 osyncstream lvalue와 각 값을 차례로 받으며 같은 ostream 계열 참조를 반환한다.
        // 문자열 리터럴은 읽기 전용 문자 배열에서 빌리고, event.service는 const string lvalue라 복사 소유하지 않고 문자를 읽는다.
        // code는 int 값, '\n'은 char 값이다. 이 식 뒤 문자는 record의 임시 동기 버퍼에 추가되고 output_에는 아직 없을 수 있다.
        // 기록량에 선형 시간이 들고 버퍼 확장 할당이 실패하면 예외가 날 수 있으나 event와 output_의 소유권은 유지된다.
        record << "service=" << event.service << " code=" << event.code << '\n';

        // 함수 블록 끝에서 record 소멸자가 버퍼를 원자적 레코드처럼 output_에 emit한다.
        // 이 RAII 수명 경계 덕분에 여러 스레드의 operator<< 조각이 서로 섞이지 않는다.
    }

private:
    // 참조 멤버는 ostream을 비소유로 가리킨다. private라 호출자는 출력 정책을 직접 바꿀 수 없다.
    std::ostream& output_;
};

// struct 결과는 두 값이 모두 공개여도 불변식이 깨지지 않는 단순 보고서다.
struct AuditSummary {
    std::size_t line_count{}; // size_t는 음수가 될 수 없는 개행 수를 값 초기화한다.
    bool all_services_present{}; // bool은 세 레코드가 모두 존재하는지 false로 값 초기화한다.
};

// const string&은 호출자 소유 문자 버퍼를 복사하지 않고 읽으며, 반환값은 새 보고서 값이다.
[[nodiscard]] AuditSummary inspect(const std::string& text) {
    std::size_t line_count{};
    // 범위 기반 for는 text의 각 char를 const 값으로 읽고, 비교 연산자 ==가 개행인지 bool로 만든다.
    for (const char character : text) {
        // if 조건 분기는 비교 결과가 true일 때만 전위 ++로 카운터를 1 늘린다.
        if (character == '\n') {
            ++line_count;
        }
    }

    // string::find(const char*)의 수신 객체는 const string lvalue text이고 호출 전 내용은 유지된다.
    // 각 인자는 null 종료 문자열을 가리키는 포인터 prvalue이며 복사·소유권 이전 없이 검색 패턴을 읽는다.
    // 반환 size_type은 첫 위치 또는 npos다. 세 호출 모두 결과만 비교하고 저장하지 않으며 O(text.size()) 시간, 추가 할당 0이다.
    // 호출 뒤 text와 포인터 대상은 변하지 않고, 예외를 던지지 않으며 반환 위치는 text가 변경되면 의미가 달라질 수 있다.
    const bool all_services_present{
        text.find("service=api code=200") != std::string::npos &&
        text.find("service=worker code=202") != std::string::npos &&
        text.find("service=db code=204") != std::string::npos};

    // AuditSummary{...}는 prvalue다. 반환 객체를 호출자 저장소에 직접 만드는 복사 생략이 가능하다.
    return AuditSummary{line_count, all_services_present};
}

int main() {
    // ostringstream 기본 생성자는 빈 소유 문자열 버퍼를 만들며 반환형이 없다. sink는 이름 있는 lvalue다.
    std::ostringstream sink{};
    // 중괄호 직접 초기화가 explicit 생성자를 올바르게 선택하고 sink를 비소유 참조로 주입한다.
    ConcurrentAuditLog log{sink};

    // vector(initializer_list)는 세 AuditEvent prvalue를 vector 소유 저장소에 복사/이동 구성한다.
    // 초기 size/capacity는 3 이상이며 할당 실패 시 예외, 성공 뒤 events가 원소 수명과 버퍼를 소유한다.
    std::vector<AuditEvent> events{
        AuditEvent{"api", 200}, AuditEvent{"worker", 202}, AuditEvent{"db", 204}};
    // 기본 생성한 vector<jthread>는 비어 있고 동적 버퍼를 아직 갖지 않을 수 있다.
    std::vector<std::jthread> workers{};

    // reserve(count)의 수신 객체는 빈 workers이고 인자 events.size()는 size_t prvalue 3이다.
    // events.size()는 원소 수를 size_type으로 반환하며 events를 바꾸지 않는다. reserve 반환형은 void라 결과는 없다.
    // 성공 뒤 workers.size()는 0을 유지하고 capacity만 3 이상이 된다. 재할당 시 기존 관찰자는 무효화되며 bad_alloc 가능성이 있다.
    workers.reserve(events.size());

    // event는 vector가 소유한 각 원소에 바인딩된 AuditEvent lvalue 참조다.
    for (AuditEvent& event : events) {
        // std::move(event)는 event lvalue를 AuditEvent&& xvalue로 바꾸고 그 자체로는 이동하지 않는다.
        // emplace_back의 수신 객체는 workers이며 lambda prvalue 하나를 jthread 생성자에 전달해 새 스레드를 즉시 시작한다.
        // lambda는 log를 참조로 빌리고 owned를 event에서 이동 구성해 string 버퍼 소유권을 worker callable로 옮긴다.
        // 반환형은 jthread&지만 무시한다. 성공 뒤 size가 1 늘고 재할당은 reserve 덕분에 없으며 스레드 생성 실패는 system_error다.
        workers.emplace_back([&log, owned = std::move(event)] {
            // owned는 lambda 객체가 소유해 스레드 종료까지 살며, 이름 있는 식이라 lvalue로 const& 매개변수에 바인딩된다.
            log.write(owned);
        });
    }

    // worker는 vector 원소인 jthread lvalue 참조다. 모든 스레드가 끝나야 sink를 단일 스레드에서 읽을 수 있다.
    for (std::jthread& worker : workers) {
        // joinable()은 인자 없이 bool을 반환하고 worker 상태를 바꾸지 않는다. true일 때만 join 전제조건을 만족한다.
        if (worker.joinable()) {
            // join()은 인자가 없고 void를 반환한다. 호출 전 worker는 현재 스레드가 아닌 실행 스레드를 소유해야 한다.
            // 성공 뒤 실행 완료가 현재 스레드에 동기화되고 worker는 non-joinable이 된다. 잘못된 상태면 system_error가 날 수 있다.
            worker.join();
        }
    }

    // str() const&는 sink의 현재 문자 시퀀스를 새 std::string 값으로 복사 반환하고 sink 상태는 유지한다.
    // 반환 prvalue로 snapshot을 직접 초기화하며 문자 수에 선형 시간·공간과 할당 실패 가능성이 있다.
    const std::string snapshot{sink.str()};
    // inspect 호출은 snapshot lvalue를 const&로 빌리고 새 AuditSummary prvalue를 반환해 summary를 직접 초기화한다.
    const AuditSummary summary{inspect(snapshot)};

    // cout 삽입 연산은 size_t, 문자, bool 값을 순서대로 쓰고 같은 ostream&를 반환해 연쇄한다.
    // 호출 뒤 cout 버퍼/상태만 바뀌며 인자는 유지된다. 출력 오류는 기본적으로 상태 비트로 남고 반환 참조는 무시한다.
    std::cout << summary.line_count << ' ' << summary.all_services_present << '\n';
    // 정확히 세 레코드가 섞이지 않고 모두 존재하면 성공 종료 코드 0을 운영체제에 반환한다.
    return summary.line_count == 3U && summary.all_services_present ? 0 : 1;
}
