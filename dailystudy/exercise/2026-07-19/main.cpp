#include <cassert>      // assert로 소유권 이전과 저장 결과를 검증한다.
#include <iostream>     // cout으로 저장된 메시지를 출력한다.
#include <memory>       // 단독 소유 스마트 포인터 unique_ptr와 make_unique를 사용한다.
#include <string>       // 수신자와 본문 문자 버퍼를 소유한다.
#include <string_view>  // 이름과 주소를 복사하지 않고 읽는다.
#include <utility>      // std::move로 소유권 이동 후보인 xvalue를 만든다.
#include <vector>       // RecordingSink가 Message 목록을 소유한다.

struct Message {
    // struct 멤버는 기본 public이며 두 string이 문자 버퍼를 각각 소유한다.
    std::string recipient;
    std::string body;
};

class MessageSink {
public:
    // = 0은 순수 가상 함수 계약이며 가상 소멸자는 다형 삭제에 필요하다.
    virtual ~MessageSink() = default; // 가상 함수 테이블(vtable)을 쓰는 구현이 일반적이나 표준이 강제하지는 않는다.
    // const Message&는 메시지를 소유하지 않고 읽으며 =0은 구현을 파생 클래스에 맡긴다.
    virtual void send(const Message& message) = 0;
};

class RecordingSink final : public MessageSink {
public:
    // final은 추가 상속을 막고 override는 기반 함수와 서명을 검사하게 한다.
    void send(const Message& message) override {
        // 표준 호출 계약: push_back(const Message&)는 복사할 원소 참조 하나를 받고 void를 반환한다.
        // 성공하면 size가 1 늘며 두 string도 독립 복사된다. 상각 O(1), 재할당 시 기존 원소 관찰자가 무효화된다.
        messages_.push_back(message);
    }
    // const 참조 반환은 vector 복사를 피하지만 RecordingSink보다 오래 보관하면 안 된다.
    [[nodiscard]] const std::vector<Message>& messages() const { return messages_; }

private:
    // private vector가 모든 복사된 Message와 그 string 버퍼의 수명을 관리한다.
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
        // name과 address는 호출자가 소유한 문자를 잠시 빌리며 함수 밖에 저장하지 않는다.
        std::string body = "Hello ";
        // +=는 기존 body 버퍼 뒤에 문자를 덧붙이며 필요하면 버퍼를 재할당한다.
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
    // main은 프로그램 진입점이며 끝까지 도달하면 정상 종료 코드 0을 반환한다.
    // make_unique<RecordingSink>()는 생성자 인자가 없고 객체를 동적 생성해 unique_ptr<RecordingSink> prvalue를 반환한다.
    // 할당 실패 시 bad_alloc이 가능하며 반환 포인터가 유일한 소유자다.
    auto recorder = std::make_unique<RecordingSink>();
    // get()은 인자 없이 같은 객체의 RecordingSink*를 반환하고 소유권·참조 횟수는 바꾸지 않는다.
    RecordingSink* observer = recorder.get(); // 비소유 포인터라 recorder/service보다 오래 사용하면 댕글링된다.

    WelcomeService service{std::move(recorder)}; // recorder의 소유권이 service로 이동한다.
    assert(recorder == nullptr);

    service.welcome("Mina", "mina@example.test");
    // messages()는 vector const&를, size()는 원소 수 size_type을 반환한다. 두 호출 모두 저장 상태를 바꾸지 않는다.
    assert(observer->messages().size() == 1);
    // front()는 인자 없이 첫 Message의 const 참조를 반환하며, 빈 vector에서 호출하면 전제조건을 어긴다.
    assert(observer->messages().front().body == "Hello Mina, welcome!");

    const Message& saved = observer->messages().front();
    // front는 첫 Message의 lvalue 참조를 돌려준다. saved는 vector 원소 수명에 의존한다.
    std::cout << "To: " << saved.recipient << '\n' << saved.body << '\n';
    std::cout << "[TESTS] ownership architecture example passed\n";
}
