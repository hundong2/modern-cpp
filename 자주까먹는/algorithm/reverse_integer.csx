using System;

public class Solution {
    public int Reverse(int x) {
        int rev = 0;

        while (x != 0) {
            // 1의 자리 추출. C#에서도 x가 음수면 pop도 음수입니다. (-123 % 10 = -3)
            int pop = x % 10;
            x /= 10;

            // 64비트 정수(long)로 캐스팅하지 않고 순수 32비트 내에서 한계치 검사
            // Int32.MaxValue = 2147483647
            if (rev > Int32.MaxValue / 10 || (rev == Int32.MaxValue / 10 && pop > 7)) return 0;
            
            // Int32.MinValue = -2147483648
            if (rev < Int32.MinValue / 10 || (rev == Int32.MinValue / 10 && pop < -8)) return 0;

            // 스택에 Push 하듯 숫자를 뒤에 붙임
            rev = rev * 10 + pop;
        }

        return rev;
    }
}

// [실행 예제]
// var sol = new Solution();
// Console.WriteLine(sol.Reverse(-123)); // -321