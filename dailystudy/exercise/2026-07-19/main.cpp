#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct Message {
    std::string recipient;
    std::string body;
};

class MessageSink {
public:
    virtual ~MessageSink() = default; // 가상 함수 테이블(vtable)을 쓰는 구현이 일반적이나 표준이 강제하지는 않는다.
    virtual void send(const Message& message) = 0;
};

class RecordingSink final : public MessageSink {
public:
    void send(const Message& message) override { messages_.push_back(message); } // lvalue message를 vector에 복사한다.
    [[nodiscard]] const std::vector<Message>& messages() const { return messages_; }

private:
    std::vector<Message> messages_;
};

class WelcomeService {
public:
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
    std::unique_ptr<MessageSink> sink_;
};

int main() {
    auto recorder = std::make_unique<RecordingSink>(); // 반환 unique_ptr prvalue로 recorder를 직접 초기화한다.
    RecordingSink* observer = recorder.get(); // 비소유 포인터. recorder/service보다 오래 사용하면 댕글링된다.

    WelcomeService service{std::move(recorder)}; // ownership moves to service
    assert(recorder == nullptr);

    service.welcome("Mina", "mina@example.test");
    assert(observer->messages().size() == 1);
    assert(observer->messages().front().body == "Hello Mina, welcome!");

    const Message& saved = observer->messages().front();
    std::cout << "To: " << saved.recipient << '\n' << saved.body << '\n';
    std::cout << "[TESTS] ownership architecture example passed\n";
}
