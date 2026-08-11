#include <iostream>
#include <vector>

// 1. [constexpr] 컴파일 타임 상수 (C++11)
// 빌드할 때 이미 10이라는 값이 기계어에 하드코딩됩니다. 런타임 비용 0.
constexpr int MAX_PLAYERS = 10;

// 2. [inline 변수] 여러 헤더 파일에서 중복 포함되어도 링커 에러를 막아줍니다. (C++17)
// 실무에서 전역 변수를 헤더에 정의할 때 무조건 사용합니다.
inline int g_active_sessions = 0; 

class NetworkManager {
public:
    // [기본 문법] const: 이 함수는 멤버 변수를 절대 수정하지 않음을 보장합니다.
    void init(int user_input) const {
        // 3. [const] 런타임 상수
        // user_input은 런타임(실행 중)에 결정되지만, 한 번 정해지면 current_limit은 변경 불가입니다.
        const int current_limit = user_input;
        
        // 에러 발생! constexpr은 실행 중에 값이 결정되는 변수(user_input)로 초기화할 수 없습니다.
        // constexpr int compile_limit = user_input; 
        
        std::cout << "제한: " << current_limit << '\n';
    }
};

int main() {
    NetworkManager net_mgr;
    net_mgr.init(5);

    std::cout << "MAX_PLAYERS: " << MAX_PLAYERS << '\n';
    std::cout << "g_active_sessions: " << g_active_sessions << '\n';

    return 0;
}

// == execute: /modern-cpp/자주까먹는/build/basic/constexpr_inline_const ==

// 제한: 5
// MAX_PLAYERS: 10
// g_active_sessions: 0