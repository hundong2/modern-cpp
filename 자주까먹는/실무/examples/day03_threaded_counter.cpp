#include <iostream>  // 최종 카운터 값을 출력한다.
#include <mutex>     // 공유 상태를 보호할 std::mutex를 제공한다.
#include <thread>    // 병렬 실행 흐름 std::thread를 제공한다.
#include <vector>    // 생성한 스레드들을 소유하는 컨테이너다.

class UserCounter {                                // 접속자 수와 보호 mutex를 한 객체에 묶는다.
public:                                            // 안전한 갱신과 읽기 API를 공개한다.
    void add_one() {                               // 여러 스레드가 동시에 호출할 수 있다.
        std::lock_guard<std::mutex> lock(mutex_);  // 스코프 동안 mutex를 잠그고 예외 시에도 해제한다.
        ++value_;                                  // 보호된 임계 구역에서만 공유 값을 쓴다.
    }                                              // lock이 파괴되며 mutex가 자동 unlock된다.

    int value() const {                            // 현재 값을 일관되게 읽는 함수다.
        std::lock_guard<std::mutex> lock(mutex_);  // 읽기도 쓰기와 같은 mutex로 동기화한다.
        return value_;                             // 잠긴 동안 값을 복사해 반환한다.
    }                                              // 반환 뒤 lock이 파괴된다.

private:                                           // mutex 없이 필드에 접근하지 못하게 감춘다.
    mutable std::mutex mutex_;                     // const value()에서도 논리 상태가 아닌 lock을 바꾼다.
    int value_ = 0;                                // mutex_가 보호하는 공유 불변식이다.
};                                                 // thread-safe counter 정의를 끝낸다.

int main() {                                       // 네 worker를 만드는 진입점이다.
    UserCounter counter;                           // 모든 worker가 참조할 공유 객체다.
    std::vector<std::thread> workers;              // join 전까지 thread 객체를 소유한다.
    for (int worker = 0; worker < 4; ++worker) {   // 네 개의 실행 흐름을 만든다.
        workers.emplace_back([&counter] {          // counter를 참조 캡처하므로 main 끝 전 join해야 한다.
            for (int n = 0; n < 1000; ++n) {      // 각 worker가 천 번 갱신한다.
                counter.add_one();                 // mutex가 매 증가의 data race를 방지한다.
            }                                      // worker의 반복을 끝낸다.
        });                                        // lambda를 새 std::thread에서 실행한다.
    }                                              // 모든 thread 시작을 끝낸다.
    for (auto& worker : workers) {                 // 소유한 thread를 하나씩 방문한다.
        worker.join();                             // 완료를 기다려 참조 수명과 종료를 보장한다.
    }                                              // 모든 worker가 끝났다.
    std::cout << counter.value() << '\n';          // 항상 4000이어야 한다.
    return counter.value() == 4000 ? 0 : 1;        // 자체 검사 결과를 종료 코드로 돌려준다.
}                                                  // counter와 thread 컨테이너를 파괴한다.
