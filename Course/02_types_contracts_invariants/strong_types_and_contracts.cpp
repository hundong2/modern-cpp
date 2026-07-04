#include <iostream>
#include <stdexcept>

class Money {
public:
    explicit Money(int won) : won_{won} {
        if (won < 0) {
            throw std::invalid_argument{"Money cannot be negative"};
        }
    }

    [[nodiscard]] int value() const {
        return won_;
    }

    friend Money operator+(Money left, Money right) {
        return Money{left.won_ + right.won_};
    }

private:
    // int는 보통 객체 내부에 직접 저장됩니다.
    // Money m{1000}; 이 지역 변수라면 m 전체가 스택 프레임에 놓입니다.
    int won_{};
};

class BankAccount {
public:
    explicit BankAccount(Money initial_balance) : balance_{initial_balance} {}

    void deposit(Money amount) {
        balance_ = balance_ + amount;
    }

    void withdraw(Money amount) {
        if (amount.value() > balance_.value()) {
            throw std::logic_error{"insufficient balance"};
        }

        // Money는 음수 값을 허용하지 않기 때문에 여기서도 불변식이 유지됩니다.
        balance_ = Money{balance_.value() - amount.value()};
    }

    [[nodiscard]] Money balance() const {
        return balance_;
    }

private:
    Money balance_;
};

int main() {
    try {
        BankAccount account{Money{10000}};
        account.deposit(Money{2500});
        account.withdraw(Money{3000});

        std::cout << "balance: " << account.balance().value() << " won\n";

        // 아래 줄을 켜면 생성자 계약 위반으로 예외가 발생합니다.
        // Money invalid{-1};
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
    }
}

