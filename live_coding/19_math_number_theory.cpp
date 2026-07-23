/*
Chapter 19. 정수론, 모듈러 연산, 조합

문제 1: 최대공약수와 최소공배수를 구하라.
접근: Euclidean algorithm. C++17에는 std::gcd가 있다.

문제 2: a^e mod MOD를 빠르게 구하라.
접근: binary exponentiation. 지수를 반씩 줄인다.
복잡도: O(log e)

문제 3: 1..N의 소수를 모두 구하라.
접근: Eratosthenes sieve.
복잡도: O(N log log N)

문제 4: MOD가 소수일 때 nCr mod MOD를 여러 번 구하라.
접근: factorial과 inverse factorial 전처리.
주의: MOD가 소수가 아니면 Fermat inverse를 그대로 쓰면 안 된다.
*/

#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

const long long MOD = 1'000'000'007LL;

long long lcmSafe(long long a, long long b) {
    return a / gcd(a, b) * b; // <numeric>의 std::gcd는 C++17 표준 최대공약수 함수다.
}

long long addMod(long long a, long long b, long long mod) {
    if (a >= mod - b) return a - (mod - b); // a+b 대신 비교와 뺄셈으로 overflow를 피한다.
    return a + b;
}

long long modMultiply(long long a, long long b, long long mod) {
    long long result = 0;
    a = (a % mod + mod) % mod;
    while (b > 0) {
        if (b & 1) result = addMod(result, a, mod); // b의 현재 비트가 1이면 해당 배수를 더한다.
        a = addMod(a, a, mod);                      // 다음 비트로 넘어가며 a를 두 배로 만든다.
        b >>= 1;
    }
    return result;
}

long long modPow(long long base, long long exp, long long mod) {
    long long result = 1 % mod;
    base = (base % mod + mod) % mod;
    while (exp > 0) {
        if (exp & 1) result = modMultiply(result, base, mod); // 지수의 현재 비트가 1이면 결과에 곱한다.
        base = modMultiply(base, base, mod);                  // 다음 비트를 위해 밑을 제곱한다.
        exp >>= 1;
    }
    return result;
}

long long extendedGcd(long long a, long long b, long long& x, long long& y) {
    if (b == 0) {
        x = 1; // a*1 + 0*0 = a
        y = 0;
        return a;
    }
    long long x1, y1;
    long long g = extendedGcd(b, a % b, x1, y1);
    x = y1;                    // 재귀 결과를 한 단계 위 식으로 변환한다.
    y = x1 - (a / b) * y1;
    return g;
}

long long modInversePrime(long long a, long long mod) {
    return modPow(a, mod - 2, mod); // Fermat: a^(mod-1)=1 이므로 역원은 a^(mod-2)다.
}

vector<int> sievePrimes(int n) {
    vector<bool> isPrime(n + 1, true); // vector<bool>은 bitset처럼 압축되는 특수화라 메모리는 아끼지만 참조 동작이 일반 vector와 다르다.
    if (n >= 0) isPrime[0] = false;
    if (n >= 1) isPrime[1] = false;

    for (long long p = 2; p * p <= n; ++p) {
        if (!isPrime[p]) continue;
        for (long long x = p * p; x <= n; x += p) {
            isPrime[(int)x] = false; // p보다 작은 배수는 더 작은 소수 단계에서 이미 지워졌다.
        }
    }

    vector<int> primes;
    for (int i = 2; i <= n; ++i) {
        if (isPrime[i]) primes.push_back(i); // 남아 있는 수가 소수다.
    }
    return primes;
}

class CombinationMod {
public:
    explicit CombinationMod(int maxN, long long mod = MOD) : mod(mod), fact(maxN + 1), invFact(maxN + 1) {
        fact[0] = 1;
        for (int i = 1; i <= maxN; ++i) fact[i] = fact[i - 1] * i % mod;
        invFact[maxN] = modInversePrime(fact[maxN], mod); // MOD가 소수일 때 Fermat inverse를 사용할 수 있다.
        for (int i = maxN; i >= 1; --i) invFact[i - 1] = invFact[i] * i % mod;
    }

    long long nCr(int n, int r) const {
        if (r < 0 || r > n) return 0;
        return fact[n] * invFact[r] % mod * invFact[n - r] % mod; // n! / (r!(n-r)!)를 모듈러 곱으로 계산한다.
    }

private:
    long long mod;
    vector<long long> fact;    // factorial 값을 vector에 전처리해 조합 질의를 O(1)에 답한다.
    vector<long long> invFact; // inverse factorial도 같은 인덱스 체계로 맞춰 둔다.
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "[gcd lcm] " << gcd(48, 18) << ' ' << lcmSafe(48, 18) << '\n';
    cout << "[mod pow] " << modPow(2, 10, MOD) << '\n';

    long long x, y;
    long long g = extendedGcd(30, 12, x, y);
    cout << "[extended gcd] g=" << g << " x=" << x << " y=" << y << '\n';

    auto primes = sievePrimes(30);
    cout << "[primes]";
    for (int p : primes) cout << ' ' << p;
    cout << '\n';

    CombinationMod comb(100);
    cout << "[10C3] " << comb.nCr(10, 3) << '\n';
    return 0;
}
