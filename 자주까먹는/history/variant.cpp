#include <iostream>
#include <variant>

// ==========================================================================
// [역사적 배경: 다형성의 진화]
// - C++98/03: virtual 함수와 상속 사용 -> 객체가 힙(Heap)에 흩어져 Cache Miss 폭발.
// - C++11: 템플릿 메타 프로그래밍 도입. 가변 인자 템플릿(...) 탄생. 하지만 constexpr 제약이 심했음.
// - C++14: constexpr 함수 내부에서 지역 변수 선언과 제어문(if)이 허용되며 혁명이 시작됨.
// - C++17: std::variant와 std::visit 표준화. constexpr 배열로 O(1) 점프 테이블 구현. (현재 코드의 기반)
// - C++20: Concept 도입 및 컴파일러의 CTAD(추론 가이드) 자동화로 코드가 극도로 짧아짐.
// - C++23: Deducing this (this auto&&) 도입으로 람다 최적화가 정점에 달함.
// ==========================================================================

// [C++17 관용구] 여러 람다를 다중 상속받아 하나의 오버로딩 셋으로 병합
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// [C++17] CTAD(추론 가이드). C++20부터는 컴파일러가 알아서 해주므로 생략 가능합니다.
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// 테스트용 데이터 타입 (0: int, 1: double)
using MyData = std::variant<int, double>;

// --------------------------------------------------------------------------
// [STL의 진짜 흑마법] 템플릿 트램펄린 함수
// 컴파일 에러를 해결하기 위해 지역 변수 람다 대신, 컴파일 타임에 100% 확정되는 템플릿 함수를 사용합니다.
// I는 variant의 index(0 또는 1)를 의미합니다.
// --------------------------------------------------------------------------
template<size_t I, typename Visitor>
constexpr void Trampoline(Visitor& vis, const MyData& data) {
    // std::get<I>를 통해 variant 안의 데이터를 정확한 타입으로 꺼내어 방문자(람다)에게 전달합니다.
    vis(std::get<I>(data)); 
}

// --------------------------------------------------------------------------
// [핵심] std::visit 내부 구현의 완벽한 재현
// --------------------------------------------------------------------------
template<typename Visitor>
void Internal_Visit_Simulation(Visitor&& vis, const MyData& var) {
    // 함수 포인터 타입 정의 (반환값은 void, 인자는 Visitor와 MyData)
    using TrampolinePtr = void(*)(Visitor&, const MyData&);

    // [하드웨어 관점: .rodata 영역 할당]
    // static constexpr은 프로그램 실행 전(Compile-time)에 바이너리의 읽기 전용 데이터 영역에 구워집니다.
    // 이전 코드의 에러는 여기에 런타임 변수(람다)를 넣으려 해서 발생했습니다.
    // 템플릿 함수(&Trampoline<0, Visitor>)는 컴파일 타임 상수로 완벽히 인정됩니다!
    static constexpr TrampolinePtr DispatchTable[] = {
        &Trampoline<0, Visitor>, // index 0 (int) 일 때 뛸 메모리 주소
        &Trampoline<1, Visitor>  // index 1 (double) 일 때 뛸 메모리 주소
    };

    // [하드웨어 관점: O(1) Cache-Friendly Jump]
    // CPU는 var.index()를 읽자마자 테이블에서 다이렉트로 주소를 찾아 점프합니다.
    // vtable처럼 객체마다 포인터를 쫓아갈 필요가 없어 명령어 캐시(I-Cache) 효율이 극대화됩니다.
    DispatchTable[var.index()](vis, var);
}

int main() {
    // 스택(Stack) 메모리에 데이터 할당 (힙 할당 없음 -> 캐시 친화적)
    MyData data1 = 42;    // 내부 index: 0
    MyData data2 = 3.14;  // 내부 index: 1

    // 캡처 없는 람다들을 묶음 (EBCO 최적화로 인해 이 객체의 크기는 1바이트)
    auto my_visitor = overloaded {
        [](int i) { std::cout << "[Integer 처리] " << i << '\n'; },
        [](double d) { std::cout << "[Double 처리] " << d << '\n'; }
    };

    std::cout << "--- 완벽히 수정된 std::visit 시뮬레이션 ---\n";
    Internal_Visit_Simulation(my_visitor, data1); // 배열[0] 점프 -> 정수 처리
    Internal_Visit_Simulation(my_visitor, data2); // 배열[1] 점프 -> 실수 처리

    return 0;
}

// == execute: /modern-cpp/자주까먹는/build/history/variant ==

// --- 완벽히 수정된 std::visit 시뮬레이션 ---
// [Integer 처리] 42
// [Double 처리] 3.14