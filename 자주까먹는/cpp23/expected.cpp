#include <iostream>
#include <string>
#include <expected> // C++23 추가됨
#include <concepts>

// 1. 런타임 에러의 종류를 Enum으로 명확하게 정의합니다.
enum class RouteError {
    EmptyPayload,
    InvalidSession,
    DbConnectionFailed
};

// (컴파일 타임 방패: 데이터 규격 검사)
template <typename T>
concept UserPacket = requires(T obj) {
    { obj.session_id } -> std::convertible_to<int>;
    { obj.payload } -> std::convertible_to<std::string>;
};

// 2. std::expected를 반환하는 처리 함수
// 해석: "성공하면 처리된 문자열(std::string)을 주고, 실패하면 RouteError를 주겠다"
std::expected<std::string, RouteError> process_message(UserPacket auto const& msg) {
    
    // 런타임 에러 1: 내용이 비어있음
    if (msg.payload.empty()) {
        // 에러를 반환할 때는 std::unexpected로 감싸서 보냅니다. (throw 대신 사용!)
        return std::unexpected(RouteError::EmptyPayload); 
    }
    
    // 런타임 에러 2: 세션 ID 불량
    if (msg.session_id < 0) {
        return std::unexpected(RouteError::InvalidSession);
    }

    // 성공: 그냥 결과값을 반환하면, 컴파일러가 알아서 expected 객체로 예쁘게 포장해 줍니다.
    return "[처리 완료] 정상 저장됨: " + msg.payload;
}

// ------------------------------------------
// 데이터 구조체 및 실행
// ------------------------------------------
struct ChatMessage {
    int session_id;
    std::string payload;
};

int main() {
    ChatMessage normal_chat = {1001, "안녕하세요!"};
    ChatMessage empty_chat = {1002, ""};       
    ChatMessage error_chat = {-1, "DB 테스트"}; 

    // 결과를 처리하는 람다 함수 (호출부)
    auto handle_result = [](const auto& result) {
        // 결과가 정상인지 확인
        if (result.has_value()) {
            // 성공: .value() (또는 *result)를 통해 알맹이를 꺼냅니다.
            std::cout << "✅ 성공: " << result.value() << '\n';
        } 
        else {
            // 실패: .error()를 통해 에러 코드를 꺼냅니다.
            std::cout << "❌ 실패: ";
            switch (result.error()) {
                case RouteError::EmptyPayload: 
                    std::cout << "메시지 내용이 비어있습니다.\n"; break;
                case RouteError::InvalidSession: 
                    std::cout << "유효하지 않은 세션입니다.\n"; break;
                case RouteError::DbConnectionFailed: 
                    std::cout << "데이터베이스 접속 실패.\n"; break;
            }
        }
    };

    std::cout << "--- std::expected 라우팅 테스트 ---\n";
    handle_result(process_message(normal_chat)); // 성공
    handle_result(process_message(empty_chat));  // 실패 (EmptyPayload)
    handle_result(process_message(error_chat));  // 실패 (InvalidSession)

    return 0;
}