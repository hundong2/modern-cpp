// <atomic>은 두 작업이 합계를 데이터 경쟁 없이 갱신하는 std::atomic을 선언한다.
#include <atomic>
// <cstddef>는 참가자 수에 사용하는 std::ptrdiff_t를 선언한다.
#include <cstddef>
// <iostream>은 실행 결과를 출력하는 std::cout을 선언한다.
#include <iostream>
// <latch>는 참가자들이 같은 시작선을 통과하게 하는 std::latch를 선언한다.
#include <latch>
// <thread>는 RAII 작업 스레드 std::jthread를 선언한다.
#include <thread>

// class의 기본 접근은 private이다. 장벽을 두 번 사용하는 잘못을 막고 도착 절차만 공개한다.
class BatchRelease {
public:
    // 생성자에는 반환형이 없고 explicit은 ptrdiff_t에서의 암시 변환을 막는다.
    // latch(ptrdiff_t)는 participants 값을 복사해 카운터로 저장하며 0 이상 max() 이하여야 한다.
    // atomic<int>(0)는 별도 원자 정수를 0으로 직접 초기화한다. 두 생성 결과는 멤버가 소유하고 반환값은 따로 없다.
    explicit BatchRelease(std::ptrdiff_t participants) : release_{participants}, passed_total_{0} {}

    void arrive_and_contribute(int contribution) noexcept {
        // latch::arrive_and_wait(ptrdiff_t update = 1)는 기본 update 1만큼 수신 release_ 카운터를 줄인 뒤 0까지 기다린다.
        // 데이터 인자는 생략됐고 반환형은 void라 저장하지 않는다. 호출 전 카운터가 1 이상이어야 하며 각 작업은 한 번만 호출한다.
        // 마지막 도착 이전 작업은 모든 반환 이후 작업보다 앞서며, latch는 0에서 다시 닫히지 않는다. 상수 시간·무할당·무예외다.
        release_.arrive_and_wait();

        // fetch_add의 첫 인자 contribution은 값 복사된 int lvalue, 둘째는 relaxed 메모리 순서다.
        // 증가 전 int 반환값은 버리며 수신 atomic만 바뀐다. 장벽 통과 순서까지 정하지는 않지만 원자 합계의 데이터 경쟁은 막는다.
        passed_total_.fetch_add(contribution, std::memory_order_relaxed);
    }

    [[nodiscard]] int total() const noexcept {
        // load(relaxed)는 인자로 메모리 순서만 받고 int 스냅샷을 반환하며 수신 atomic을 바꾸지 않는다.
        // 호출자는 두 jthread가 합류한 뒤에만 부르므로 작업 완료 수명 경계가 최종 값 관찰을 보장한다.
        return passed_total_.load(std::memory_order_relaxed);
    }

private:
    std::latch release_;             // 정확히 두 참가자가 도착하면 영구히 열린다.
    std::atomic<int> passed_total_; // 장벽을 통과한 작업이 기여한 합계를 소유한다.
};

int main() {
    BatchRelease release{2};

    {
        // jthread(F&&)의 F는 참조 캡처 람다 타입이며 추가 Args는 없다. 람다 prvalue를 이동해 새 스레드에서 호출한다.
        // 생성 성공 뒤 first가 스레드를 소유하고, 생성 실패는 system_error다. 캡처한 release는 이 블록보다 오래 산다.
        std::jthread first{[&release] { release.arrive_and_contribute(4); }};
        std::jthread second{[&release] { release.arrive_and_contribute(6); }};

        // 블록 끝에서 second와 first 소멸자가 역순으로 실행된다. joinable이면 중지를 요청하고 join해 작업 종료까지 기다린다.
        // jthread는 복사할 수 없고 여기서는 이동도 하지 않으므로 각 객체가 정확히 한 실행 스레드를 계속 소유한다.
    }

    const int total{release.total()};
    // ostream operator<<는 total과 줄바꿈을 출력 버퍼에 쓰고 ostream&를 반환해 연쇄하며 마지막 반환 참조는 버린다.
    // 출력은 release나 total을 바꾸지 않고, 실패 시 기본적으로 cout의 상태 비트가 설정된다.
    std::cout << total << '\n';

    return total == 10 ? 0 : 1;
}
