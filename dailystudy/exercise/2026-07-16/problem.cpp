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
