#include <iostream> // 입출력을 위한 표준 라이브러리를 포함합니다.

// [1] C++17 이상: inline 변수
// 헤더에 포함되어 여러 cpp 파일에서 include 되더라도, 링커가 중복 정의 에러를 내지 않고 하나로 병합합니다.
struct SystemConfig {
    inline static int optimizationLevel = 3; 
};

// [2] inline 함수 정의
// 컴파일러에게 이 함수의 바디(body)를 호출부에 직접 삽입하라고 제안합니다.
// 덧셈 연산처럼 매우 짧은 함수에 적합합니다.
inline int calculate_sum(int a, int b) {
    return a + b; // 레지스터에 a와 b를 담아 더한 후 바로 반환합니다.
}

// [3] 강제 인라인 (컴파일러 종속적)
// 때로는 컴파일러의 판단을 무시하고 무조건 인라인화 하고 싶을 때가 있습니다.
#if defined(_MSC_VER)
    __forceinline int multiply(int a, int b) { return a * b; } // MSVC 용 강제 인라인
#elif defined(__GNUC__) || defined(__clang__)
    __attribute__((always_inline)) inline int multiply(int a, int b) { return a * b; } // GCC/Clang 용 강제 인라인
#endif

int main() {
    // 변수 초기화: 기초적인 복습 (int형 변수 두 개를 스택에 할당합니다)
    int x = 10; 
    int y = 20; 

    // [최적화 포인트]
    // 릴리스 모드(-O3)에서는 calculate_sum(10, 20)이 호출되지 않습니다.
    // 컴파일러는 이 코드를 단순히 'int sum_result = 30;' 으로 상수 폴딩(Constant Folding) 해버립니다.
    int sum_result = calculate_sum(x, y); 

    // 강제 인라인 함수 호출
    // 함수 호출 오버헤드 없이 x * y 코드가 여기에 직접 치환됩니다.
    int mul_result = multiply(x, y); 

    // 결과 출력
    std::cout << "최적화 레벨: " << SystemConfig::optimizationLevel << "\n";
    std::cout << "Sum: " << sum_result << "\n";
    std::cout << "Mul: " << mul_result << "\n";

    return 0; // 프로그램 정상 종료를 OS에 알립니다.
}

// == execute: /modern-cpp/자주까먹는/build/history/inline ==

// 최적화 레벨: 3
// Sum: 30
// Mul: 200