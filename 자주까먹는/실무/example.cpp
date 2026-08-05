#include <iostream>  // 표준 출력 스트림 std::cout을 선언한다.
#include <memory>    // 단독 소유 스마트 포인터 std::unique_ptr를 선언한다.
#include <string>    // 가변 길이 문자열 std::string을 선언한다.

class Session {                                      // 하나의 연결 세션을 흉내 내는 타입을 정의한다.
public:                                              // 아래 멤버를 클래스 밖에서 호출할 수 있게 공개한다.
    explicit Session(std::string name)               // 문자열을 값으로 받아 이 객체가 소유하게 하는 생성자다.
        : name_(std::move(name)) {                   // 매개변수의 버퍼를 멤버로 이동해 불필요한 복사를 피한다.
        std::cout << "create " << name_ << '\n';    // 생성 시점을 관찰하도록 로그를 남긴다.
    }                                                // 생성자 본문이 끝나도 name_은 Session과 함께 살아 있다.

    ~Session() {                                     // Session의 수명이 끝날 때 자동 호출되는 소멸자다.
        std::cout << "destroy " << name_ << '\n';   // RAII의 결정적 파괴 시점을 눈으로 확인한다.
    }                                                // std::string name_의 소멸자는 이 뒤 자동 호출된다.

    void ping() const {                              // 객체 상태를 바꾸지 않는 멤버 함수임을 const로 약속한다.
        std::cout << "ping " << name_ << '\n';      // 소유 문자열을 읽어 출력한다.
    }                                                // 함수의 지역 상태가 없으므로 별도 정리는 없다.

private:                                             // 구현 세부를 외부에서 직접 바꾸지 못하게 숨긴다.
    std::string name_;                               // Session이 직접 소유하며 Session과 수명을 같이한다.
};                                                   // 클래스 정의 끝에는 세미콜론이 필요하다.

int main() {                                         // 운영체제가 프로그램을 시작할 때 진입하는 함수다.
    auto owner = std::make_unique<Session>("Alice"); // 힙 객체를 만들고 단독 소유권을 owner에 둔다.
    Session* observer = owner.get();                  // 삭제 권한 없는 잠깐의 관찰 포인터를 얻는다.
    observer->ping();                                 // owner가 살아 있으므로 이 시점의 관찰 포인터는 유효하다.
    owner.reset();                                    // Session을 즉시 파괴하고 owner를 빈 상태로 만든다.
    observer = nullptr;                               // dangling 주소를 실수로 재사용하지 않도록 지운다.
    return 0;                                         // 0은 정상 종료이며 남은 지역 객체가 역순 파괴된다.
}                                                     // main 스코프와 프로그램 실행이 끝난다.
