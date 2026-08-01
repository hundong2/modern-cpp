#include <iostream>
#include <vector>
#include <variant>

// 1. 상속(virtual)을 전혀 쓰지 않는 완전히 독립적인 구조체들
struct Player { void attack() { std::cout << "플레이어가 검을 휘두릅니다!\n"; } };
struct Monster { void roar() { std::cout << "몬스터가 포효합니다!\n"; } };
struct Trap { void trigger() { std::cout << "함정이 발동되어 데미지를 입힙니다!\n"; } };

// 2. 이 세 가지 타입 중 하나를 담을 수 있는 '안전한 상자' 타입을 정의합니다.
using EntityVariant = std::variant<Player, Monster, Trap>;

// 3. [모던 C++의 흑마법] 여러 개의 람다 함수를 하나로 합쳐주는 헬퍼 구조체 (Overloaded 패턴)
// 실무에서 std::visit을 쓸 때 무조건 세트로 쓰이는 C++17 표준 관용구입니다.
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int main() {
    // 4. 서로 다른 타입들을 가상 함수(부모 포인터) 없이 하나의 벡터에 값(Value)으로 때려 넣습니다.
    std::vector<EntityVariant> entities;
    entities.push_back(Player{});
    entities.push_back(Monster{});
    entities.push_back(Trap{});

    // 5. 배열을 순회하며 다형성을 실행합니다.
    for (auto& entity : entities) {
        
        // std::visit이 entity 상자를 열어보고, 내부 타입에 맞는 람다 함수를 자동으로 골라 실행합니다.
        std::visit(overloaded {
            [](Player& p)  { p.attack(); },
            [](Monster& m) { m.roar(); },
            [](Trap& t)    { t.trigger(); }
        }, entity);
        
    }
    return 0;
}