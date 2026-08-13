// <iostream>은 학습 결과를 출력하는 std::cout을 제공한다.
#include <iostream>
// <memory>는 shared_ptr과 weak_ptr 등 수명 관리 스마트 포인터를 제공한다.
#include <memory>
// <string>은 소유 문자열 타입을 제공한다.
#include <string>
// <utility>는 이동을 명시하는 std::move를 제공한다.
#include <utility>

// struct는 기본 접근이 public이다. 단순 결과 묶음에 적합하다.
struct Delivery {
    std::string text{}; // public 멤버이며 중괄호로 빈 문자열 초기화한다.
};

// class는 기본 접근이 private이므로 외부 API를 public 아래에 명시한다.
class MessageStore final {
public:
    // 생성자 매개변수를 값으로 받아 멤버로 이동하면 호출자가 복사/이동을 선택할 수 있다.
    explicit MessageStore(std::string value) : value_{std::move(value)} {}
    [[nodiscard]] Delivery read() const { return Delivery{value_}; } // 반환 prvalue는 결과 객체로 직접 만들어질 수 있다.

private:
    std::string value_{}; // 저장소가 문자열의 수명과 소유권을 가진다.
};

// 서비스는 저장소를 소유하지 않고 관찰한다. 그래서 저장소보다 오래 살아도 댕글링 포인터가 되지 않는다.
class ReaderService final {
public:
    explicit ReaderService(std::weak_ptr<MessageStore> store) : store_{std::move(store)} {}

    [[nodiscard]] Delivery execute() const {
        // lock() 결과 shared_ptr이 비어 있지 않은 동안 대상의 수명을 임시로 연장한다.
        if (const auto store{store_.lock()}) {
            return store->read(); // 가상 호출이 아닌 일반 간접 함수 호출이며 반환값은 복사 생략 대상이다.
        }
        return Delivery{"expired"}; // 문자열 리터럴에서 std::string 멤버를 직접 초기화한다.
    }

private:
    std::weak_ptr<MessageStore> store_{}; // 비소유 멤버가 아키텍처의 관찰 의존성을 표현한다.
};

int main() {
    auto store{std::make_shared<MessageStore>(std::string{"hello"})}; // auto는 shared_ptr<MessageStore>로 추론된다.
    const ReaderService reader{store}; // lvalue shared_ptr에서 weak_ptr을 만들어 소유권은 넘기지 않는다.
    const Delivery before{reader.execute()};
    store.reset(); // 마지막 shared_ptr을 비워 MessageStore 객체 수명을 끝낸다.
    const Delivery after{reader.execute()};
    std::cout << before.text << ' ' << after.text << '\n'; // << 연산자는 값을 출력 스트림에 순서대로 삽입한다.
    // == 비교 두 개와 && 논리곱으로 초보자 검증을 수행한다.
    return before.text == "hello" && after.text == "expired" ? 0 : 1;
}

