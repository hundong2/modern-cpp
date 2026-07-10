#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

struct User {
    std::string name;
    int score{};
    bool active{};
};

int main() {
    std::vector<User> users{
        {"kim", 75, true},
        {"lee", 90, false},
        {"park", 82, true},
        {"choi", 60, true},
    };

    const auto found = std::find_if(users.begin(), users.end(),
        [](const User& user) {
            // 람다는 작은 함수 객체입니다.
            // 여기서는 User를 복사하지 않고 const 참조로 읽습니다.
            return user.active && user.score >= 80;
        });

    if (found != users.end()) {
        std::cout << "first active high scorer=" << found->name << '\n';
    }

    std::sort(users.begin(), users.end(),
        [](const User& left, const User& right) {
            return left.score > right.score;
        });

    const int minimum_score = 70;
    std::erase_if(users, [minimum_score](const User& user) {
        // minimum_score는 값으로 캡처됩니다.
        // int는 작으므로 값 캡처가 안전하고 저렴합니다.
        // 큰 객체는 const 참조 캡처를 고려하되, 람다가 원본보다 오래 살지 않아야 합니다.
        return user.score < minimum_score;
    });

    for (const auto& user : users) {
        std::cout << user.name << " " << user.score << '\n';
    }
}

