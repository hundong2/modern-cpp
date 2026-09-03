// <cstddef>는 컨테이너 크기를 표현하는 부호 없는 표준 타입 std::size_t를 선언한다.
#include <cstddef>
// <future>는 한 번 값을 게시하는 std::promise와 한 번 결과를 소비하는 std::future를 선언한다.
#include <future>
// <iostream>은 학습 결과를 기록할 표준 출력 객체 std::cout과 스트림 삽입 연산을 선언한다.
#include <iostream>
// <numeric>은 반복자 구간을 왼쪽부터 누적하는 std::accumulate를 선언한다.
#include <numeric>
// <string>은 배치 이름의 문자 저장소를 소유하는 std::string을 선언한다.
#include <string>
// <thread>는 실행 스레드의 수명을 RAII로 소유하는 std::jthread를 선언한다.
#include <thread>
// <utility>는 이름 붙은 객체를 xvalue 식으로 바꾸는 std::move를 선언한다.
#include <utility>
// <vector>는 점수 원소를 연속 저장소에 소유하는 std::vector를 선언한다.
#include <vector>

// struct는 별도 접근 지정자가 없으면 멤버가 public이다. 계산 스레드가 완성해 소비자에게 넘길 값 DTO다.
struct BatchReport {
    std::string name;       // 클래스 타입 string이 배치 이름 문자를 소유한다.
    long long total{};      // 기본 타입 long long의 {} 값 초기화는 0을 만든다.
    std::size_t item_count{}; // size_t{}도 0이며 음수가 될 수 없는 원소 수를 나타낸다.

    // 생성자는 반환형이 없다. explicit은 세 인자의 `BatchReport r = {"x", 1, 1};` 같은
    // copy-list-initialization을 막고 `BatchReport r{"x", 1, 1};` 직접 초기화만 허용한다.
    // std::move의 대표 시그니처는 template<class T> remove_reference_t<T>&& move(T&&) noexcept다.
    // received_name은 이름 있는 string lvalue라 T=string&이고, 유일한 인자는 소유하지 않는 전달 참조에
    // 바인딩된다. 이 인자의 반환형 string&& xvalue는 같은 객체를 가리키며 즉시 name 이동 생성에 사용한다.
    // move 호출 자체는 O(1)·무할당·noexcept이고 상태를 바꾸거나 수명을 늘리지 않는다. 선택된 string 이동
    // 생성자는 반환값 없이 name을 구성하고 원본을 유효하지만 값이 미지정된 상태로 둔다. 기본 allocator에서는
    // 상수 시간·noexcept지만 SSO/저장소 이전 방식은 구현 세부이며, 두 객체 동시 접근은 따로 동기화해야 한다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    explicit BatchReport(std::string received_name,
                         long long received_total,
                         std::size_t received_count) noexcept
        : name{std::move(received_name)},
          total{received_total},
          item_count{received_count} {}
};

// 함수의 반환형은 long long, 매개변수는 const vector<int>&다. 참조는 호출자 vector를 소유하지 않고
// 기존 객체에 바인딩하며 const 때문에 원소·크기·용량을 이 함수에서 바꿀 수 없다. 포인터와 참조는 그 자체로
// 수명을 연장하지 않으므로 호출 동안 values가 살아 있어야 한다. 여기서는 worker lambda가 vector를 값으로 소유한다.
[[nodiscard]] long long sum_scores(const std::vector<int>& values) {
    // begin()/end()의 수신자는 살아 있는 const vector<int> lvalue values이고 데이터 인자는 없다. 선택된 const
    // overload들은 각각 첫 원소와 past-the-end를 가리키는 const_iterator 값을 O(1)·무할당·noexcept로 반환해
    // accumulate의 두 입력에 사용한다. vector 상태는 그대로이며 구조 변경 전까지만 반복자가 유효하다.
    // accumulate(first,last,init)는 InputIt=vector<int>::const_iterator, T=long long으로 추론된다. 첫 두 인자는
    // 반열린 유효 구간 [first,last), 셋째 0LL은 long long prvalue 초기값이다. long long 합을 값으로 반환해
    // 호출자가 사용하며 입력은 바뀌지 않는다. 시간 O(values.size()), 추가 공간 O(1), 할당·무효화는 없다.
    // 덧셈이 long long 범위를 벗어나지 않아야 하며 표준 자체는 병렬 실행·스레드 동기화를 제공하지 않는다.
    // 대표 문서: ../standard-library/algorithms-and-ranges.md
    return std::accumulate(values.begin(), values.end(), 0LL);
}

// class는 기본 접근이 private이다. 생산 promise, 소비 future, 실행 jthread를 한 수명 단위로 캡슐화한다.
class PendingBatch {
private:
    // 선언 순서의 역순으로 파괴되므로 worker_가 먼저 합류한 뒤 result_가 공유 상태를 놓는다.
    std::future<BatchReport> result_;
    std::jthread worker_;

public:
    // 이름과 점수 vector를 값으로 받아 호출자가 복사 또는 이동을 명시하게 한다. explicit은 암시적 변환 경로를 막는다.
    explicit PendingBatch(std::string name, std::vector<int> values)
        // future 기본 생성자는 수신·인자·반환값 없이 연관 공유 상태가 없는 invalid result_를 만든다.
        // jthread 기본 생성자도 실행 스레드가 없는 non-joinable worker_를 만든다. 둘 다 상수 시간·무할당·
        // noexcept이며 다른 객체나 참조를 무효화하지 않고 동기화 효과도 만들지 않는다.
        : result_{}, worker_{} {
        // promise<BatchReport>() 기본 생성자의 목적 객체는 새 producer이고 입력 인자와 반환값은 없다. 성공하면
        // BatchReport 한 값을 위한 새 공유 상태의 생산자 끝점을 유일 소유한다. 보통 동적 할당이 필요해 bad_alloc
        // 또는 공유 상태 생성 오류를 던질 수 있다. 아직 값은 없고 consumer도 없다. 표준이 지정한 get_future와
        // setter 계열 호출 사이는 data race가 없고 setter 계열끼리는 상호 배제되는 것처럼 동작한다. 그렇다고 같은
        // handle의 이동·소멸 등 임의 조작까지 동시 안전한 것은 아니며, 논리적으로는 한 setter만 성공할 수 있다.
        // 대표 문서: ../standard-library/concurrency-time-filesystem.md
        std::promise<BatchReport> producer{};

        // get_future()의 수신자는 아직 future를 꺼내지 않은 유효한 promise<BatchReport> lvalue producer이고 인자는
        // 없다. 연관된 std::future<BatchReport> prvalue를 반환해 result_ 이동 대입의 입력으로 쓴다. 표준은 이 호출의
        // 복잡도·할당 여부를 별도로 보장하지 않으며, 일반 구현에서는 기존 shared state handle을 연결하는 상수 규모다.
        // 호출 뒤 producer는 같은 생산 상태를 유지하고 consumer 끝점 하나가 생긴다. 두 번째 호출 또는 no-state면
        // future_error가 발생하며, shared state나 producer를 무효화하지 않는다. 이 호출 자체는 값을 ready로 만들지 않는다.
        // future 이동 대입의 수신자는 invalid result_이고 인자는 반환 prvalue가 바인딩된 future&&다. 이전 상태는
        // 없고 새 shared-state handle을 넘겨받아 valid가 된다. 반환형 future&는 버리며 원본 임시는 invalid가 된다.
        // future 이동 대입은 noexcept지만 표준은 복잡도·할당 여부를 별도로 보장하지 않는다. 이 호출 뒤 공유 결과
        // 객체의 수명은 result_ handle로 이어진다.
        result_ = producer.get_future();

        // 세 std::move 호출은 각각 promise, string, vector lvalue를 같은 객체의 && xvalue로 바꾼다. 함수는 각 인자를
        // 빌릴 뿐 반환 참조를 즉시 lambda init-capture 초기화에 쓰며 O(1)·무할당·noexcept다. 실제 이동 생성 후
        // 세 바깥 원본은 유효하지만 값/상태가 미지정된다. promise 원본은 no-state, vector/string 저장소 이전의
        // 구체 방식은 구현별이며 기존 원소·문자 관찰자를 보관하지 않는다. 공유 객체 동시 접근을 자동 보호하지 않는다.
        // jthread(F&&)에서 F는 closure 타입, 유일한 함수 인자는 closure prvalue다. 새 스레드가 이동 저장한 closure를
        // 인자 없이 호출하며 생성식 전체는 jthread prvalue다. 성공하면 joinable 스레드를 소유하고, OS 자원 실패 시
        // system_error를 던진다. 시간 상한은 스케줄러에 따라 없고 내부 할당 가능성이 있으며 thread 시작은 작업과
        // 동시 진행될 수 있다. closure 밖으로 예외가 빠지면 프로그램이 종료되므로 아래 불변식으로 예외 조건을 배제한다.
        // jthread 이동 대입의 수신자는 non-joinable worker_, 인자는 임시 jthread&&다. 기존 합류 대상 없이 소유권을
        // 넘겨받고 jthread& 반환은 버린다. 임시는 non-joinable, worker_는 joinable이다. 호출은 noexcept이나 기존
        // 수신자가 joinable이면 먼저 stop 요청과 join으로 오래 막힐 수 있다. 여기서는 기본 생성 상태라 즉시 이전한다.
        // 대표 문서: ../standard-library/concurrency-time-filesystem.md
        worker_ = std::jthread{
            [producer = std::move(producer),
             name = std::move(name),
             values = std::move(values)]() mutable {
                const long long total{sum_scores(values)};

                // size()의 수신자는 closure가 소유한 const가 아닌 vector<int> lvalue values이고 인자는 없다.
                // size_type 값을 O(1)·무할당·noexcept로 반환해 BatchReport의 셋째 인자에 사용한다. vector와
                // 반복자·원소 수명은 그대로이며 동시 구조 변경은 없고 반환값은 현재 원소 수의 스냅샷이다.
                const std::size_t count{values.size()};

                // set_value(BatchReport&&)의 수신자는 아직 만족되지 않은 유효 promise<BatchReport> lvalue다. 유일한
                // 인자는 BatchReport{std::move(name), total, count} prvalue로 공유 상태에 이동 저장할 소유 값이다.
                // 반환형 void라 값은 없고, 성공 뒤 shared state가 ready가 되어 future::get 대기를 깨우며 이 호출 전
                // 작업과 성공적으로 반환한 get 뒤 작업 사이에 동기화가 생긴다. 이미 만족/no-state면 future_error,
                // 값 이동이 던지면 그 예외가 전달될 수 있다. 여기서는 단 한 번 호출하고 Report 이동이 noexcept다.
                // 작업량은 값 이동과 대기자 깨우기에 따르며 vector 관찰자를 무효화하지 않고 producer만 생산권을 유지한다.
                producer.set_value(BatchReport{std::move(name), total, count});
            }};
    }

    // promise/future와 jthread는 독점 handle이므로 이 aggregate 책임도 복제하지 않는다. 이동까지 막아 take 전에는
    // result_가 valid이고 take 성공 뒤에는 invalid라는 상태 전이와 worker_ 파괴 순서를 단순하게 유지한다.
    PendingBatch(const PendingBatch&) = delete;
    PendingBatch& operator=(const PendingBatch&) = delete;
    PendingBatch(PendingBatch&&) = delete;
    PendingBatch& operator=(PendingBatch&&) = delete;

    [[nodiscard]] bool valid() const noexcept {
        // valid()의 수신자는 const future<BatchReport> lvalue result_이고 인자는 없다. shared state 연관 여부 bool을
        // noexcept로 반환해 호출자가 분기/출력에 사용한다. 표준은 복잡도·할당 여부를 별도로 보장하지 않으며 일반
        // 구현에서는 handle 상태만 확인하는 상수 규모다. 호출로 상태·소유권·수명은 변하지 않는다.
        // true는 결과가 ready라는 뜻이 아니며, 같은 future의 get과 동시 호출하는 외부 동기화는 제공하지 않는다.
        return result_.valid();
    }

    [[nodiscard]] BatchReport take() {
        // get()의 수신자는 아직 소비하지 않은 valid future<BatchReport> lvalue result_이고 인자는 없다. 결과가 ready일
        // 때까지 대기한 뒤 공유 상태의 BatchReport를 이동해 값으로 반환하며 호출자가 report를 직접 초기화한다.
        // 성공 뒤 result_는 invalid다. 이 상태의 두 번째 get은 valid 전제조건을 어겨 표준상 undefined behavior이며,
        // 구현이 future_error(no_state)로 진단할 수 있어도 의존하면 안 된다. 기다림의 벽시계 상한은 없고 Report 이동 외
        // 별도 알고리즘 복잡도는 정해지지 않는다. producer가 예외를 저장했다면 여기서 재던지며, ready 동기화 덕분에
        // 생산 스레드의 set_value 이전 쓰기를 소비 스레드가 볼 수 있다. 반환 값은 호출자가 새로 소유한다.
        return result_.get();
    }
};

int main() {
    // initializer_list 생성자의 목적 객체는 새 vector<int> scores이고 세 int prvalue 4, 7, 9를 순서대로 복사한다.
    // 생성자는 반환값이 없고 성공 뒤 size 3의 연속 저장소를 scores가 소유한다. 시간·공간 O(3), 할당과 bad_alloc
    // 가능성이 있으며 실패 시 구성된 원소를 정리한다. 새 객체이므로 기존 반복자 무효화나 스레드 동기화는 없다.
    std::vector<int> scores{4, 7, 9};

    // basic_string(const char*)의 목적 객체는 string prvalue이고 non-null·null 종료된 리터럴 포인터를 입력으로
    // 받아 9자를 복사 소유한다. 반환값은 없고 시간 O(문자 수), 할당과 length_error/bad_alloc 가능성이 있으며
    // 리터럴은 유지된다. PendingBatch의 첫 값 매개변수는 이 prvalue로 직접 구성된다. 둘째 인자의 std::move는
    // scores lvalue에서 vector<int>&& xvalue를 O(1)·무할당·noexcept로 반환하고 값 매개변수의 이동 생성자가
    // 저장소를 소유한다. scores는 유효하지만 값이 미지정되며 기존 원소 관찰자는 저장하지 않는다.
    PendingBatch pending{std::string{"orders-42"}, std::move(scores)};

    const bool before_take{pending.valid()};
    // if는 bool 값을 비교해 실패 경로로 조건 분기한다. 정상 구성 직후 future는 valid이므로 실행되지 않는다.
    if (!before_take) {
        return 1;
    }

    // take()가 반환하는 BatchReport prvalue로 report를 직접 초기화한다. future shared state에서 Report로 옮기는
    // 이동은 있지만 동일 타입 반환 prvalue를 위한 추가 중간 객체 복사·이동은 C++17 직접 구성 규칙으로 필요 없다.
    BatchReport report{pending.take()};
    const bool after_take{pending.valid()};

    // 첫 operator<<(ostream&, const string&)의 수신/첫 피연산자는 std::cout ostream lvalue, 둘째는 report.name
    // const string&다. long long/size_t/bool 멤버 삽입 overload는 각 값을 복사하고, 비멤버 char overload는
    // ' '와 '\n' prvalue를 기록한다. 각 호출은 같은 ostream&를 반환해 다음 삽입의 왼쪽 피연산자로 사용하고
    // 마지막 반환은 버린다. 입력 값·소유권·수명은 유지되고 cout의 위치와 상태 비트만 변한다. 전체 비용은 기록
    // 문자 수·locale·버퍼·장치에 의존해 별도 Big-O/시간 상한이 없고, 실패는 기본적으로 예외 대신 상태 비트다.
    // 할당/참조 무효화는 보장 대상이 아니며 기본 동기화된 표준 스트림도 이 연쇄 전체의 원자성은 보장하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cout << report.name << ' ' << report.total << ' ' << report.item_count << ' '
              << before_take << ' ' << after_take << '\n';

    // main을 빠져나가면 지역 객체의 선언 역순에 따라 report가 먼저 자신의 string을 정리한다. 그 뒤 pending 안에서
    // worker_ 소멸자가 joinable 스레드에 중지를 요청하고 합류하며, 마지막으로 invalid result_가 파괴된다. get이
    // 이미 결과를 받았으므로 작업은 완료 상태지만 worker_는 아직 joinable일 수 있다.
    return 0;
}
