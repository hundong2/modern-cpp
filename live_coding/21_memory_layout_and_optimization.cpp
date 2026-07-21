/*
Chapter 21. 메모리 배치, 캐시, 어셈블리 관점 최적화 기초

목표:
- 실무 C++ 코드에서 "왜 느린지"를 읽어내는 기본 관점을 익힌다.
- 실제 어셈블리를 외우기보다, 어떤 C++ 패턴이 load/store, branch, cache miss를 만드는지 이해한다.

리뷰 체크리스트:
- 큰 객체를 값으로 복사하고 있지 않은가?
- vector를 push_back하면서 reserve 없이 재할당을 반복하고 있지 않은가?
- 2차원 배열을 메모리 순서와 반대로 순회하고 있지 않은가?
- struct 필드 순서 때문에 padding이 불필요하게 커지지 않았는가?
- 분기 예측이 계속 실패할 만한 if가 hot loop 안에 있지 않은가?
*/

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct BadLayout {
    char tag;        // 1바이트지만 뒤의 long long 정렬 때문에 padding이 생긴다.
    long long value; // 8바이트 정렬이 필요한 필드는 앞쪽에 두는 편이 보통 낫다.
    int flag;
};

struct BetterLayout {
    long long value; // 큰 정렬 단위 필드를 먼저 배치하면 padding 낭비를 줄이기 쉽다.
    int flag;
    char tag;
};

long long sumContiguous(const vector<int>& values) {
    const int* current = values.data();          // vector는 원소가 연속된 메모리에 저장된다.
    const int* end = current + values.size();    // 포인터 끝을 미리 계산하면 루프 조건이 단순해진다.
    long long total = 0;

    while (current != end) {
        total += *current; // 어셈블리 관점에서는 load -> add -> pointer increment 흐름으로 내려가기 쉽다.
        ++current;         // 연속 접근은 CPU prefetcher와 cache line 활용에 유리하다.
    }
    return total;
}

long long sumMatrixRowMajor(const vector<int>& matrix, int rows, int cols) {
    long long total = 0;
    for (int r = 0; r < rows; ++r) {
        int base = r * cols; // row-major 저장에서는 같은 행의 원소가 메모리에 붙어 있다.
        for (int c = 0; c < cols; ++c) {
            total += matrix[base + c]; // 순차 접근이라 cache miss가 적고 벡터화 가능성도 높다.
        }
    }
    return total;
}

long long sumMatrixColumnMajorAccess(const vector<int>& matrix, int rows, int cols) {
    long long total = 0;
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
            total += matrix[r * cols + c]; // 큰 stride 접근은 cache line을 충분히 쓰지 못해 느려질 수 있다.
        }
    }
    return total;
}

vector<int> buildWithoutReserve(int n) {
    vector<int> values;
    for (int i = 0; i < n; ++i) {
        values.push_back(i); // capacity가 부족할 때마다 재할당과 전체 복사가 발생할 수 있다.
    }
    return values;
}

vector<int> buildWithReserve(int n) {
    vector<int> values;
    values.reserve(n); // 최종 크기를 알면 한 번에 메모리를 확보해 재할당 비용을 줄인다.
    for (int i = 0; i < n; ++i) {
        values.push_back(i);
    }
    return values;
}

long long countPositiveBranchy(const vector<int>& values) {
    long long count = 0;
    for (int x : values) {
        if (x > 0) { // 데이터 분포가 랜덤이면 branch predictor가 자주 틀릴 수 있다.
            ++count;
        }
    }
    return count;
}

long long countPositiveBranchLight(const vector<int>& values) {
    long long count = 0;
    for (int x : values) {
        count += (x > 0); // compiler가 조건 이동 또는 setcc 형태로 바꿀 수 있어 분기 비용이 줄 수 있다.
    }
    return count;
}

void printAssemblyReadingHints() {
    cout << "[assembly hints]\n";
    cout << "mov/load/store: memory traffic\n";       // mov가 많으면 값 복사나 메모리 왕복이 많은지 의심한다.
    cout << "cmp/test + jcc: branch\n";               // cmp/test 뒤 조건 점프가 hot loop에 많으면 branch miss를 본다.
    cout << "call: function boundary\n";              // 작은 함수 call이 남아 있으면 inline 실패나 ABI 비용을 검토한다.
    cout << "add/lea pointer step: loop stride\n";    // 포인터 증가 폭이 cache-friendly한 연속 접근인지 확인한다.
    cout << "vmov/vadd: SIMD/vectorized loop\n";      // v로 시작하는 명령이 보이면 자동 벡터화 가능성을 점검한다.
}

template <typename Func>
long long elapsedMicroseconds(Func&& func) {
    auto start = chrono::steady_clock::now(); // wall-clock 대신 monotonic clock을 사용한다.
    volatile long long guard = func();        // 결과를 사용해 최적화로 루프가 사라지는 일을 줄인다.
    (void)guard;
    auto end = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::microseconds>(end - start).count();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "[layout] BadLayout=" << sizeof(BadLayout)
         << " BetterLayout=" << sizeof(BetterLayout) << '\n';

    vector<int> values = buildWithReserve(100'000);
    cout << "[sum contiguous] " << sumContiguous(values) << '\n';

    int rows = 256;
    int cols = 256;
    vector<int> matrix(rows * cols);
    iota(matrix.begin(), matrix.end(), 1); // 연속 메모리 위에 row-major 2차원 배열을 흉내 낸다.

    cout << "[row major sum] " << sumMatrixRowMajor(matrix, rows, cols) << '\n';
    cout << "[column access sum] " << sumMatrixColumnMajorAccess(matrix, rows, cols) << '\n';

    vector<int> mixed = {-3, 4, -1, 2, 0, 7, -8};
    cout << "[branchy] " << countPositiveBranchy(mixed) << '\n';
    cout << "[branch light] " << countPositiveBranchLight(mixed) << '\n';
    printAssemblyReadingHints();

    cout << "[benchmark hint] reserve_us="
         << elapsedMicroseconds([&] { return (long long)buildWithReserve(10'000).size(); }) << '\n';
    return 0;
}
