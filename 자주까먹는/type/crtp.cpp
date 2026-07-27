#include <iostream>

// 1. 부모 클래스는 템플릿으로 만들어집니다. (Derived: 자식 타입)
template <typename Derived>
class Base {
public:
    void interface() {
        // [핵심] 부모 클래스는 템플릿 인자를 통해 자식이 누군지 '컴파일 타임'에 이미 알고 있습니다.
        // 따라서 vtable을 뒤질 필요 없이, 자신의 포인터(this)를 자식 타입으로 즉시 캐스팅합니다.
        static_cast<Derived*>(this)->implementation();
    }
    
    // 기본 구현체 (자식에게 구현이 없을 경우를 대비한 Fallback)
    void implementation() {
        std::cout << "Base default implementation\n";
    }
};

// 2. 자식 클래스는 상속을 받을 때 '자기 자신(Derived1)'을 부모의 템플릿 인자로 꽂아 넣습니다.
class Derived1 : public Base<Derived1> {
public:
    // virtual 키워드 없이 부모의 함수를 덮어씁니다.
    void implementation() {
        std::cout << "Derived1 고유의 빠른 실행!\n";
    }
};

class Derived2 : public Base<Derived2> {
    // implementation()을 구현하지 않으면 부모의 기본 구현이 호출됩니다.
};

int main() {
    Derived1 d1;
    Derived2 d2;

    // 부모 클래스의 interface()를 호출하면, 자식 타입에 맞는 구현체가 호출됩니다.
    d1.interface(); // Derived1 고유의 빠른 실행!
    d2.interface(); // Base default implementation

    return 0;
}

// 실행 결과:
// Derived1 고유의 빠른 실행!
// Base default implementation