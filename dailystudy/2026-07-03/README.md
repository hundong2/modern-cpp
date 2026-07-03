# 2026-07-03 Daily Study: Compile-Time State Machine, Dispatch, Two Pointers

## 오늘의 목표

2026년 7월 3일 금요일 Modern C++ 데일리 브리핑 주제는 정적 리플렉션을 응용한 컴파일 타임 상태 머신과 네트워크 메시지 분기 가속이다.

원문 브리핑에는 C++26 정적 리플렉션 `^^T`, splicing `[:M:]`, expansion `[:expand:]`, pack indexing `args...[n]`, `std::execution::just/then` 같은 최신 문법이 등장한다. 다만 현재 로컬 컴파일러에서 이 문법들을 그대로 빌드하기는 어렵다.

그래서 이 폴더의 코드는 **실제로 빌드 가능한 C++23**으로 작성했다. 핵심 학습 목표는 다음과 같다.

- `enum class`로 네트워크 연결 상태를 안전하게 표현
- `constexpr` 상태 전이 테이블로 런타임 상태 판단 비용 줄이기
- `std::string_view`로 문자열 복사 없이 메트릭 전달
- `std::expected`로 게이트웨이 성공/실패를 값으로 표현
- `std::tuple`과 `std::get<N>`으로 C++26 pack indexing의 의도 체험
- 코딩 테스트 문제: 컨테이너에 담을 수 있는 가장 많은 물의 양

## 폴더 구성

```text
dailystudy/2026-07-03/
  CMakeLists.txt
  main.cpp
  README.md
```

## 빌드 및 실행

저장소 루트 기준으로 portable GCC가 이미 설치되어 있다.

```text
D:\workspace\modern-cpp\tools\w64devkit
```

PowerShell에서 다음처럼 빌드한다.

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-03
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_03.exe
```

## 코드가 다루는 실무 시나리오

예제는 "런타임 분기문을 계속 늘리는 대신, 허용 가능한 상태 전이를 컴파일 타임 데이터로 고정한다"는 생각을 작은 네트워크 게이트웨이 코드로 표현한다.

```cpp
constexpr std::array<StateTransition, 2> allowed_transitions{{
    {ConnectionState::Disconnected, ConnectionState::Connecting},
    {ConnectionState::Connecting, ConnectionState::Connected},
}};
```

왜 필요한가:

- 상태 전이 규칙이 코드 한가운데 흩어지지 않는다.
- `constexpr` 함수로 검증하면 상수 입력에 대해 컴파일 타임 평가가 가능하다.
- 나중에 정적 리플렉션이 안정화되면 구조체 메타데이터 기반 자동 직렬화, 검증, 로깅으로 확장하기 쉽다.

## 기본 문법 복습

### `enum class`

```cpp
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected
};
```

`enum class`는 정수와 암묵적으로 섞이지 않는 안전한 열거형이다.

왜 필요한가:

- `Disconnected`, `Connecting`, `Connected` 같은 상태 값을 타입으로 묶는다.
- 일반 `int`와 섞이는 실수를 줄인다.
- 상태 머신의 입력과 출력을 읽기 쉽게 만든다.

### `std::string_view`

```cpp
struct StateMetric {
    ConnectionState current;
    std::string_view node_ip;
};
```

`std::string_view`는 문자열을 소유하지 않고 `[주소 + 길이]`만 바라본다.

왜 필요한가:

- 로그, 라우팅 키, IP 주소처럼 읽기만 하는 문자열을 복사하지 않는다.
- 네트워크 처리처럼 반복 호출이 많은 코드에서 불필요한 할당을 줄인다.

주의:

- 원본 문자열보다 오래 살아서는 안 된다.
- 저장용 필드에는 `std::string`이 더 안전한 경우가 많다.

### `constexpr`

```cpp
constexpr bool verify_state_transition(ConnectionState from, ConnectionState to);
```

`constexpr`는 가능한 경우 컴파일 중에 값을 계산할 수 있게 하는 함수 또는 객체 지정자다.

왜 필요한가:

- 정해진 상태 전이 검증은 실행 중 매번 계산할 필요가 없다.
- `static_assert`와 함께 쓰면 잘못된 상태 규칙을 빌드 단계에서 잡을 수 있다.

### `std::expected`

```cpp
std::expected<std::string_view, std::string_view> run_exchange_gateway();
```

`std::expected<T, E>`는 성공하면 `T`, 실패하면 `E`를 담는다.

왜 필요한가:

- 예외를 쓰지 않는 코드에서도 실패 이유를 명시적으로 전달한다.
- 호출자는 `if (!result)` 형태로 실패 처리를 확인하게 된다.

### C++26 pack indexing의 C++23 대체

원문은 다음과 같은 C++26 문법을 사용한다.

```cpp
auto& target_metric = args...[1];
```

현재 코드는 같은 의도를 다음처럼 표현한다.

```cpp
auto packet = std::forward_as_tuple(std::forward<Args>(args)...);
const auto& target_metric = std::get<1>(packet);
```

왜 필요한가:

- 가변 인자 묶음에서 특정 위치의 값을 꺼내는 감각을 익힐 수 있다.
- 지금은 `std::tuple` 방식이 안정적으로 빌드된다.
- C++26 pack indexing이 보편화되면 더 짧고 직접적인 문법으로 바꿀 수 있다.

## 컴파일러 내부 처리 감각

### 정적 상태 전이의 상수 축약

```cpp
constexpr bool is_valid = verify_state_transition(
    ConnectionState::Disconnected,
    ConnectionState::Connecting
);
static_assert(is_valid);
```

`from`과 `to`가 컴파일 타임에 정해져 있으므로 컴파일러는 `is_valid`를 상수 `true`로 평가할 수 있다. 즉 이 검증은 실행 중 상태 맵을 검색하는 코드가 아니라, 빌드 단계에서 확정된 사실로 취급된다.

### 리플렉션의 현재 대체 방식

원문의 정적 리플렉션 예시는 구조체 멤버를 컴파일러가 자동으로 순회하는 그림이다. 현재 코드는 안정적인 C++23 문법으로 직접 출력한다.

```cpp
void auto_state_inspector(const StateMetric& metric) {
    std::cout << "=> state field [current] : " << to_string(metric.current) << '\n';
    std::cout << "=> meta field [node_ip] : " << metric.node_ip << '\n';
}
```

핵심은 "상태 메트릭을 한곳에서 일관되게 검사하고 출력한다"는 구조다. 나중에 리플렉션이 실무 컴파일러에 안정적으로 들어오면 이 반복 코드를 자동화할 수 있다.

## 코딩 테스트 문제: 컨테이너에 담을 수 있는 가장 많은 물

문제:

길이가 `n`인 비음수 정수 배열 `height`가 주어진다. 두 개의 벽을 골라 그 사이에 물을 채울 때 담을 수 있는 물의 최대 양을 구한다. 컨테이너를 기울일 수는 없다.

예시:

```text
height = [1,8,6,2,5,4,8,3,7]
answer = 49
```

인덱스 `1`의 높이 `8`과 인덱스 `8`의 높이 `7`을 고르면 너비는 `7`, 유효 높이는 `7`이므로 `7 * 7 = 49`다.

제한 사항:

- `2 <= height.length <= 100,000`
- `0 <= height[i] <= 10,000`
- 시간 복잡도 `O(N)` 필요

## 알고리즘: 투 포인터

```cpp
int max_area(std::span<const int> height) {
    int left = 0;
    int right = static_cast<int>(height.size()) - 1;
    int max_water = 0;

    while (left < right) {
        const int width = right - left;
        const int current_height = std::min(height[left], height[right]);
        max_water = std::max(max_water, width * current_height);

        if (height[left] < height[right]) {
            ++left;
        } else {
            --right;
        }
    }

    return max_water;
}
```

핵심 탐색 전략:

- 양 끝에서 시작하면 너비는 가장 크다.
- 포인터를 안쪽으로 옮길 때마다 너비는 줄어든다.
- 더 큰 물의 양을 기대하려면 낮은 쪽 벽을 버리고 더 높은 벽을 찾아야 한다.
- 높은 쪽 벽을 움직여 봐야 현재 낮은 벽이 병목이므로 높이 개선을 기대하기 어렵다.

복잡도:

- 시간 복잡도: `O(N)`
- 공간 복잡도: `O(1)`

## 브리핑의 C++26 내용과 현재 코드의 관계

| 브리핑 키워드 | 현재 예제의 대체 구현 | 이유 |
| --- | --- | --- |
| 정적 리플렉션 `^^T` | `auto_state_inspector()` 수동 출력 | 현재 로컬 컴파일러에서 안정 빌드 필요 |
| splicing `[:M:]` | 명시적 멤버 접근 `metric.current` | 같은 관찰 목적을 현재 문법으로 표현 |
| pack indexing `args...[1]` | `std::forward_as_tuple()` + `std::get<1>()` | 가변 인자 위치 선택 의도 유지 |
| `std::execution::just/then` | `run_exchange_gateway()` 함수 파이프라인 | 비동기 프레임워크 없이 실행 흐름 학습 |
| erroneous value 자동 처리 | 명시적 값 초기화 `int status{}` | 현재 표준에서도 안전하고 명확함 |

## 오늘 기억할 것

- 상태 머신은 상태와 전이 규칙을 분리하면 읽기 쉬워진다.
- `constexpr` 전이 검증은 정해진 입력에 대해 런타임 비용을 줄일 수 있다.
- 최신 표준 문법이 아직 컴파일러에 없을 때는 같은 의도를 안정 문법으로 먼저 익히는 것이 실용적이다.
- 투 포인터는 양끝에서 좁히며 병목 조건을 제거하는 문제에 강하다.
- "컨테이너에 담을 수 있는 가장 많은 물"은 `O(N^2)` 브루트 포스가 아니라 `O(N)` 투 포인터로 풀어야 한다.

