/*
[기초 문법부터 읽는 순서]
1. AddStock과 RemoveStock은 같은 필드를 가져도 의미가 다른 별도 타입입니다.
2. using은 긴 타입에 별명을 붙이며, variant<A, B>는 A 또는 B 중 한 값만 담습니다.
3. class의 public은 외부에 공개, private은 클래스 내부에서만 사용할 수 있다는 뜻입니다.
4. [[nodiscard]]는 반환값을 무시하지 말라고 컴파일러에 알려 줍니다.
5. const T&는 복사 없이 읽고, int&는 원본 정수를 직접 수정하는 참조입니다.
6. map<string, int>는 상품 이름을 키로 재고 수량을 찾습니다. find 결과가 end와
   같으면 키가 없다는 뜻입니다.
7. std::visit는 variant의 현재 값을 처리합니다. 람다의 [this]는 현재 서비스
   객체의 멤버 함수 handle을 호출하기 위해 this를 캡처합니다.
8. Overloaded는 여러 람다의 operator()를 합쳐 각 상태별 처리를 한 방문자에
   모으는 Modern C++ 도우미입니다.
*/

/*
Daily Modern C++ Exercise - 2026-07-16

Architecture: Command -> InventoryService -> Event -> Console adapter

The service owns business rules. It does not print. Instead, it returns one of
several event types in a std::variant. The program edge decides how to display
the event.
*/

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <variant>

struct AddStock {
    std::string item;
    int amount{};
};

struct RemoveStock {
    std::string item;
    int amount{};
};

using InventoryCommand = std::variant<AddStock, RemoveStock>;

struct StockChanged {
    std::string item;
    int old_quantity{};
    int new_quantity{};
};

struct CommandRejected {
    std::string reason;
};

using InventoryEvent = std::variant<StockChanged, CommandRejected>;

template <class... Callables>
struct Overloaded : Callables... {
    using Callables::operator()...;
};

template <class... Callables>
Overloaded(Callables...) -> Overloaded<Callables...>;

class InventoryService {
public:
    [[nodiscard]] InventoryEvent execute(const InventoryCommand& command) {
        return std::visit(
            Overloaded{
                [this](const AddStock& add) { return handle(add); },
                [this](const RemoveStock& remove) { return handle(remove); },
            },
            command);
    }

    [[nodiscard]] int quantity_of(std::string_view item) const {
        const auto found = quantities_.find(item);
        return found == quantities_.end() ? 0 : found->second;
    }

private:
    InventoryEvent handle(const AddStock& command) {
        if (command.item.empty() || command.amount <= 0) {
            return CommandRejected{"item must be non-empty and amount must be positive"};
        }

        int& current = quantities_[command.item];
        const int old_quantity = current;
        current += command.amount;
        return StockChanged{command.item, old_quantity, current};
    }

    InventoryEvent handle(const RemoveStock& command) {
        if (command.item.empty() || command.amount <= 0) {
            return CommandRejected{"item must be non-empty and amount must be positive"};
        }

        const int current = quantity_of(command.item);
        if (command.amount > current) {
            return CommandRejected{"not enough stock for " + command.item};
        }

        quantities_[command.item] = current - command.amount;
        return StockChanged{command.item, current, current - command.amount};
    }

    std::map<std::string, int, std::less<>> quantities_;
};

void print_event(const InventoryEvent& event) {
    std::visit(
        Overloaded{
            [](const StockChanged& changed) {
                std::cout << "[CHANGED] " << changed.item << ": "
                          << changed.old_quantity << " -> " << changed.new_quantity << '\n';
            },
            [](const CommandRejected& rejected) {
                std::cout << "[REJECTED] " << rejected.reason << '\n';
            },
        },
        event);
}

void run_tests() {
    InventoryService inventory;

    const InventoryEvent added = inventory.execute(AddStock{"book", 5});
    assert(std::holds_alternative<StockChanged>(added));
    assert(inventory.quantity_of("book") == 5);

    const InventoryEvent removed = inventory.execute(RemoveStock{"book", 2});
    assert(std::get<StockChanged>(removed).new_quantity == 3);
    assert(inventory.quantity_of("book") == 3);

    const InventoryEvent rejected = inventory.execute(RemoveStock{"book", 10});
    assert(std::holds_alternative<CommandRejected>(rejected));
    assert(inventory.quantity_of("book") == 3);

    const InventoryEvent invalid = inventory.execute(AddStock{"book", 0});
    assert(std::holds_alternative<CommandRejected>(invalid));
}

int main() {
    run_tests();

    InventoryService inventory;
    const InventoryCommand commands[] = {
        AddStock{"keyboard", 4},
        RemoveStock{"keyboard", 1},
        RemoveStock{"keyboard", 10},
    };

    for (const InventoryCommand& command : commands) {
        print_event(inventory.execute(command));
    }

    std::cout << "final keyboard quantity: " << inventory.quantity_of("keyboard") << '\n';
    std::cout << "[TESTS] inventory exercise passed\n";
}
