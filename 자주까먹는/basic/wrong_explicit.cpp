#include <iostream>
#include <string>
#include <vector>

class StringBuffer {
public:
    // [문제의 생성자] explicit이 없습니다!
    // 원래 의도: "size만큼의 바이트를 힙 메모리에 미리 할당해라"
    StringBuffer(size_t capacity) {
        std::cout << "[시스템] " << capacity << " 바이트 메모리 할당됨.\n";
        data_.reserve(capacity);
    }
    
    // 문자열을 덧붙이는 함수
    // [기본 문법] const std::string&: 불필요한 복사를 막고 읽기 전용으로 참조합니다.
    void append(const std::string& text) {
        std::cout << "데이터 추가: " << text << '\n';
    }

private:
    std::vector<char> data_;
};

// 버퍼를 받아서 로직을 처리하는 외부 함수
void processBuffer(StringBuffer buf) {
    buf.append("Processing...");
}

int main() {
    // ---------------------------------------------------------
    // [비극 1] 의도치 않은 객체 생성 (할당 연산자)
    // ---------------------------------------------------------
    // 개발자의 의도: "StringBuffer 객체에 1024라는 텍스트를 넣고 싶은 건가?"
    // 컴파일러의 해석: "아, 정수 1024를 StringBuffer로 형변환(생성자 호출)하라는 거구나!"
    StringBuffer my_buffer = 1024; // (X) 마치 문자열이 들어가는 것처럼 보이지만, 1024바이트가 할당됩니다.
    
    // ---------------------------------------------------------
    // [비극 2] 함수의 매개변수로 던질 때 터지는 폭탄
    // ---------------------------------------------------------
    // 개발자의 실수로 객체 대신 단순 숫자 50을 던졌습니다.
    // 원래라면 컴파일 에러가 나야 정상이지만...
    processBuffer(50); 
    
    // 컴파일러는 몰래 이렇게 코드를 바꿔치기해서 통과시킵니다.
    // processBuffer( StringBuffer(50) );
    // 결과: 아무 의미 없는 50바이트짜리 버퍼가 생성되었다가 즉시 소멸하며 메모리와 CPU를 낭비합니다.

    return 0;
}