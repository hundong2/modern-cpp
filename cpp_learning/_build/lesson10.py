"""Lesson 10: 성능 최적화 — 캐시, 분기 예측, 컴파일러 옵션, RVO, UB, 프로파일링."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from nb_helper import md, code, make_notebook, save

cells = []

cells.append(md(r"""# Lesson 10 · 성능 최적화

> **이 강의의 목표** — "왜 C++ 이 빠른가" 의 진짜 이유를 이해한다. 캐시 메모리, 분기 예측, 컴파일러 최적화, 복사 생략(RVO), 그리고 *측정 없는 최적화는 없다* 는 원칙.

성능 최적화는 직관이 가장 자주 틀리는 분야다. 책에서 "이렇게 하면 빠르다" 는 말은 모두 한 구절을 빠뜨리고 있다 — *"이런 하드웨어, 이런 컴파일러, 이런 워크로드에서"*. 그래서 우리는 **측정** 한다.

이번 강의는 다음 순서로 간다:

1. 측정 헬퍼 만들기 (`std::chrono`)
2. **캐시 메모리** — vector vs list 의 진짜 격차
3. **분기 예측** — 정렬된 배열이 더 빠른 이유
4. **컴파일러 최적화 레벨** (-O0/-O1/-O2/-O3, -flto)
5. **복사 생략 (RVO/NRVO)** — `std::move` 가 오히려 해가 되는 순간
6. **constexpr/consteval** — 런타임 0
7. **Undefined Behavior 함정** — signed overflow, strict aliasing
8. **프로파일링 도구** — perf, callgrind"""))

cells.append(md(r"""## 1. 빌드/측정 헬퍼

이 강의도 셀에서 g++ 을 직접 호출한다. 측정용 코드는 보통 `-O2` 이상으로 컴파일해야 의미가 있다 — `-O0` 은 표준 라이브러리 인라인이 안 되어서 측정값이 거짓말을 한다."""))

cells.append(code(r"""#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

// O 레벨을 지정해 빌드/실행하는 헬퍼
void run_cpp(const std::string& code,
             const std::string& opt = "-O2",
             const std::string& std_ver = "c++20",
             const std::string& extra = "") {
    std::system("mkdir -p /tmp/cpplab");
    FILE* f = std::fopen("/tmp/cpplab/snippet.cpp", "w");
    std::fputs(code.c_str(), f);
    std::fclose(f);
    std::string cmd = "g++ -std=" + std_ver + " " + opt + " -Wall " + extra
                    + " /tmp/cpplab/snippet.cpp -o /tmp/cpplab/snippet 2>&1 "
                      "&& /tmp/cpplab/snippet";
    std::system(cmd.c_str());
}

// 측정 패턴: std::chrono::steady_clock 으로 ns 단위 측정
std::cout << "헬퍼 준비 완료. -O0 vs -O2 차이를 확인하자." << std::endl;"""))

cells.append(md(r"""## 2. 캐시 메모리 — 데이터 구조 선택의 진짜 이유

현대 CPU 는 메모리(DRAM)보다 100배 이상 빠르다. 그래서 CPU 와 DRAM 사이에 **캐시** 가 있다:

| 계층 | 크기 | 지연(approx) |
|------|------|--------------|
| 레지스터 | 64-bit ×수십 개 | 0 cycle |
| L1 캐시 | 32–64 KB / 코어 | 4 cycle |
| L2 캐시 | 256 KB–1 MB / 코어 | 12 cycle |
| L3 캐시 | 4–64 MB / 칩 | 40 cycle |
| DRAM | 수 GB | 200+ cycle |

캐시는 **캐시 라인** 단위로 동작한다 — 보통 **64바이트**. CPU 가 어떤 주소 X 를 읽으면, X 부터 시작하는 64바이트 한 줄을 통째로 캐시에 가져온다. 그래서 *연속 메모리* 를 순차 접근하면 한 번 메모리에서 가져온 캐시 라인을 16번(64B / sizeof(int)=4) 재사용한다.

`std::vector` 는 연속 메모리. `std::list` 는 노드마다 따로 할당 (포인터로 연결). 인덱싱 능력 외에도 이 차이가 성능에 압도적이다."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <vector>
#include <list>
#include <chrono>
#include <numeric>
#include <cstdio>

int main() {
    constexpr int N = 1'000'000;
    std::vector<int> v(N);
    std::list<int>   l;
    for (int i = 0; i < N; ++i) { v[i] = i; l.push_back(i); }

    // vector 순회
    auto t1 = std::chrono::steady_clock::now();
    long long sv = 0;
    for (int x : v) sv += x;
    auto t2 = std::chrono::steady_clock::now();

    // list 순회
    long long sl = 0;
    for (int x : l) sl += x;
    auto t3 = std::chrono::steady_clock::now();

    auto ns_v = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    auto ns_l = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
    std::printf("vector 순회: %8lld ns (sum=%lld)\n", ns_v, sv);
    std::printf("list   순회: %8lld ns (sum=%lld)\n", ns_l, sl);
    std::printf("배율: %.1f×\n", (double)ns_l / ns_v);
}
)CPP", "-O2");"""))

cells.append(md(r"""대개 `list` 가 5–20배 느리게 나온다. 데이터가 똑같고 노드 개수도 같은데 왜?

**list 노드는 메모리 여기저기에 흩어져 있다.** 캐시 라인 하나를 가져와도 그 안에 있는 다음 노드 포인터가 가리키는 곳은 또 다른 라인이다. 캐시 미스가 매번 발생한다.

> **교훈** — *기본은 `std::vector` 다.* `list` 가 정말 필요한 경우는 거의 없다. 중간 삽입이 잦아도 vector 가 보통 빠르다."""))

cells.append(md(r"""### 2-1. AoS vs SoA (Structure of Arrays)

게임/시뮬레이션에서 자주 쓰는 패턴. 같은 데이터를 두 가지 방식으로 저장:

- **AoS** (Array of Structs): `vector<Particle>` — Particle 안에 pos, vel, mass 가 같이.
- **SoA** (Struct of Arrays): pos 배열, vel 배열, mass 배열 따로.

질량만 업데이트한다면 SoA 가 캐시 효율이 압도적이다 (필요 없는 pos/vel 데이터를 캐시에 안 채움)."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <vector>
#include <chrono>
#include <cstdio>

struct ParticleAoS {
    double x, y, z;       // pos (24B)
    double vx, vy, vz;    // vel (24B)
    double mass;          // 8B
    char   pad[8];        // 64B 정렬
};

int main() {
    constexpr int N = 1'000'000;

    // AoS
    std::vector<ParticleAoS> a(N);
    for (int i = 0; i < N; ++i) a[i].mass = 1.0;

    auto t1 = std::chrono::steady_clock::now();
    double sum_a = 0;
    for (const auto& p : a) sum_a += p.mass;     // mass 만 필요한데 64B 통째 읽힘
    auto t2 = std::chrono::steady_clock::now();

    // SoA
    std::vector<double> mass(N, 1.0);
    double sum_b = 0;
    for (double m : mass) sum_b += m;            // 8B 씩 빽빽하게
    auto t3 = std::chrono::steady_clock::now();

    auto ns_a = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    auto ns_b = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
    std::printf("AoS mass 합계: %lld ns\n", ns_a);
    std::printf("SoA mass 합계: %lld ns  (배율 %.1f×)\n", ns_b, (double)ns_a / ns_b);
}
)CPP", "-O2");"""))

cells.append(md(r"""## 3. 분기 예측 — 정렬된 배열이 더 빠르다는 그 유명한 이야기

CPU 는 파이프라인을 채우려고 *if 의 결과를 미리 추측* 한다. 추측이 맞으면 빠르고, 틀리면 파이프라인을 비우고 다시 시작한다 (~15 cycle 손해).

같은 데이터, 같은 코드라도 **데이터가 정렬되어 있으면** 분기 예측이 잘 맞는다."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <cstdio>

int main() {
    constexpr int N = 1'000'000;
    std::vector<int> data(N);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> d(0, 255);
    for (auto& x : data) x = d(rng);

    auto bench = [&](const char* tag) {
        auto t1 = std::chrono::steady_clock::now();
        long long sum = 0;
        for (int x : data) {
            if (x >= 128) sum += x;          // 분기!
        }
        auto t2 = std::chrono::steady_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        std::printf("%-12s: %8lld ns (sum=%lld)\n", tag, ns, sum);
    };

    bench("랜덤");
    std::sort(data.begin(), data.end());
    bench("정렬됨");
}
)CPP", "-O2");"""))

cells.append(md(r"""정렬된 쪽이 보통 2–5배 빠르다. 데이터는 똑같은데 *분기 패턴* 만 달라졌다 (전반은 다 false, 후반은 다 true → CPU 가 쉽게 예측).

**교훈** — branchless 코드(분기 없이 산술로 처리)가 빠를 때가 많다. 단, 측정해봐야 한다. `-O3` 가 알아서 변환하는 경우도 있다."""))

cells.append(md(r"""## 4. 컴파일러 최적화 레벨

| 플래그 | 의미 |
|--------|------|
| `-O0` | 기본. 디버깅용. 인라인 거의 없음. 변수 모두 메모리에. |
| `-O1` | 가벼운 최적화. 죽은 코드 제거, 단순 인라인. |
| `-O2` | **실무 기본값**. 거의 모든 최적화. 빌드 시간 합리적. |
| `-O3` | -O2 + 자동 벡터화(SIMD), 적극적 인라인. 코드 크기 증가. |
| `-Os` | 코드 크기 최소화. |
| `-Ofast` | -O3 + 부동소수 표준 위반 허용 (수치 정확도 포기 가능). |
| `-flto` | Link-Time Optimization. 파일 경계 넘어 인라인. |
| `-march=native` | 현재 CPU 명령어 셋 모두 사용 (AVX2, AVX-512 등). |

같은 코드를 -O0 과 -O2 로 비교해보자."""))

cells.append(code(r"""// 동일 코드를 -O0, -O2, -O3 로 비교
std::string bench_code = R"CPP(
#include <vector>
#include <chrono>
#include <cstdio>

int main() {
    constexpr int N = 10'000'000;
    std::vector<int> v(N, 1);
    auto t1 = std::chrono::steady_clock::now();
    long long s = 0;
    for (int x : v) s += x * 3 - 1;
    auto t2 = std::chrono::steady_clock::now();
    std::printf("sum=%lld  ns=%lld\n", s,
        (long long)std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count());
}
)CPP";

std::cout << "===== -O0 =====\n";
run_cpp(bench_code, "-O0");
std::cout << "\n===== -O2 =====\n";
run_cpp(bench_code, "-O2");
std::cout << "\n===== -O3 -march=native =====\n";
run_cpp(bench_code, "-O3 -march=native");"""))

cells.append(md(r"""차이가 10배 넘게 나는 게 보통이다. **-O3 + -march=native 조합** 에서는 컴파일러가 SIMD 명령어로 한 번에 4–8개 정수를 더한다 (자동 벡터화).

> **주의** — `-march=native` 로 빌드한 바이너리는 같은 CPU 가 아니면 안 돌 수 있다 (illegal instruction). 배포용은 `-march=x86-64-v3` 정도로 보수적으로."""))

cells.append(md(r"""### 4-1. inline 키워드의 진짜 의미

C++ 의 `inline` 은 *컴파일러에게 인라인하라는 명령이 아니다*. 두 가지 역할:

1. **ODR(One Definition Rule) 완화** — 같은 함수가 여러 번역 단위에 있어도 OK.
2. *컴파일러에게 힌트* — 이건 진짜 약한 힌트. `-O2` 면 `inline` 안 붙여도 인라인하고, 안 붙였다고 안 하지도 않는다.

헤더에 함수 정의를 쓰려면 `inline` 이 필요하다 (ODR). 그게 본 용도다."""))

cells.append(md(r"""## 5. 복사 생략 (RVO/NRVO) — `std::move` 의 함정

C++17 부터 컴파일러는 다음을 *의무적으로* 적용한다:

```cpp
std::string make() { return std::string("hello"); }    // 임시 객체 반환 → 복사/이동 0회
auto s = make();                                        // 곧장 s 에 구성됨
```

**RVO (Return Value Optimization)**: 임시 객체를 반환하면 호출자 슬롯에 직접 구성한다. **복사도 이동도 안 일어난다.**

**NRVO (Named RVO)**: 이름 있는 지역 변수도 같이 적용 (의무는 아니지만 거의 다 함):

```cpp
std::string make() {
    std::string s;
    s += "hello";
    return s;        // NRVO — s 는 반환 슬롯에 직접 만들어짐
}
```

**함정**: `return std::move(s)` 라고 쓰면 **NRVO 가 막힌다.** 일부러 이동을 강제해서 *복사 생략보다 못한 결과* 가 된다."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <string>

struct Loud {
    Loud()                       { std::cout << "  생성\n"; }
    Loud(const Loud&)            { std::cout << "  복사\n"; }
    Loud(Loud&&) noexcept        { std::cout << "  이동\n"; }
    ~Loud()                      { std::cout << "  소멸\n"; }
};

Loud good() {
    Loud x;
    return x;                    // NRVO: 복사도 이동도 0회
}

Loud bad() {
    Loud x;
    return std::move(x);         // NRVO 막힘 → 이동 1회 발생
}

int main() {
    std::cout << "--- good() ---\n";
    auto a = good();
    std::cout << "--- bad() ---\n";
    auto b = bad();
}
)CPP", "-O2");"""))

cells.append(md(r"""**교훈** — 반환문에서 `std::move` 는 거의 항상 잘못된 코드. *컴파일러를 믿어라.*

예외: 매개변수로 받은 값을 그대로 반환할 때는 `return std::move(param)` 이 필요 (NRVO 비적용). 이건 드문 경우."""))

cells.append(md(r"""## 6. constexpr / consteval — 런타임 0

`constexpr` 함수는 *컴파일타임에 호출 가능* 하면 컴파일타임에 실행된다. 결과 값이 바이너리에 박힌다."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <array>

// 컴파일타임에 피보나치 계산
constexpr long long fib(int n) {
    return n < 2 ? n : fib(n-1) + fib(n-2);
}

// C++20: consteval — 반드시 컴파일타임에만 (런타임 호출 불가)
consteval int square(int x) { return x * x; }

int main() {
    constexpr long long f30 = fib(30);    // 컴파일타임 계산
    std::cout << "fib(30) = " << f30 << " (런타임 비용 0)\n";

    constexpr int s = square(7);
    std::cout << "square(7) = " << s << "\n";

    // 컴파일타임 룩업 테이블
    constexpr auto squares = []{
        std::array<int, 10> a{};
        for (int i = 0; i < 10; ++i) a[i] = i * i;
        return a;
    }();
    for (int x : squares) std::cout << x << " ";
    std::cout << "\n";
}
)CPP", "-O2");"""))

cells.append(md(r"""**활용 포인트** — 설정 값, 룩업 테이블, 컴파일타임 검증. 표준 라이브러리도 점점 더 `constexpr` 로 변환되고 있다 (C++20 `std::vector` 도 `constexpr` 가능)."""))

cells.append(md(r"""## 7. Undefined Behavior — 최적화의 그림자

컴파일러는 "프로그래머는 UB 를 작성하지 않는다" 고 *가정* 하고 최적화한다. UB 가 있으면 코드가 사라지거나 미친 짓을 한다.

### 7-1. Signed integer overflow

`int` 의 오버플로는 **UB**. `unsigned` 는 wrap (정의됨)."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <climits>

int main() {
    int x = INT_MAX;
    // 컴파일러는 "x+1 > x 는 항상 참" 으로 최적화 가능 (signed overflow UB 가정)
    if (x + 1 < x) std::cout << "overflow detected\n";
    else           std::cout << "컴파일러는 이 분기를 살리지 않을 수 있다\n";

    // 안전: unsigned wrap
    unsigned u = UINT_MAX;
    std::cout << "unsigned wrap: " << u + 1 << " (정의된 동작: 0)\n";
}
)CPP", "-O2");"""))

cells.append(md(r"""### 7-2. Strict aliasing

서로 다른 타입의 포인터로 같은 메모리를 다루면 UB (예외: char*, std::byte*).

```cpp
float f = 1.0f;
int* p = (int*)&f;       // UB!
int  i = *p;             // 컴파일러는 f, *p 가 같은 메모리라고 *추론하지 않을 수* 있다
```

**올바른 방법**: `std::memcpy` 또는 C++20 `std::bit_cast`."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <cstring>
#include <bit>
#include <cstdint>
#include <iostream>

int main() {
    float f = 3.14f;

    // 안전: memcpy
    std::uint32_t bits;
    std::memcpy(&bits, &f, sizeof(bits));
    std::cout << "memcpy: " << std::hex << bits << "\n";

    // 더 안전 + constexpr: bit_cast (C++20)
    auto bits2 = std::bit_cast<std::uint32_t>(f);
    std::cout << "bit_cast: " << std::hex << bits2 << "\n";
}
)CPP", "-O2");"""))

cells.append(md(r"""**UB 검출 도구** (꼭 빌드/CI 에 포함):

- `-fsanitize=undefined` — UndefinedBehaviorSanitizer (UBSan)
- `-fsanitize=address` — AddressSanitizer (메모리 오류)
- `-fsanitize=thread` — ThreadSanitizer (데이터 경쟁)

이 셋만 통과해도 C++ 의 가장 흔한 버그 90% 는 잡힌다."""))

cells.append(md(r"""## 8. 프로파일링 도구

### 8-1. perf — 리눅스의 표준

```bash
g++ -O2 -g program.cpp -o program     # -g 로 디버그 심볼
perf record -g ./program              # 콜스택 포함 프로파일
perf report                           # 어디서 시간 쓰는지 트리 뷰
```

**중요**: `-O2 -g` 같이 둘 다. 최적화 켠 채로 디버그 심볼만 추가.

### 8-2. callgrind / kcachegrind

```bash
valgrind --tool=callgrind ./program
kcachegrind callgrind.out.*
```

함수별 누적 시간을 그래프로 본다. perf 보다 100배 느리지만 정확하다 (시뮬레이션 기반).

### 8-3. 직접 측정 — 가장 정직

벤치마크 라이브러리(Google Benchmark) 가 표준이지만, 우리는 `std::chrono` 만으로 충분하다. 핵심 규칙:

1. **워밍업**: 첫 측정은 버린다 (캐시 채우기, 분기 예측 학습).
2. **여러 번 측정**: 평균보다 *중앙값* 이 안정적.
3. **컴파일러가 코드 제거 못하게**: 결과를 사용하거나 `volatile` 또는 `asm volatile("" : : "r"(x))` 트릭."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <chrono>
#include <cstdio>
#include <vector>
#include <algorithm>

template <class F>
double bench_ns(F&& f, int iters = 100) {
    std::vector<long long> samples;
    samples.reserve(iters);
    for (int i = 0; i < iters; ++i) {
        auto t1 = std::chrono::steady_clock::now();
        f();
        auto t2 = std::chrono::steady_clock::now();
        samples.push_back(
            std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
    }
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];   // 중앙값
}

int main() {
    auto work = [] {
        long long s = 0;
        for (int i = 0; i < 100'000; ++i) s += i;
        // 컴파일러가 통째로 날리지 못하게 결과 사용
        asm volatile("" : : "r"(s) : "memory");
    };
    work();   // 워밍업

    double median_ns = bench_ns(work, 200);
    std::printf("중앙값: %.0f ns\n", median_ns);
}
)CPP", "-O2");"""))

cells.append(md(r"""## 9. 최적화의 황금률

> *"Premature optimization is the root of all evil."* — Donald Knuth

순서:

1. **돌게 만든다** (정확하게).
2. **테스트한다**.
3. **측정한다** — 어디가 느린지 *추측하지 말고*.
4. **알고리즘부터 본다** — O(n²) 를 O(n log n) 으로 바꾸는 게 마이크로 최적화 100개보다 크다.
5. **데이터 구조** — 캐시 친화적인지.
6. **그 다음에** 마이크로 최적화 — 핫 루프만, 측정으로 확인하면서.

수치로:

- 알고리즘 개선: 10×–1000×
- 캐시 친화 자료구조: 5×–20×
- SIMD/벡터화: 2×–8×
- 분기 제거: 1.2×–3×
- 마이크로 튜닝: 1.05×–1.2×

대부분의 시간은 **상위 두 개** 에서 온다."""))

cells.append(md(r"""## 10. 실습

다음 순서로 측정해보자:

1. `int sum_arr(const std::vector<int>& v)` 작성. -O0 / -O2 / -O3 시간 비교.
2. AoS 와 SoA 로 같은 작업을 짜고 시간 비교.
3. `std::vector` vs `std::deque` vs `std::list` 의 순회 시간 비교.

힌트: 위 셀들의 패턴을 그대로 쓰면 된다."""))

cells.append(code(r"""// TODO: 위 실습 중 하나를 작성하라.
// 예시 시작점:
run_cpp(R"CPP(
#include <vector>
#include <chrono>
#include <cstdio>

int main() {
    // 여기에 작성
    std::printf("결과: ...\n");
}
)CPP", "-O2");"""))

cells.append(md(r"""## 11. 정리

- **캐시가 왕이다.** 자료 구조의 메모리 레이아웃이 빅 O 보다 자주 결정적이다.
- **컴파일러를 믿어라.** -O2 가 기본. `inline` 직접 쓰지 말고, `std::move` 를 반환문에 쓰지 말고.
- **UB 를 피하라.** UBSan / ASan / TSan 을 CI 에 넣어라.
- **측정하라.** 추측은 50% 확률로 틀린다.

다음 강의(Lesson 11) 는 **종합 프로젝트** 다. 지금까지 배운 모든 것 — 클래스/RAII/스마트포인터/템플릿/concepts/ranges/concurrency/networking/optimization — 을 합쳐 작은 키-값 저장소 + HTTP 서버를 만든다."""))

nb = make_notebook(cells, kernel="xcpp17")
out = os.path.join(os.path.dirname(__file__), "..", "notebooks", "10_optimization.ipynb")
save(nb, out)
