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
    return a / gcd(a, b) * b;
}

long long addMod(long long a, long long b, long long mod) {
    if (a >= mod - b) return a - (mod - b);
    return a + b;
}

long long modMultiply(long long a, long long b, long long mod) {
    long long result = 0;
    a = (a % mod + mod) % mod;
    while (b > 0) {
        if (b & 1) result = addMod(result, a, mod);
        a = addMod(a, a, mod);
        b >>= 1;
    }
    return result;
}

long long modPow(long long base, long long exp, long long mod) {
    long long result = 1 % mod;
    base = (base % mod + mod) % mod;
    while (exp > 0) {
        if (exp & 1) result = modMultiply(result, base, mod);
        base = modMultiply(base, base, mod);
        exp >>= 1;
    }
    return result;
}

long long extendedGcd(long long a, long long b, long long& x, long long& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    long long x1, y1;
    long long g = extendedGcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

long long modInversePrime(long long a, long long mod) {
    return modPow(a, mod - 2, mod);
}

vector<int> sievePrimes(int n) {
    vector<bool> isPrime(n + 1, true);
    if (n >= 0) isPrime[0] = false;
    if (n >= 1) isPrime[1] = false;

    for (long long p = 2; p * p <= n; ++p) {
        if (!isPrime[p]) continue;
        for (long long x = p * p; x <= n; x += p) {
            isPrime[(int)x] = false;
        }
    }

    vector<int> primes;
    for (int i = 2; i <= n; ++i) {
        if (isPrime[i]) primes.push_back(i);
    }
    return primes;
}

class CombinationMod {
public:
    explicit CombinationMod(int maxN, long long mod = MOD) : mod(mod), fact(maxN + 1), invFact(maxN + 1) {
        fact[0] = 1;
        for (int i = 1; i <= maxN; ++i) fact[i] = fact[i - 1] * i % mod;
        invFact[maxN] = modInversePrime(fact[maxN], mod);
        for (int i = maxN; i >= 1; --i) invFact[i - 1] = invFact[i] * i % mod;
    }

    long long nCr(int n, int r) const {
        if (r < 0 || r > n) return 0;
        return fact[n] * invFact[r] % mod * invFact[n - r] % mod;
    }

private:
    long long mod;
    vector<long long> fact;
    vector<long long> invFact;
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
