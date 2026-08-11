// <iostream>은 실행 결과를 출력하는 std::cout을 제공한다.
#include <iostream>
// <memory>는 단독 소유 스마트 포인터 std::unique_ptr와 std::make_unique를 제공한다.
#include <memory>
// <utility>는 소유권 이동에 쓰는 std::move를 제공한다.
#include <utility>

// struct는 기본 접근이 public이라 단순 결과 데이터를 표현하기 알맞다.
struct Measurement {
    long long nanoseconds{}; // long long 기본 타입을 중괄호로 0 초기화한다.
};

// class는 기본 접근이 private이며 public 인터페이스를 명시한다.
class ClockPort {
public:
    virtual ~ClockPort() = default; // 기반 포인터로 파생 객체를 지울 때 가상 소멸이 안전하게 동작한다.
    [[nodiscard]] virtual long long now() const = 0; // const 함수는 관찰 중 객체 상태를 바꾸지 않는다.
};

class FakeClock final : public ClockPort {
public:
    // 생성자에는 반환형이 없고, explicit은 정수가 시계 객체로 암시 변환되는 것을 막는다.
    explicit FakeClock(long long value) : value_{value} {} // 생성자 매개변수를 private 멤버에 직접 저장한다.
    [[nodiscard]] long long now() const override { return value_; } // override가 가상 함수 시그니처를 검증한다.

private:
    long long value_{}; // 값 멤버의 수명은 FakeClock 객체 수명과 같다.
};

class TimerService final {
public:
    // unique_ptr 값 매개변수는 소유권 이전을 API에 드러내고, explicit은 암시 변환을 막는다.
    explicit TimerService(std::unique_ptr<ClockPort> clock) : clock_{std::move(clock)} {}

    [[nodiscard]] Measurement sample() const {
        // ->는 포인터가 소유한 객체의 가상 함수를 간접 호출한다.
        // 실제 가상 간접 호출·로드·분기는 CPU·ABI·컴파일러·최적화 옵션에 따라 달라지며 특정 명령으로 단정할 수 없다.
        return Measurement{clock_->now()}; // 반환 prvalue는 호출자의 결과 객체에 복사 생략으로 직접 만들어진다.
    }

private:
    std::unique_ptr<ClockPort> clock_{}; // 단독 소유 포인터가 객체 수명과 자동 해제를 책임진다.
};

int main() {
    // make_unique가 FakeClock을 동적 생성하며 반환 prvalue unique_ptr이 직접 초기화된다.
    auto clock{std::make_unique<FakeClock>(42)};
    // clock은 lvalue이고 std::move(clock)은 xvalue다. 이동 뒤 clock은 비소유 상태가 된다.
    const TimerService service{std::move(clock)};
    const Measurement result{service.sample()}; // 함수 반환 prvalue로 결과를 직접 초기화한다.
    std::cout << result.nanoseconds << '\n'; // . 연산자로 public 멤버를 읽어 출력한다.
    return result.nanoseconds == 42 ? 0 : 1; // 비교와 조건 연산자로 검증 종료 코드를 고른다.
}
