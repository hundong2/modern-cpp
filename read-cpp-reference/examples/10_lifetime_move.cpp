#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class Tracer {
public:
    explicit Tracer(std::string name) : name_(std::move(name)) {
        std::cout << "construct " << name_ << '\n';
    }

    Tracer(const Tracer& other) : name_(other.name_ + "-copy") {
        std::cout << "copy " << other.name_ << '\n';
    }

    Tracer(Tracer&& other) noexcept : name_(std::move(other.name_)) {
        std::cout << "move " << name_ << '\n';
    }

    ~Tracer() {
        std::cout << "destroy " << name_ << '\n';
    }

private:
    std::string name_;
};

Tracer make_tracer() {
    // C++17에서는 반환 객체에 직접 구성되는 guaranteed copy elision이 적용될 수 있는 형태.
    return Tracer{"result"};
}

std::string_view safe_view() {
    return "literal lives for the whole program";
}

// 이렇게 쓰면 안 된다.
// std::string_view dangling_view() {
//     std::string local = "dies at return";
//     return local;
// }

int main() {
    Tracer result = make_tracer();

    std::vector<std::string> words;
    words.push_back("first");

    // vector 수정 전의 reference를 저장해 두고 재할당 뒤 사용하면 dangling일 수 있다.
    const auto capacity_before = words.capacity();
    words.push_back("second");
    std::cout << "reallocated? " << std::boolalpha
              << (words.capacity() != capacity_before) << '\n';

    std::cout << safe_view() << '\n';

    std::string source = "move me";
    std::string destination = std::move(source);
    std::cout << "destination = " << destination << '\n';
    // moved-from string은 유효하지만 값은 unspecified. 비었음을 가정하지 않는다.
    std::cout << "source is still valid; size = " << source.size() << '\n';

    (void)result; // 교육용 객체가 사용됐음을 명시
}
