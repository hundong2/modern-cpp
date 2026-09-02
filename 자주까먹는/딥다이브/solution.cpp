#include <atomic>    // acquire/release publish에 사용할 std::atomic과 memory order를 선언한다.
#include <iostream>  // 정답 검증값을 출력할 std::cout을 선언한다.
#include <mutex>     // std::mutex, lock_guard, scoped_lock을 선언한다.
#include <thread>    // 동시 transfer와 consumer를 실행할 std::thread를 선언한다.
#include <utility>   // 두 잔액을 반환할 std::pair를 선언한다.

class Wallet final {                                      // 잔액과 보호 mutex를 같은 객체 수명에 묶는 정답 타입이다.
public:                                                    // caller가 사용할 생성자와 읽기 API를 공개한다.
    explicit Wallet(int initial) : balance_{initial} {}   // 변환을 막는 생성자로 초기 잔액을 직접 초기화한다.

    int balance() const {                                 // 내부 참조 대신 안전한 int snapshot을 반환한다.
        std::lock_guard<std::mutex> lock{mutex_};          // 정답 1: 단순 범위이므로 최소 상태의 lock_guard를 선택한다.
        return balance_;                                  // mutex를 소유한 동안 int 값을 복사해 반환한다.
    }                                                     // lock_guard 소멸로 mutex를 자동 해제한다.

    friend void transfer(Wallet& from, Wallet& to, int amount); // 두 private 상태를 함께 바꾸는 함수다.
    friend std::pair<int, int> snapshot(const Wallet& first, const Wallet& second); // 두 상태를 함께 읽는 함수다.

private:                                                   // 외부에서 보호 규약을 우회하지 못하게 한다.
    mutable std::mutex mutex_;                              // const 읽기에서도 잠글 수 있고 balance_ 접근을 보호한다.
    int balance_;                                          // mutex_ ownership 아래에서만 접근하는 공유 값이다.
};                                                         // Wallet class 정의를 끝낸다.

void transfer(Wallet& from, Wallet& to, int amount) {      // 두 wallet 합계를 보존하는 정답 함수를 정의한다.
    if (&from == &to || amount <= 0) {                     // 동일 mutex 중복과 잘못된 양을 lock 전에 거른다.
        return;                                            // 변경할 일이 없으므로 즉시 반환한다.
    }                                                      // 사전 검사 분기를 닫는다.
    std::scoped_lock lock{from.mutex_, to.mutex_};         // 정답 2: C++17 scoped_lock이 두 mutex를 RAII로 함께 획득한다.
    if (from.balance_ >= amount) {                         // 두 mutex 소유 중 출금 가능성을 검사한다.
        from.balance_ -= amount;                           // 출발 값을 감소시킨다.
        to.balance_ += amount;                             // 도착 값을 증가시켜 총합을 보존한다.
    }                                                      // 잔액 부족 시에는 둘 다 바꾸지 않는다.
}                                                          // scoped_lock이 두 mutex를 모두 해제한다.

std::pair<int, int> snapshot(const Wallet& first, const Wallet& second) { // 일관된 두 값 snapshot을 반환한다.
    if (&first == &second) {                               // 같은 객체인 경우 같은 mutex를 두 번 넘기지 않는다.
        const int one = first.balance();                   // 단일 mutex 공개 API로 안전한 값을 얻는다.
        return {one, one};                                 // 동일 값을 두 원소에 복사해 반환한다.
    }                                                      // 동일 객체 처리 분기를 닫는다.
    std::scoped_lock lock{first.mutex_, second.mutex_};    // 두 const 객체의 mutable mutex를 교착 회피 방식으로 획득한다.
    return {first.balance_, second.balance_};              // 두 lock을 소유한 같은 시점의 값을 pair로 복사한다.
}                                                          // scoped_lock이 파괴되어 두 mutex를 해제한다.

struct PublishedValue final {                              // publish protocol의 payload와 atomic flag를 묶는다.
    int payload{0};                                        // release 전에 쓰고 acquire 뒤 읽을 비원자 값이다.
    std::atomic<bool> ready{false};                         // synchronizes-with 간선을 운반하는 atomic 상태다.
};                                                         // PublishedValue 정의를 끝낸다.

int main() {                                               // 정답 프로그램의 진입점이다.
    Wallet left{500};                                      // 왼쪽 wallet을 500으로 생성한다.
    Wallet right{500};                                     // 오른쪽 wallet을 500으로 생성해 총합 1000을 만든다.
    std::thread one{[&] {                                  // join까지 안전한 두 wallet을 참조 capture한다.
        for (int i = 0; i < 1'000; ++i) {                  // 1000번 반대 worker와 경쟁한다.
            transfer(left, right, 1);                      // 왼쪽에서 오른쪽으로 1을 옮긴다.
        }                                                  // 첫 반복을 끝낸다.
    }};                                                    // 첫 worker thread를 생성한다.
    std::thread two{[&] {                                  // 같은 wallet을 반대 방향으로 사용할 worker다.
        for (int i = 0; i < 1'000; ++i) {                  // 1000번 transfer한다.
            transfer(right, left, 1);                      // 오른쪽에서 왼쪽으로 1을 옮긴다.
        }                                                  // 둘째 반복을 끝낸다.
    }};                                                    // 둘째 worker thread를 생성한다.
    one.join();                                            // snapshot 전에 첫 worker 완료를 기다린다.
    two.join();                                            // snapshot 전에 둘째 worker 완료를 기다린다.

    PublishedValue published;                              // producer/consumer보다 오래 사는 publish 객체다.
    int observed = 0;                                      // consumer 쓰기와 main 읽기를 join으로 정렬할 결과다.
    std::thread consumer{[&] {                             // publish 객체와 observed를 참조 capture한다.
        while (!published.ready.load(std::memory_order_acquire)) { // 정답 3: release의 true를 읽어 payload 쓰기를 acquire한다.
            std::this_thread::yield();                     // C++17 polling 동안 다른 thread에 실행 기회를 주는 힌트다.
        }                                                  // ready=true를 관찰하면 반복을 끝낸다.
        observed = published.payload;                      // payload write happens-before 이 read가 되어 race가 없다.
    }};                                                    // consumer thread를 생성한다.
    published.payload = 99;                                // producer가 payload를 먼저 쓴다.
    published.ready.store(true, std::memory_order_release); // 앞선 payload 쓰기를 flag의 true와 함께 publish한다.
    consumer.join();                                       // observed 읽기 전에 consumer 완료를 기다린다.

    const auto [left_value, right_value] = snapshot(left, right); // 두 잔액을 일관된 구조화 바인딩 값으로 얻는다.
    const int total = left_value + right_value;            // 총합 불변식을 local 값으로 계산한다.
    std::cout << "total=" << total << ", observed=" << observed << '\n'; // 예상 결과를 한 줄로 출력한다.
    return total == 1'000 && observed == 99 ? 0 : 1;       // 모든 검증이 맞으면 성공 0을 반환한다.
}                                                          // 자동 객체를 파괴하고 프로그램을 끝낸다.
