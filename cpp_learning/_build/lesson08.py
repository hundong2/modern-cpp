"""Lesson 08: 동시성, 동기화, 메모리 모델."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from nb_helper import md, code, make_notebook, save

cells = []

cells.append(md(r"""# Lesson 08 · 동시성 — 스레드, 뮤텍스, 원자적 연산, 메모리 모델

> **이 강의의 목표** — `std::thread` 를 만들 줄 알고, 데이터 경쟁이 *왜* 일어나는지 메모리 레벨에서 설명할 수 있고, atomic 의 memory_order 가 무엇을 보장하는지 한 줄로 말할 수 있게 된다.

## 1. 첫 스레드"""))

cells.append(code(r"""#include <iostream>
#include <thread>
#include <chrono>

void worker(int id) {
    for (int i = 0; i < 3; ++i) {
        std::cout << "worker " << id << ": " << i << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::thread t1(worker, 1);
std::thread t2(worker, 2);
t1.join();   // t1 이 끝날 때까지 메인이 기다린다
t2.join();
std::cout << "끝\n";"""))

cells.append(md(r"""**필수 규칙**:

- `std::thread` 객체는 소멸되기 전에 *반드시* `join()` 또는 `detach()` 가 불려야 한다. 그러지 않으면 `std::terminate()` 가 호출되어 프로그램이 죽는다.
- C++20 부터의 `std::jthread` 는 소멸 시 자동으로 join 한다 — 가능하면 jthread 를 써라.

## 2. 데이터 경쟁 — 왜 일어나는가"""))

cells.append(code(r"""#include <iostream>
#include <thread>
#include <vector>

int counter = 0;            // 보호되지 않은 공유 변수

auto inc = [] {
    for (int i = 0; i < 100'000; ++i) ++counter;
};

std::vector<std::thread> ts;
for (int i = 0; i < 4; ++i) ts.emplace_back(inc);
for (auto& t : ts) t.join();

std::cout << "기대값: 400000, 실제: " << counter << "\n";"""))

cells.append(md(r"""**왜 400000 이 안 나오나** — `++counter` 는 한 줄짜리 코드지만 기계어로는 *세 단계*다:

```
1. memory[counter] → register      (load)
2. register + 1 → register         (add)
3. register → memory[counter]      (store)
```

스레드 A 가 1번을 읽고 (counter=42), 스레드 B 도 1번을 읽고 (counter=42), A 가 2,3 을 해서 43, B 도 2,3 을 해서 43 — 두 번 더했는데 한 번만 증가했다. 이걸 **데이터 경쟁 (data race)** 이라 한다. C++ 표준은 데이터 경쟁이 있는 프로그램의 동작을 *정의되지 않음* 이라고 단호하게 못박는다.

## 3. mutex 로 동기화"""))

cells.append(code(r"""#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int counter = 0;
std::mutex mtx;

auto inc = [] {
    for (int i = 0; i < 100'000; ++i) {
        std::lock_guard<std::mutex> lock(mtx);   // RAII: 스코프 끝나면 자동 unlock
        ++counter;
    }
};

std::vector<std::thread> ts;
for (int i = 0; i < 4; ++i) ts.emplace_back(inc);
for (auto& t : ts) t.join();

std::cout << "counter = " << counter << "\n";   // 정확히 400000"""))

cells.append(md(r"""**`std::lock_guard`** — Lesson 03 에서 직접 만들어본 그 RAII 타입의 표준 버전. `mtx.lock()` 을 해주고 스코프가 끝날 때 `mtx.unlock()` 을 해준다. 절대 손으로 lock/unlock 호출하지 마라 — 예외나 early return 한 번에 데드락이 생긴다.

C++17 부터의 **`std::scoped_lock`** 은 lock_guard 의 상위 호환 — 여러 뮤텍스를 한 번에 잠그면서 데드락을 피하는 알고리즘이 들어있다.

```cpp
std::mutex m1, m2;
std::scoped_lock lock(m1, m2);   // 두 개를 데드락 없이 잠근다
```

## 4. atomic — 락 없이 동시 접근

뮤텍스는 비싸다 (시스템 콜이 끼어들 수도 있고). 단순 카운터 같은 경우엔 **`std::atomic`** 을 쓴다."""))

cells.append(code(r"""#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

std::atomic<int> counter = 0;

auto inc = [] {
    for (int i = 0; i < 100'000; ++i) ++counter;   // atomic 의 ++ 는 원자적
};

std::vector<std::thread> ts;
for (int i = 0; i < 4; ++i) ts.emplace_back(inc);
for (auto& t : ts) t.join();

std::cout << "counter = " << counter.load() << "\n";"""))

cells.append(md(r"""`atomic<int>::operator++` 는 단일한 원자적 명령어로 컴파일된다 (x86 의 경우 `lock xadd`). 락이 없어 일반적으로 mutex 보다 빠르다.

## 5. 메모리 모델 — `memory_order` 의 진짜 정체

> 여기부터는 *심장 약한 사람을 위한 게 아니다*. 처음 보면 한 번에 이해하지 못해도 정상이다 — 다시 와서 보라.

CPU 와 컴파일러는 코드의 실행 순서를 *재배치한다* (관찰 가능한 동작이 같다는 조건 하에). 단일 스레드에서는 영향이 없지만, 여러 스레드가 같은 변수를 다룰 때는 누군가가 본 순서가 다른 누군가가 본 순서와 다를 수 있다.

C++ 의 `memory_order` 는 *얼마나 강한 순서 보장을 요구할지* 를 명시한다:

| memory_order | 보장 | 비용 |
|--------------|------|------|
| `relaxed` | 원자성만. 다른 메모리 연산과의 순서는 보장 안 됨 | 매우 싸 |
| `acquire` | 이 연산 이전의 모든 메모리 연산이 *그 다음에 다른 스레드에서* 보이도록 한다 (load 에 사용) | 보통 |
| `release` | 이 연산 *이후의* 메모리 연산이 *이전 스레드의 연산보다 늦게* 보이도록 한다 (store 에 사용) | 보통 |
| `acq_rel` | acquire + release | 보통 |
| `seq_cst` | 모든 스레드가 *같은* 전역 순서로 본다. 디폴트 | 가장 비쌈 |

대부분의 경우 디폴트(`seq_cst`)를 쓰면 된다. acquire/release 는 락 없는 자료구조 (`spinlock`, `lock-free queue`)를 직접 만들 때 등장한다."""))

cells.append(code(r"""#include <atomic>
#include <iostream>
#include <thread>
#include <string>

// 전형적인 release-acquire 패턴: '준비됨' 플래그
std::atomic<bool> ready{false};
std::string payload;

auto producer = [] {
    payload = "Hello from producer";    // 1. 데이터 준비
    ready.store(true, std::memory_order_release);  // 2. 플래그 set
    // release: 1번이 2번보다 늦게 보일 수 없다
};

auto consumer = [] {
    while (!ready.load(std::memory_order_acquire)) { /* spin */ }
    // acquire: ready 가 true 로 보이는 순간, 이전의 모든 store(payload 포함)도 보임
    std::cout << "consumer received: " << payload << "\n";
};

std::thread t1(producer), t2(consumer);
t1.join(); t2.join();"""))

cells.append(md(r"""**핵심 직관**: release 는 "내가 한 일을 다른 스레드에 *내보낸다*", acquire 는 "다른 스레드가 내보낸 것을 *받아온다*". 이 두 연산이 한 atomic 변수에서 만나야 데이터가 안전하게 건너간다.

이 패턴이 있어야 spinlock, message queue, double-checked locking 같은 락 없는 구조를 정확히 만들 수 있다. 그렇지 않으면 데이터 경쟁이다.

## 6. `std::async` 와 `std::future` — 함수의 결과를 비동기로"""))

cells.append(code(r"""#include <iostream>
#include <future>
#include <chrono>
#include <thread>

auto slow_compute = [](int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return x * x;
};

// 백그라운드에서 실행
std::future<int> f = std::async(std::launch::async, slow_compute, 7);

// 그 사이 다른 일
std::cout << "다른 일 하는 중...\n";

// 결과를 받을 때 — 아직 안 끝났으면 여기서 대기
int result = f.get();
std::cout << "결과 = " << result << "\n";"""))

cells.append(md(r"""`std::async` + `std::future` 는 한 번 결과를 받는 *일회용*이다. 진짜 비동기 인프라가 필요하면 — 라이브러리(boost::asio, folly, libuv) 또는 C++20 coroutines 를 본다.

## 7. False Sharing — 캐시 레벨에서의 함정

CPU 캐시는 **캐시 라인 (보통 64바이트)** 단위로 동작한다. 두 개의 별도 atomic 이 *같은 캐시 라인 안에* 들어가면, 두 스레드가 각각 자기 변수를 수정해도 *서로의 캐시를 무효화* 시킨다 — 락도 없는데 성능이 폭락한다."""))

cells.append(code(r"""#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

struct Bad {
    std::atomic<int> a;
    std::atomic<int> b;          // a 와 같은 캐시 라인일 가능성 큼
};

struct Good {
    alignas(64) std::atomic<int> a;
    alignas(64) std::atomic<int> b;          // 별도 캐시 라인 보장
};

template <typename T>
void bench(const char* name) {
    T s{};
    auto worker_a = [&] { for (int i = 0; i < 5'000'000; ++i) s.a.fetch_add(1, std::memory_order_relaxed); };
    auto worker_b = [&] { for (int i = 0; i < 5'000'000; ++i) s.b.fetch_add(1, std::memory_order_relaxed); };

    auto start = std::chrono::high_resolution_clock::now();
    std::thread t1(worker_a), t2(worker_b);
    t1.join(); t2.join();
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << name << ": "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
              << " ms\n";
}

bench<Bad>("Bad (false sharing)");
bench<Good>("Good (aligned)");"""))

cells.append(md(r"""두 결과를 비교하면 — *같은 작업량인데* aligned 버전이 보통 2~10배 빠르다. 멀티스레드 환경에서 atomic 변수는 **한 캐시 라인에 하나씩** 두는 게 정석이다. C++17 의 `std::hardware_destructive_interference_size` 가 이 값을 표준으로 제공한다.

## 8. 데드락 — 어떻게 일어나고 어떻게 막나"""))

cells.append(code(r"""#include <iostream>
#include <mutex>
#include <thread>

std::mutex m1, m2;

auto thread_A = [] {
    std::lock_guard<std::mutex> lk1(m1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lk2(m2);   // m2 를 기다린다
    std::cout << "A done\n";
};

auto thread_B_buggy = [] {
    std::lock_guard<std::mutex> lk2(m2);   // m1 과 반대 순서!
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lk1(m1);   // 데드락
    std::cout << "B done\n";
};

// 위는 데드락 가능성. 아래가 해법.
auto thread_B_fixed = [] {
    std::scoped_lock lk(m1, m2);   // 여러 뮤텍스를 한 번에, 데드락 회피 알고리즘
    std::cout << "B done\n";
};

// 시연: 안전 버전만 실행 (데드락 버전 돌리면 노트북이 멈춤)
std::thread tA(thread_A), tB(thread_B_fixed);
tA.join(); tB.join();
std::cout << "둘 다 끝\n";"""))

cells.append(md(r"""**데드락 회피 4 규칙**:

1. 락은 항상 *같은 순서로* 잡아라.
2. 락을 들고 있는 동안 다른 스레드와 동기화해야 하는 코드를 호출하지 마라.
3. 여러 뮤텍스가 필요하면 `std::scoped_lock` 으로 한 번에 잡아라.
4. 락의 *수명을 짧게* 유지하라.

## 9. 실습"""))

cells.append(code(r"""#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>

// TODO: 스레드 안전한 간단한 큐를 작성하라.
//   - push(T)  — 락 잡고 큐에 push
//   - try_pop(T&) -> bool  — 비어 있으면 false 반환, 아니면 *out 에 값 넣고 pop, true
//   - 두 멤버 함수 모두 mutex 로 보호
//   - mutex 를 들고 있는 시간을 *최소화* 하라

template <typename T>
class ThreadSafeQueue {
public:
    void push(T x) {
        // 채워라
    }
    bool try_pop(T& out) {
        // 채워라
        return false;
    }
private:
    std::queue<T> q_;
    std::mutex m_;
};

// 테스트
ThreadSafeQueue<int> q;
auto producer = [&] { for (int i = 0; i < 100; ++i) q.push(i); };
auto consumer = [&] {
    int v, taken = 0;
    while (taken < 100) if (q.try_pop(v)) ++taken;
    std::cout << "consumer 끝\n";
};
std::thread t1(producer), t2(consumer);
t1.join(); t2.join();"""))

cells.append(md(r"""## 10. 정리

- `std::thread` 는 join 또는 detach 필수. C++20 의 `std::jthread` 가 더 안전.
- `++counter` 는 원자적이지 않다. mutex 또는 atomic 으로 보호.
- 단순 카운터에는 atomic 이 mutex 보다 빠르다. 복잡한 임계구역은 mutex.
- memory_order: 모르겠으면 디폴트 `seq_cst` 가 안전. 락 없는 구조 만들 때 acquire/release 등장.
- False sharing 에 주의 — 멀티스레드 atomic 은 캐시 라인 align.
- 데드락 회피: 락 순서 고정, scoped_lock 활용, 락 수명 짧게.

다음 강의 — Lesson 09: 네트워크 프로그래밍 (소켓, TCP/UDP, OSI 계층, 에코 서버 실습).
"""))

nb = make_notebook(cells, kernel="xcpp17")
save(nb, "/home/claude/cpp_learning/notebooks/08_concurrency.ipynb")
