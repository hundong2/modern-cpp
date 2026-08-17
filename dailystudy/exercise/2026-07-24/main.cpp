// <coroutine>은 코루틴 핸들, suspend_always처럼 코루틴을 제어하는 표준 타입을 제공한다.
#include <coroutine>
// <iostream>은 표준 출력 객체 std::cout과 스트림 삽입 연산자 <<를 제공한다.
#include <iostream>
// <utility>는 식을 xvalue로 바꾸는 std::move를 제공한다.
#include <utility>

// T는 호출자가 정하는 템플릿 타입 인자이며 Sequence<int>라면 T는 int가 된다.
template <typename T>
class Sequence {
public:
    struct promise_type;
    // using은 긴 템플릿 타입에 짧은 별칭을 붙인다. 템플릿 인자는 promise_type이다.
    using Handle = std::coroutine_handle<promise_type>;

    // 코루틴 반환 타입 안의 promise_type은 컴파일러가 찾는 약속 타입이다.
    struct promise_type {
        // T{}는 T의 값 초기화이며 int라면 0으로 초기화된다.
        T current_value{};

        // 반환형 Sequence와 빈 매개변수 목록 ()을 가진 멤버 함수다.
        [[nodiscard]] Sequence get_return_object() {
            // from_promise는 promise 객체의 주소와 연결된 코루틴 핸들을 만든다.
            return Sequence{Handle::from_promise(*this)};
        }

        // suspend_always를 반환하면 호출 직후 본문에 들어가지 않고 처음부터 일시 중단한다.
        // suspend_always{}는 생성자 인자 없이 만드는 값이며 await_ready()가 false라 항상 중단시키는 awaiter다.
        [[nodiscard]] std::suspend_always initial_suspend() const noexcept { return {}; }

        // 마지막에도 멈춰야 소유자인 Sequence가 안전하게 프레임을 destroy할 수 있다.
        [[nodiscard]] std::suspend_always final_suspend() const noexcept { return {}; }

        // terminate()는 입력 인자와 정상 반환값이 없고([[noreturn]]) 등록된 종료 처리기를 거쳐 프로세스를 끝낸다.
        // 예외가 코루틴 경계를 넘으면 복구하지 않는다는 교육용 정책이며 자동 지역 소멸을 기대하면 안 된다.
        [[noreturn]] void unhandled_exception() const { std::terminate(); }

        // co_yield value는 이 함수를 호출하고, 매개변수 value를 멤버에 복사한 뒤 중단한다.
        [[nodiscard]] std::suspend_always yield_value(T value) noexcept {
            current_value = value; // 대입 연산자 =가 오른쪽 값을 왼쪽 멤버에 저장한다.
            return {};
        }

        // 값 없는 코루틴 종료를 나타내며 생성자처럼 보이지만 반환형 void인 일반 멤버 함수다.
        void return_void() const noexcept {}
    };

    // 생성자는 반환형이 없고, explicit은 Handle이 Sequence로 암시적 변환되는 것을 막는다.
    explicit Sequence(Handle handle) noexcept
        : handle_{handle} { // 멤버 초기화 목록이 private 멤버 handle_을 직접 초기화한다.
    }

    // 복사 생성과 복사 대입을 막아 하나의 프레임을 두 객체가 파괴하는 이중 소유를 방지한다.
    Sequence(const Sequence&) = delete;
    Sequence& operator=(const Sequence&) = delete;

    // 이동 생성자는 반환형이 없고, 매개변수 other는 이동 가능한 객체에 바인딩한 rvalue 참조다.
    Sequence(Sequence&& other) noexcept
        // std::move는 handle_ 식을 xvalue로 바꿀 뿐 coroutine_handle 자체는 포인터처럼 복사된다. 그래서 아래에서 원본을 명시적으로 비운다.
        : handle_{std::move(other.handle_)} {
        other.handle_ = {}; // 이동 후 원본을 빈 핸들로 저장해 소유권이 하나뿐이게 한다.
    }

    // 이동 대입 연산자는 현재 자원을 정리한 뒤 소유권을 넘기고 자기 자신 lvalue 참조를 반환한다.
    Sequence& operator=(Sequence&& other) noexcept {
        if (this != &other) { // 포인터 비교 !=로 자기 대입이 아닌지 검사해 조건 분기한다.
            if (handle_) { // 핸들을 bool처럼 검사하여 유효한 프레임만 파괴한다.
                // destroy()는 인자·반환값이 없고 가리키는 중단 코루틴 프레임과 그 안 객체를 파괴한다.
                // 호출 뒤 이 핸들을 다시 사용하면 안 되며 완료 전 실행 중 프레임에 호출하는 것은 전제조건 위반이다.
                handle_.destroy();
            }
            handle_ = std::move(other.handle_); // std::move 자체는 이동이 아니라 xvalue 변환이며 대입이 값을 넘긴다.
            other.handle_ = {};
        }
        return *this; // this는 현재 객체를 가리키는 포인터이고 *this는 현재 객체 lvalue다.
    }

    // 소멸자는 반환형이 없으며 RAII로 소유한 코루틴 프레임의 수명을 끝낸다.
    ~Sequence() {
        if (handle_) {
            handle_.destroy(); // 표준 라이브러리 함수 호출로 프레임과 그 안의 객체를 파괴한다.
        }
    }

    // bool 반환형 함수 next는 다음 값이 생기면 true, 끝났으면 false를 반환한다.
    [[nodiscard]] bool next() {
        // done()은 인자 없이 마지막 중단점 도달 여부 bool을 반환하며 프레임 상태를 바꾸지 않는다. 유효한 중단 핸들이어야 한다.
        if (!handle_ || handle_.done()) {
            return false;
        }
        // resume()은 인자·반환값 없이 다음 중단점까지 프레임 실행을 재개한다. 완료되었거나 실행 중인 핸들에는 호출하지 않는다.
        handle_.resume();
        return !handle_.done();
    }

    // const 멤버 함수는 Sequence의 논리 상태를 바꾸지 않고 값에 대한 const 참조를 반환한다.
    [[nodiscard]] const T& value() const {
        return handle_.promise().current_value;
    }

private:
    // class의 기본 접근은 private지만, 접근 경계를 학습하도록 private을 명시했다.
    Handle handle_{}; // 멤버 변수는 코루틴 프레임을 가리키는 핸들을 소유한다.
};

// 반환형 Sequence<int>, 매개변수 int start인 코루틴 함수다.
Sequence<int> make_countdown(int start) {
    // int는 기본 정수 타입이고, current{start}는 축소 변환을 막는 중괄호 초기화다.
    for (int current{start}; current > 0; --current) {
        // > 비교 결과로 반복 여부를 분기하고, 전위 -- 연산자가 current에서 1을 뺀다.
        co_yield current;
    }
}

// struct의 기본 접근은 public이다. 여기서는 가상 호출 아키텍처 경계를 명확히 보이게 한다.
struct ISequencePrinter {
    virtual ~ISequencePrinter() = default; // 기반 포인터로 파괴할 때를 위한 가상 소멸자다.
    virtual void print(int value) const = 0; // 순수 가상 함수이며 구현 클래스가 동작을 제공한다.
};

class ConsoleSequencePrinter final : public ISequencePrinter {
public:
    // override는 기반 클래스 가상 함수와 정확히 일치하는지 컴파일러가 검사한다.
    void print(int value) const override {
        std::cout << value << '\n'; // << 연산자가 int와 문자 값을 출력 스트림에 차례로 보낸다.
    }
};

// int 반환형 main은 운영체제에 종료 상태를 돌려주는 프로그램 진입 함수다.
int main() {
    // make_countdown(3)은 prvalue를 만들고 sequence에 직접 구성되어 보장된 복사 생략이 적용된다.
    Sequence<int> sequence{make_countdown(3)};
    const ConsoleSequencePrinter printer{}; // const 객체는 생성 뒤 상태를 변경할 수 없다.
    const ISequencePrinter* printer_ptr{&printer}; // 포인터는 printer의 주소를 저장하며 nullptr일 수도 있는 표현이다.

    // next 함수 호출 결과 bool을 조건으로 사용해 false가 될 때까지 반복한다.
    while (sequence.next()) {
        const int& value_ref{sequence.value()}; // 기존 promise 멤버 lvalue에 const lvalue 참조를 바인딩한다.
        printer_ptr->print(value_ref); // ->로 포인터 대상의 가상 함수를 호출하며 간접 호출이 될 수 있다.
    }

    return 0; // 정수 리터럴 0은 성공 종료 상태를 뜻한다.
}
