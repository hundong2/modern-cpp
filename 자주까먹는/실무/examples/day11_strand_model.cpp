#include <iostream>  // 최종 직렬 실행 결과를 출력한다.
#include <mutex>     // strand가 보장하는 직렬화를 단순 모형으로 표현한다.
#include <thread>    // 경쟁하는 두 호출자를 만든다.
#include <vector>    // thread 소유 컨테이너를 제공한다.

class SerializedSession {                          // 같은 세션의 handler가 겹치지 않는 모형이다.
public:                                            // 여러 thread가 호출할 dispatch를 공개한다.
    void dispatch_increment() {                    // 실제 Asio에서는 strand executor로 post한다.
        std::lock_guard<std::mutex> serial(lock_); // 모형에서는 mutex로 한 번에 한 handler만 실행한다.
        ++value_;                                  // 직렬 영역에서 세션 상태를 안전하게 갱신한다.
    }                                              // handler 끝에서 직렬화 잠금을 푼다.

    int value() const {                            // 결과를 thread-safe하게 읽는다.
        std::lock_guard<std::mutex> serial(lock_); // 갱신과 동일한 직렬화 영역에 들어간다.
        return value_;                             // 보호된 값을 복사해 반환한다.
    }                                              // 읽기 handler도 겹치지 않는다.

private:                                           // 모든 상태 접근을 API로 제한한다.
    mutable std::mutex lock_;                      // 교육 모형의 직렬 executor 역할을 한다.
    int value_ = 0;                                // lock_이 보호하는 세션 상태다.
};                                                 // 실제 strand는 mutex 자체가 아니라 실행 순서 계약이다.

int main() {                                       // 두 producer가 같은 세션에 작업을 보내는 모형이다.
    SerializedSession session;                     // 공유 세션을 만든다.
    std::vector<std::thread> producers;            // join할 두 thread를 소유한다.
    for (int i = 0; i < 2; ++i) {                 // producer 두 개를 시작한다.
        producers.emplace_back([&session] {        // session은 join까지 살아 있으므로 참조 캡처가 안전하다.
            for (int n = 0; n < 500; ++n) session.dispatch_increment(); // 직렬 handler를 반복한다.
        });                                        // producer 실행을 시작한다.
    }                                              // thread 생성을 끝낸다.
    for (auto& producer : producers) producer.join(); // 모든 갱신 완료를 기다린다.
    std::cout << session.value() << '\n';          // 1000을 출력한다.
    return session.value() == 1000 ? 0 : 1;        // 직렬화 결과를 검증한다.
}                                                  // main을 끝낸다.
