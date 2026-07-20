/*
[기초 문법부터 읽는 순서]
1. Formatter는 format 함수의 모양만 정하는 추상 인터페이스입니다.
2. const std::string&는 문자열을 복사하지 않고 읽으며, 함수 뒤 const는 멤버를 바꾸지 않습니다.
3. BracketFormatter의 override는 기반 함수와 선언이 정확히 일치하는지 검사합니다.
4. `"[" + text + "]"`는 문자열을 이어 붙여 새 std::string을 반환합니다.
5. Printer 생성자의 explicit는 원치 않는 암시적 변환을 막습니다.
6. 멤버 초기화 목록 `: formatter_(...)`는 함수 본문 전에 멤버를 직접 초기화합니다.
7. unique_ptr는 복사할 수 없으므로 std::move로 소유권을 Printer에 이전합니다.
8. 이동 후 formatter가 nullptr인지, render 결과가 맞는지 assert로 검증합니다.
*/

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
        return "[" + text + "]";
    }
};

class Printer {
public:
    explicit Printer(std::unique_ptr<Formatter> formatter)
        : formatter_(std::move(formatter)) {
        assert(formatter_ != nullptr);
    }

    [[nodiscard]] std::string render(const std::string& text) const {
        return formatter_->format(text);
    }

private:
    std::unique_ptr<Formatter> formatter_;
};

int main() {
    auto formatter = std::make_unique<BracketFormatter>();
    Printer printer{std::move(formatter)};

    assert(formatter == nullptr);
    assert(printer.render("learn ownership") == "[learn ownership]");
    std::cout << printer.render("learn ownership") << '\n';
    std::cout << "[TESTS] ownership syntax problem passed\n";
}
