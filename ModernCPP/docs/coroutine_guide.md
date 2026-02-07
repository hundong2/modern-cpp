# C++20 Coroutines 상세 학습 가이드

## Coroutines란?

코루틴은 실행을 일시 중단하고 나중에 재개할 수 있는 함수입니다.

## 기본 개념

### 일반 함수 vs 코루틴

```cpp
// 일반 함수
int normal_function() {
    return 42;  // 한 번 실행하고 종료
}

// 코루틴
Generator<int> coroutine_function() {
    co_yield 1;  // 값을 반환하지만 실행은 중단
    co_yield 2;  // 다시 호출하면 여기서부터 재개
    co_yield 3;
}
```

## 세 가지 키워드

### 1. co_yield

값을 반환하고 실행을 일시 중단합니다.

```cpp
Generator<int> counter() {
    for (int i = 0; i < 5; ++i) {
        co_yield i;  // i를 반환하고 중단
    }
}
```

### 2. co_await

비동기 작업이 완료될 때까지 대기합니다.

```cpp
Task async_read() {
    auto data = co_await socket.async_read();  // 읽기 완료까지 대기
    process(data);
}
```

### 3. co_return

코루틴을 종료하고 값을 반환합니다.

```cpp
Task compute() {
    int result = 42;
    co_return result;  // 코루틴 종료
}
```

## Promise Type

코루틴의 동작을 제어하는 타입입니다.

```cpp
struct promise_type {
    // 코루틴 객체 생성
    Generator get_return_object();
    
    // 시작 시 동작
    std::suspend_always initial_suspend();
    
    // 종료 시 동작
    std::suspend_always final_suspend() noexcept;
    
    // yield 값 처리
    std::suspend_always yield_value(T value);
    
    // 예외 처리
    void unhandled_exception();
};
```

## 실전 활용

### 예제 1: 무한 시퀀스

```cpp
Generator<int> infinite_numbers() {
    int n = 0;
    while (true) {
        co_yield n++;
    }
}

// 사용
auto gen = infinite_numbers();
for (int i = 0; i < 10 && gen.next(); ++i) {
    std::cout << gen.value() << " ";
}
```

### 예제 2: 파일 읽기 (지연 평가)

```cpp
Generator<std::string> read_lines(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    
    while (std::getline(file, line)) {
        co_yield line;  // 한 줄씩 반환
    }
}

// 메모리 효율적: 전체 파일을 메모리에 올리지 않음
```

## Boost.Asio와 코루틴

### 비동기 서버 예제

```cpp
awaitable<void> handle_client(tcp::socket socket) {
    try {
        std::array<char, 1024> buffer;
        
        while (true) {
            // 비동기 읽기 (블로킹처럼 보이지만 논블로킹)
            size_t n = co_await socket.async_read_some(
                boost::asio::buffer(buffer),
                use_awaitable
            );
            
            // 비동기 쓰기
            co_await async_write(
                socket,
                boost::asio::buffer(buffer, n),
                use_awaitable
            );
        }
    }
    catch (std::exception& e) {
        std::cerr << "에러: " << e.what() << std::endl;
    }
}
```

## 장점

1. **가독성**: 비동기 코드를 동기 코드처럼 작성
2. **효율성**: 메모리 효율적인 지연 평가
3. **제어**: 실행 흐름을 세밀하게 제어

## 주의사항

1. Promise type 구현이 복잡함
2. 모든 컴파일러가 완벽히 지원하지 않음 (g++ 10 이상 권장)
3. 디버깅이 어려울 수 있음

## 학습 체크리스트

- [ ] co_yield 이해 및 사용
- [ ] Generator 패턴 구현
- [ ] Promise type의 역할 이해
- [ ] Boost.Asio와 co_await 사용
- [ ] 실제 프로젝트에 적용

## 연습 문제

1. 짝수만 생성하는 Generator 작성
2. 파일을 역순으로 읽는 Generator 작성
3. 비동기 타이머 코루틴 작성
