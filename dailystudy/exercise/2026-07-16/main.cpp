#include <cassert>      // assert로 서비스 상태와 이벤트 타입을 검증한다.
#include <iostream>     // cout으로 콘솔 어댑터의 출력을 만든다.
#include <map>          // 상품 이름을 수량에 연결하는 정렬 연관 컨테이너를 사용한다.
#include <string>       // 상품명과 오류 문자를 소유하는 string을 사용한다.
#include <string_view>  // 조회용 상품명을 복사하지 않고 받는다.
#include <variant>      // 여러 후보 타입 중 하나만 담는 variant를 사용한다.

struct AddStock {
    // struct는 관련 값을 묶는 사용자 정의 타입이며 멤버가 기본 public이다.
    std::string item;
    int amount{};
};

struct RemoveStock {
    std::string item;  // 문자 버퍼를 소유하는 상품 이름이다.
    int amount{};      // 제거 수량을 0으로 값 초기화한다.
};

using InventoryCommand = std::variant<AddStock, RemoveStock>; // 후보 중 하나의 수명만 활성화된다.
// using은 새 타입을 만들지 않고 긴 variant 타입에 읽기 쉬운 별칭을 붙인다.

struct StockChanged {
    std::string item;    // 변경된 상품 이름을 이벤트가 독립적으로 소유한다.
    int old_quantity{};  // 변경 전 수량을 0으로 값 초기화한다.
    int new_quantity{};  // 변경 후 수량을 0으로 값 초기화한다.
};

struct CommandRejected {
    std::string reason;  // 거절 이유 문자의 메모리와 수명을 이벤트가 소유한다.
};

// 성공 이벤트 또는 거절 이벤트 하나를 저장하는 합 타입 별칭이다.
using InventoryEvent = std::variant<StockChanged, CommandRejected>;

// class...는 0개 이상의 타입을 받는 템플릿 매개변수 팩이다.
template <class... Callables>
struct Overloaded : Callables... {
    // 각 람다의 operator()를 현재 구조체 범위로 가져와 오버로드 집합을 만든다.
    using Callables::operator()...;
};

template <class... Callables>
// 추론 가이드는 전달한 람다 타입들로 Overloaded<...> 타입을 자동 결정하게 한다.
Overloaded(Callables...) -> Overloaded<Callables...>;

class InventoryService {
public:
    // public 아래 함수는 외부 계층이 호출할 수 있는 서비스 인터페이스다.
    [[nodiscard]] InventoryEvent execute(const InventoryCommand& command) {
        // command는 이름 있는 const 참조이므로 lvalue다. visit는 활성 객체를 const lvalue 참조로 전달한다.
        return std::visit(
            Overloaded{
                [this](const AddStock& add) { return handle(add); }, // this 포인터를 값으로 캡처한다.
                [this](const RemoveStock& remove) { return handle(remove); },
            },
            command);
    }

    [[nodiscard]] int quantity_of(std::string_view item) const {
        // auto는 map 반복자 타입을 추론한다. find는 키가 없으면 end 반복자를 돌려준다.
        const auto found = quantities_.find(item);
        // 조건 연산자 ?:는 조건에 따라 0 또는 저장된 수량 중 하나를 값으로 만든다.
        return found == quantities_.end() ? 0 : found->second;
    }

private:
    // private 아래 구현 함수와 상태는 서비스 내부에서만 접근할 수 있다.
    InventoryEvent handle(const AddStock& command) {
        if (command.item.empty() || command.amount <= 0) {
            // ||는 왼쪽이 참이면 오른쪽을 생략하는 단락 평가 논리합이다.
            return CommandRejected{"item must be non-empty and amount must be positive"};
        }

        // operator[] 결과는 map 내부 int의 lvalue. int&는 그 저장 위치에 별명을 붙여 직접 수정한다.
        int& current = quantities_[command.item];
        const int old_quantity = current;
        current += command.amount;
        // 중괄호로 만든 StockChanged는 prvalue이며 반환 객체를 직접 구성할 수 있다(복사 생략).
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
    // map이 키 string과 값 int를 소유한다. std::less<>는 string_view 조회도 허용한다.
};

void print_event(const InventoryEvent& event) {
    // 출력 책임은 서비스가 아니라 프로그램 가장자리의 콘솔 어댑터에 둔다.
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
    // assert가 거짓이면 테스트 실행을 중단해 규칙 위반을 눈에 보이게 한다.
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
    // main은 프로그램 진입점이며 끝까지 도달하면 C++에서 0을 반환한 것으로 처리된다.
    run_tests();

    InventoryService inventory;
    const InventoryCommand commands[] = {
        AddStock{"keyboard", 4},
        RemoveStock{"keyboard", 1},
        RemoveStock{"keyboard", 10},
    };

    for (const InventoryCommand& command : commands) {
        // const 참조로 variant 내부 string까지 복사하지 않고 각 명령을 읽는다.
        print_event(inventory.execute(command));
    }

    std::cout << "final keyboard quantity: " << inventory.quantity_of("keyboard") << '\n';
    std::cout << "[TESTS] inventory exercise passed\n";
}
