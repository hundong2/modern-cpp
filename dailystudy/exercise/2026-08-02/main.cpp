// <iostream>은 표준 출력 객체 std::cout과 출력 연산자 <<를 제공한다.
#include <iostream>
// <memory>는 단독 소유 스마트 포인터 unique_ptr와 make_unique를 제공한다.
#include <memory>
// <optional>은 값이 없을 수도 있음을 타입으로 나타내는 std::optional을 제공한다.
#include <optional>
// <string>은 문자 데이터를 소유하는 std::string을 제공한다.
#include <string>
// <utility>는 소유권 이동에 쓰는 std::move를 제공한다.
#include <utility>
// <vector>는 연속 메모리 동적 배열 std::vector를 제공한다.
#include <vector>

// struct의 기본 접근은 public이므로 단순 데이터 묶음의 멤버를 공개하기 알맞다.
struct User {
    int id{};           // int 기본 타입을 중괄호로 0 값 초기화한다.
    std::string name{}; // string이 문자 버퍼를 직접 소유한다.
};

// 포트는 응용 서비스가 필요로 하는 조회 계약만 선언한다.
struct UserRepository {
    // 기반 포인터로 제거할 때 파생 소멸자까지 호출되도록 가상 소멸자를 둔다.
    virtual ~UserRepository() = default;
    // 반환형 optional<User>는 실패를 -1 같은 마법 값 없이 표현하고, const는 저장소를 바꾸지 않는다는 계약이다.
    [[nodiscard]] virtual std::optional<User> find(int id) const = 0;
};

// class의 기본 접근은 private이며, public: 아래에서 외부에 허용할 동작만 연다.
class MemoryUserRepository final : public UserRepository {
public:
    // 생성자에는 반환형이 없다. explicit은 vector 한 개가 저장소로 암시적으로 변환되는 일을 막는다.
    explicit MemoryUserRepository(std::vector<User> users)
        // 멤버 초기화 목록에서 매개변수의 자원을 이동해 복사 비용을 피한다.
        : users_{std::move(users)} {}

    [[nodiscard]] std::optional<User> find(int id) const override {
        // const User&는 벡터 원소 lvalue에 바인딩되어 복사하지 않으며 벡터보다 오래 쓰지 않는다.
        for (const User& user : users_) { // 범위 for가 모든 원소를 한 번씩 검사하므로 O(n)이다.
            if (user.id == id) { // == 비교 결과가 참이면 조건 분기로 들어간다.
                // optional<User> 변환 생성자는 user lvalue를 입력받아 User 복사본을 소유하는 성공 값을 반환한다.
                return user; // 저장소 내부 참조를 노출하지 않는다.
            }
        }
        return std::nullopt; // 전용 빈 값으로 조회 실패를 명시한다.
    }

private:
    std::vector<User> users_{}; // 어댑터가 사용자 객체들의 수명과 메모리를 소유한다.
};

class GreetingService {
public:
    // 올바른 사용은 GreetingService service{std::move(repository)} 같은 직접 초기화다.
    explicit GreetingService(std::unique_ptr<UserRepository> repository)
        : repository_{std::move(repository)} {} // xvalue에서 unique_ptr 소유권을 멤버로 옮긴다.

    [[nodiscard]] std::string greet(int id) const {
        // 내부 포트 find(id)는 정수 id 하나를 입력받고 optional<User> 값을 반환하며 저장소를 바꾸지 않는다.
        // 함수 반환 prvalue로 optional을 초기화하며 복사 생략 또는 이동이 적용될 수 있다.
        const std::optional<User> result{repository_->find(id)};
        // has_value()는 인자가 없고 result를 바꾸지 않으며 User 존재 여부를 bool로 반환한다.
        if (!result.has_value()) {
            return "not found"; // 문자열 리터럴로 string 반환 객체를 만든다.
        }
        // *는 optional 내부 User lvalue를 얻고 const 참조가 그 객체에 바인딩된다.
        const User& user{*result};
        return "hello, " + user.name; // +가 새 소유 string prvalue를 만들어 반환한다.
    }

private:
    std::unique_ptr<UserRepository> repository_{}; // 서비스가 포트 구현 하나를 단독 소유한다.
};

int main() { // int 반환값은 운영체제에 전달할 종료 상태다.
    // 템플릿 인자 MemoryUserRepository를 지정하고 User prvalue 두 개로 vector를 구성한다.
    // make_unique<MemoryUserRepository>(users)는 vector<User> prvalue 하나를 생성자에 전달하고 소유 포인터를 반환한다.
    // 내부 vector는 저장소로 이동되며 동적 할당 실패 시 bad_alloc이 가능하다.
    auto repository{std::make_unique<MemoryUserRepository>(
        std::vector<User>{User{7, "Ada"}, User{9, "Bjarne"}})};
    // move(repository)는 unique_ptr&&를 반환해 서비스 생성자가 단독 소유권을 넘겨받게 한다. 원본은 빈 상태가 된다.
    GreetingService service{std::move(repository)};
    const std::string message{service.greet(7)}; // const 지역 변수는 초기화 뒤 다른 값을 대입할 수 없다.
    std::cout << message << '\n'; // 함수 호출 결과를 출력하고 줄을 바꾼다.
    return message == "hello, Ada" ? 0 : 1; // ?: 조건 연산자로 성공 0과 실패 1을 고른다.
}

