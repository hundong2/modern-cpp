#include <iostream>
#include <variant>
#include <vector>
#include <numeric>
#include <utility>

// ==========================================
// [C++11] final 키워드와 가상화 해제 (Devirtualization)
// ==========================================
class MathBase {
public:
    virtual int compute(int x) const = 0;
};

// [최적화 포인트] final을 붙이면 링커(LTO)가 "더 이상 상속받는 클래스가 없다"고 확신합니다.
// vtable(가상 함수 테이블)을 조회하는 런타임 오버헤드를 삭제하고 코드를 인라인화합니다.
class FastMath final : public MathBase {
public:
    int compute(int x) const override { return x * 2; }
};

// ==========================================
// [C++17] std::variant (정적 다형성)
// ==========================================
// 가상 함수 자체가 필요 없는 LTO 최고의 파트너입니다.
// 메모리 할당 없이 스택에만 존재하며, 컴파일러가 모든 타입을 100% 추론 가능합니다.
using ValueType = std::variant<int, double>;

// ==========================================
// [C++20] Concepts (컴파일 타임 제약)
// ==========================================
// 템플릿에 들어올 수 있는 타입을 제한하여, 링커가 최적화할 코드의 범위를 좁혀줍니다.
template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

auto process_variant_data(Numeric auto val) {
    return val * 10;
}

// ==========================================
// [C++23] std::unreachable (분기 예측 최적화)
// ==========================================
int optimize_branch(int type) {
    if (type == 1) return 100;
    if (type == 2) return 200;
    
    // [최적화 포인트] LTO 링커에게 "이 밑으로는 절대 실행 안 됨"을 알립니다.
    // 어셈블리 단에서 불필요한 jmp(점프) 명령어나 예외 처리 코드를 완전히 삭제합니다.
    std::unreachable(); 
}

int main() {
    // 1. C++11 Devirtualization 테스트
    FastMath engine;
    // 릴리스 모드+LTO 환경에서는 함수 호출 없이 컴파일 타임에 '20'으로 상수 폴딩(Constant Folding) 됩니다.
    int result = engine.compute(10); 
    std::cout << "C++11 final 결과: " << result << "\n";

    // 2. C++17 variant와 C++20 Concepts 테스트
    ValueType data = 5;
    
    // std::visit는 원래 복잡한 함수 포인터 배열을 쓰지만, 
    // LTO는 이를 완전히 해체하여 단순한 'if(type == int) process(5);' 형태의 어셈블리로 평탄화(Flattening) 합니다.
    std::visit([](auto&& arg) {
        std::cout << "C++17/20 처리 결과: " << process_variant_data(arg) << "\n";
    }, data);

    // 3. C++23 Branch 최적화
    std::cout << "C++23 Branch 결과: " << optimize_branch(1) << "\n";

    return 0; // OS에 정상 종료 알림
}
