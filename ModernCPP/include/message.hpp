#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <cstdint>
#include <array>
#include <span>

namespace common {

// C++20 개념을 사용한 메시지 타입
enum class MessageType : uint8_t {
    TEXT = 1,
    BINARY = 2,
    CONTROL = 3
};

// C++20 구조체 바인딩과 designated initializers 지원
struct Message {
    MessageType type;
    std::string data;
    
    // C++20 삼방향 비교 (spaceship operator)
    auto operator<=>(const Message&) const = default;
    
    // 메시지 직렬화
    std::string serialize() const {
        std::string result;
        result.push_back(static_cast<char>(type));
        result += data;
        return result;
    }
    
    // 메시지 역직렬화
    static Message deserialize(std::string_view raw) {
        if (raw.empty()) {
            return {MessageType::CONTROL, ""};
        }
        
        MessageType type = static_cast<MessageType>(raw[0]);
        std::string data(raw.substr(1));
        
        return {type, data};
    }
};

} // namespace common

#endif // MESSAGE_HPP
