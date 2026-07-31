#include <iostream>
#include <concepts>
#include <string>
#include <stdexcept> //error handling

template<typename T>
concept HasSession = requires(T obj) {
    { obj.session_id } -> std::convertible_to<int>;
};

template<typename T>
concept HasPayload = requires(T obj) {
    { obj.payload } -> std::convertible_to<std::string>;
};

template<typename T>
concept IsLoggable = requires(T obj) {
    { obj.to_string() } -> std::convertible_to<std::string>;
};

template<typename T>
concept UserPacket = HasSession<T> && HasPayload<T>;

template<typename T>
concept SystemPacket = IsLoggable<T>;

template<typename T>
concept ProcessableMessage = UserPacket<T> || SystemPacket<T>;

template<ProcessableMessage T>
void route_message(T const& message) {
    if constexpr (UserPacket<T>) {
        std::cout << "Routing user packet: Session ID = " << message.session_id
                  << ", Payload = " << message.payload << std::endl;
    } else if constexpr (SystemPacket<T>) {
        std::cout << "Routing system packet: Log = " << message.to_string() << std::endl;
    }
}

template <UserPacket T>
void route_message(T const& msg) {
    // 런타임 방패: 실행 중 발생하는 논리적/환경적 에러를 잡아냅니다.
    try {
        std::cout << "[라우팅] 세션(" << msg.session_id << ") 처리 시작...\n";

        // 런타임 예외 1: 데이터 논리 에러 (빈 메시지)
        if (msg.payload.empty()) {
            throw std::invalid_argument("페이로드가 비어 있습니다.");
        }

        // 런타임 예외 2: 인프라 에러 시뮬레이션
        if (msg.session_id < 0) {
            throw std::runtime_error("데이터베이스 연결에 실패했습니다.");
        }

        std::cout << " -> 메시지 정상 처리 완료: " << msg.payload << "\n";

    } 
    // 예외 캐치 블록 (구체적인 에러부터 먼저 잡음)
    catch (const std::invalid_argument& e) {
        std::cerr << "[논리 에러] 클라이언트에게 경고 전송: " << e.what() << '\n';
    } 
    catch (const std::runtime_error& e) {
        std::cerr << "[시스템 에러] 서버 관리자에게 알림: " << e.what() << '\n';
    } 
    catch (const std::exception& e) {
        std::cerr << "[알 수 없는 에러] " << e.what() << '\n';
    }
}

struct UserMessage {
    int session_id;
    std::string payload;
};

struct SystemLog {
    int timestamp;
    std::string to_string() const {
        return "System log entry" + std::to_string(timestamp);
    }
};
struct IvalidPacket {
    int session_id;
    std::string payload;
    std::string to_string() const {
        return "Invalid packet";
    }
};

int main() {
    UserMessage chat = {1042, "안녕하세요, 파티 구합니다!"};
    UserMessage purchase = {2055, "buy_item_req"};
    SystemLog heartbeat = {1690000000};
    IvalidPacket bad_data = {1042, "해킹_페이로드"};

    std::cout << "--- 서버 메시지 처리 시작 ---\n";

    // 컴파일러는 각 호출마다 route_message 내부의 필요한 if constexpr 블록만 남기고 나머지는 지워버립니다.
    route_message(chat);      // 유저 패킷으로 처리됨
    route_message(purchase);  // 유저 패킷으로 처리됨
    route_message(heartbeat); // 시스템 패킷으로 처리됨

    // route_message(bad_data); 
    // 컴파일 에러 발생! 상속 기반(가상 함수)이었다면 런타임에 터지거나 버그를 유발했을 
    // 잘못된 데이터가 '컴파일 단계'에서 안전하고 완벽하게 차단됩니다.

    return 0;
}


// --- 서버 메시지 처리 시작 ---
// Routing user packet: Session ID = 1042, Payload = 안녕하세요, 파티 구합니다!
// Routing user packet: Session ID = 2055, Payload = buy_item_req
// Routing system packet: Log = System log entry1690000000