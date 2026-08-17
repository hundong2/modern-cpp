// <atomic>은 잠금 없이 독립 통계 값을 원자적으로 갱신하는 std::atomic을 제공한다.
#include <atomic>
// <cstdint>은 크기가 명확한 부호 없는 정수 std::uint64_t를 제공한다.
#include <cstdint>
// <iostream>은 직접 검증 결과를 출력하는 std::cout을 제공한다.
#include <iostream>

// struct는 기본 public이라 여러 통계 값을 반환하는 단순 값 객체에 알맞다.
struct RequestSnapshot {
    // uint64_t는 매우 많은 요청 수도 담을 수 있고 {}는 0으로 값 초기화한다.
    std::uint64_t accepted{};
    std::uint64_t rejected{}; // 같은 기본 타입으로 거절 건수를 독립 저장한다.
};

// class는 기본 private이므로 원자 변수의 비원자 접근을 막고 멤버 함수만 공개한다.
class RequestMetrics {
// public 접근 지정자 아래 함수만 호출자가 사용할 수 있어 atomic 상태의 캡슐화가 유지된다.
public:
    // 반환형 없는 생성자는 두 카운터의 시작값을 받고, explicit은 정수 하나의 암시 변환을 막는다.
    explicit RequestMetrics(std::uint64_t initial = 0U)
        // 각 atomic<uint64_t> 생성자는 initial 값을 입력받아 저장값을 만들고, 생성자이므로 별도 반환값은 없다.
        // initial은 값 매개변수라 두 번 읽어도 호출자의 원본은 바뀌지 않으며 두 원자 객체는 서로 독립적이다.
        : accepted_{initial}, rejected_{initial} {}

    // noexcept는 이 단순 원자 갱신 함수가 예외를 던지지 않는다는 계약이다.
    void record_accepted() noexcept {
        // 호출 계약: accepted_가 수신 atomic<uint64_t>이고 fetch_add(difference, order)를 원자적으로 수행한다.
        // 첫 인자 1U는 difference이며 uint64_t로 변환되어 현재 값에 더해진다. 최대값 다음에는 0으로 순환한다.
        // 둘째 인자 memory_order_relaxed는 데이터가 아니라 순서 규칙이며 주변 비원자 데이터를 게시하지 않는다.
        // 반환형은 uint64_t이고 증가 전 값을 돌려주지만 여기서는 사용하지 않는다. 저장값은 정확히 한 번 +1 된다.
        accepted_.fetch_add(1U, std::memory_order_relaxed);
    }

    // 거절 요청 한 건을 독립 원자 카운터에 기록하고 반환값은 없다.
    void record_rejected() noexcept {
        // 같은 fetch_add 계약에서 첫 인자 1U는 더할 값, 둘째 relaxed는 메모리 순서이며 둘은 역할이 다르다.
        // 반환되는 증가 전 uint64_t는 버리고, rejected_의 저장값만 원자적으로 1 증가시킨다.
        rejected_.fetch_add(1U, std::memory_order_relaxed);
    }

    // const 함수는 카운터를 바꾸지 않고 현재 두 값을 새 값 객체로 복사해 반환한다.
    [[nodiscard]] RequestSnapshot snapshot() const noexcept {
        // load(order)의 유일한 인자 relaxed는 읽을 숫자가 아니라 메모리 순서이고, load는 수신 atomic 값을 바꾸지 않는다.
        // 각 반환형은 uint64_t이며 호출 순간 관찰한 값을 복사한다. 두 독립 load는 하나의 트랜잭션 스냅샷이 아니다.
        const std::uint64_t accepted{accepted_.load(std::memory_order_relaxed)};
        // 두 번째 load도 입력은 relaxed 하나, 출력은 rejected 값 하나이며 accepted와의 동시 시점 일치는 보장하지 않는다.
        const std::uint64_t rejected{rejected_.load(std::memory_order_relaxed)};
        // RequestSnapshot{...} prvalue가 반환 객체를 직접 초기화해 C++17 이후 복사 생략 대상이 된다.
        return RequestSnapshot{accepted, rejected};
    }

// private 접근 지정자는 외부의 비원자적 직접 접근을 금지한다.
private:
    // atomic<uint64_t>의 템플릿 인자는 원자적으로 읽고 쓸 값 타입이며 {}는 정수 저장값을 0으로 초기화한다.
    // atomic 객체 자체는 복사할 수 없고 값을 꺼내려면 load 같은 원자 멤버 함수를 사용한다.
    std::atomic<std::uint64_t> accepted_{};
    std::atomic<std::uint64_t> rejected_{}; // 별도 cache/순서 의미를 가진 독립 원자 객체다.
};

// main은 연습 코드를 실행해 기대 상태를 초보자가 눈으로 검증하게 한다.
int main() {
    // 직접 초기화는 explicit 생성자를 올바르게 호출하고 0U를 생성자 매개변수로 전달한다.
    RequestMetrics metrics{0U};

    // 점 연산자는 lvalue 객체 metrics의 public 멤버 함수를 호출한다.
    metrics.record_accepted(); // 첫 호출은 accepted를 0에서 1로 원자 증가시킨다.
    metrics.record_accepted(); // 두 번째 호출은 accepted를 1에서 2로 원자 증가시킨다.
    metrics.record_rejected(); // rejected는 0에서 1로 원자 증가한다.

    // snapshot() 반환 prvalue가 const 지역 값 객체를 직접 초기화한다.
    const RequestSnapshot current{metrics.snapshot()};
    // ==와 &&는 두 기댓값을 비교하고, 조건 분기는 검증 성공과 실패 경로를 나눈다.
    if (current.accepted == 2U && current.rejected == 1U) {
        // << 연산자는 각 값을 출력 스트림에 차례로 삽입한다.
        std::cout << "accepted=" << current.accepted
                  << ", rejected=" << current.rejected << '\n'; // 마지막에 개행 문자를 출력한다.
        return 0; // 검증 성공 경로의 정상 종료 코드다.
    }

    // 실제 원자 로드·저장·조건 분기의 기계 명령은 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.
    // relaxed는 원자성은 보장하지만 다른 메모리의 공개 순서를 만들지 않는다는 점이 핵심이다.
    return 1;
}
