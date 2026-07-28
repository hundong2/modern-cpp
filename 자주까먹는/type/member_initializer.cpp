#include <iostream>
#include <string>

// std::string을 흉내 내어 내부 동작을 추적하는 클래스
class MyString {
public:
    // 1. 빈 껍데기가 만들어질 때 (기본 생성자)
    MyString() { 
        std::cout << "  -> [비효율] 기본 생성자 호출 (빈 메모리 할당)\n"; 
    }
    
    // 2. 실제 문자열을 받을 때
    MyString(const std::string& str) { }
    
    // 3. 다른 MyString을 복사할 때
    MyString(const MyString& other) { 
        std::cout << "  -> [비효율] 복사 생성자 호출 (무거운 데이터 복사!)\n"; 
    }
    
    // 4. 기존 변수에 다른 값을 덮어씌울 때
    MyString& operator=(const MyString& other) { 
        std::cout << "  -> [비효율] 대입 연산자 호출 (기존 메모리 버리고 다시 복사!)\n"; 
        return *this; 
    }
    
    // 5. C++11 이동 생성자 (가장 빠름)
    MyString(MyString&& other) noexcept { 
        std::cout << "  -> [최적화 ⚡] 이동 생성자 호출 (메모리 껍데기만 가로채기!)\n"; 
    }
};

// ❌ 나쁜 예시: 본문에서 대입
class BadPlayer {
private:
    MyString name_;
public:
    BadPlayer(MyString name) {
        std::cout << "  [BadPlayer 생성자 본문 진입]\n";
        name_ = name; // 여기서 덮어쓰기 발생
    }
};

// ✅ 좋은 예시: 초기화 리스트 + std::move 사용
class GoodPlayer {
private:
    MyString name_;
public:
    GoodPlayer(MyString name) : name_(std::move(name)) {
        std::cout << "  [GoodPlayer 생성자 본문 진입]\n";
    }
};

int main() {
    std::cout << "====================================\n";
    std::cout << "1. BadPlayer 생성 테스트 (본문 대입)\n";
    std::cout << "====================================\n";
    MyString name1("Faker"); // 테스트용 문자열 준비
    
    std::cout << "\n[객체 생성 시작]\n";
    BadPlayer p1(name1);
    
    std::cout << "\n====================================\n";
    std::cout << "2. GoodPlayer 생성 테스트 (초기화 리스트)\n";
    std::cout << "====================================\n";
    MyString name2("Chovy"); // 테스트용 문자열 준비
    
    std::cout << "\n[객체 생성 시작]\n";
    GoodPlayer p2(name2);

    return 0;
}