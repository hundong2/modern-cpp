#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

// 기존 타입 앨리어스 유지
template<typename T>
using GroupMap = std::unordered_map<std::string, std::vector<T>>;

template<typename T>
using ResourceDict = std::unordered_map<std::string, std::unique_ptr<T>>;

template<typename T>
using EventHandler = std::function<void(const T&, const std::string&)>;

class Player {
public: // C++은 기본 접근 제어자가 private이므로 public 명시 필요
    std::string name;
    int level;

    // 1-1. 단순 멤버 함수로 ToString 구현 (선택 사항)
    std::string ToString() const {
        return "Player: " + name + " (Lv." + std::to_string(level) + ")";
    }

    // 1-2. C++ 표준 방식의 문자열 출력 (operator<< 오버로딩)
    // friend 키워드를 사용해 전역 함수처럼 작동하게 함
    friend std::ostream& operator<<(std::ostream& os, const Player& p) {
        os << "Player{name: " << p.name << ", level: " << p.level << "}";
        return os;
    }

    // 2. C# Equals() 대안 (operator== 오버로딩)
    // C++20 이상: 컴파일러가 알아서 name과 level이 같은지 비교해줌
    bool operator==(const Player& other) const = default; 
    
    // C++17 이하인 경우 직접 구현:
    // bool operator==(const Player& other) const {
    //     return name == other.name && level == other.level;
    // }
};

// 3. C# GetHashCode() 대안 (std::hash 특수화)
// namespace std 안에 선언해야 합니다.
namespace std {
    template<>
    struct hash<Player> {
        size_t operator()(const Player& p) const {
            // 멤버 변수들의 해시값을 조합 (XOR 비트 연산)
            return hash<string>()(p.name) ^ (hash<int>()(p.level) << 1);
        }
    };
}

int main()
{
    Player p1{"Faker", 99};
    Player p2{"Chovy", 99};
    Player p3{"Faker", 99};

    // operator<< 사용
    std::cout << p1 << "\n"; 
    
    // 직접 만든 ToString() 사용
    std::cout << p1.ToString() << "\n"; 

    // operator== 사용 (결과: 0(false), 1(true))
    std::cout << "p1 == p2: " << (p1 == p2) << "\n";
    std::cout << "p1 == p3: " << (p1 == p3) << "\n";

    // std::hash 특수화 덕분에 Player 객체를 그대로 맵의 Key로 사용 가능
    std::unordered_map<Player, int> player_stats;
    player_stats[p1] = 1000;

    return 0;
}