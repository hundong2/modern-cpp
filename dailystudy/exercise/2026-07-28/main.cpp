// <algorithm>은 조건에 맞는 원소를 지우는 C++20 std::erase_if를 제공한다.
#include <algorithm>
// <iostream>은 표준 출력 객체 std::cout과 << 스트림 삽입 연산자를 제공한다.
#include <iostream>
// <memory>는 shared_ptr, weak_ptr, make_shared 같은 소유권 도구를 제공한다.
#include <memory>
// <string>은 문자열을 소유하는 std::string 타입을 제공한다.
#include <string>
// <utility>는 lvalue를 xvalue로 표현하는 std::move를 제공한다.
#include <utility>
// <vector>는 같은 타입의 여러 원소를 연속 저장하는 가변 길이 컨테이너를 제공한다.
#include <vector>

// struct의 기본 접근은 public이다. 이 추상 구조체는 애플리케이션의 이벤트 수신 포트다.
struct IEventListener {
public:
    // 기반 포인터로 파괴할 때 실제 파생 소멸자가 호출되도록 가상 소멸자를 둔다.
    virtual ~IEventListener() = default;
    // void는 반환값이 없고, const string& 매개변수는 기존 문자열을 복사 없이 읽는다.
    virtual void on_event(const std::string& message) const = 0;
};

// using은 긴 템플릿 타입에 별칭을 붙인다. <IEventListener>는 shared_ptr의 템플릿 인자다.
using ListenerPtr = std::shared_ptr<IEventListener>;

// class의 기본 접근은 private다. 출력 어댑터는 포트의 구현 세부사항을 감춘다.
class ConsoleListener final : public IEventListener {
public:
    // 생성자는 반환형이 없다. explicit은 문자열 하나가 객체로 암시 변환되는 일을 막는다.
    explicit ConsoleListener(std::string name)
        // 생성자 매개변수 name은 이름 있는 lvalue이고, std::move가 xvalue로 바꿔 멤버 이동을 요청한다.
        : name_{std::move(name)} {
    }

    // override는 기반 가상 함수와 서명이 맞는지 컴파일러가 검사하며 const는 멤버를 바꾸지 않음을 뜻한다.
    void on_event(const std::string& message) const override {
        // << 연산자가 이름과 메시지를 std::cout에 차례로 저장하도록 요청한다.
        std::cout << name_ << ": " << message << '\n';
    }

private:
    std::string name_{}; // 멤버 변수는 구독자 이름 문자열의 수명과 소유권을 가진다.
};

class EventBus {
public:
    // ListenerPtr은 값 매개변수이므로 shared_ptr을 복사해 호출 중 강한 소유권을 확보한다.
    void subscribe(ListenerPtr listener) {
        // push_back은 weak_ptr로 변환해 저장한다. 버스는 객체를 소유하지 않아 순환 소유를 피한다.
        listeners_.push_back(listener);
    }

    // 반환형 int는 실제 호출한 구독자 수다. message는 읽기 전용 lvalue 참조 매개변수다.
    [[nodiscard]] int publish(const std::string& message) {
        int delivered{0}; // 기본 정수 타입 int를 중괄호로 0 초기화한다.

        // 반복문은 각 weak_ptr 원소를 비-const 참조로 바인딩해 복사하지 않는다.
        for (std::weak_ptr<IEventListener>& weak : listeners_) {
            // lock()은 살아 있으면 shared_ptr prvalue를 반환하며 지역 strong이 호출 동안 수명을 지킨다.
            if (ListenerPtr strong{weak.lock()}; strong != nullptr) {
                // ->는 포인터가 가리키는 가상 함수를 호출한다. 실제 구현은 ConsoleListener에서 선택된다.
                strong->on_event(message);
                ++delivered; // 전위 ++ 연산자는 값을 1 증가시켜 다시 저장한다.
            }
        }

        // erase_if는 조건이 참인 만료 weak_ptr를 컨테이너에서 제거하는 표준 라이브러리 함수다.
        std::erase_if(listeners_, [](const std::weak_ptr<IEventListener>& weak) {
            return weak.expired(); // 반환형 bool 값이 제거 조건 분기를 결정한다.
        });
        return delivered; // 지역 정수 값을 호출자에게 값으로 반환한다.
    }

private:
    // vector의 템플릿 인자는 weak_ptr이며 버스는 구독자의 수명을 소유하지 않는다.
    std::vector<std::weak_ptr<IEventListener>> listeners_{};
};

// ListenerPtr 반환형 함수이며, 매개변수는 없다.
ListenerPtr make_listener() {
    // make_shared 결과는 prvalue이며 반환 목적 객체를 직접 만들 수 있어 불필요한 shared_ptr 복사가 생략된다.
    return std::make_shared<ConsoleListener>("ui");
}

// main의 int 반환값 0은 운영체제에 성공을 알린다.
int main() {
    EventBus bus{}; // 이름 있는 bus는 lvalue이며 기본 생성으로 직접 초기화한다.
    ListenerPtr listener{make_listener()}; // 반환 prvalue로 강한 소유권을 직접 초기화한다.
    bus.subscribe(listener); // lvalue shared_ptr를 값 매개변수로 복사했다가 함수 종료 시 임시 소유권을 놓는다.

    const int first{bus.publish("ready")}; // 문자열 리터럴로 string 임시값이 생기고 const 참조가 호출 동안 바인딩된다.
    listener.reset(); // 마지막 강한 소유권을 해제해 ConsoleListener 객체 수명을 끝낸다.
    const int second{bus.publish("after reset")};

    // ==와 &&는 비교 및 논리 AND 연산자이며, 왼쪽이 거짓이면 오른쪽을 평가하지 않는다.
    if (first == 1 && second == 0) {
        return 0;
    }
    return 1;
}
