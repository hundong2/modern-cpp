#include <iostream>
#include <string>
#include <string_view>
#include <vector>

struct User {
    std::string name;
    std::vector<int> permissions;
};

void PrintId(int id) {
    // int처럼 작고 단순한 타입은 값으로 받는 것이 보통 가장 단순합니다.
    // 레지스터 하나로 전달될 수 있어 const int&보다 유리하거나 비슷한 경우가 많습니다.
    std::cout << "id=" << id << '\n';
}

void PrintUser(const User& user) {
    // User는 string과 vector를 포함하므로 값으로 받으면 깊은 복사가 발생할 수 있습니다.
    // 읽기만 한다면 const User&가 실무 기본값입니다.
    std::cout << "user=" << user.name
              << " permission_count=" << user.permissions.size() << '\n';
}

void RenameUser(User& user, std::string_view new_name) {
    // User&는 원본을 수정하겠다는 뜻입니다.
    // string_view는 문자열을 소유하지 않는 뷰입니다. 복사 비용이 포인터+길이 정도입니다.
    user.name = new_name;
}

void MaybePrintUser(const User* user) {
    // 포인터 인자는 nullptr일 수 있다는 의미를 표현할 때 사용합니다.
    // nullptr이 허용되지 않는다면 const User&가 더 명확합니다.
    if (user == nullptr) {
        std::cout << "no user\n";
        return;
    }
    PrintUser(*user);
}

std::string BuildDisplayName(const User& user) {
    // 출력 파라미터보다 반환값을 선호하는 코드가 많습니다.
    // C++17부터는 많은 경우 반환값 최적화(RVO/NRVO)와 이동 의미론 덕분에
    // 큰 비용 없이 자연스럽게 값을 반환할 수 있습니다.
    return "user:" + user.name;
}

int main() {
    User user{"kim", {1, 3, 7}};

    PrintId(42);
    PrintUser(user);
    RenameUser(user, "lee");
    MaybePrintUser(&user);
    MaybePrintUser(nullptr);

    const std::string display_name = BuildDisplayName(user);
    std::cout << display_name << '\n';
}

