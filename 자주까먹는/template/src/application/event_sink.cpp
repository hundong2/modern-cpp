#include "modern_cpp/application/event_sink.hpp"  // 구현할 ConsoleEventSink 선언을 가져옵니다.

#include <iostream>  // 프로세스의 표준 출력 스트림 std::cout을 제공합니다.

namespace modern_cpp::application {  // application 계층 이름 공간에서 함수를 정의합니다.

void ConsoleEventSink::publish(std::string_view message) {  // 호출자 문자열을 소유하지 않고 함수 실행 중에만 읽습니다.
    std::cout << "[event] " << message << '\n';              // stream 버퍼에 문자를 쓰며 '\n'은 불필요한 강제 flush를 하지 않습니다.
}  // message view는 파괴되지만 가리키던 원본 문자열의 소유권에는 영향이 없습니다.

}  // namespace modern_cpp::application: 콘솔 이벤트 구현을 닫습니다.
