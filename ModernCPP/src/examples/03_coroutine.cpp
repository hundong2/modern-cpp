// C++20 Coroutine 기본 예제
#include <iostream>
#include <coroutine>
#include <memory>
#include <exception>

// 간단한 제너레이터 구현
template<typename T>
struct Generator {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    
    struct promise_type {
        T current_value;
        std::exception_ptr exception;
        
        Generator get_return_object() {
            return Generator{handle_type::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        void unhandled_exception() {
            exception = std::current_exception();
        }
        
        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }
        
        void return_void() {}
    };
    
    handle_type h;
    
    Generator(handle_type h) : h(h) {}
    ~Generator() { if (h) h.destroy(); }
    
    // 복사 금지
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    
    // 이동 가능
    Generator(Generator&& other) noexcept : h(other.h) {
        other.h = {};
    }
    
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (h) h.destroy();
            h = other.h;
            other.h = {};
        }
        return *this;
    }
    
    // 다음 값 가져오기
    bool next() {
        h.resume();
        return !h.done();
    }
    
    // 현재 값
    T value() const {
        return h.promise().current_value;
    }
};

// 코루틴 예제 1: 범위 생성기
Generator<int> range(int start, int end) {
    for (int i = start; i < end; ++i) {
        co_yield i;
    }
}

// 코루틴 예제 2: 피보나치 수열 생성기
Generator<long long> fibonacci(int count) {
    long long a = 0, b = 1;
    
    for (int i = 0; i < count; ++i) {
        co_yield a;
        long long next = a + b;
        a = b;
        b = next;
    }
}

// 코루틴 예제 3: 무한 수열 (홀수)
Generator<int> odd_numbers() {
    int n = 1;
    while (true) {
        co_yield n;
        n += 2;
    }
}

int main() {
    std::cout << "=== C++20 Coroutine 예제 ===" << std::endl << std::endl;
    
    // 1. 범위 생성기
    std::cout << "1. 범위 생성기 (1부터 10까지):" << std::endl;
    auto r = range(1, 11);
    while (r.next()) {
        std::cout << r.value() << " ";
    }
    std::cout << std::endl << std::endl;
    
    // 2. 피보나치 수열
    std::cout << "2. 피보나치 수열 (처음 15개):" << std::endl;
    auto fib = fibonacci(15);
    while (fib.next()) {
        std::cout << fib.value() << " ";
    }
    std::cout << std::endl << std::endl;
    
    // 3. 무한 수열 (홀수) - 처음 10개만
    std::cout << "3. 홀수 무한 수열 (처음 10개):" << std::endl;
    auto odds = odd_numbers();
    for (int i = 0; i < 10 && odds.next(); ++i) {
        std::cout << odds.value() << " ";
    }
    std::cout << std::endl << std::endl;
    
    // 4. 범위 기반 for와 유사한 사용
    std::cout << "4. 제곱 수열 (1²부터 10²까지):" << std::endl;
    auto squares = range(1, 11);
    while (squares.next()) {
        int val = squares.value();
        std::cout << val * val << " ";
    }
    std::cout << std::endl;
    
    std::cout << "\n코루틴은 지연 평가(lazy evaluation)를 가능하게 합니다!" << std::endl;
    std::cout << "필요할 때만 값을 생성하므로 메모리 효율적입니다." << std::endl;
    
    return 0;
}
