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
    // make_shared<Session>(string)는 소유 string prvalue를 Session 생성자에 전달해 객체와 제어 블록을 만들고 shared_ptr을 반환한다.
    // 보통 한 번의 할당을 사용하며 메모리 부족 시 bad_alloc이 가능하다.
    const auto session{std::make_shared<Session>(std::string{"study"})};
    // weak_ptr 생성자는 session shared_ptr를 입력받고 반환값 없이 같은 제어 블록을 관찰한다. 강한 참조 횟수는 늘리지 않는다.
    const std::weak_ptr<Session> observer{session};
    // lock()은 인자 없이 살아 있으면 shared_ptr<Session>, 만료됐으면 빈 shared_ptr를 반환하고 observer는 바뀌지 않는다.
    // 성공한 handle은 강한 소유권 한 몫을 얻어 if 분기 끝까지 Session 수명을 연장한다.
    if (const auto handle{observer.lock()}) {
        // Session::name()은 표준 함수가 아니지만 내부 std::string의 const&를 반환하므로 handle보다 오래 보관하면 안 된다.
        std::cout << handle->name() << '\n';
    }
    // 실제 실행은 로드·참조 횟수 변경·비교·조건 분기·함수 호출을 포함할 수 있으나 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return 0; // int 값 0은 정상 종료를 뜻한다.
}
