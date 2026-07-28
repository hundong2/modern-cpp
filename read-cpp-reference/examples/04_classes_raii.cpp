#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Resource {
public:
    explicit Resource(std::string name) : name_(std::move(name)) {
        std::cout << "acquire " << name_ << '\n';
    }

    ~Resource() {
        std::cout << "release " << name_ << '\n';
    }

    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    Resource(Resource&&) noexcept = default;
    Resource& operator=(Resource&&) noexcept = default;

    const std::string& name() const noexcept { return name_; }

private:
    std::string name_;
};

struct Shape {
    virtual ~Shape() = default; // base pointer로 안전하게 삭제하기 위해 virtual
    virtual std::string name() const = 0;
};

struct Circle final : Shape {
    std::string name() const override { return "circle"; }
};

int main() {
    {
        Resource file{"lesson.txt"};
        std::cout << "using " << file.name() << '\n';
    } // scope 끝에서 소멸자가 자동으로 자원을 정리한다.

    auto owned = std::make_unique<Resource>("unique");
    auto moved = std::move(owned); // 단독 소유권 이전
    std::cout << "owned is empty? " << std::boolalpha
              << (owned == nullptr) << '\n';
    std::cout << "moved owns " << moved->name() << '\n';

    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>());
    for (const auto& shape : shapes) {
        std::cout << shape->name() << '\n'; // virtual dispatch
    }

    // TRY_COMPILE_ERROR: unique_ptr와 Resource의 복사는 금지되어 있다.
    // auto copied = moved;
}
