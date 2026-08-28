#include <cassert>    // 이동 뒤 소유권과 RAII 반환을 assert로 검증한다.
#include <iostream>   // 결과를 std::cout으로 출력한다.
#include <semaphore>  // std::counting_semaphore로 동시 자원 상한을 표현한다.
#include <utility>    // std::exchange와 std::move를 사용한다.

// 실무의 DB 연결·GPU 슬롯·외부 API 동시 요청처럼 최대 두 개만 빌리는 용량 게이트다.
class ConnectionGate {
private:
    // using은 긴 템플릿 특수화에 이 클래스 안에서만 쓰는 별칭 이름을 붙인다.
    using Semaphore = std::counting_semaphore<2>;

public:
    class Lease {
    public:
        // 복사를 허용하면 같은 permit을 두 번 release하므로 복사 생성자와 복사 대입을 명시적으로 삭제한다.
        Lease(const Lease&) = delete;
        Lease& operator=(const Lease&) = delete;

        // 이동 생성자도 반환형이 없다. other.gate_를 nullptr로 교체하면서 이전 포인터를 새 멤버에 저장한다.
        Lease(Lease&& other) noexcept
            // exchange(T&, U&&)의 첫 인자는 수정할 포인터 lvalue, 둘째는 nullptr prvalue다. 옛 포인터를 값으로 반환해 gate_에 저장한다.
            // 호출 뒤 other.gate_는 null이고 동적 할당·예외가 없으며 자원 자체의 소유권이 아니라 permit 반환 책임만 이동한다.
            : gate_{std::exchange(other.gate_, nullptr)} {}

        // 소멸자는 반환형과 매개변수가 없다. 아직 permit을 맡고 있을 때만 counter를 되돌린다.
        ~Lease() {
            if (gate_ != nullptr) {  // != 비교와 조건 분기로 이동된 빈 Lease의 중복 release를 막는다.
                // 수신 객체는 이 Lease보다 오래 사는 Semaphore다. release(1)는 void이고 counter를 1 늘려 대기자 하나를 깨울 수 있다.
                // acquire 뒤 정확히 한 번만 호출하므로 최대값 2를 넘지 않는다. 다른 메모리 게시가 필요하면 release/acquire 동기화도 제공한다.
                gate_->release();
            }
        }

        [[nodiscard]] bool owns_permit() const noexcept {
            return gate_ != nullptr;  // const 관찰은 포인터만 비교하며 객체 상태를 바꾸지 않는다.
        }

    private:
        // 외부가 acquire 없이 Lease를 만들 수 없게 하고 바깥 ConnectionGate만 이 private 생성자를 호출하게 한다.
        friend class ConnectionGate;

        // explicit은 Semaphore& 하나가 Lease로 암시 변환되는 것을 막는다. 올바른 사용은 Lease{slots_} 직접 초기화다.
        explicit Lease(Semaphore& gate) noexcept : gate_{&gate} {}

        // 원시 포인터는 Semaphore를 소유하지 않는다. ConnectionGate가 모든 Lease보다 오래 살아야 한다.
        Semaphore* gate_{};
    };

    // 생성자는 반환형이 없고 counter를 2로 직접 초기화해 동시에 두 permit까지 허용한다.
    ConnectionGate() : slots_{2} {}

    [[nodiscard]] Lease acquire() {
        // 수신 객체는 counter 0..2를 가진 Semaphore다. acquire()는 인자·반환값 없이 양수가 될 때까지 기다렸다 counter를 1 줄인다.
        // 호출 뒤 현재 스레드가 permit 하나를 맡는다. 대기 시간·공정성·lock-free 구현은 보장하지 않는다.
        slots_.acquire();
        // Lease{slots_}는 explicit 생성자를 쓰는 prvalue이고 반환 객체를 직접 초기화할 수 있다.
        return Lease{slots_};
    }

private:
    // counting_semaphore는 복사·이동할 수 없으므로 gate 자체도 값 이동 대상으로 설계하지 않는다.
    Semaphore slots_;
};

int main() {
    ConnectionGate gate{};
    {
        // acquire 반환 prvalue가 first를 직접 초기화하고 first가 permit 반환 책임을 맡는다.
        ConnectionGate::Lease first{gate.acquire()};
        // std::move(first)는 first lvalue를 xvalue로 바꾸고 Lease 이동 생성자가 책임을 moved로 옮긴다.
        ConnectionGate::Lease moved{std::move(first)};
        assert(!first.owns_permit());
        assert(moved.owns_permit());
    }  // moved 소멸자가 정확히 한 번 release하고 first 소멸자는 아무것도 하지 않는다.

    // 앞 scope의 RAII release가 없었다면 permit 누수가 누적된다. 새 Lease가 다시 안전하게 하나를 빌린다.
    ConnectionGate::Lease reused{gate.acquire()};
    assert(reused.owns_permit());

    // ostream 연산은 bool을 기본적으로 1/0으로 출력하고 같은 ostream&를 반환한다. reused의 상태는 바뀌지 않는다.
    std::cout << reused.owns_permit() << '\n';
    return 0;
}
