#include <iostream>
#include <string>
#include <string_view>

// 함수 선언(declaration). 정의는 아래에 있다.
int add(int left, int right) noexcept;

// 같은 이름이지만 매개변수 타입이 다르므로 overload다.
void describe(int value);
void describe(double value);
void describe(std::string_view value);

struct Meter {
    explicit Meter(int value) : value_(value) {}

    // 뒤의 const는 반환 타입이 아니라 이 함수를 호출하는 *this를 한정한다.
    [[nodiscard]] int value() const noexcept { return value_; }

    // & ref-qualifier: 이름 있는(lvalue) Meter에서만 호출할 수 있다.
    void reset() & noexcept { value_ = 0; }

    // && ref-qualifier: 임시/rvalue Meter에서만 호출할 수 있다.
    int take() && noexcept { return value_; }

private:
    int value_;
};

int add(int left, int right) noexcept {
    return left + right;
}

void describe(int value) {
    std::cout << "int: " << value << '\n';
}

void describe(double value) {
    std::cout << "double: " << value << '\n';
}

void describe(std::string_view value) {
    std::cout << "text: " << value << '\n';
}

int main() {
    const int answer = add(20, 22);
    std::cout << "answer = " << answer << '\n';

    describe(7);           // int overload
    describe(3.5);         // double overload
    describe("signature"); // string_view로 변환 가능한 overload

    Meter meter{10};
    std::cout << "meter = " << meter.value() << '\n';
    meter.reset();

    const int moved_value = Meter{99}.take();
    std::cout << "temporary = " << moved_value << '\n';

    // TRY_COMPILE_ERROR: 이름 있는 meter는 lvalue라서 && 함수 호출 불가.
    // meter.take();

    // TRY_COMPILE_ERROR: 임시 객체에는 & 함수 호출 불가.
    // Meter{1}.reset();
}
