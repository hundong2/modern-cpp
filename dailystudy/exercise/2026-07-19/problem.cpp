#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

class Formatter {
public:
    virtual ~Formatter() = default;
    [[nodiscard]] virtual std::string format(const std::string& text) const = 0;
};

class BracketFormatter final : public Formatter {
public:
    [[nodiscard]] std::string format(const std::string& text) const override {
        // 연결 과정에서 임시 string(prvalue)이 만들어진다. 반환 객체로 이동되거나 복사 생략될 수 있다.
        return "[" + text + "]";
    }
};

class Printer {
public:
    explicit Printer(std::unique_ptr<Formatter> formatter)
        : formatter_(std::move(formatter)) { // lvalue formatter를 xvalue로 바꿔 소유권을 멤버로 이동한다.
        assert(formatter_ != nullptr);
    }

    [[nodiscard]] std::string render(const std::string& text) const {
        // formatter_가 가리키는 실제 타입에 따라 format을 고르는 가상 간접 호출이 일반적이다.
        return formatter_->format(text);
    }

private:
    std::unique_ptr<Formatter> formatter_;
};

int main() {
    auto formatter = std::make_unique<BracketFormatter>(); // 힙 객체 주소를 소유하는 unique_ptr 지역 변수(lvalue).
    Printer printer{std::move(formatter)};

    assert(formatter == nullptr);
    assert(printer.render("learn ownership") == "[learn ownership]");
    std::cout << printer.render("learn ownership") << '\n';
    std::cout << "[TESTS] ownership syntax problem passed\n";
}
