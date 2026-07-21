#include <cassert>   // assert로 포인터 상태와 렌더링 결과를 검증한다.
#include <iostream>  // cout으로 포맷 결과를 출력한다.
#include <memory>    // unique_ptr와 make_unique로 단독 소유권을 표현한다.
#include <string>    // 포맷 전후 문자의 메모리와 수명을 소유한다.
#include <utility>   // std::move로 unique_ptr 소유권을 이전한다.

class Formatter {
public:
    // 순수 가상 함수는 파생 클래스의 구현 계약이고 const는 객체를 바꾸지 않음을 뜻한다.
    virtual ~Formatter() = default;
    // 반환 string은 호출자가 소유하고, const 참조 매개변수는 입력 복사를 피한다.
    [[nodiscard]] virtual std::string format(const std::string& text) const = 0;
};

class BracketFormatter final : public Formatter {
public:
    // final은 추가 상속을 막고 override는 함수 서명 실수를 컴파일 때 찾는다.
    [[nodiscard]] std::string format(const std::string& text) const override {
        // 연결 과정에서 임시 string(prvalue)이 만들어진다. 반환 객체로 이동되거나 복사 생략될 수 있다.
        return "[" + text + "]";
    }
};

class Printer {
public:
    // 생성자는 반환형이 없다. explicit은 unique_ptr에서 Printer로의 암시 변환을 막는다.
    // 멤버 초기화 목록은 std::move로 단독 소유권을 formatter_에 전달한다.
    explicit Printer(std::unique_ptr<Formatter> formatter)
        : formatter_(std::move(formatter)) { // lvalue formatter를 xvalue로 바꿔 소유권을 멤버로 이동한다.
        assert(formatter_ != nullptr);
        // nullptr은 어떤 객체도 가리키지 않는 포인터 상태다.
    }

    [[nodiscard]] std::string render(const std::string& text) const {
        // formatter_가 가리키는 실제 타입에 따라 format을 고르는 가상 간접 호출이 일반적이다.
        return formatter_->format(text);
    }

private:
    // private 멤버는 외부에서 직접 접근할 수 없고 Printer가 그 수명을 관리한다.
    std::unique_ptr<Formatter> formatter_;
};

int main() {
    // main은 프로그램 진입점이며 끝까지 도달하면 0으로 정상 종료한다.
    auto formatter = std::make_unique<BracketFormatter>(); // 힙 객체 주소를 소유하는 unique_ptr 지역 변수(lvalue).
    Printer printer{std::move(formatter)};
    // 이동 뒤 formatter는 유효하지만 비어 있고, Printer만 힙 객체를 소유한다.

    assert(formatter == nullptr);
    assert(printer.render("learn ownership") == "[learn ownership]");
    std::cout << printer.render("learn ownership") << '\n';
    std::cout << "[TESTS] ownership syntax problem passed\n";
}
