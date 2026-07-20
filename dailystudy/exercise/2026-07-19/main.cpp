/*
[기초 문법부터 읽는 순서]
1. struct Message는 받는 사람과 본문을 하나의 값으로 묶습니다.
2. MessageSink의 virtual 함수와 `= 0`은 모든 출력 어댑터가 send를 구현해야 한다는
   인터페이스 계약입니다. 가상 소멸자는 기반 포인터로 삭제해도 전체 객체를 정리합니다.
3. `class RecordingSink final : public MessageSink`는 인터페이스를 공개 상속해 구현합니다.
4. vector<Message>는 메시지를 소유하고 push_back은 끝에 새 값을 추가합니다.
5. unique_ptr<T>는 T를 소유하는 포인터가 정확히 하나임을 나타내며 자동으로 delete합니다.
6. make_unique는 객체를 안전하게 만들고 std::move는 소유권을 service로 이전합니다.
7. get()이 반환한 raw pointer는 관찰만 하며 소유하지 않습니다. 실제 소유자보다 오래 쓰면 안 됩니다.
8. string_view는 이름과 주소를 빌려 읽고, `+=`는 기존 문자열 뒤에 내용을 붙입니다.
9. main은 구체 객체를 만들고 서비스에 연결하는 조립 지점이며 assert로 소유권과 결과를 검증합니다.
*/

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
    virtual ~MessageSink() = default;
    virtual void send(const Message& message) = 0;
};

class RecordingSink final : public MessageSink {
public:
    void send(const Message& message) override { messages_.push_back(message); }
    [[nodiscard]] const std::vector<Message>& messages() const { return messages_; }

private:
    std::vector<Message> messages_;
};

class WelcomeService {
public:
    explicit WelcomeService(std::unique_ptr<MessageSink> sink)
        : sink_(std::move(sink)) {
        assert(sink_ != nullptr);
    }

    void welcome(std::string_view name, std::string_view address) {
        std::string body = "Hello ";
        body += name;
        body += ", welcome!";
        sink_->send(Message{std::string{address}, std::move(body)});
    }

private:
    std::unique_ptr<MessageSink> sink_;
};

int main() {
    auto recorder = std::make_unique<RecordingSink>();
    RecordingSink* observer = recorder.get(); // observes; it does not own

    WelcomeService service{std::move(recorder)}; // ownership moves to service
    assert(recorder == nullptr);

    service.welcome("Mina", "mina@example.test");
    assert(observer->messages().size() == 1);
    assert(observer->messages().front().body == "Hello Mina, welcome!");

    const Message& saved = observer->messages().front();
    std::cout << "To: " << saved.recipient << '\n' << saved.body << '\n';
    std::cout << "[TESTS] ownership architecture example passed\n";
}
