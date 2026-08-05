#include <cstdint>   // 크기가 명확한 정수 타입 std::uint16_t를 제공한다.
#include <iostream>  // 조립된 패킷을 출력한다.
#include <optional>  // 아직 패킷이 없음을 값으로 표현한다.
#include <stdexcept> // 잘못된 길이에 std::runtime_error를 사용한다.
#include <string>    // 수신 바이트와 payload를 보관한다.

class LengthPrefixedParser {                       // 2바이트 big-endian 길이 프레임을 조립한다.
public:                                            // 수신 추가와 패킷 추출 API를 공개한다.
    void append(const std::string& bytes) {        // read가 준 임의 크기 조각을 받는다.
        buffer_.append(bytes);                     // 메시지 경계라고 가정하지 않고 누적한다.
    }                                              // 입력 문자열은 복사 후 없어져도 안전하다.

    std::optional<std::string> pop() {             // 완전한 패킷 하나가 있을 때만 반환한다.
        constexpr std::size_t header_size = 2;     // wire header가 정확히 두 바이트임을 명시한다.
        constexpr std::size_t max_body = 4096;     // 공격적 길이로 메모리가 커지는 것을 제한한다.
        if (buffer_.size() < header_size) {        // 길이 필드조차 덜 왔는지 검사한다.
            return std::nullopt;                   // 다음 read까지 기다린다.
        }                                          // 이제 첫 두 바이트를 안전하게 읽을 수 있다.
        const auto high = static_cast<unsigned char>(buffer_[0]); // signed char 부호 확장을 막는다.
        const auto low = static_cast<unsigned char>(buffer_[1]);  // 두 번째 wire byte를 양수로 읽는다.
        const std::size_t length = (high << 8U) | low;             // big-endian 길이를 host 정수로 조립한다.
        if (length > max_body) {                   // 프로토콜 상한을 넘는 peer를 거부한다.
            throw std::runtime_error("frame too large"); // 호출자가 연결 종료를 결정하게 한다.
        }                                          // 허용 가능한 길이다.
        if (buffer_.size() < header_size + length) { // body 전체가 아직 누적됐는지 검사한다.
            return std::nullopt;                   // 부분 패킷을 보존하고 기다린다.
        }                                          // 첫 패킷 전체가 준비됐다.
        std::string body = buffer_.substr(header_size, length); // body를 독립 소유 문자열로 복사한다.
        buffer_.erase(0, header_size + length);    // 소비한 프레임만 제거하고 다음 프레임은 남긴다.
        return body;                               // 완성된 패킷 하나를 호출자에게 넘긴다.
    }                                              // 다음 호출에서 남은 패킷을 계속 추출할 수 있다.

private:                                           // parser 내부 누적 상태를 감춘다.
    std::string buffer_;                           // 여러 read에 걸친 미완성 바이트를 소유한다.
};                                                 // parser 정의를 끝낸다.

int main() {                                       // 분할과 병합 수신을 동시에 시험한다.
    LengthPrefixedParser parser;                   // 빈 수신 버퍼를 만든다.
    parser.append(std::string{"\0\5he", 4});     // 길이 5 프레임의 헤더와 body 일부만 넣는다.
    if (parser.pop().has_value()) return 1;        // 아직 완성되면 버그이므로 실패한다.
    parser.append(std::string{"llo\0\3bye", 8}); // 첫 body 나머지와 두 번째 프레임을 한 번에 넣는다.
    const auto first = parser.pop();               // 첫 프레임 hello를 꺼낸다.
    const auto second = parser.pop();              // 남은 프레임 bye를 꺼낸다.
    if (!first || !second) return 1;               // 둘 중 하나라도 없으면 자체 검사 실패다.
    std::cout << *first << ' ' << *second << '\n'; // hello bye를 출력한다.
    return (*first == "hello" && *second == "bye") ? 0 : 1; // 내용까지 검증한다.
}                                                  // parser와 결과 문자열을 파괴한다.
