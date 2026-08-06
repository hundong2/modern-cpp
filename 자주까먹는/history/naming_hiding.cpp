#include <iostream>
#include <variant>

// 1. 컴파일러가 람다 1을 보고 몰래 생성한 첫 번째 구조체 (부모 1)
struct __CompilerGenerated_Lambda1 {
    // int를 받는 함수 호출 연산자
    inline void operator()(int i) const { 
        std::cout << "[Lambda1] 정수 처리: " << i << '\n'; 
    }
};

// 2. 컴파일러가 람다 2를 보고 몰래 생성한 두 번째 구조체 (부모 2)
struct __CompilerGenerated_Lambda2 {
    // double을 받는 함수 호출 연산자
    inline void operator()(double d) const { 
        std::cout << "[Lambda2] 실수 처리: " << d << '\n'; 
    }
};

// 3. 우리가 사용했던 overloaded 구조체의 실제 내부 모습 (자식)
// 부모 1과 부모 2를 다중 상속 받습니다.
struct MyOverloadedChild : __CompilerGenerated_Lambda1, __CompilerGenerated_Lambda2 {
    
    // [핵심] 만약 아래 두 줄이 없다면, 다중 상속의 모호성 때문에 에러가 납니다.
    // using을 통해 부모들의 operator()를 내(자식) 스코프로 끌어와 오버로딩을 완성합니다.
    using __CompilerGenerated_Lambda1::operator();
    using __CompilerGenerated_Lambda2::operator();
};

int main() {
    // 4. 자식 객체 생성
    MyOverloadedChild visitor;

    // 5. 부모 이름(Lambda1, 2)은 전혀 쓸 필요가 없습니다!
    // 오직 자식 객체 이름(visitor)만으로 호출하면, 
    // 컴파일러가 인자의 타입(int, double)을 보고 알맞은 부모의 함수를 찰떡같이 찾아갑니다.
    visitor(100);   // -> __CompilerGenerated_Lambda1::operator()(100) 호출됨
    visitor(3.14);  // -> __CompilerGenerated_Lambda2::operator()(3.14) 호출됨

    return 0;
}