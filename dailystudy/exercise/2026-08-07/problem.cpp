// <iostream>은 결과 출력에 사용한다.
#include <iostream>
// <string>은 상태 설명 문자열을 소유한다.
#include <string>
// <utility>는 이동 요청 함수 std::move를 제공한다.
#include <utility>
// <variant>는 합 타입과 안전한 타입 조회를 제공한다.
#include <variant>

struct Connected { std::string endpoint{}; }; // struct는 기본 public이라 값 객체 표현에 간단하다.
struct Disconnected {}; // 데이터가 없어도 서로 다른 상태 타입이라는 의미가 있다.
using Connection = std::variant<Disconnected, Connected>; // 두 상태 중 하나만 존재한다.

class Client { // class 멤버는 기본 private이다.
public:
    explicit Client(std::string name) : name_{std::move(name)} {} // 생성자에는 반환형이 없고 문자열 소유권을 이동한다.
    void connect(std::string endpoint) { state_ = Connected{std::move(endpoint)}; }
    [[nodiscard]] bool is_connected() const { return std::holds_alternative<Connected>(state_); }
    [[nodiscard]] const std::string& name() const { return name_; } // const lvalue 참조는 멤버 수명 동안 복사 없이 별칭을 제공한다.
private:
    std::string name_{};
    Connection state_{Disconnected{}}; // prvalue 상태로 직접 초기화한다.
};

int main() {
    Client client{std::string{"learner"}}; // 임시 문자열 prvalue의 자원은 매개변수를 거쳐 멤버로 이동한다.
    client.connect("localhost"); // 문자열 리터럴에서 임시 std::string을 만드는 명시적 함수 호출이다.
    const bool ok{client.is_connected()}; // bool 기본 타입은 참/거짓 조건을 나타낸다.
    if (ok) { std::cout << client.name() << " connected\n"; } // 조건이 참일 때만 출력 호출을 수행한다.
    // std::move 식은 xvalue이며 이동을 허용할 뿐이다. 실제 저장·호출 방식은 CPU·ABI·컴파일러·옵션에 따라 달라진다.
    return ok ? 0 : 1;
}
