#include <iostream>

class Animal { public: virtual ~Animal() {} }; // 가상 함수가 있어야 RTTI 작동
class Dog : public Animal { public: void bark() { std::cout << "멍멍\n"; } };
class Cat : public Animal { public: void meow() { std::cout << "야옹\n"; } };

void checkAnimalType(Animal* animal) {
    // animal 포인터가 실제로 Dog 타입인지 실행 중에 체크합니다.
    if (Dog* dog = dynamic_cast<Dog*>(animal)) {
        // 성공하면 nullptr이 아니므로 이 블록이 실행됩니다.
        dog->bark();
    } 
    else if (Cat* cat = dynamic_cast<Cat*>(animal)) {
        cat->meow();
    }
}
void printType(Animal* animal)

int main() {
    Dog dog;
    Cat cat;

    // 기반 클래스 포인터로 전달해도 dynamic_cast는 객체의 실제 타입을 확인합니다.
    checkAnimalType(&dog);
    checkAnimalType(&cat);

    return 0;
}
