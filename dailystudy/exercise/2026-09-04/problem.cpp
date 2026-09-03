// <future>는 값 생산자 std::promise와 단일 소비자 std::future를 선언한다.
#include <future>
// <iostream>은 연습 결과를 기록할 std::cout과 스트림 삽입 연산을 선언한다.
#include <iostream>
// <string>은 메시지 문자를 독점 소유하는 std::string을 선언한다.
#include <string>
// <utility>는 소유권 전달 후보를 xvalue로 표시하는 std::move를 선언한다.
#include <utility>

// class template의 T는 한 번 게시하고 한 번 꺼낼 값 타입이다. 타입 인자마다 별도 클래스가 컴파일 시 생성된다.
// class 기본 접근은 private이므로 생산/소비 handle을 외부에서 임의로 두 번 사용할 수 없다.
template <class T>
class OneShotChannel {
private:
    std::promise<T> producer_; // 공유 상태의 생산자 끝점을 독점 소유한다.
    std::future<T> consumer_;  // 같은 공유 상태의 단일 소비자 끝점을 독점 소유한다.

public:
    OneShotChannel()
        // promise<T>() 기본 생성자의 목적 객체는 producer_이며 입력 인자와 반환값이 없다. 성공하면 T 한 값을 위한
        // 새 shared state와 생산자 끝점을 만든다. 보통 할당이 필요해 bad_alloc/상태 생성 오류가 가능하고 아직
        // ready가 아니다. get_future와 setter 계열은 서로 data race가 없고 setter끼리는 상호 배제되는 것처럼
        // 동작하지만, 같은 handle의 이동·소멸 등 모든 임의 조작이 동시 안전하다는 뜻은 아니다.
        // get_future()의 수신자는 아직 consumer를 발급하지 않은 valid promise<T> lvalue producer_이고 인자는 없다.
        // future<T> prvalue를 반환해 consumer_를 직접 초기화한다. 표준은 이 호출의 복잡도·할당 여부를 별도로
        // 보장하지 않으며 일반 구현에서는 handle 연결의 상수 규모다. 호출 뒤 두 handle이 같은 상태를 가리키되
        // 소유 역할은 다르다. 두 번째 호출/no-state는 future_error이고 값·반복자 무효화는 없다.
        // 대표 문서: ../standard-library/concurrency-time-filesystem.md
        : producer_{}, consumer_{producer_.get_future()} {}

    // promise/future는 copy constructor가 삭제된 독점 handle이다. 채널도 복사와 이동을 삭제해 한 게시·소비 경로만 둔다.
    OneShotChannel(const OneShotChannel&) = delete;
    OneShotChannel& operator=(const OneShotChannel&) = delete;
    OneShotChannel(OneShotChannel&&) = delete;
    OneShotChannel& operator=(OneShotChannel&&) = delete;

    void publish(T value) {
        // std::move(T&&)에서 이름 있는 value는 T lvalue이므로 함수 템플릿의 별도 U는 T&로 추론된다. 유일한
        // 비소유 인자에 바인딩해 value와 같은 객체의 T&& xvalue를 반환하며 set_value(T&&) 입력으로 즉시 쓴다.
        // move 자체의 반환 참조는 저장하지 않고 O(1)·무할당·noexcept이며 상태·수명·동기화를 바꾸지 않는다.
        // set_value의 수신자는 아직 만족되지 않은 valid promise<T> lvalue producer_이고, 인자는 xvalue가 바인딩된
        // T&&로 공유 상태에 이동 저장할 소유 값이다. 반환형 void이며 성공 뒤 ready이고 consumer_를 깨울 수 있다.
        // 이동 뒤 value의 객체 수명은 계속되지만 값의 보장은 T의 이동 생성자 계약에 따른다(오늘 StatusMessage는
        // std::string 규칙에 따라 유효하지만 값 미지정). 두 번째 게시/no-state는 future_error, T 이동 예외도 가능하다.
        // setter 계열끼리는 data race 없이 직렬화되지만 논리적으로 한 번만 성공한다. 비용은 T 이동과 대기자 깨우기에
        // 따르며, 다른 참조의 수명을 연장하거나 같은 promise의 이동·소멸 같은 임의 동시 조작을 보호하지 않는다.
        // 대표 문서: ../standard-library/io-parsing-and-utilities.md
        // 대표 문서: ../standard-library/concurrency-time-filesystem.md
        // std::move에는 수신 객체가 없고 선택 시그니처는 remove_reference_t<T>&& move(T&) noexcept다. 유일한
        // 매개변수 value는 T lvalue·비소유 입력이고 허용값은 T가 유지하는 모든 상태다. 반환 T&& xvalue는 즉시
        // set_value에 사용한다. 호출 뒤 value 자체는 아직 바뀌지 않으며, O(1)·무할당·비무효화·수명 연장 없음·
        // noexcept이고 스레드 보장을 추가하지 않는다. 실제 상태 변화와 T 이동 예외 가능성은 set_value에 속한다.
        producer_.set_value(std::move(value));
    }

    [[nodiscard]] bool valid() const noexcept {
        // valid()의 수신자는 const future<T> lvalue consumer_이고 인자는 없다. 공유 상태 연관 여부 bool을 noexcept로
        // 반환해 호출자가 사용한다. 표준은 복잡도·할당 여부를 따로 보장하지 않고 일반 구현은 handle 확인의 상수
        // 규모다. 객체 상태와 수명은 그대로이며 ready 여부는 말하지 않는다.
        return consumer_.valid();
    }

    [[nodiscard]] T consume() {
        // get()의 수신자는 아직 소비 전인 valid future<T> lvalue consumer_이고 인자는 없다. ready까지 기다린 뒤
        // 저장된 T를 이동해 값으로 반환하고 호출자가 소유한다. 성공 뒤 consumer_는 invalid다. 이 상태의 두 번째
        // get/no-state 호출은 valid 전제조건을 위반해 표준상 undefined behavior다. 구현이 future_error(no_state)로
        // 진단할 수 있어도 의존하지 않는다. 생산자가 저장한 예외는 재던지고 대기 시간 상한은 없으며, ready를
        // 만든 set_value 이전 작업과 반환 뒤 작업 사이에 동기화가 성립한다.
        return consumer_.get();
    }
};

// struct 기본 public을 이용한 작은 값 타입이다. string 수명을 객체 안에 소유한다.
struct StatusMessage {
    std::string text;

    // 생성자는 반환형이 없고 멤버 초기화 목록은 본문 전에 text를 최초 구성한다. explicit은 string 하나가
    // StatusMessage로 암시 변환되는 것을 막는다. received_text 값 매개변수는 이름 있는 lvalue이고 std::move가
    // string&& xvalue를 O(1)·무할당·noexcept로 반환한다. string 이동 생성자는 반환값 없이 text를 소유 상태로
    // 만들고 원본을 유효하지만 값 미지정으로 둔다. 기본 allocator에서 상수 시간·noexcept이며 SSO 방식은 구현별이다.
    explicit StatusMessage(std::string received_text) noexcept
        : text{std::move(received_text)} {}
};

int main() {
    // OneShotChannel<StatusMessage>에서 명시 템플릿 인자 T는 StatusMessage다. 기본 생성은 내부 promise 공유 상태를
    // 만들고 future를 한 번 발급한다. channel이 두 handle을 소유하며 scope 끝까지 살아 있으므로 raw pointer나
    // 외부 참조의 댕글링 위험이 없다.
    OneShotChannel<StatusMessage> channel{};

    // basic_string(const char*)는 non-null·null 종료 리터럴 포인터를 입력으로 문자를 복사해 string prvalue를
    // 만든다. 생성자 반환값은 없고 StatusMessage 값 매개변수/멤버가 차례로 소유한다. 시간 O(문자 수), 할당과
    // length_error/bad_alloc 가능성이 있으며 원본 리터럴 수명·내용은 유지된다. StatusMessage prvalue는 source에
    // 직접 구성되므로 동일 타입 중간 객체를 위한 추가 복사·이동이 없다.
    const StatusMessage source{std::string{"deployment-ready"}};

    // publish(T)의 값 매개변수는 source const lvalue에서 StatusMessage 복사 생성된다. 내부 string도 O(n)에 깊게
    // 복사되어 source는 그대로다. 값 매개변수의 복사 생성 실패 시 publish 본문은 시작하지 않고 channel도 게시 전이다.
    channel.publish(source);
    const bool before_consume{channel.valid()};
    StatusMessage received{channel.consume()};
    const bool after_consume{channel.valid()};

    // operator<<(ostream&, const string&)은 cout lvalue와 source.text/received.text const reference를 받아 같은
    // ostream&를 반환해 연쇄에 사용한다. bool 삽입은 값을 복사하고 char 비멤버 overload는 ' ', '\n' prvalue를
    // 기록한다. 마지막 반환은 버린다. 입력 객체는 유지되고 cout 위치·상태만 변한다. 비용은 문자 수·locale·장치에
    // 의존하며 별도 시간 상한이 없고, 실패는 기본적으로 예외 대신 상태 비트다. 할당·참조 무효화나 연쇄 전체의
    // 스레드 원자성은 보장되지 않지만 이 예제는 한 스레드에서만 출력한다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cout << source.text << ' ' << received.text << ' '
              << before_consume << ' ' << after_consume << '\n';

    return 0;
}
