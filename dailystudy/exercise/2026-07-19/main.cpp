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
