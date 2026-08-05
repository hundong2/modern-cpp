#include <iostream>  // 생성·소멸과 컨테이너 상태를 출력한다.
#include <map>       // 정렬된 key-value 컨테이너 std::map을 제공한다.
#include <memory>    // std::shared_ptr와 std::make_shared를 제공한다.
#include <string>    // 소유 문자열 std::string을 제공한다.

template <typename T>                               // T를 컴파일 타임 타입 매개변수로 선언한다.
class PacketBox {                                   // 여러 payload 타입에 재사용할 클래스 템플릿이다.
public:                                             // 사용자가 호출할 생성자와 관찰 함수를 공개한다.
    explicit PacketBox(T value)                     // 값을 받아 box가 독립적으로 소유하게 한다.
        : data_(std::move(value)) {}                // 매개변수의 자원을 멤버로 이동한다.

    const T& data() const noexcept {                // 복사 없이 읽기 전용 참조를 반환한다.
        return data_;                               // 참조는 *this보다 오래 보관하면 dangling이 된다.
    }                                               // noexcept이므로 이 함수는 예외를 던지지 않겠다고 약속한다.

private:                                            // 표현 세부를 외부에서 숨긴다.
    T data_;                                        // PacketBox가 T 객체를 값으로 직접 소유한다.
};                                                  // 클래스 템플릿 정의를 끝낸다.

class Session {                                     // 접속 세션의 최소 수명 모형이다.
public:                                             // 외부에서 생성하고 호출할 연산을 공개한다.
    Session(int id, std::string name)               // ID는 값, 이름은 소유할 값으로 받는다.
        : id_(id), name_(std::move(name)) {         // 선언 순서대로 두 멤버를 초기화한다.
        std::cout << "create " << name_ << '\n';   // 실제 생성 시점을 표시한다.
    }                                               // 생성된 객체는 두 멤버를 소유한다.

    ~Session() {                                    // 마지막 shared_ptr가 사라질 때 호출된다.
        std::cout << "destroy " << name_ << '\n';  // 자동 해제 시점을 관찰한다.
    }                                               // 문자열 멤버도 이어서 자동 파괴된다.

    void work() const {                             // 상태를 바꾸지 않는 세션 동작이다.
        std::cout << id_ << ':' << name_ << '\n';   // 두 멤버를 읽어 출력한다.
    }                                               // const 함수이므로 non-mutable 멤버를 수정할 수 없다.

private:                                            // 불변식에 속하는 데이터를 감춘다.
    int id_;                                        // 값 타입 ID는 Session 내부에 직접 저장된다.
    std::string name_;                              // 문자열 버퍼도 Session이 소유한다.
};                                                  // Session 정의를 끝낸다.

int main() {                                        // 예제 실행 진입점이다.
    PacketBox<int> number{1002};                    // T=int인 별도 템플릿 인스턴스를 만든다.
    PacketBox<std::string> token{"AUTH_TOKEN"};     // T=std::string인 인스턴스를 만든다.
    std::cout << number.data() << ' ' << token.data() << '\n'; // 두 타입이 같은 API를 쓰는지 확인한다.

    std::map<int, std::shared_ptr<Session>> registry; // map과 지역 변수가 세션을 공동 소유할 수 있다.
    {                                               // 지역 shared_ptr의 짧은 수명을 보여 주는 블록이다.
        auto alice = std::make_shared<Session>(1, "Alice"); // 객체와 제어 블록을 보통 한 번에 할당한다.
        registry.emplace(1, alice);                 // shared_ptr를 복사해 강한 참조 수를 하나 늘린다.
        std::cout << alice.use_count() << '\n';     // 학습용으로 2를 관찰하되 로직 판단에는 쓰지 않는다.
    }                                               // alice만 파괴되고 registry 소유권은 남는다.
    registry.at(1)->work();                         // map이 소유하므로 Session은 여전히 유효하다.
    registry.clear();                               // 마지막 shared_ptr를 없애 Session을 파괴한다.
    return 0;                                       // 정상 종료를 알리고 남은 지역 객체를 파괴한다.
}                                                   // main 스코프를 끝낸다.
