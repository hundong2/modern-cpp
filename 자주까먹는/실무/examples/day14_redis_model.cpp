#include <chrono>    // TTL을 강한 시간 단위로 표현한다.
#include <iostream>  // 저장 결과를 출력한다.
#include <memory>    // reply RAII를 위한 unique_ptr를 제공한다.
#include <string>    // key와 token을 소유한다.

struct Reply {                                      // hiredis redisReply의 수명만 흉내 내는 타입이다.
    bool ok;                                        // 명령 성공 여부를 값으로 보관한다.
};                                                  // 실제 reply에는 타입/길이/문자열 검사가 더 필요하다.

struct ReplyDeleter {                               // C API 전용 해제 함수를 타입에 묶는 custom deleter다.
    void operator()(Reply* reply) const noexcept {  // unique_ptr 소멸 때 정확히 한 번 호출된다.
        delete reply;                               // 모형에서는 delete, hiredis에서는 freeReplyObject를 쓴다.
    }                                               // 해제 함수는 예외를 던지지 않는다.
};                                                  // stateless deleter 정의를 끝낸다.

class RedisGateway {                                // 비즈니스 계층에서 Redis SDK를 감추는 adapter다.
public:                                             // 세션 저장 연산을 공개한다.
    bool put_session(const std::string& key, const std::string& token, // key와 token은 호출자 소유 값을 읽기만 한다.
                     std::chrono::seconds ttl) const { // TTL 단위를 int 대신 타입으로 받는다.
        if (key.empty() || token.empty() || ttl.count() <= 0) return false; // 명령 전 입력을 검증한다.
        std::unique_ptr<Reply, ReplyDeleter> reply(new Reply{true}); // C reply 소유권을 즉시 RAII에 넣는다.
        return reply && reply->ok;                  // null과 reply 상태를 모두 확인한다.
    }                                               // 모든 return 경로에서 reply가 자동 해제된다.
};                                                  // gateway 정의를 끝낸다.

int main() {                                        // Redis 호출 경계의 수명을 시험한다.
    RedisGateway redis;                             // 실제 구현이라면 connection/pool을 주입한다.
    const bool ok = redis.put_session("session:1", "token", std::chrono::seconds{60}); // 단위가 명확한 TTL을 보낸다.
    std::cout << std::boolalpha << ok << '\n';      // true를 출력한다.
    return ok ? 0 : 1;                              // 성공 여부를 종료 코드로 반환한다.
}                                                   // gateway를 파괴한다.
