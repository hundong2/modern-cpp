/*
[기초 문법부터 읽는 순서]
1. enum class TransactionKind는 거래 종류를 incoming 또는 outgoing으로 제한합니다.
2. struct는 item, kind, amount처럼 함께 다닐 데이터를 묶습니다. amount{}는 0 초기화입니다.
3. 멤버 함수 뒤의 const는 그 함수가 객체의 멤버를 변경하지 않는다는 약속입니다.
4. span<const Transaction>은 vector를 복사하지 않고 읽기 전용으로 전달합니다.
5. 범위 for의 const Transaction&는 각 원소를 복사하지 않고 읽습니다.
6. continue는 현재 반복만 건너뛰고 다음 원소로 이동합니다.
7. switch는 거래 종류에 맞는 합계를 고르고, break는 해당 case 실행을 끝냅니다.
8. +=는 기존 값에 오른쪽 값을 더해 다시 저장하며 assert는 계산 결과를 검증합니다.
*/

/*
Daily syntax drill - 2026-07-16

Summarize transactions using enum class, a struct, a range-based for loop,
references, and a switch.
*/

#include <cassert>
#include <iostream>
#include <span>
#include <string>
#include <vector>

enum class TransactionKind {
    incoming,
    outgoing,
};

struct Transaction {
    std::string item;
    TransactionKind kind{};
    int amount{};
};

struct Summary {
    int incoming_total{};
    int outgoing_total{};

    [[nodiscard]] int net_change() const {
        return incoming_total - outgoing_total;
    }
};

[[nodiscard]] Summary summarize(std::span<const Transaction> transactions) {
    Summary result{};

    for (const Transaction& transaction : transactions) {
        if (transaction.amount <= 0) {
            continue;
        }

        switch (transaction.kind) {
        case TransactionKind::incoming:
            result.incoming_total += transaction.amount;
            break;
        case TransactionKind::outgoing:
            result.outgoing_total += transaction.amount;
            break;
        }
    }

    return result;
}

void run_tests() {
    const std::vector<Transaction> transactions = {
        {"book", TransactionKind::incoming, 10},
        {"book", TransactionKind::outgoing, 3},
        {"pen", TransactionKind::incoming, 5},
        {"ignored", TransactionKind::outgoing, -2},
    };

    const Summary result = summarize(transactions);
    assert(result.incoming_total == 15);
    assert(result.outgoing_total == 3);
    assert(result.net_change() == 12);

    const std::vector<Transaction> empty;
    assert(summarize(empty).net_change() == 0);
}

int main() {
    run_tests();

    const std::vector<Transaction> transactions = {
        {"keyboard", TransactionKind::incoming, 8},
        {"keyboard", TransactionKind::outgoing, 2},
        {"mouse", TransactionKind::incoming, 3},
    };

    const Summary result = summarize(transactions);
    std::cout << "incoming: " << result.incoming_total << '\n';
    std::cout << "outgoing: " << result.outgoing_total << '\n';
    std::cout << "net change: " << result.net_change() << '\n';
    std::cout << "[TESTS] syntax problem passed\n";
}
