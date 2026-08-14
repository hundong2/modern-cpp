#include <iostream>
#include <string>
#include <climits> // INT_MAX, INT_MIN 상수 사용
#include <cctype>  // isdigit 함수 사용 (문자가 숫자인지 판별)

using namespace std;

class Solution {
public:
    int myAtoi(string s) {
        int i = 0;              // 문자열을 순회할 인덱스 포인터
        int n = s.length();     // 문자열의 총 길이
        int sign = 1;           // 부호를 저장할 변수 (기본값 양수)
        int result = 0;         // 최종 숫자를 누적할 변수

        // [State 1] 선행 공백 무시
        // i가 문자열 길이 내에 있고, 현재 문자가 공백(' ')인 동안 계속 전진
        while (i < n && s[i] == ' ') {
            i++;
        }

        // 공백만 있다가 문자열이 끝났다면 0 반환
        if (i == n) return 0;

        // [State 2] 부호 판별
        // '+' 또는 '-' 인지 확인
        if (s[i] == '+' || s[i] == '-') {
            // '-' 이면 sign을 -1로, 아니면 1로 설정
            sign = (s[i] == '-') ? -1 : 1;
            i++; // 부호를 읽었으니 다음 칸으로 이동
        }

        // [State 3] 숫자 읽기 및 누적
        // isdigit(s[i])는 현재 문자가 '0'~'9' 사이인지 검사해주는 표준 함수입니다.
        while (i < n && isdigit(s[i])) {
            // 문자를 실제 정수값으로 변환 ('4' - '0' = 4)
            int digit = s[i] - '0';

            // [오버플로우 / 언더플로우 방어 (Clamping)]
            // result에 10을 곱하기 전에 미리 한계치를 넘을지 검사합니다.
            // INT_MAX = 2147483647 이므로, 10을 나누면 214748364 가 됩니다.
            if (result > INT_MAX / 10 || (result == INT_MAX / 10 && digit > 7)) {
                // 부호에 따라 최댓값(INT_MAX) 또는 최솟값(INT_MIN)으로 고정(Clamp)하여 반환
                return sign == 1 ? INT_MAX : INT_MIN;
            }

            // 안전함이 보장되었으므로 자릿수를 올리고 현재 숫자를 더함
            result = result * 10 + digit;
            i++;
        }

        // [State 4] 최종 결과 반환 (숫자 * 부호)
        return result * sign;
    }
};

// [실행 예제]
// int main() {
//     Solution sol;
//     cout << sol.myAtoi("   -042abc") << endl; // 출력: -42
//     cout << sol.myAtoi("2147483648") << endl;  // 출력: 2147483647 (오버플로우 Clamping)
//     return 0;
// }