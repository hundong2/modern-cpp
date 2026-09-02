// <iostream>은 실행 결과를 관찰할 std::cout과 operator<<를 선언한다.
#include <iostream>
// <utility>는 콜백 값을 멤버로 이동시키기 위한 std::move를 선언한다.
#include <utility>

// Action은 실패 때 실행할 보상 작업 타입이며 템플릿이라 람다의 구체 타입을 지우지 않는다.
template <class Action>
class TransactionGuard {
private:
    Action action_; // class 기본 private 멤버가 보상 콜백을 값으로 소유한다.
    bool active_{true}; // true면 소멸 시 보상해야 한다.

public:
    // 생성자는 반환형이 없으며 explicit이라 `TransactionGuard guard = callback;` 암시 변환을 막는다.
    explicit TransactionGuard(Action action)
        // std::move<Action>(action)는 lvalue 매개변수 하나를 Action&& xvalue로 바꾼다. 반환 참조를 action_ 생성에
        // 즉시 사용하고 저장하지 않는다. 선택된 이동 생성자가 콜백 상태를 멤버로 옮기며 원본은 곧 파괴된다.
        // 대표 문서: ../standard-library/io-parsing-and-utilities.md
        : action_{std::move(action)} {}

    TransactionGuard(const TransactionGuard&) = delete; // 보상 책임의 복제는 금지한다.
    TransactionGuard& operator=(const TransactionGuard&) = delete;

    ~TransactionGuard() noexcept { // 자동 객체 수명 끝에서 반환형 없이 실행되는 RAII 소멸자다.
        if (active_) {
            action_(); // 인자·반환값 없는 noexcept 람다가 저장한 이전 잔액을 복원한다.
        }
    }

    void commit() noexcept {
        active_ = false; // 성공 상태를 저장하고 void를 반환한다. 할당·수명·소유권 변화가 없다.
    }
};

template <class Action>
TransactionGuard(Action) -> TransactionGuard<Action>; // 직접 초기화 인자에서 Action을 추론한다.

class Account {
private:
    int balance_{}; // class의 기본 private 멤버가 잔액 불변식을 감춘다.

public:
    explicit Account(int opening_balance) noexcept
        : balance_{opening_balance} {} // 생성자 매개변수 값을 멤버에 복사하며 생성자에는 반환형이 없다.

    [[nodiscard]] int balance() const noexcept {
        return balance_; // const 멤버 함수는 객체를 바꾸지 않고 int 값을 복사 반환한다.
    }

    void set_balance(int next) noexcept {
        balance_ = next; // 교육용 포트가 새 값을 저장하며 void 반환, O(1), 예외 없음이다.
    }
};

[[nodiscard]] bool transfer(Account& from, Account& to, int amount, bool ledger_write_succeeds) {
    if (amount <= 0 || from.balance() < amount) { // 두 비소유 lvalue 참조가 가리키는 객체를 검사한다.
        return false;
    }

    const int from_before{from.balance()}; // 반환 int prvalue를 값으로 저장해 복구 snapshot을 만든다.
    const int to_before{to.balance()};
    from.set_balance(from_before - amount); // 호출 뒤 from 잔액만 바뀌고 인자 int 값은 유지된다.
    to.set_balance(to_before + amount); // 호출 뒤 to 잔액만 바뀌어 총액 불변식을 유지한다.

    // [&from, &to]는 Account를 소유하지 않는 lvalue 참조 캡처, 두 before는 int 값 캡처다.
    TransactionGuard guard{[&from, &to, from_before, to_before]() noexcept {
        from.set_balance(from_before); // 실패 시 원본 계좌를 snapshot 값으로 복원한다.
        to.set_balance(to_before); // 대상 계좌도 함께 복원해 부분 성공을 남기지 않는다.
    }};

    if (!ledger_write_succeeds) {
        return false; // guard가 먼저 소멸해 두 계좌를 되돌린 뒤 false가 호출자에 전달된다.
    }

    guard.commit(); // 성공 시 보상 작업을 해제하고 두 계좌의 새 상태를 확정한다.
    return true;
}

int main() {
    Account source{100}; // explicit 생성자의 올바른 직접 초기화다.
    Account target{20};
    const bool transferred{transfer(source, target, 30, false)}; // 실패 반환 prvalue를 const bool로 받는다.

    // operator<< 연쇄는 cout lvalue, 두 int prvalue, 문자, bool 값을 읽고 ostream&를 반환한다.
    // 계좌와 transferred는 유지되고 cout 버퍼/상태만 바뀐다. 시간은 출력 길이에 선형이고 예외 대신 상태 비트를 쓸 수 있다.
    std::cout << source.balance() << ' ' << target.balance() << ' ' << transferred << '\n';

    return (source.balance() == 100 && target.balance() == 20 && !transferred) ? 0 : 1;
}
