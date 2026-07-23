/*
Chapter 23. thread, mutex, atomic, false sharing, 병렬 합산 기초

목표:
- 실무 C++ 동시성 코드를 읽을 때 race condition, lock contention, false sharing을 찾는다.
- "스레드를 많이 만들면 빨라진다"가 아니라 공유 상태와 메모리 접근 패턴을 먼저 본다.

리뷰 체크리스트:
- 여러 스레드가 같은 변수에 lock 없이 쓰고 있지 않은가?
- 너무 작은 작업을 스레드로 쪼개 thread 생성/동기화 비용이 더 커지지 않았는가?
- hot loop 안에서 mutex를 매번 잡아 lock contention을 만들고 있지 않은가?
- 서로 다른 counter라도 같은 cache line에 있어 false sharing이 발생하지 않는가?
- atomic memory_order를 쓰는 경우 ordering 요구사항을 설명할 수 있는가?
*/

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>
using namespace std;

struct PackedCounter {
    atomic<long long> value{0}; // std::atomic은 data race 없이 동시 읽기/쓰기를 보장하는 표준 원자 타입이다.
};

struct alignas(64) PaddedCounter {
    atomic<long long> value{0}; // 64바이트 정렬은 일반적인 cache line 크기를 기준으로 false sharing을 줄인다.
};

long long sumSingleThread(const vector<int>& values) {
    long long total = 0;
    for (int x : values) {
        total += x; // 단일 스레드는 동기화 비용이 없어 작은 입력에서는 오히려 빠를 수 있다.
    }
    return total;
}

long long sumWithMutexPerElement(const vector<int>& values) {
    mutex m; // std::mutex는 한 번에 한 스레드만 critical section에 들어가게 하는 상호 배제 객체다.
    long long total = 0;

    for (int x : values) {
        lock_guard<mutex> lock(m); // lock_guard는 생성 시 lock, 소멸 시 unlock하는 RAII wrapper라 예외에도 mutex가 풀린다.
        total += x;
    }
    return total;
}

long long parallelSum(const vector<int>& values) {
    unsigned workers = thread::hardware_concurrency();
    workers = max(1u, min(workers == 0 ? 2u : workers, 4u)); // 데모에서는 과도한 스레드 생성을 막기 위해 상한을 둔다.

    vector<long long> partial(workers, 0); // 각 스레드가 자기 칸에만 쓰면 mutex가 필요 없다.
    vector<thread> threads; // std::thread는 실행 중인 OS 스레드 handle을 소유하므로 join 또는 detach가 필요하다.
    threads.reserve(workers);

    size_t block = (values.size() + workers - 1) / workers; // 마지막 worker가 남은 꼬리 구간을 처리하게 한다.
    for (unsigned id = 0; id < workers; ++id) {
        size_t begin = id * block;
        size_t end = min(values.size(), begin + block);

        threads.emplace_back([&, id, begin, end] {
            long long local = 0; // hot loop에서는 공유 변수 대신 thread-local 누적값을 쓴다.
            for (size_t i = begin; i < end; ++i) {
                local += values[i];
            }
            partial[id] = local; // 스레드당 한 번만 공유 배열에 기록해 contention을 줄인다.
        });
    }

    for (thread& t : threads) {
        t.join(); // join 이후 partial 쓰기가 끝났다는 happens-before 관계가 생긴다.
    }

    return accumulate(partial.begin(), partial.end(), 0LL); // <numeric>의 accumulate는 iterator 범위를 왼쪽부터 누적한다.
}

long long atomicCounterDemo(int iterations) {
    atomic<long long> counter{0};
    thread t1([&] {
        for (int i = 0; i < iterations; ++i) {
            counter.fetch_add(1, memory_order_relaxed); // 단순 카운트라 순서 보장 없이 원자성만 필요하다.
        }
    });
    thread t2([&] {
        for (int i = 0; i < iterations; ++i) {
            counter.fetch_add(1, memory_order_relaxed);
        }
    });

    t1.join();
    t2.join();
    return counter.load(memory_order_relaxed); // join 이후라 최신 값을 관찰할 수 있다.
}

template <typename Counter>
long long incrementCounters(int workers, int iterations) {
    vector<Counter> counters(workers);
    vector<thread> threads;
    threads.reserve(workers);

    for (int id = 0; id < workers; ++id) {
        threads.emplace_back([&, id] {
            for (int i = 0; i < iterations; ++i) {
                counters[id].value.fetch_add(1, memory_order_relaxed); // 논리적으로 독립된 counter라도 메모리 배치가 성능에 영향을 준다.
            }
        });
    }

    for (thread& t : threads) {
        t.join();
    }

    long long total = 0;
    for (Counter& counter : counters) {
        total += counter.value.load(memory_order_relaxed);
    }
    return total;
}

template <typename Func>
long long elapsedMilliseconds(Func&& func) {
    auto start = chrono::steady_clock::now();
    volatile long long guard = func(); // benchmark 대상 코드가 최적화로 제거되는 일을 줄인다.
    (void)guard;
    auto end = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<int> values(100'000);
    iota(values.begin(), values.end(), 1);

    cout << "[single] " << sumSingleThread(values) << '\n';
    cout << "[parallel] " << parallelSum(values) << '\n';
    cout << "[atomic] " << atomicCounterDemo(10'000) << '\n';

    int workers = 4;
    int iterations = 20'000;
    cout << "[packed counters] " << incrementCounters<PackedCounter>(workers, iterations) << '\n';
    cout << "[padded counters] " << incrementCounters<PaddedCounter>(workers, iterations) << '\n';

    cout << "[timing hint ms] mutex_per_element="
         << elapsedMilliseconds([&] { return sumWithMutexPerElement(values); }) << '\n';
    return 0;
}
