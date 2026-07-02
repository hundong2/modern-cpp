# 2026-07-02 Daily Study: Legacy C API, RAII, Architecture, Two Pointers

## 오늘의 목표

첨부 브리핑은 C++26의 `std::out_ptr`, 정적 리플렉션, 가변 인자 직접 선택, 비동기 실행 모델을 주제로 다룬다. 다만 `^^T`, `[:expand:]`, `args...[n]` 같은 문법은 현재 로컬 GCC 16.1.0 환경에서 그대로 컴파일할 수 있는 안정 문법이 아니다.

그래서 이 폴더의 코드는 **실제로 빌드 가능한 C++23**으로 작성했다. 핵심 학습 목표는 다음과 같다.

- 레거시 C API가 `T**`로 메모리를 넘겨주는 구조 이해
- `std::unique_ptr`과 custom deleter로 메모리 누수 방지
- `std::expected`로 실패 이유를 명시적으로 전달
- `std::string_view`, `std::span`, variadic template, fold expression 복습
- 실무에서 자주 쓰는 계층형 아키텍처 감각 익히기
- 코딩 테스트 문제: 합이 `target` 이상인 가장 짧은 연속 부분 배열

## 폴더 구성

```text
dailystudy/2026-07-02/
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
cd D:\workspace\modern-cpp\dailystudy\2026-07-02
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_02.exe
```

직접 `target` 값을 넘길 수도 있다.

```powershell
.\build\daily_modern_cpp_2026_07_02.exe 11
```

## 코드가 다루는 실무 시나리오

현업에서는 순수 Modern C++ 코드만 작성하는 경우보다, 오래된 C 라이브러리나 OS API와 연결해야 하는 경우가 많다.

예를 들어 네트워크, 그래픽스, 데이터베이스 드라이버, 보안 라이브러리, 하드웨어 SDK는 다음과 같은 C 스타일 API를 제공할 수 있다.

```cpp
int legacy_network_recv(LegacyPacket** out_packet);
void legacy_network_free(LegacyPacket* packet);
```

이 구조의 의미:

- `LegacyPacket** out_packet`은 “함수 안에서 새 객체를 만들고 그 주소를 호출자에게 돌려주겠다”는 뜻이다.
- 호출자는 반드시 `legacy_network_free()`를 호출해야 한다.
- 해제를 잊으면 메모리 누수가 난다.
- 잘못된 함수로 해제하면 프로그램이 깨질 수 있다.

Modern C++에서는 이런 자원을 `std::unique_ptr`로 감싸서 **소유권과 해제 책임을 타입에 기록**한다.

```cpp
struct LegacyPacketDeleter {
    void operator()(LegacyPacket* packet) const noexcept {
        legacy_network_free(packet);
    }
};

using PacketOwner = std::unique_ptr<LegacyPacket, LegacyPacketDeleter>;
```

왜 필요한가:

- 사람이 `delete` 또는 `legacy_network_free()` 호출을 기억하지 않아도 된다.
- 함수가 중간에 실패하거나 예외가 발생해도 자동으로 정리된다.
- “이 포인터는 누가 해제해야 하는가?”라는 실무 디버깅 질문이 타입만 봐도 명확해진다.

## 기본 문법 복습

### `struct`

```cpp
struct LegacyPacket {
    int packet_id;
    const char* raw_payload;
};
```

`struct`는 여러 값을 하나의 묶음으로 표현한다. 여기서는 패킷 ID와 payload 포인터를 하나의 패킷으로 묶는다.

왜 필요한가:

- 관련 있는 데이터를 한 단위로 전달할 수 있다.
- 함수 인자가 많아지는 것을 줄인다.
- 네트워크 메시지, 설정값, 결과값 표현에 자주 쓰인다.

### 포인터와 포인터의 포인터

```cpp
LegacyPacket* raw_packet = nullptr;
legacy_network_recv(&raw_packet);
```

`LegacyPacket*`은 `LegacyPacket` 객체의 주소를 담는다. `&raw_packet`은 그 포인터 변수 자체의 주소이므로 타입은 `LegacyPacket**`가 된다.

왜 필요한가:

- C API는 반환값 하나만으로 성공/실패 코드를 돌려주고, 실제 데이터는 출력 인자로 돌려주는 패턴을 자주 쓴다.
- `T**`는 함수 내부에서 호출자의 포인터 값을 바꾸기 위한 전형적인 C 방식이다.

### `std::unique_ptr`

```cpp
PacketOwner packet(raw_packet);
```

`std::unique_ptr`은 하나의 객체를 오직 한 명만 소유하게 만든다. 복사는 금지되고 이동만 가능하다.

왜 필요한가:

- 같은 메모리를 두 번 해제하는 double-free를 막는다.
- 소유자가 사라질 때 자동으로 해제된다.
- 함수 반환이나 컨테이너 저장에서도 소유권 이동 규칙이 명확하다.

### `std::move`

```cpp
PacketOwner packet = std::move(packet_result.value());
```

`std::move`는 “이 객체의 자원을 다른 곳으로 옮겨도 된다”고 표시한다. 실제 이동은 타입의 move constructor 또는 move assignment가 수행한다.

왜 필요한가:

- `unique_ptr`처럼 복사할 수 없는 타입의 소유권을 넘길 수 있다.
- 큰 객체를 불필요하게 복사하지 않고 전달할 수 있다.

주의:

- `std::move` 이후의 객체는 유효하지만 값이 비어 있을 수 있다.
- 이동한 객체를 다시 사용할 때는 새 값을 넣거나 상태를 확인해야 한다.

### `std::expected`

```cpp
std::expected<PacketOwner, std::string> fetch_network_packet_safe();
```

`std::expected<T, E>`는 성공하면 `T`, 실패하면 `E`를 담는다.

왜 필요한가:

- `nullptr`만 반환하면 왜 실패했는지 알기 어렵다.
- 예외를 쓰지 않는 코드베이스에서도 실패 이유를 명시적으로 전달할 수 있다.
- 호출자가 `if (!result)`로 실패 처리를 강제하게 된다.

### `std::string_view`

```cpp
void print_packet(std::string_view channel, const LegacyPacket& packet);
```

`std::string_view`는 문자열을 복사하지 않고 `[주소 + 길이]`만 바라본다.

왜 필요한가:

- 로그, 파싱, 읽기 전용 문자열 전달에서 복사를 줄인다.
- `std::string`, 문자열 리터럴, 일부 버퍼를 같은 방식으로 받을 수 있다.

주의:

- 소유하지 않는다.
- 원본 문자열이 먼저 사라지면 dangling view가 된다.

### `std::span`

```cpp
int min_subarray_len(int target, std::span<const int> nums);
```

`std::span`은 연속된 배열 데이터를 복사 없이 바라보는 뷰다.

왜 필요한가:

- `std::vector<int>`, C 배열, `std::array<int, N>`을 한 함수로 받을 수 있다.
- 알고리즘 함수가 컨테이너 타입에 덜 묶인다.
- 대량 데이터를 복사하지 않는다.

### 가변 인자 템플릿과 fold expression

```cpp
template <typename... Metadata>
void dispatch_to_broker(std::string_view channel,
                        const LegacyPacket& packet,
                        Metadata&&... metadata) {
    ((std::cout << "  metadata   : " << std::forward<Metadata>(metadata) << '\n'), ...);
}
```

`Metadata&&... metadata`는 개수와 타입이 다른 인자들을 여러 개 받을 수 있다는 뜻이다. fold expression은 이 인자 묶음을 순서대로 펼친다.

왜 필요한가:

- 로그 태그, 옵션, 메타데이터처럼 개수가 고정되지 않은 값을 유연하게 받을 수 있다.
- C++26의 pack indexing이 없어도 현재 C++에서 실무적으로 충분히 유용하다.

## 실무 아키텍처 설명

이번 예제는 작은 코드지만 실무에서 자주 쓰는 경계를 의도적으로 나누었다.

### 1. Legacy API Layer

```cpp
int legacy_network_recv(LegacyPacket** out_packet);
void legacy_network_free(LegacyPacket* packet);
```

역할:

- 외부 C 라이브러리 또는 OS API를 흉내 낸다.
- 메모리 할당과 해제를 C 스타일로 수행한다.

왜 필요한가:

- 실무 코드는 외부 시스템과 연결된다.
- 외부 API를 그대로 프로젝트 전체에 퍼뜨리면 테스트와 유지보수가 어려워진다.

### 2. RAII Wrapper Layer

```cpp
std::expected<PacketOwner, std::string> fetch_network_packet_safe();
```

역할:

- 위험한 `T**` 호출을 한 곳에 가둔다.
- 결과를 `unique_ptr`과 `expected`로 바꿔 안전한 Modern C++ 인터페이스를 제공한다.

왜 필요한가:

- 위험한 코드는 한 곳에 모을수록 좋다.
- 나머지 코드가 raw pointer 해제 규칙을 몰라도 된다.
- 장애 분석 시 실패 지점이 명확해진다.

### 3. Inspection / Logging Layer

```cpp
void print_packet(std::string_view channel, const LegacyPacket& packet);
```

역할:

- 패킷 내용을 사람이 읽을 수 있게 출력한다.
- 브리핑의 “정적 리플렉션으로 자동 출력” 아이디어를 현재 컴파일 가능한 수동 출력으로 대체한다.

왜 필요한가:

- 운영 시스템에서는 관찰 가능성(observability)이 중요하다.
- 로그가 없으면 장애가 났을 때 원인을 재현하기 어렵다.

### 4. Service Layer

```cpp
class PacketGateway {
public:
    std::expected<void, std::string> receive_and_dispatch() const;
};
```

역할:

- “패킷을 받는다 -> 검증한다 -> 전달한다”라는 업무 흐름을 표현한다.
- 하위 구현 세부사항을 호출자에게 숨긴다.

왜 필요한가:

- main 함수가 복잡해지는 것을 막는다.
- 나중에 실제 네트워크, 테스트용 mock, 파일 입력 등으로 교체하기 쉽다.
- 실무 아키텍처에서는 이런 경계가 테스트 단위가 된다.

## 코딩 테스트 문제: 가장 짧은 연속 부분 배열

문제:

양의 정수 배열 `nums`와 양의 정수 `target`이 주어진다. 연속 부분 배열의 합이 `target` 이상이 되는 가장 짧은 길이를 구한다. 없으면 `0`을 반환한다.

예시:

```text
target = 7
nums = [2, 3, 1, 2, 4, 3]
answer = 2
```

정답 구간은 `[4, 3]`이다.

## 알고리즘: 투 포인터 / 슬라이딩 윈도우

배열의 값이 모두 양수이므로 오른쪽 포인터를 이동하면 합은 증가하고, 왼쪽 포인터를 이동하면 합은 감소한다. 이 성질 때문에 윈도우를 한 방향으로만 움직일 수 있다.

```cpp
int min_subarray_len(int target, std::span<const int> nums) {
    int left = 0;
    int current_sum = 0;
    int min_length = std::numeric_limits<int>::max();

    for (int right = 0; right < static_cast<int>(nums.size()); ++right) {
        current_sum += nums[right];

        while (current_sum >= target) {
            min_length = std::min(min_length, right - left + 1);
            current_sum -= nums[left];
            ++left;
        }
    }

    return min_length == std::numeric_limits<int>::max() ? 0 : min_length;
}
```

왜 필요한가:

- 모든 시작점과 끝점을 이중 루프로 검사하면 `O(N^2)`라서 입력이 100,000일 때 느리다.
- 투 포인터는 각 원소가 최대 한 번 들어오고 한 번 나가므로 `O(N)`이다.
- 실무에서도 스트림 처리, 로그 윈도우, 시간 구간 집계에 같은 사고방식이 쓰인다.

복잡도:

- 시간 복잡도: `O(N)`
- 공간 복잡도: `O(1)`

## 브리핑의 C++26 내용과 현재 코드의 관계

브리핑의 의도는 좋지만, 현재 로컬 빌드 환경에서는 모든 C++26 문법을 그대로 사용할 수 없다. 그래서 다음처럼 실무적으로 컴파일 가능한 구조로 바꾸었다.

| 브리핑 키워드 | 현재 예제의 대체 구현 | 이유 |
| --- | --- | --- |
| `std::out_ptr` | raw pointer를 받은 뒤 즉시 `unique_ptr`로 이전 | 현재 환경에서 확실히 빌드되며 같은 소유권 의도를 학습 가능 |
| 정적 리플렉션 `^^T` | `print_packet()` 수동 출력 | 아직 일반 컴파일러에서 안정적으로 쓰기 어렵기 때문 |
| Pack indexing `args...[n]` | 명시 인자 + variadic metadata | 읽기 쉽고 지금 바로 실무에 적용 가능 |
| `std::execution::just/then` | `PacketGateway::receive_and_dispatch()` 서비스 흐름 | 비동기 프레임워크 없이도 아키텍처 흐름 학습 가능 |

핵심은 “최신 문법을 암기하는 것”보다, 왜 그 문법이 필요한지 이해하는 것이다.

- `out_ptr` 계열 기능이 필요한 이유: C API와 스마트 포인터 사이의 위험한 접점을 줄이기 위해
- 리플렉션이 필요한 이유: 반복적인 구조체 출력, 직렬화, 검증 코드를 자동화하기 위해
- pack indexing이 필요한 이유: 템플릿 인자 묶음에서 특정 위치의 값을 쉽게 꺼내기 위해
- 실행 파이프라인이 필요한 이유: 작업 흐름을 조합 가능하게 표현하기 위해

## 오늘 기억할 것

- raw pointer는 “주소”일 뿐이고 소유권을 설명하지 못한다.
- `unique_ptr`은 소유권을 타입으로 표현한다.
- custom deleter는 C 라이브러리의 전용 해제 함수를 Modern C++ 자원 관리에 연결한다.
- `expected`는 실패를 값으로 표현한다.
- `span`과 `string_view`는 복사를 줄이는 읽기 전용 뷰다.
- 실무 아키텍처에서는 위험한 외부 API를 얇은 wrapper로 감싸고, 나머지 코드는 안전한 타입만 다루게 만든다.
- 투 포인터는 양수 배열의 연속 구간 문제에서 `O(N)`을 만드는 핵심 패턴이다.

