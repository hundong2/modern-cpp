#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

// ==============================================================================
// [실무 패턴 1] 복잡한 중첩 자료구조 단축
// ==============================================================================
// 실무에서는 "문자열 키"를 기준으로 "특정 타입의 배열"을 관리하는 해시맵을 자주 씁니다.
// 과거: std::unordered_map<std::string, std::vector<int>>, std::unordered_map<std::string, std::vector<Player>>...
template <typename T>
using GroupMap = std::unordered_map<std::string, std::vector<T>>;


// ==============================================================================
// [실무 패턴 2] 스마트 포인터와 결합한 리소스 관리
// ==============================================================================
// 메모리 누수를 막기 위해 객체를 독점(unique_ptr)하여 딕셔너리 형태로 관리하는 패턴입니다.
// T 자리에 어떤 리소스(Player, Texture, Sound)가 오든 이 한 줄로 타입 정의가 끝납니다.
template <typename T>
using ResourceDict = std::unordered_map<std::string, std::unique_ptr<T>>;


// ==============================================================================
// [실무 패턴 3] 제네릭 콜백(Callback) 함수 포인터
// ==============================================================================
// 특정 타입(T)의 객체와 문자열(IP 주소 등)을 받아서 처리하는 이벤트 핸들러 함수입니다.
// std::function을 매번 길게 쓰는 대신, 직관적인 'EventHandler'라는 이름을 부여합니다.
template <typename T>
using EventHandler = std::function<void(const T&, const std::string&)>;


// ==============================================================================
// 간단한 테스트용 데이터 클래스
// ==============================================================================
struct Player {
    std::string name;
    int level;
};


int main() {
    std::cout << "--- 1. GroupMap 사용 (자료구조 단축) ---\n";
    // 아주 깔끔하게 '정수를 담는 그룹 맵', '플레이어를 담는 그룹 맵'을 생성합니다.
    GroupMap<int> team_scores;
    team_scores["RedTeam"].push_back(100);
    team_scores["RedTeam"].push_back(85);

    GroupMap<Player> guild_members;
    guild_members["Warriors"].push_back({"Faker", 99});
    
    std::cout << "길드원 추가 완료: " << guild_members["Warriors"][0].name << "\n";


    std::cout << "\n--- 2. ResourceDict 사용 (스마트 포인터 팩토리) ---\n";
    ResourceDict<Player> connected_players;
    
    // 모던 C++의 std::make_unique를 사용하여 자원을 안전하게 할당하고 해시맵에 넣습니다.
    connected_players["user_1"] = std::make_unique<Player>(Player{"Chovy", 90});
    
    std::cout << "접속된 유저: " << connected_players["user_1"]->name << "\n";


    std::cout << "\n--- 3. EventHandler 사용 (이벤트 등록 및 실행) ---\n";
    // Player 타입을 처리하는 콜백 함수(람다)를 정의합니다.
    EventHandler<Player> on_player_login = [](const Player& p, const std::string& ip) {
        std::cout << "[System] " << p.name << " (Lv." << p.level << ") 님이 접속했습니다. (IP: " << ip << ")\n";
    };

    // 저장해둔 리소스(스마트 포인터)를 꺼내서 콜백 함수에 전달합니다.
    on_player_login(*connected_players["user_1"], "192.168.0.123");
    return 0; // 프로그램 종료 시 ResourceDict 내부에 있는 모든 Player 객체가 자동으로 메모리에서 해제됩니다.
}

// == execute: /modern-cpp/자주까먹는/build/alias_template ==

// --- 1. GroupMap 사용 (자료구조 단축) ---
// 길드원 추가 완료: Faker

// --- 2. ResourceDict 사용 (스마트 포인터 팩토리) ---
// 접속된 유저: Chovy

// --- 3. EventHandler 사용 (이벤트 등록 및 실행) ---
// [System] Chovy (Lv.90) 님이 접속했습니다. (IP: 192.168.0.123)