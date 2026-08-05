#include <iostream>  // 복사/이동 호출 로그를 출력한다.
#include <string>    // 내부 자원을 안전하게 관리하는 std::string을 쓴다.
#include <utility>   // std::move를 선언한다.

#define BAD_SQUARE(x) ((x) * (x))                  // 인자를 두 번 평가하는 위험한 텍스트 매크로다.

template <typename T>                              // 숫자 타입을 일반화할 템플릿을 선언한다.
constexpr T square(T value) {                      // 타입 검사되고 인자를 한 번만 받는 함수다.
    return value * value;                          // 상수 문맥에서는 컴파일 타임 계산될 수 있다.
}                                                  // 함수 템플릿 정의를 끝낸다.

class Packet {                                     // Rule of Zero로 자원을 관리하는 패킷 타입이다.
public:                                            // 교육용 생성/복사/이동 함수를 공개한다.
    explicit Packet(std::string bytes)             // 문자열 소유권을 값 매개변수로 받는다.
        : bytes_(std::move(bytes)) {               // 멤버로 이동해 별도 raw delete를 피한다.
        std::cout << "construct\n";               // 생성 호출을 표시한다.
    }                                              // std::string이 자기 버퍼를 관리한다.

    Packet(const Packet& other)                    // lvalue로부터 호출되는 복사 생성자다.
        : bytes_(other.bytes_) {                   // 새 문자열 버퍼에 내용을 복사한다.
        std::cout << "copy\n";                    // 복사 비용이 발생했음을 표시한다.
    }                                              // 새 객체는 원본과 독립적으로 소유한다.

    Packet(Packet&& other) noexcept                // xvalue/prvalue로부터 호출 가능한 이동 생성자다.
        : bytes_(std::move(other.bytes_)) {        // string의 이동 연산에 자원 이전을 맡긴다.
        std::cout << "move\n";                    // 이동 경로 선택을 표시한다.
    }                                              // other는 유효하지만 내용은 미지정 상태다.

    const std::string& bytes() const noexcept {    // 패킷을 복사하지 않고 읽는 접근자다.
        return bytes_;                             // 반환 참조는 Packet 수명을 넘기면 안 된다.
    }                                              // 관찰만 하므로 예외가 없다.

private:                                           // 자원 표현을 감춘다.
    std::string bytes_;                            // raw pointer 대신 표준 RAII 타입을 조합한다.
};                                                 // Packet 정의를 끝낸다.

int main() {                                       // 값 범주 실험을 시작한다.
    int i = 5;                                     // 부작용 관찰용 정수 객체다.
    const int safe = square(i++);                  // i를 한 번 평가해 25를 얻고 i는 6이 된다.
    std::cout << safe << ' ' << i << '\n';        // 안전한 함수 결과를 확인한다.
    // BAD_SQUARE(i++)는 i를 두 번 수정하므로 일부러 실행하지 않는다. // 위험 코드를 설명만 남긴다.

    Packet a{"payload"};                          // 이름 있는 객체 a를 생성하며 식 a는 lvalue다.
    Packet b = a;                                  // const Packet& 오버로드로 복사한다.
    Packet c = std::move(a);                       // a를 xvalue로 cast해 이동 생성자를 선택하게 한다.
    std::cout << b.bytes() << ' ' << c.bytes() << '\n'; // 두 대상의 유효한 내용을 확인한다.
    return 0;                                      // a도 유효하게 파괴되며 세 Packet 수명이 끝난다.
}                                                  // main을 끝낸다.
