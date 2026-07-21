#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct Message {
    // struct 멤버는 기본 public이며 두 string이 문자 버퍼를 각각 소유한다.
    std::string recipient;
    std::string body;
};

class MessageSink {
public:
    // = 0은 순수 가상 함수 계약이며 가상 소멸자는 다형 삭제에 필요하다.
    virtual ~MessageSink() = default; // 가상 함수 테이블(vtable)을 쓰는 구현이 일반적이나 표준이 강제하지는 않는다.
    virtual void send(const Message& message) = 0;
};

class RecordingSink final : public MessageSink {
public:
    // final은 추가 상속을 막고 override는 기반 함수와 서명을 검사하게 한다.
    void send(const Message& message) override { messages_.push_back(message); } // lvalue message를 vector에 복사한다.
    [[nodiscard]] const std::vector<Message>& messages() const { return messages_; }

private:
    std::vector<Message> messages_;
};

class WelcomeService {
public:
    // 생성자는 반환형이 없다. explicit은 unique_ptr에서 서비스로의 암시 변환을 막는다.
    // 값 매개변수 unique_ptr는 호출자로부터 단독 소유권을 받는 경계를 표현한다.
    explicit WelcomeService(std::unique_ptr<MessageSink> sink)
        // sink는 이름이 있어 lvalue. move가 xvalue로 변환해 unique_ptr 이동 생성자를 선택하게 한다.
        : sink_(std::move(sink)) {
        assert(sink_ != nullptr);
    }

    void welcome(std::string_view name, std::string_view address) {
        std::string body = "Hello ";
        body += name;
        body += ", welcome!";
        // Message{...}는 prvalue. body는 xvalue로 전달되어 문자열 버퍼 이전이 가능하다.
        // sink_의 동적 타입을 따라 send를 찾는 간접 호출이 일반적이며 직접 호출보다 최적화가 어려울 수 있다.
        sink_->send(Message{std::string{address}, std::move(body)});
    }

private:
    // private unique_ptr가 sink의 수명을 소유하며 서비스 소멸 때 자동 해제한다.
    std::unique_ptr<MessageSink> sink_;
};

int main() {
    auto recorder = std::make_unique<RecordingSink>(); // 반환 unique_ptr prvalue로 recorder를 직접 초기화한다.
    RecordingSink* observer = recorder.get(); // 비소유 포인터. recorder/service보다 오래 사용하면 댕글링된다.

    WelcomeService service{std::move(recorder)}; // recorder의 소유권이 service로 이동한다.
    assert(recorder == nullptr);

    service.welcome("Mina", "mina@example.test");
    assert(observer->messages().size() == 1);
    assert(observer->messages().front().body == "Hello Mina, welcome!");

    const Message& saved = observer->messages().front();
    std::cout << "To: " << saved.recipient << '\n' << saved.body << '\n';
    std::cout << "[TESTS] ownership architecture example passed\n";
}
