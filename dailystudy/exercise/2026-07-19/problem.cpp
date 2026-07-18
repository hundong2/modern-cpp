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
