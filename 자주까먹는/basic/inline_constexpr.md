# 모던 C++(C++11 ~ C++20 이후)에서 `inline` 키워드의 의미와 최적화 패러다임

- [problem algorithm with inline](../algorithm/12_roman_map.md). 

- 과거와 크게 달라졌습니다. 현대의 컴파일러(GCC, Clang, MSVC)는 개발자가 `inline`을 붙였다고 해서 무조건 인라인화를 수행하지 않으며, 반대로 안 붙여도 스스로 판단하여 인라인화를 수행합니다.

실무에서 `inline`과 최적화를 다루는 가장 현대적이고 트렌디한 패턴 4가지를 정리해 드립니다.

---

### 1. `constexpr`와 `consteval`: 진정한 의미의 '컴파일 타임' 최적화

모던 C++에서는 런타임 오버헤드를 아예 없애기 위해 **컴파일 타임에 값을 계산**해버리는 방식을 선호합니다. `constexpr` 키워드를 사용하면 함수가 암시적으로 `inline` 처리되며, 컴파일 타임에 평가될 수 있습니다.

```cpp
// C++14 이상 권장 패턴
[[nodiscard]] constexpr int getVal(char c) noexcept {
    switch(c) {
        case 'I': return 1;   case 'V': return 5;   case 'X': return 10;
        case 'L': return 50;  case 'C': return 100; case 'D': return 500;
        case 'M': return 1000; default: return 0;
    }
}

```

* **`constexpr`**: 인자로 컴파일 타임 상수(예: `getVal('X')`)가 들어오면, 런타임에 함수를 호출하지 않고 어셈블리 단에서 바로 숫자 `10`으로 치환해 버립니다.
* **`[[nodiscard]]`**: 반환값을 무시하면 컴파일러 경고를 띄워 실수를 방지합니다.
* **`noexcept`**: 이 함수가 예외를 던지지 않음을 보장하여, 컴파일러가 예외 처리 코드를 생략하고 더 강력하게 최적화하도록 돕습니다.

*(참고: C++20부터는 무조건 컴파일 타임에만 실행되도록 강제하는 `consteval` 키워드도 추가되었습니다.)*

---

### 2. 분기 예측 최적화: `[[likely]]` / `[[unlikely]]` (C++20)

`switch-case`는 컴파일러가 내부적으로 '점프 테이블(Jump Table)'로 최적화하여 $O(1)$에 가깝게 동작하지만, 특정 예외 케이스(예: 잘못된 문자가 들어오는 경우)에 대해 CPU의 분기 예측(Branch Prediction)을 도울 수 있습니다.

```cpp
constexpr int getVal(char c) noexcept {
    switch(c) {
        case 'I': return 1;   
        // ... (생략) ...
        case 'M': return 1000; 
        
        // 정상적인 로마자가 아닌 값이 들어올 확률이 매우 낮다고 컴파일러에 힌트 제공
        [[unlikely]] default: return 0; 
    }
}

```

---

### 3. 진정한 $O(1)$ 초고속 매핑: `constexpr` 룩업 테이블 (Lookup Table)

실무에서 극단적인 성능(Low-Latency)이 필요할 때, `switch-case`의 분기조차 없애기 위해 **컴파일 타임 배열 매핑**을 사용합니다. 문자의 아스키코드 값을 인덱스로 활용합니다.

```cpp
#include <array>

// 컴파일 타임에 256크기의 매핑 배열을 생성
constexpr auto make_roman_table() {
    std::array<int, 256> table{}; // 0으로 초기화
    table['I'] = 1; table['V'] = 5; table['X'] = 10;
    table['L'] = 50; table['C'] = 100; table['D'] = 500;
    table['M'] = 1000;
    return table;
}

// ROMAN_MAP은 컴파일 타임에 이미 완성된 데이터로 메모리에 올라감
inline constexpr auto ROMAN_MAP = make_roman_table();

// 사용 시 함수 호출이나 분기 없이 메모리 접근 1번으로 끝
// int val = ROMAN_MAP['X']; 

```

이 패턴은 분기문 자체가 사라지기 때문에 현대 CPU 파이프라인에서 성능 저하를 일으킬 수 있는 분기 예측 실패(Branch Misprediction)를 원천 차단합니다.

---

### 4. `inline` 변수 (C++17): 헤더 파일 전용(Header-only) 라이브러리 제작

현대의 `inline`은 '함수 최적화 힌트'라기보다 'ODR(One Definition Rule, 단일 정의 규칙) 회피용'으로 더 많이 쓰입니다. C++17부터는 변수에도 `inline`을 붙일 수 있게 되어, 헤더 파일에 전역 변수나 정적(static) 멤버 변수를 정의해도 중복 정의 링크 에러가 나지 않습니다.

```cpp
// config.hpp
struct GameConfig {
    // .cpp 파일에 따로 초기화할 필요 없이 헤더에서 바로 정의 가능!
    static inline int max_players = 100; 
    static inline const char* server_ip = "127.0.0.1";
};

```

> **요약하자면:** 현대 실무에서는 단순 `inline` 대신 `constexpr`와 `noexcept`를 조합하여 컴파일러의 최적화 잠재력을 극대화하고, 링크 에러를 방지하는 용도로 `inline`을 활용하는 것이 정석입니다.