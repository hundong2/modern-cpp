// <iostream>은 표준 출력 객체 std::cout을 제공한다.
#include <iostream>
// <memory>는 단일 소유 스마트 포인터와 std::make_unique를 제공한다.
#include <memory>
// <string>은 문자열을 소유하는 std::string을 제공한다.
#include <string>
// <utility>는 이동 요청 함수 std::move를 제공한다.
#include <utility>

// struct의 기본 접근은 public이며 단순 데이터 묶음에 알맞다.
struct Message {
    std::string text{}; // 멤버 변수는 문자열 값을 직접 소유하며 {}는 빈 문자열 초기화다.
};

// class의 기본 접근은 private이지만 public 지정자로 인터페이스를 공개한다.
class Sink {
public:
    // 가상 소멸자는 기반 포인터로 파생 객체를 지울 때 파생 소멸까지 호출되게 한다.
    virtual ~Sink() = default;
    // const lvalue 참조 매개변수는 Message를 복사하거나 소유하지 않고 읽기만 한다.
    virtual void write(const Message& message) const = 0;
};

class ConsoleSink final : public Sink {
public:
    // override는 기반 가상 함수를 정확히 재정의했는지 컴파일러가 검사하게 한다.
    void write(const Message& message) const override {
        std::cout << message.text << '\n'; // << 연산자로 문자열과 개행을 출력 스트림에 보낸다.
    }
};

class ReportService {
public:
    // 생성자에는 반환형이 없다. explicit은 unique_ptr에서의 의도치 않은 암시적 변환을 막는다.
    explicit ReportService(std::unique_ptr<Sink> sink)
        : sink_{std::move(sink)} {} // 멤버 초기화 목록이 xvalue에서 소유권을 이동한다.

    void publish(const std::string& text) const {
        const Message message{text}; // text lvalue를 복사해 독립적인 Message 수명을 만든다.
        sink_->write(message); // ->는 포인터가 가리키는 객체의 가상 함수를 호출한다.
    }

private:
    std::unique_ptr<Sink> sink_{}; // 서비스가 Sink 한 개를 독점 소유하고 자동 파괴한다.
};

int main() {
    // make_unique<ConsoleSink>()는 생성자 인자 없이 객체를 동적 생성하고 unique_ptr<ConsoleSink> prvalue를 반환한다.
    // 반환 포인터가 객체 수명을 단독 소유하며 할당 실패 시 bad_alloc이 가능하다.
    auto sink{std::make_unique<ConsoleSink>()};
    // move(sink)는 unique_ptr&&를 반환해 서비스 생성자가 소유권을 이동하게 한다. sink는 이후 빈 유효 상태다.
    ReportService service{std::move(sink)};
    service.publish("ownership is explicit"); // 이름 있는 service는 lvalue이고 멤버 함수를 호출한다.
    const bool moved{sink == nullptr}; // 이동 후 원본이 비었는지 == 비교 결과를 bool로 저장한다.
    // 가상 간접 호출과 로드·저장·조건 분기의 명령 형태는 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return moved ? 0 : 1; // 삼항 연산자가 성공이면 0, 아니면 1을 운영체제에 반환한다.
}
