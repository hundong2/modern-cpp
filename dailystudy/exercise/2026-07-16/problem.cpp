#include <cassert>   // assert로 집계 결과를 검증한다.
#include <iostream>  // cout으로 요약 결과를 출력한다.
#include <span>      // 거래 배열을 소유하지 않는 읽기 전용 뷰로 받는다.
#include <string>    // 상품명 문자 버퍼를 소유하는 string을 사용한다.
#include <vector>    // 여러 Transaction 객체를 연속 저장하고 소유한다.

enum class TransactionKind {
    // enum class는 수입과 지출을 별도 타입으로 묶어 잘못된 정수 대입을 막는다.
    // 내부적으로 정수로 표현되는 경우가 많지만 enum class는 타입 안전성을 제공한다.
    incoming,
    outgoing,
};

struct Transaction {
    // struct 멤버는 기본 public이며 각 객체가 문자열 버퍼를 직접 소유한다.
    std::string item;
    TransactionKind kind{};
    int amount{};
};

struct Summary {
    int incoming_total{};  // 수입 합계를 0으로 값 초기화한다.
    int outgoing_total{};  // 지출 합계를 0으로 값 초기화한다.

    // [[nodiscard]]는 반환한 순변화 값을 무시할 때 경고를 유도한다.
    [[nodiscard]] int net_change() const { // 뒤 const는 숨은 this가 const Summary*임을 뜻한다.
        return incoming_total - outgoing_total;
    }
};

[[nodiscard]] Summary summarize(std::span<const Transaction> transactions) {
    // 반환형 Summary는 집계 결과를 값으로 돌려준다. span은 입력을 소유하지 않는다.
    Summary result{};

    // transaction은 각 원소에 바인딩된 const lvalue 참조라 문자열을 포함한 구조체 복사를 피한다.
    for (const Transaction& transaction : transactions) {
        if (transaction.amount <= 0) {
            continue; // 기계어 수준에서는 다음 반복 조건으로 분기하는 형태가 일반적이다.
        }

        switch (transaction.kind) {
        // switch는 enum 값을 비교해 일치하는 case로 분기하고 break로 빠져나온다.
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
    // 중괄호 원소 목록으로 vector와 각 Transaction을 직접 초기화한다.
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
    // const 입력 컨테이너는 검증 중 원소가 바뀌지 않음을 타입으로 보장한다.
    run_tests();

    const std::vector<Transaction> transactions = {
        {"keyboard", TransactionKind::incoming, 8},
        {"keyboard", TransactionKind::outgoing, 2},
        {"mouse", TransactionKind::incoming, 3},
    };

    const Summary result = summarize(transactions);
    // <<는 오른쪽 값을 출력 스트림에 연결하고 '\n'은 줄바꿈 문자 하나다.
    std::cout << "incoming: " << result.incoming_total << '\n';
    std::cout << "outgoing: " << result.outgoing_total << '\n';
    std::cout << "net change: " << result.net_change() << '\n';
    std::cout << "[TESTS] syntax problem passed\n";
}
