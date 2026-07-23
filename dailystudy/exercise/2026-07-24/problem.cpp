// <coroutine>은 코루틴 핸들과 일시 중단 타입을 제공한다.
#include <coroutine>
// <iostream>은 결과를 화면에 출력하는 std::cout을 제공한다.
#include <iostream>
// <utility>는 값을 넘기고 원본을 바꾸는 std::exchange를 제공한다.
#include <utility>

// T는 수열 원소 타입을 받는 템플릿 인자다.
template <typename T>
class Generator {
public:
    struct promise_type;
    // using 타입 별칭이며 promise_type은 coroutine_handle 템플릿에 전달되는 타입 인자다.
    using Handle = std::coroutine_handle<promise_type>;

    // struct의 기본 접근은 public이므로 컴파일러가 아래 멤버를 찾을 수 있다.
    struct promise_type {
        T current{}; // 멤버 변수 current는 T{}로 값 초기화된다.

        [[nodiscard]] Generator get_return_object() {
            // 직접 초기화는 explicit 생성자를 올바르게 호출하며 암시적 변환은 허용하지 않는다.
            return Generator{Handle::from_promise(*this)};
        }
        [[nodiscard]] std::suspend_always initial_suspend() const noexcept { return {}; }
        [[nodiscard]] std::suspend_always final_suspend() const noexcept { return {}; }
        void return_void() const noexcept {}
        [[noreturn]] void unhandled_exception() const { std::terminate(); }

        // T value는 호출할 때 값을 복사해 받는 매개변수다.
        [[nodiscard]] std::suspend_always yield_value(T value) noexcept {
            current = value; // 대입 연산자 =가 산출 값을 코루틴 프레임에 저장한다.
            return {};
        }
    };

    // 생성자는 반환형이 없고, explicit이 Handle 하나의 암시적 객체 변환을 막는다.
    explicit Generator(Handle handle) noexcept : handle_{handle} {}
    Generator(const Generator&) = delete; // 복사는 같은 프레임의 중복 소유를 만들므로 금지한다.
    Generator& operator=(const Generator&) = delete;

    // 이동 생성자의 Generator&&는 이름 붙이기 전 이동 가능한 객체에 바인딩한다.
    Generator(Generator&& other) noexcept
        : handle_{std::exchange(other.handle_, {})} {}

    // 소멸자는 반환형이 없고 객체 수명 끝에 소유한 코루틴 프레임을 해제한다.
    ~Generator() {
        if (handle_) { // 유효성 검사 결과에 따라 destroy 호출 여부를 조건 분기한다.
            handle_.destroy();
        }
    }

    // 반환형 bool, 매개변수 없음인 next를 호출할 때 코루틴이 한 단계 실행된다.
    [[nodiscard]] bool next() {
        if (!handle_ || handle_.done()) {
            return false;
        }
        handle_.resume();
        return !handle_.done();
    }

    // const 참조 반환은 값을 복사하지 않으며 Generator보다 오래 보관하면 안 된다.
    [[nodiscard]] const T& value() const { return handle_.promise().current; }

private:
    // class의 기본 접근은 private이며 public/private 경계를 명시해 소유권을 외부에서 못 바꾸게 한다.
    Handle handle_{};
};

// 연습: 1부터 limit까지 제곱을 co_yield하는 코루틴이다.
Generator<int> squares(int limit) {
    // int i{1}은 기본 정수 타입 변수를 중괄호로 초기화한다.
    for (int i{1}; i <= limit; ++i) {
        // TODO: 아래 식을 먼저 직접 작성해 보자. *는 두 정수 피연산자를 곱한다.
        co_yield i * i;
    }
}

int main() {
    // squares(5)는 Generator<int> prvalue이며 result에 직접 만들어져 복사 생략된다.
    Generator<int> result{squares(5)};
    int sum{0}; // 합계를 저장하는 lvalue이며 중괄호로 0을 초기화한다.

    while (result.next()) { // 함수 호출의 bool 결과를 비교해 반복 여부를 결정한다.
        const int& value{result.value()}; // 코루틴 프레임 속 기존 값 lvalue에 const 참조를 바인딩한다.
        sum += value; // 복합 대입 연산자 +=는 기존 sum을 읽고 더한 값을 다시 저장한다.
        std::cout << value << ' '; // 표준 라이브러리 스트림과 << 연산자로 값을 출력한다.
    }
    std::cout << "\nsum=" << sum << '\n';

    // 포인터는 주소 또는 nullptr을 저장한다. 여기서는 sum의 주소를 관찰하되 소유하지 않는다.
    const int* sum_ptr{&sum};
    if (sum_ptr != nullptr && *sum_ptr == 55) { // 비교, 논리 AND, 역참조 뒤 성공 여부를 분기한다.
        return 0;
    }
    return 1;
}
