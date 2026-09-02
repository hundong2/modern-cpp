#include <atomic>    // TODO 3의 publish protocol에 사용할 std::atomic과 memory order를 선언한다.
#include <iostream>  // 실습 결과와 검증값을 출력할 std::cout을 선언한다.
#include <mutex>     // mutex, unique_lock, lock_guard, std::lock을 선언한다.
#include <thread>    // 두 worker와 publish/consume thread를 만들 std::thread를 선언한다.
#include <utility>   // std::pair를 선언한다.

class Wallet final {                                      // 잔액과 그 mutex 규약을 함께 소유하는 실습 타입이다.
public:                                                    // 생성자와 snapshot API를 공개하는 구역을 시작한다.
    explicit Wallet(int initial) : balance_{initial} {}   // 초기 잔액을 받아 balance_를 직접 초기화한다.

    int balance() const {                                 // 잠금으로 보호한 잔액 snapshot을 값으로 반환한다.
        std::unique_lock<std::mutex> lock{mutex_};         // TODO 1: 수동 unlock이 필요 없으므로 더 단순한 guard 타입으로 바꿔 본다.
        return balance_;                                  // int 값 복사 뒤 lock 소멸자가 mutex를 해제한다.
    }                                                     // const member 함수와 lock의 수명을 끝낸다.

    friend void transfer(Wallet& from, Wallet& to, int amount); // 두 wallet의 private 상태를 원자적 불변식 단위로 바꾼다.
    friend std::pair<int, int> snapshot(const Wallet& first, const Wallet& second); // 두 잔액 snapshot helper다.

private:                                                   // 동기화 규약을 우회하지 못하게 구현 멤버를 숨긴다.
    mutable std::mutex mutex_;                              // const snapshot에서도 잠글 수 있고 balance_ 접근을 보호한다.
    int balance_;                                          // mutex_ 소유 중에만 읽고 쓰는 비원자 공유 값이다.
};                                                         // Wallet class 정의를 끝낸다.

void transfer(Wallet& from, Wallet& to, int amount) {      // 두 wallet과 양을 받아 합계를 보존하는 함수다.
    if (&from == &to || amount <= 0) {                     // 같은 mutex 중복 획득과 의미 없는 양을 먼저 배제한다.
        return;                                            // 공유 상태를 건드리지 않고 함수에서 빠져나온다.
    }                                                      // 사전 검사 분기를 닫는다.
    std::unique_lock<std::mutex> from_lock{from.mutex_, std::defer_lock}; // 첫 mutex와 연결하되 아직 잠그지 않는다.
    std::unique_lock<std::mutex> to_lock{to.mutex_, std::defer_lock}; // 둘째 mutex도 미소유 상태로 wrapper에 연결한다.
    std::lock(from_lock, to_lock);                         // C++11 방식의 교착 회피 알고리즘으로 둘을 모두 획득한다.
    // TODO 2: 위 세 줄을 C++17 std::scoped_lock 한 줄로 바꾸고 소유 상태가 어떻게 단순해지는지 설명한다.
    if (from.balance_ >= amount) {                         // 두 lock을 모두 보유한 상태에서 출금 가능성을 검사한다.
        from.balance_ -= amount;                           // 출발 wallet 값을 감소시킨다.
        to.balance_ += amount;                             // 도착 wallet 값을 증가시켜 총합 불변식을 복원한다.
    }                                                      // 부족하면 두 값 모두 그대로 둔다.
}                                                          // 두 unique_lock이 역순 파괴되며 소유 mutex를 자동 해제한다.

std::pair<int, int> snapshot(const Wallet& first, const Wallet& second) { // 같은 시점의 두 잔액을 pair로 반환한다.
    if (&first == &second) {                               // 같은 wallet이면 mutex를 두 번 잠그지 않도록 처리한다.
        const int one = first.balance();                   // 공개 snapshot API로 단일 값을 안전하게 얻는다.
        return {one, one};                                 // 두 pair 원소에 같은 값을 복사해 반환한다.
    }                                                      // 동일 객체 분기를 닫는다.
    std::unique_lock<std::mutex> first_lock{first.mutex_, std::defer_lock}; // 첫 const 객체의 mutable mutex와 연결한다.
    std::unique_lock<std::mutex> second_lock{second.mutex_, std::defer_lock}; // 두 번째 mutex도 아직 잠그지 않는다.
    std::lock(first_lock, second_lock);                    // 두 mutex를 교착 회피 방식으로 모두 획득한다.
    return {first.balance_, second.balance_};              // lock 보유 중 일관된 두 int를 pair로 복사한다.
}                                                          // 두 lock을 해제하고 snapshot 함수를 끝낸다.

struct PublishedValue final {                              // payload와 publish flag를 한 객체 수명으로 묶는다.
    int payload{0};                                        // producer가 release 전 쓰고 consumer가 acquire 후 읽을 비원자 값이다.
    std::atomic<bool> ready{false};                         // 두 스레드의 publish 관계를 운반하는 원자 flag다.
};                                                         // PublishedValue 정의를 끝낸다.

int main() {                                               // 실습 프로그램의 진입점이며 검증 결과를 반환한다.
    Wallet left{500};                                      // 첫 wallet을 500으로 생성한다.
    Wallet right{500};                                     // 둘째 wallet도 500으로 생성해 총합 1000을 만든다.

    std::thread one{[&] {                                  // 두 wallet이 join까지 살아 있으므로 참조 capture한다.
        for (int i = 0; i < 1'000; ++i) {                  // 첫 worker가 1000번 transfer를 시도한다.
            transfer(left, right, 1);                      // 왼쪽에서 오른쪽으로 1을 안전하게 옮긴다.
        }                                                  // 첫 worker 반복을 끝낸다.
    }};                                                    // 첫 std::thread 생성 표현을 끝낸다.
    std::thread two{[&] {                                  // 반대 방향 worker도 같은 객체를 참조 capture한다.
        for (int i = 0; i < 1'000; ++i) {                  // 둘째 worker가 1000번 transfer를 시도한다.
            transfer(right, left, 1);                      // 오른쪽에서 왼쪽으로 1을 안전하게 옮긴다.
        }                                                  // 둘째 worker 반복을 끝낸다.
    }};                                                    // 둘째 std::thread 생성을 끝낸다.
    one.join();                                            // snapshot 전에 첫 worker 완료를 기다린다.
    two.join();                                            // snapshot 전에 둘째 worker 완료도 기다린다.

    PublishedValue published;                              // main scope에 publish 상태를 만들어 두 thread보다 오래 살게 한다.
    int observed = 0;                                      // consumer만 쓰고 join 뒤 main만 읽는 결과 변수다.
    std::thread consumer{[&] {                             // publish 객체와 observed를 안전한 수명 동안 참조 capture한다.
        while (!published.ready.load(std::memory_order_acquire)) { // TODO 3: acquire가 필요한 이유를 happens-before로 적는다.
            std::this_thread::yield();                     // C++17 실습의 busy wait가 CPU를 독점하지 않도록 양보 힌트를 준다.
        }                                                  // release store를 읽으면 loop를 끝낸다.
        observed = published.payload;                      // release 전 쓰기가 acquire 후 읽기에 happens-before라 안전하다.
    }};                                                    // consumer thread 생성을 끝낸다.
    published.payload = 99;                                // main producer가 비원자 payload를 먼저 완성한다.
    published.ready.store(true, std::memory_order_release); // 앞선 payload 쓰기를 true flag와 함께 publish한다.
    consumer.join();                                       // observed를 읽기 전에 consumer 쓰기 완료를 기다린다.

    const auto [left_value, right_value] = snapshot(left, right); // C++17 구조화 바인딩으로 일관된 pair를 푼다.
    const int total = left_value + right_value;            // 두 local snapshot을 더해 총합 불변식을 검사한다.
    std::cout << "total=" << total << ", observed=" << observed << '\n'; // 실습 결과를 한 줄로 출력한다.
    return total == 1'000 && observed == 99 ? 0 : 1;       // 두 불변식이 맞으면 성공 0, 아니면 실패 1을 반환한다.
}                                                          // main scope의 mutex/atomic 객체를 안전하게 파괴하고 끝낸다.
