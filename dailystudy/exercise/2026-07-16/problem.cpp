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
    // 내부적으로 정수로 표현되는 경우가 많지만 enum class는 타입 안전성을 제공한다.
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

    [[nodiscard]] int net_change() const { // 뒤 const는 숨은 this가 const Summary*임을 뜻한다.
        return incoming_total - outgoing_total;
    }
};

[[nodiscard]] Summary summarize(std::span<const Transaction> transactions) {
    Summary result{};

    // transaction은 각 원소에 바인딩된 const lvalue 참조라 문자열을 포함한 구조체 복사를 피한다.
    for (const Transaction& transaction : transactions) {
        if (transaction.amount <= 0) {
            continue; // 기계어 수준에서는 다음 반복 조건으로 분기하는 형태가 일반적이다.
        }

        switch (transaction.kind) {
        case TransactionKind::incoming:
            result.incoming_total += transaction.amount; // 멤버 lvalue에서 읽고 더한 뒤 같은 위치에 저장한다.
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
