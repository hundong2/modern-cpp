#include<string_view>
#include<iostream>
#include<cstdlib>
#include<string>

void ParseLog(std::string_view logView){
    std::string_view dateView = logView.substr(0, 10); // "2026-08-19"
    std::string_view timeView = logView.substr(11, 8); // "14:30:00"
    int year = std::stoi(std::string(dateView.substr(0, 4)));
    std::cout << "year: " << year << std::endl;
    std::cout << "year type: " << typeid(year).name() << std::endl; // int
    // C++에서는 std::string_view를 사용하여 문자열을 복사하지 않고도 부분 문자열을 효율적으로 참조할 수 있습니다.
    // 이는 C#의 ReadOnlySpan<char>와 유사한 개념으로, 메모리 복사 없이 문자열을 처리할 수 있어 성능 최적화에 유리합니다.
}

int main() {
    std::string log = "2026-08-19 14:30:00 INFO User logged in";
    ParseLog(log);
    return 0;
}