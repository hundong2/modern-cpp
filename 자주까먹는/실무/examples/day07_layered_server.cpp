#include <iostream>  // 계층을 지난 결과를 출력한다.
#include <stdexcept> // 검증 실패를 예외 값으로 전달한다.
#include <string>    // wire bytes와 command 문자열을 소유한다.

struct Command {                                    // 프로토콜 계층이 만든 의미 있는 요청이다.
    int user_id;                                    // 인증된 사용자를 가리키는 값 타입이다.
    std::string verb;                               // 실행할 명령 이름을 소유한다.
};                                                  // transport 바이트와 분리된 DTO 정의 끝이다.

class Protocol {                                    // wire 표현을 Command로 바꾸는 경계다.
public:                                             // parser API를 공개한다.
    Command parse(const std::string& frame) const { // 완성된 frame을 읽기 전용으로 받는다.
        if (frame != "1:PING") {                   // 교육용 허용 목록으로 입력을 검증한다.
            throw std::runtime_error("bad frame"); // 잘못된 입력이 logic으로 가지 않게 막는다.
        }                                           // 검증에 성공한 경로다.
        return Command{1, "PING"};                 // wire와 독립적인 값 객체를 반환한다.
    }                                               // 지역 임시 객체는 반환값 최적화될 수 있다.
};                                                  // protocol 계층 정의를 끝낸다.

class GameService {                                 // 비즈니스 규칙만 담당하는 계층이다.
public:                                             // command 처리 API를 공개한다.
    std::string handle(const Command& command) const { // 복사 없이 검증된 명령을 읽는다.
        return command.verb == "PING" ? "PONG" : "ERROR"; // 네트워크 API 없이 규칙을 실행한다.
    }                                               // 응답 의미를 소유 문자열로 반환한다.
};                                                  // service 계층 정의를 끝낸다.

int main() {                                        // 계층 연결(composition root)을 구성한다.
    Protocol protocol;                              // 상태 없는 parser 객체를 만든다.
    GameService service;                            // 상태 없는 logic 객체를 만든다.
    const Command command = protocol.parse("1:PING"); // transport에서 받았다고 가정한 frame을 해석한다.
    const std::string response = service.handle(command); // 검증된 명령만 logic에 전달한다.
    std::cout << response << '\n';                  // transport send 대신 결과를 관찰한다.
    return response == "PONG" ? 0 : 1;             // 자체 검증을 종료 코드로 반환한다.
}                                                   // 모든 계층 객체를 역순 파괴한다.
