// <iostream>은 표준 출력 객체 std::cout을 제공한다.
#include <iostream>
// <memory>는 공유 소유권 shared_ptr, 비소유 관찰 weak_ptr, make_shared를 제공한다.
#include <memory>
// <string>은 문자를 소유하며 수명을 관리하는 std::string을 제공한다.
#include <string>
// <utility>는 lvalue를 xvalue로 바꾸는 std::move를 제공한다.
#include <utility>

// class의 멤버는 기본적으로 private이다. final은 이 클래스를 더 상속하지 못하게 한다.
class Session final {
public:
    // 생성자에는 반환형이 없다. explicit은 문자열이 Session으로 암시적으로 바뀌는 것을 막는다.
    explicit Session(std::string name) : name_{std::move(name)} {} // 멤버 초기화 목록으로 소유 문자열을 이동한다.

    // const 멤버 함수는 관찰 중 객체 상태를 바꾸지 않는다. const 참조는 복사 없이 별명을 제공한다.
    [[nodiscard]] const std::string& name() const { return name_; }

private:
    std::string name_{}; // 중괄호 초기화는 빈 문자열로 확실히 초기화한다.
};

int main() {
    // make_shared의 Session 생성 결과는 prvalue shared_ptr이며 session을 직접 초기화한다(복사 생략 가능).
    const auto session{std::make_shared<Session>(std::string{"study"})};
    // weak_ptr은 객체를 소유하지 않아 참조 횟수를 늘리지 않는다. 관찰자 관계의 순환 소유권을 막는다.
    const std::weak_ptr<Session> observer{session};
    // lock()은 살아 있으면 shared_ptr prvalue를 반환한다. if 초기화문의 handle 수명은 분기 끝까지다.
    if (const auto handle{observer.lock()}) {
        // handle은 lvalue이고 ->는 포인터가 가리키는 객체의 함수를 호출한다.
        std::cout << handle->name() << '\n';
    }
    // 실제 실행은 로드·참조 횟수 변경·비교·조건 분기·함수 호출을 포함할 수 있으나 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return 0; // int 값 0은 정상 종료를 뜻한다.
}

