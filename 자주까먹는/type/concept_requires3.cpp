#include <iostream>
#include <concepts>
#include <string>

template<typename T>
concept HasId = requires(T obj) {
    { obj.id } -> std::convertible_to<int>;
};

template<typename T>
concept HasName = requires(T obj) {
    { obj.name } -> std::convertible_to<std::string>;
};

template<typename T>
concept HasToken = requires(T obj) {
    { obj.token } -> std::convertible_to<std::string>;
};

template<typename T>
concept ValidUser = HasId<T> && HasName<T>;

template<typename T>
concept Authentication = ValidUser<T> || HasToken<T>;

template<typename T>
void login_system(T const& user) {
    if constexpr (ValidUser<T>) {
        std::cout << "로그인 성공: ID = " << user.id << ", 이름 = " << user.name << std::endl;
    } else if constexpr (HasToken<T>) {
        std::cout << "로그인 성공: 토큰 = " << user.token << std::endl;
    } else {
        std::cout << "로그인 실패: 유효하지 않은 사용자 정보" << std::endl;
    }
}

struct User {
    static constexpr int id = 1;
    static constexpr const char* name = "Alice";
};

struct Guest {
    static constexpr const char* token = "guest_token_123";
};

struct CorruptData {
    int id = 9999;
};

int main() {
    User user;
    Guest guest;
    CorruptData corrupt;

    login_system(user);   // ValidUser 조건 만족
    login_system(guest);  // HasToken 조건 만족
    //login_system(corrupt); // 컴파일 에러: ValidUser도 아니고 HasToken도 아님

    return 0;
}
