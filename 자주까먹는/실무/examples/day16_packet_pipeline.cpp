#include <iostream>  // 파이프라인 완료 결과를 출력한다.
#include <optional>  // 검증 실패를 빈 값으로 표현한다.
#include <string>    // 각 계층이 소유하는 값을 제공한다.

struct Request {                                    // parsing 이후의 안전한 입력 DTO다.
    int user_id;                                    // 인증/logic에 전달할 사용자 ID다.
    std::string action;                             // 길이 검증된 동작 이름을 소유한다.
};                                                  // wire buffer 수명과 분리된다.

std::optional<Request> parse_and_validate(const std::string& frame) { // network→protocol 경계다.
    if (frame != "7:MOVE") return std::nullopt;     // malformed/미지원 입력을 logic 전에 차단한다.
    return Request{7, "MOVE"};                     // 검증된 값만 다음 계층으로 보낸다.
}                                                   // parser 상태가 없는 순수 모형이다.

std::string run_logic(const Request& request) {     // protocol→world logic 경계다.
    return request.action == "MOVE" ? "MOVED" : "DENIED"; // 권위 서버가 결과를 결정한다.
}                                                   // 네트워크/DB SDK를 직접 알지 않는다.

std::string serialize_response(const std::string& result) { // logic→outbound protocol 경계다.
    return std::to_string(result.size()) + ':' + result; // 길이 접두사 모형으로 응답을 만든다.
}                                                   // 실제 wire 정수는 endian/상한 규칙이 필요하다.

int main() {                                        // 한 패킷의 계층 이동을 실행한다.
    const auto request = parse_and_validate("7:MOVE"); // receive buffer에서 frame이 나왔다고 가정한다.
    if (!request) return 1;                         // 검증 실패면 즉시 연결 정책 경로로 간다.
    const std::string result = run_logic(*request); // 값 객체만 logic executor에 전달한다.
    const std::string outbound = serialize_response(result); // send queue에 넣을 소유 bytes를 만든다.
    std::cout << outbound << '\n';                  // 5:MOVED를 출력한다.
    return outbound == "5:MOVED" ? 0 : 1;          // 파이프라인 전체를 자체 검사한다.
}                                                   // 중간 값들을 역순 파괴한다.
