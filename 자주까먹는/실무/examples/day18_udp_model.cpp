#include <cstdint>  // sequence number의 고정 폭 타입을 제공한다.
#include <iostream> // 수신 판정 결과를 출력한다.
#include <string>   // datagram payload를 소유한다.

struct Datagram {                                   // UDP 한 datagram의 애플리케이션 헤더 모형이다.
    std::uint32_t sequence;                         // 순서/중복 검사용 monotonically increasing 번호다.
    std::string payload;                            // UDP가 보존하는 메시지 body다.
};                                                  // 실제 wire encode는 endian 변환이 필요하다.

class Receiver {                                    // 가장 단순한 stale/duplicate 필터다.
public:                                             // datagram 수락 함수를 공개한다.
    bool accept(const Datagram& packet) {           // packet을 복사하지 않고 읽는다.
        if (packet.sequence <= last_sequence_) return false; // 오래되거나 중복된 번호를 버린다.
        last_sequence_ = packet.sequence;           // 새 번호를 수신 상태로 기록한다.
        return true;                                // payload를 simulation에 적용해도 됨을 알린다.
    }                                               // wrap-around, reorder window는 이 단순 모형에 없다.

private:                                            // peer별 수신 상태를 감춘다.
    std::uint32_t last_sequence_ = 0;               // 아직 받은 패킷이 없다는 초기 상태다.
};                                                  // RUDP에는 ACK/재전송/혼잡 제어가 추가로 필요하다.

int main() {                                        // 순서가 섞인 datagram을 시험한다.
    Receiver receiver;                              // 한 peer의 상태를 만든다.
    const bool first = receiver.accept({1, "position=10"}); // 새 번호 1을 수락한다.
    const bool duplicate = receiver.accept({1, "position=10"}); // 중복 번호 1을 거부한다.
    const bool next = receiver.accept({2, "position=11"}); // 새 번호 2를 수락한다.
    std::cout << first << duplicate << next << '\n'; // 101을 출력한다.
    return first && !duplicate && next ? 0 : 1;     // 단순 필터 동작을 검증한다.
}                                                   // receiver를 파괴한다.
