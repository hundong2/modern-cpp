#include <iostream>  // std::cout 출력 스트림을 선언한다; 출력은 join 뒤 main 스레드만 수행한다.
#include <mutex>     // std::mutex, std::lock_guard, std::unique_lock, std::scoped_lock을 선언한다.
#include <thread>    // std::thread와 std::this_thread를 선언한다.
#include <utility>   // std::pair를 선언한다.
#include <vector>    // std::vector를 선언해 move-only std::thread 객체들을 소유한다.

class Account final {                                      // 상속이 필요 없는 계좌와 그 동기화 규약을 한 객체에 묶는다.
public:                                                    // 호출자가 사용할 공개 API 구역을 시작한다.
    explicit Account(int initial_balance)                 // int 초기 잔액 하나를 받는 변환 방지 생성자다.
        : balance_{initial_balance} {}                    // balance_ 객체를 본문 전에 직접 초기화하고 mutex_는 기본 생성한다.

    Account(const Account&) = delete;                     // mutex는 복사할 수 없으므로 계좌 복사도 명시적으로 금지한다.
    Account& operator=(const Account&) = delete;          // 복사 대입 역시 금지하며 반환 타입은 관례적인 Account&다.

    void deposit(int amount) {                            // 입금액을 받고 결과를 반환하지 않는 스레드 안전 함수다.
        std::lock_guard<std::mutex> guard{mutex_};         // 생성자가 mutex_.lock(), 소멸자가 unlock()을 호출하는 RAII guard다.
        balance_ += amount;                               // lock 소유 중 read-modify-write하여 다른 계좌 연산과 충돌하지 않는다.
    }                                                     // guard 수명이 끝나므로 정상·예외 경로 모두 여기서 mutex_가 해제된다.

    bool try_deposit(int amount) {                        // 기다리지 않는 입금을 시도하고 성공 여부를 bool로 반환한다.
        std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock}; // try_lock을 한 번 수행하고 소유 상태를 저장한다.
        if (!lock.owns_lock()) {                          // owns_lock은 mutex 획득 성공 여부를 bool로 반환한다.
            return false;                                 // 경쟁으로 실패한 정상 제어 흐름이며 balance_에는 접근하지 않는다.
        }                                                 // 실패 처리 분기를 닫는다.
        balance_ += amount;                               // 이 경로에서는 lock이 mutex_를 소유하므로 안전하게 값을 바꾼다.
        return true;                                      // 성공을 반환한 뒤 lock 소멸자가 자동으로 mutex_를 해제한다.
    }                                                     // 함수와 unique_lock 객체의 수명을 끝낸다.

    int balance() const {                                 // 현재 잔액의 snapshot을 값으로 반환하며 내부 참조를 노출하지 않는다.
        std::lock_guard<std::mutex> guard{mutex_};         // const 함수에서도 mutable mutex_를 잠가 읽기와 쓰기를 정렬한다.
        return balance_;                                  // int를 값 복사해 반환하므로 unlock 후에도 caller가 안전하게 사용한다.
    }                                                     // guard가 파괴되어 mutex_를 해제한다.

    friend void transfer(Account& from, Account& to, int amount); // 두 객체 private 멤버를 함께 갱신할 함수에 접근을 허용한다.
    friend std::pair<int, int> balances(const Account& left, const Account& right); // 일관된 두 잔액 snapshot 함수다.

private:                                                   // 외부에서 mutex 규약을 우회할 수 없는 구현 구역을 시작한다.
    mutable std::mutex mutex_;                              // const snapshot도 잠글 수 있게 mutable이며 balance_의 모든 접근을 보호한다.
    int balance_;                                          // mutex_ 아래에서만 읽고 쓰는 비원자 공유 상태다.
};                                                         // Account 클래스 정의를 끝내며 세미콜론이 필요하다.

void transfer(Account& from, Account& to, int amount) {    // 출발/도착 계좌와 이동량을 참조/값으로 받으며 반환값은 없다.
    if (&from == &to || amount <= 0) {                     // 같은 mutex를 두 번 잠그거나 잘못된 양을 처리하지 않는 사전 검사다.
        return;                                            // 공유 상태에 손대지 않고 즉시 반환한다.
    }                                                      // 사전 검사 분기를 닫는다.
    std::scoped_lock lock{from.mutex_, to.mutex_};         // C++17 CTAD로 타입을 추론하고 두 mutex를 교착 회피 방식으로 획득한다.
    if (from.balance_ >= amount) {                         // 두 mutex를 모두 소유한 일관된 상태에서 출금 가능성을 검사한다.
        from.balance_ -= amount;                           // 출발 잔액을 먼저 줄이지만 lock 중이라 중간 불변식은 외부에 보이지 않는다.
        to.balance_ += amount;                             // 도착 잔액을 늘려 두 계좌 합계 불변식을 복원한다.
    }                                                      // 잔액 부족이면 두 값 모두 바꾸지 않는다.
}                                                          // scoped_lock이 두 mutex를 해제하며 함수가 끝난다.

std::pair<int, int> balances(const Account& left, const Account& right) { // 두 잔액의 같은 시점 snapshot을 pair로 반환한다.
    if (&left == &right) {                                 // 같은 객체라면 동일 mutex를 두 번 넘기지 않도록 분리한다.
        const int value = left.balance();                  // 단일 객체의 공개 잠금 API로 snapshot을 얻는다.
        return {value, value};                             // pair의 두 원소를 같은 값으로 초기화해 반환한다.
    }                                                      // 동일 객체 예외 처리를 닫는다.
    std::scoped_lock lock{left.mutex_, right.mutex_};      // mutable mutex이므로 const Account에서도 두 lock을 함께 획득한다.
    return {left.balance_, right.balance_};                // 두 mutex 소유 중 두 int를 값으로 복사해 일관된 pair를 반환한다.
}                                                          // scoped_lock 소멸로 두 mutex를 해제한다.

int main() {                                               // 운영체제가 호출하는 프로그램 진입점이며 성공 시 0을 반환한다.
    Account left{1'000};                                   // 왼쪽 계좌를 잔액 1000으로 자동 저장 기간에 생성한다.
    Account right{1'000};                                  // 오른쪽 계좌도 잔액 1000으로 생성해 총합 2000 불변식을 만든다.
    std::vector<std::thread> workers;                       // thread 관리 객체를 소유하는 vector를 생성한다.
    workers.reserve(4);                                    // 재할당을 피하도록 thread 네 개의 저장 공간을 미리 확보한다.

    for (int worker_id = 0; worker_id < 4; ++worker_id) {  // worker ID 0~3으로 네 실행 흐름을 만든다.
        workers.emplace_back([worker_id, &left, &right] {  // ID는 값, 계좌는 참조 capture하며 main이 join까지 수명을 보장한다.
            for (int i = 0; i < 2'000; ++i) {              // 각 worker가 충분한 lock 경합을 만들도록 2000번 반복한다.
                if ((worker_id % 2) == 0) {                // 짝수 worker인지 계산하며 이 값은 스레드 local capture다.
                    transfer(left, right, 1);              // 짝수 worker는 왼쪽에서 오른쪽으로 1을 안전하게 옮긴다.
                } else {                                   // 홀수 worker 경로를 시작한다.
                    transfer(right, left, 1);              // 반대 방향도 같은 scoped_lock 프로토콜을 사용한다.
                }                                          // 방향 분기를 닫는다.
            }                                              // worker 반복을 끝낸다.
        });                                                // lambda로 새 std::thread를 만들고 vector 끝에 move-construct한다.
    }                                                      // 네 worker 생성 loop를 끝낸다.

    for (std::thread& worker : workers) {                  // vector의 각 move-only thread 객체를 비소유 참조로 순회한다.
        worker.join();                                     // worker 완료를 기다려 shared account 접근이 모두 끝났음을 보장한다.
    }                                                      // 모든 join을 마쳐 main이 유일한 실행 접근자가 된다.

    left.deposit(5);                                       // 가장 단순한 단일 mutex 범위에는 lock_guard 기반 API를 사용한다.
    const bool try_succeeded = right.try_deposit(7);       // 현재 경쟁이 없어 보통 성공하며 bool 결과를 저장한다.
    const auto [left_balance, right_balance] = balances(left, right); // C++17 구조화 바인딩으로 pair snapshot을 두 값에 푼다.
    const int total = left_balance + right_balance;        // join 뒤 local int 두 개를 더해 총합을 계산한다.

    std::cout << "left=" << left_balance                  // main 스레드만 cout을 사용해 출력 interleaving을 피한다.
              << ", right=" << right_balance             // 두 번째 잔액을 같은 출력 식에 이어 붙인다.
              << ", total=" << total                     // 초기 2000 + 확정 입금 5 + 성공 시 7을 출력한다.
              << ", try_deposit=" << std::boolalpha << try_succeeded // bool을 0/1 대신 true/false로 표시한다.
              << '\n';                                    // 한 줄을 끝내며 불필요한 강제 flush는 하지 않는다.

    const int expected_total = 2'005 + (try_succeeded ? 7 : 0); // try 결과를 반영한 정확한 예상 합계를 계산한다.
    return total == expected_total ? 0 : 1;                // 불변식이 맞으면 성공 0, 아니면 테스트 실패 1을 OS에 반환한다.
}                                                          // main의 자동 객체가 역순 파괴되고 프로그램이 끝난다.
