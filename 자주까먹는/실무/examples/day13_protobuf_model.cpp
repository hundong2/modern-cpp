#include <cstdint>  // wire와 무관한 고정 폭 request ID를 표현한다.
#include <iostream> // round-trip 결과를 출력한다.
#include <string>   // 입력/직렬화 결과를 소유한다.

struct LoginRequest {                               // 실제로는 protoc가 생성할 타입의 교육 모형이다.
    std::string user_id;                            // schema의 field 1에 대응한다.
    std::string password;                           // schema의 field 2에 대응하며 로그 금지 정보다.
    std::uint64_t request_id;                       // schema의 field 3에 대응한다.
};                                                  // 메모리 배치는 wire format 계약이 아니다.

std::string serialize_for_demo(const LoginRequest& value) { // Protobuf 대신 경계를 보여 주는 함수다.
    return value.user_id + ':' + std::to_string(value.request_id); // 비밀번호를 직렬화/로그에서 제외한 모형이다.
}                                                   // 실제 코드는 SerializeToString 반환값을 검사한다.

int main() {                                        // schema 객체를 만드는 진입점이다.
    const LoginRequest request{"alice", "secret", 42}; // 타입 있는 요청 값을 초기화한다.
    const std::string bytes = serialize_for_demo(request); // 객체 의미를 wire bytes와 분리한다.
    std::cout << bytes << '\n';                     // 민감한 password는 출력하지 않는다.
    return bytes == "alice:42" ? 0 : 1;             // 교육용 직렬화 경계를 자체 검사한다.
}                                                   // 소유 문자열을 자동 파괴한다.
