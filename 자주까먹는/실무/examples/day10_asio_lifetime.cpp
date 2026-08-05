#include <functional> // 완료 callback 타입을 표현한다.
#include <iostream>   // 세션 수명 로그를 출력한다.
#include <memory>     // shared_from_this 수명 연장을 제공한다.
#include <string>     // 비동기 결과를 소유한다.

class Session : public std::enable_shared_from_this<Session> { // callback 동안 자기 수명을 연장할 타입이다.
public:                                                        // factory와 비동기 시작 API를 공개한다.
    static std::shared_ptr<Session> create() {                  // 생성자 대신 shared_ptr 소유를 먼저 확립한다.
        return std::shared_ptr<Session>(new Session);           // private 생성 객체를 제어 블록에 넣는다.
    }                                                           // 반환값이 최초 강한 소유자가 된다.

    ~Session() = default;                                       // shared_ptr가 마지막 소유권에서 공개적으로 파괴할 수 있게 한다.

    std::function<void(std::string)> begin_read() {             // Asio 완료 handler와 같은 모형을 만든다.
        auto self = shared_from_this();                          // 이미 shared_ptr가 소유하므로 안전하게 참조 수를 늘린다.
        return [self = std::move(self)](std::string bytes) {     // handler가 완료 때까지 Session을 소유한다.
            self->on_read(std::move(bytes));                    // 수신 데이터를 세션 전용 처리로 이동한다.
        };                                                      // handler가 파괴되면 캡처한 강한 참조도 감소한다.
    }                                                           // 호출자는 반환 handler의 실행/폐기를 책임진다.

private:                                                        // shared ownership 없이 직접 생성하지 못하게 한다.
    Session() { std::cout << "session create\n"; }             // factory만 호출 가능한 생성자다.
    void on_read(std::string bytes) { std::cout << bytes << '\n'; } // 완료 데이터를 소비하는 handler다.
};                                                              // session 수명 모형 정의를 끝낸다.

int main() {                                                    // pending handler의 수명 연장을 시험한다.
    auto session = Session::create();                           // 최초 강한 소유자를 만든다.
    auto completion = session->begin_read();                    // callback이 두 번째 강한 소유자가 된다.
    session.reset();                                            // 외부 소유자를 버려도 callback 때문에 객체가 남는다.
    completion("PING");                                        // 살아 있는 Session에서 on_read를 호출한다.
    return 0;                                                   // completion 파괴 때 마지막 참조가 사라진다.
}                                                               // main을 끝낸다.
