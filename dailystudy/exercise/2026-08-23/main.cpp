// <atomic>은 여러 스레드가 데이터 경쟁 없이 합계를 갱신하는 std::atomic을 선언한다.
#include <atomic>
// <cstddef>는 std::latch의 참가자 수 타입으로 쓸 std::ptrdiff_t를 선언한다.
#include <cstddef>
// <iostream>은 검증 결과를 쓰는 std::cout과 스트림 삽입 연산자를 선언한다.
#include <iostream>
// <latch>는 한 번만 열리는 C++20 동기화 장벽 std::latch를 선언한다.
#include <latch>
// <string>은 구성 요소 이름의 문자 버퍼를 소유하는 std::string을 선언한다.
#include <string>
// <thread>는 수명 종료 때 중지 요청과 합류를 수행하는 std::jthread를 선언한다.
#include <thread>
// <utility>는 lvalue를 xvalue로 바꾸는 std::move 함수 템플릿을 선언한다.
#include <utility>

// struct는 기본 접근이 public이다. 이름과 작업량만 묶는 값 레코드는 공개 집합체가 알맞다.
struct Component {
    std::string name{}; // 문자열 버퍼를 Component가 소유하며 객체와 함께 수명이 끝난다.
    int work_units{};   // int{}는 0으로 값 초기화되고 여기서는 완료 작업량을 나타낸다.
};

// class는 기본 접근이 private이다. 카운터를 정확히 한 번씩 줄이는 규칙을 public API 뒤에 숨긴다.
class StartupCoordinator {
public:
    // 생성자는 반환형이 없다. explicit은 정수 하나가 Coordinator로 뜻밖에 암시 변환되는 일을 막는다.
    explicit StartupCoordinator(std::ptrdiff_t component_count)
        // latch(ptrdiff_t)는 component_count 값을 복사해 내부 카운터를 만든다. 0 이상 max() 이하여야 한다.
        // atomic<int>(0)는 별도 int 저장소를 0으로 직접 초기화한다. 두 멤버는 본문보다 먼저 생성된다.
        : ready_{component_count}, completed_units_{0} {}

    // noexcept는 이 함수가 예외를 밖으로 보내지 않는다는 호출 계약을 타입에 표현한다.
    void complete(int units) noexcept {
        // atomic<int>::fetch_add(int, memory_order)는 수신 completed_units_의 현재 값에 units를 원자적으로 더한다.
        // units는 값 복사된 int lvalue이고 relaxed는 다른 메모리 순서를 만들지 않는 열거값 prvalue다.
        // 반환 int는 증가 전 값이지만 여기서는 버린다. 호출 뒤 원자 값만 바뀌며 정수 원자 연산 자체는 할당·예외가 없다.
        completed_units_.fetch_add(units, std::memory_order_relaxed);

        // latch::count_down(ptrdiff_t update = 1)는 인자를 생략해 기본값 1을 쓴다.
        // 반환형은 void라 저장값이 없고 호출 뒤 ready_의 카운터가 1 감소한다. 호출 전 카운터가 1 이상이어야 한다.
        // 0이 되는 감소는 wait 관찰자와 동기화한다. 이 latch는 재사용할 수 없고 호출 자체는 상수 시간·무할당이다.
        ready_.count_down();
    }

    [[nodiscard]] int wait_until_ready() const noexcept {
        // latch::wait() const는 데이터 인자 없이 ready_ 카운터가 0이 될 때까지 현재 스레드를 기다리게 한다.
        // 반환형은 void이며 latch 상태를 바꾸지 않는다. 0을 관찰한 반환은 마지막 count_down 이전 작업과 동기화한다.
        // 진행 순서나 공정성은 보장하지 않고, 이미 0이면 즉시 돌아오며 latch와 대기 스레드의 수명은 겹쳐야 한다.
        ready_.wait();

        // atomic<int>::load(memory_order) const는 relaxed를 입력으로 현재 int 스냅샷을 값으로 반환하고 원자 값을 바꾸지 않는다.
        // 반환값을 호출자에게 돌려준다. 모든 fetch_add가 latch 감소보다 앞서고 wait가 완료를 관찰했으므로 여기서는 최종 합계를 읽는다.
        return completed_units_.load(std::memory_order_relaxed);
    }

private:
    std::latch ready_;                // 구성 요소마다 한 번 도착하면 영구히 열리는 비복사 동기화 객체다.
    std::atomic<int> completed_units_; // 여러 작업 스레드가 공유하는 정수 저장소다.
};

// 반환형 std::jthread는 새 작업 스레드의 소유권을 호출자에게 값으로 넘긴다.
std::jthread launch(StartupCoordinator& coordinator, Component component) {
    // std::move<Component&>(component)는 이름 있는 매개변수 lvalue를 Component&& xvalue로 바꾼다.
    // 람다 init-capture가 Component 이동 생성자를 선택해 name 버퍼를 작업 객체 안으로 넘길 수 있다.
    // coordinator는 참조 캡처라 소유하지 않으므로 모든 jthread보다 오래 살아야 한다.
    auto task{[&coordinator, component = std::move(component)]() {
        // component는 람다 객체가 소유하고 이 본문 안에서는 const가 아닌 lvalue다.
        // 멤버 함수 호출은 work_units 값을 complete의 int 매개변수로 복사하고 coordinator 상태를 갱신한다.
        coordinator.complete(component.work_units);
    }};

    // jthread(F&&, Args&&...)는 task xvalue를 내부 호출 객체로 이동하고 즉시 새 실행 스레드를 시작한다.
    // 실제 오버로드의 F는 람다 타입, Args는 빈 팩이다. 반환 jthread prvalue가 호출자의 목적 객체를 직접 초기화할 수 있다.
    // 생성 실패 시 system_error가 가능하다. 성공하면 소멸자가 joinable 스레드에 stop 요청 후 join하며 실행 순서는 보장하지 않는다.
    return std::jthread{std::move(task)};
}

int main() {
    // 3은 준비를 보고할 구성 요소 수다. explicit 생성자이므로 중괄호 직접 초기화한다.
    StartupCoordinator coordinator{3};

    // basic_string(const char*) 오버로드는 각 널 종료 리터럴 포인터를 읽어 문자를 새 버퍼에 복사하고 string prvalue를 만든다.
    // 리터럴은 바뀌지 않고 string이 문자를 소유한다. 길이만큼 시간·공간이 들며 할당 실패 시 bad_alloc이 가능하다.
    // Component{string,int}는 prvalue 집합체다. launch 값 매개변수와 람다 캡처를 거쳐 문자열 소유권이 작업으로 이동한다.
    // launch 반환 prvalue는 database를 직접 초기화하며 jthread는 복사 불가·이동 가능 소유권 타입이다.
    std::jthread database{launch(coordinator, Component{std::string{"database"}, 7})};
    std::jthread cache{launch(coordinator, Component{std::string{"cache"}, 5})};
    std::jthread metrics{launch(coordinator, Component{std::string{"metrics"}, 6})};

    // wait_until_ready 반환 int prvalue로 total을 직접 초기화한다. 반환 시 세 작업의 보고는 모두 완료됐다.
    const int total{coordinator.wait_until_ready()};

    // operator<<는 cout에 int와 문자를 차례로 쓰고 같은 ostream&를 연쇄 반환한다.
    // 최종 참조는 버리고 cout의 소유권은 유지된다. I/O 실패는 기본적으로 예외 대신 스트림 상태 비트에 남는다.
    std::cout << total << '\n';

    // 비교 operator==는 bool prvalue를 만들고 조건 연산자는 0 또는 1을 반환한다.
    // 함수가 끝날 때 metrics, cache, database가 역순 소멸하며 아직 실행 중이면 각각 안전하게 합류한다.
    return total == 18 ? 0 : 1;
}
