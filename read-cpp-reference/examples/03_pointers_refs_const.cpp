#include <iostream>
#include <utility>

void increment(int& value) noexcept {
    ++value; // 수정 가능한 lvalue 참조
}

void print(const int& value) {
    std::cout << value << '\n'; // 복사 없이 읽기
}

struct Qualified {
    int value{0};

    int read() const noexcept {
        return value;
    }

    // 교육용 overload. volatile은 스레드 동기화 수단이 아니다.
    int read() const volatile noexcept {
        return value;
    }
};

int main() {
    int first = 10;
    int second = 20;

    const int* pointer_to_const = &first;
    pointer_to_const = &second;       // 포인터가 가리키는 대상은 변경 가능
    // *pointer_to_const = 30;        // TRY_COMPILE_ERROR: 대상은 const로 보임

    int* const const_pointer = &first;
    *const_pointer = 30;              // 대상 값은 수정 가능
    // const_pointer = &second;       // TRY_COMPILE_ERROR: 포인터 자체가 const

    const int* const both_const = &second;
    std::cout << "*both_const = " << *both_const << '\n';

    increment(first);
    print(first);
    print(42); // const lvalue 참조는 임시 값에도 바인딩 가능

    const Qualified normal{1};
    const volatile Qualified special{2};
    std::cout << "normal = " << normal.read() << '\n';
    std::cout << "special = " << special.read() << '\n';

    int&& rvalue_reference = 7;
    // 변수 이름 표현식은 선언 타입이 int&&여도 lvalue다.
    int& lvalue_reference = rvalue_reference;
    lvalue_reference = 8;
    std::cout << "rvalue_reference = " << rvalue_reference << '\n';

    // std::move는 이동 자체가 아니라 xvalue로 바꾸는 cast다.
    int&& another = std::move(rvalue_reference);
    std::cout << "another = " << another << '\n';
}
