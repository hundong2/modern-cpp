#include <iostream>

// ==============================================================================
// [비교군] 일반적인 가상 함수 방식 (동적 다형성)
// 실행 시간에 어떤 함수를 부를지 결정하므로 vtable 조회 오버헤드가 발생함
// ==============================================================================
class DynamicFighter {
public:
    virtual void attack() { std::cout << "기본 공격!\n"; }
    virtual ~DynamicFighter() = default;
};

class DynamicWarrior : public DynamicFighter {
public:
    void attack() override { std::cout << "[동적] 전사가 칼을 휘두릅니다!\n"; } // vtable 조회 후 실행됨
};


// ==============================================================================
// [핵심] CRTP 방식 (정적 다형성)
// 컴파일 타임에 결정되므로 vtable이 아예 없습니다!
// ==============================================================================
template <typename Derived>
class StaticFighter {
public:
    void attack() {
        // 부모의 this 포인터를 자식 타입으로 캐스팅하여 자식의 함수를 직접 호출합니다.
        // 컴파일 시점에 Derived가 누군지 100% 확정되어 있으므로 static_cast가 절대적으로 안전합니다.
        static_cast<Derived*>(this)->attackImpl();
    }
};

// 상속받을 때 자기 자신(StaticWarrior)을 부모의 템플릿 인자로 넘깁니다.
class StaticWarrior : public StaticFighter<StaticWarrior> {
public:
    // virtual 키워드가 없습니다! 오버헤드 제로!
    void attackImpl() { 
        std::cout << "[CRTP] 전사가 칼을 휘두릅니다!\n"; 
    }
};

class StaticArcher : public StaticFighter<StaticArcher> {
public:
    void attackImpl() { 
        std::cout << "[CRTP] 궁수가 활을 쏩니다!\n"; 
    }
};

// ==============================================================================
// 다형성 호출 테스트
// ==============================================================================

// CRTP 객체들을 위한 공통 실행 함수 (템플릿으로 만듦)
template <typename T>
void executeAttack(StaticFighter<T>& fighter) {
    fighter.attack(); // 인라인화되어 어셈블리 레벨에서는 함수 호출 비용마저 사라질 수 있습니다.
}

int main() {
    StaticWarrior warrior;
    StaticArcher archer;

    // 객체의 타입에 맞춰 정확한 attackImpl()이 컴파일 타임에 맵핑됩니다.
    executeAttack(warrior);
    executeAttack(archer);

    return 0;
}