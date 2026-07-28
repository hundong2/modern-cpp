#include <string>
#include <vector>
#include <iostream>
class Player {
private: 
    // 실무에서는 멤버 변수임을 명확히 하기 위해 이름 뒤나 앞에 언더바(_)를 주로 붙입니다.
    std::string name_;
    int level_;
    std::vector<int> inventory_;

public:
    Player(std::string name, int level) 
        : name_(std::move(name)), level_(level) {}

    // 1. 무거운 객체(string)의 Getter
    // - [[nodiscard]]: 반환값 무시 방지
    // - const std::string&: 원본 복사 없이 안전하게 참조만 반환
    // - () const: 객체 상태 수정 불가
    [[nodiscard]] const std::string& name() const {
        return name_;
    }

    // 2. 가벼운 기본 타입(int)의 Getter
    // - int: 4바이트라 복사하는 게 참조보다 빠르거나 비슷하므로 값 반환
    [[nodiscard]] int level() const {
        return level_;
    }

    // 3. 컬렉션(vector)의 Getter
    // - 무조건 const 참조로 반환해야 메모리 폭탄을 피할 수 있음
    [[nodiscard]] const std::vector<int>& inventory() const {
        return inventory_;
    }

    Player& set_name(std::string name) {
        if (name.empty()) {
            // 실무에서는 로깅을 남기거나 예외를 던짐
            return *this; 
        }
        name_ = std::move(name); 
        return *this; // 메서드 체이닝을 위해 자기 자신 반환
    }

    // 2. 가벼운 타입(int)의 Setter
    // - int는 가벼우므로 그냥 값으로 받음
    // - 음수 레벨이 들어오지 못하도록 유효성 검증 추가
    Player& set_level(int level) {
        if (level < 1) {
            level_ = 1; // 최소 레벨 보정
        } else {
            level_ = level;
        }
        return *this;
    }
};

int main() {
    Player player("Faker", 99);
    auto& CopyPlayer = player; // 참조 복사: 얕은 복사
    player.set_name("Faker").set_level(99);
    Player CopyPlayer2 { player.name(), player.level() }; // 값 복사: 깊은 복사
    std::cout << "CopyPlayer2 이름: " << CopyPlayer2.name() << std::endl;
    std::cout << "CopyPlayer2 레벨: " << CopyPlayer2.level() << std::endl;
    std::cout << "CopyPlayer2 인벤토리 크기: " << CopyPlayer2.inventory().size() << std::endl;
    std::cout << "CopyPlayer 이름: " << CopyPlayer.name() << std::endl;
    std::cout << "CopyPlayer 레벨: " << CopyPlayer.level() << std::endl;
    std::cout << "CopyPlayer 인벤토리 크기: " << CopyPlayer.inventory().size() << std::endl;
    std::cout << "플레이어 이름: " << player.name() << std::endl;
    std::cout << "플레이어 레벨: " << player.level() << std::endl;
    std::cout << "인벤토리 크기: " << player.inventory().size() << std::endl;

    return 0;
}

// == execute: /modern-cpp/자주까먹는/build/gettersetter ==

// CopyPlayer2 이름: Faker
// CopyPlayer2 레벨: 99
// CopyPlayer2 인벤토리 크기: 0
// CopyPlayer 이름: Faker
// CopyPlayer 레벨: 99
// CopyPlayer 인벤토리 크기: 0
// 플레이어 이름: Faker
// 플레이어 레벨: 99
// 인벤토리 크기: 0