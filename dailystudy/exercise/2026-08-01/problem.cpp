// <iostream>은 연습 결과를 출력할 std::cout과 << 연산자를 제공합니다.
#include <iostream>
// <string>은 문자를 소유하는 기본 문자열 타입 std::string을 제공합니다.
#include <string>
// <string_view>는 원본 문자열을 복사하지 않는 읽기 전용 뷰를 제공합니다.
#include <string_view>

// using 별칭으로 긴 표준 타입 이름을 학습 코드에서 간단히 표현합니다.
using Text = std::string;

// struct의 기본 접근은 public이라 입력과 결과를 단순하게 공개할 수 있습니다.
struct ParseResult {
    bool matched{}; // bool 기본 타입은 중괄호 값 초기화 시 false가 됩니다.
    Text value{};   // 멤버 변수 value가 결과 문자를 직접 소유합니다.
};

class PrefixParser { // class의 기본 접근은 private입니다.
public:
    // 생성자는 반환형이 없고, explicit은 Text 하나가 PrefixParser로 암시적 변환되는 것을 막습니다.
    explicit PrefixParser(Text prefix)
        // 생성자 매개변수 prefix를 멤버 초기화 목록에서 멤버 prefix_로 복사합니다.
        : prefix_{prefix} {}

    // 반환형 ParseResult, 매개변수 const 참조, 끝의 const는 객체 상태를 바꾸지 않는다는 계약입니다.
    [[nodiscard]] ParseResult parse(const Text& input) const {
        const std::string_view view{input};   // input lvalue의 문자 버퍼에 비소유 뷰를 바인딩합니다.
        const std::string_view prefix{prefix_}; // prefix_는 파서가 소유하여 뷰보다 오래 삽니다.
        // TODO 1: starts_with를 ends_with로 바꾸어 결과 차이를 예측하고 확인하세요.
        if (view.starts_with(prefix)) {
            // size() 표준 함수가 접두사의 문자 수를 반환하고 substr가 나머지 범위를 고릅니다.
            return ParseResult{true, Text{view.substr(prefix.size())}};
        }
        return ParseResult{false, {}}; // prvalue 반환 결과는 목적지에 직접 생성되어 복사가 생략될 수 있습니다.
    }

private:
    Text prefix_{}; // private 접근 지정자 영역의 멤버는 클래스 밖에서 직접 읽을 수 없습니다.
};

int main() { // 함수 main의 반환형 int는 성공 0 또는 실패 1을 운영체제에 전달합니다.
    const PrefixParser parser{Text{"say:"}}; // explicit 생성자의 올바른 직접 초기화 예입니다.
    const Text input{"say:hello"}; // const 변수는 초기화 뒤 다른 문자열을 저장할 수 없습니다.
    const ParseResult result{parser.parse(input)}; // 함수 호출 결과 prvalue로 결과 객체를 초기화합니다.

    // &는 lvalue 객체의 주소를 얻고 포인터는 소유권 없이 그 주소를 관찰합니다.
    const ParseResult* const pointer{&result};
    // *는 포인터가 가리키는 lvalue를 얻고, 참조는 그 기존 객체에 별명을 붙입니다.
    const ParseResult& reference{*pointer};
    if (pointer != nullptr && reference.matched) { // 비교와 단락 AND 뒤에 조건 분기가 일어납니다.
        std::cout << reference.value << '\n';
    }

    // TODO 2: 반복 횟수를 2에서 3으로 바꾸고 마지막 출력 숫자를 먼저 예측하세요.
    for (int i{0}; i < 2; ++i) { // int 변수, < 비교, ++ 증가 연산자가 반복을 제어합니다.
        std::cout << "practice " << i << '\n';
    }

    // 개념적으로 로드·비교·조건 분기·함수 호출·저장이 있지만 실제 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라집니다.
    return result.matched == true ? 0 : 1; // == 비교와 ?: 조건 연산자로 종료 코드를 고릅니다.
}
