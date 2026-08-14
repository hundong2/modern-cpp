#include <iostream>
#include <climits> // INT_MAX, INT_MIN을 가져오기 위한 표준 라이브러리

using namespace std;

class Solution {
public:
    int reverse(int x) {
        int rev = 0; // 뒤집힌 숫자를 저장할 변수

        // x가 0이 될 때까지 (음수든 양수든) 계속 반복합니다.
        while (x != 0) {
            int pop = x % 10; // 1의 자리 숫자를 추출 (음수면 pop도 음수가 됨)
            x /= 10;          // 원본 숫자에서 1의 자리 숫자를 제거

            // [오버플로우 방어 로직] 64비트(long)를 쓰지 않고 사전 차단!
            // 1. rev가 INT_MAX/10 보다 크거나, 같으면서 pop이 7보다 크면 폭발
            if (rev > INT_MAX / 10 || (rev == INT_MAX / 10 && pop > 7)) {
                return 0; 
            }
            // 2. 음수 방향 언더플로우 방어. INT_MIN의 끝자리는 8이므로 pop이 -8보다 작으면 폭발
            if (rev < INT_MIN / 10 || (rev == INT_MIN / 10 && pop < -8)) {
                return 0;
            }

            // 안전함이 증명되었으므로 기존 값의 자릿수를 하나 올리고(*10) pop을 더함
            rev = rev * 10 + pop;
        }

        return rev;
    }
};

// [실행 예제]
// int main() {
//     Solution sol;
//     cout << sol.reverse(123) << endl;  // 321
//     cout << sol.reverse(-123) << endl; // -321
//     cout << sol.reverse(1534236469) << endl; // 0 (오버플로우 처리)
//     return 0;
// }