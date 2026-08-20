// <iostream>은 연습 결과를 출력하는 std::cout을 선언한다.
#include <iostream>
// <string>은 보고서 본문을 소유하고 append로 이어 붙이는 std::string을 선언한다.
#include <string>
// <utility>는 std::move를 선언해 이동 가능한 xvalue를 표현한다.
#include <utility>

// class의 기본 접근은 private이다. 사용자는 public 연산만으로 유효한 보고서를 조립한다.
class ReportBuilder {
public:
    // 생성자는 반환형이 없고 explicit은 문자열에서 빌더로의 뜻밖의 암시 변환을 막는다.
    explicit ReportBuilder(std::string title) : body_{std::move(title)} {}

    // & 한정 오버로드는 이름 있는 빌더를 계속 재사용하도록 같은 객체의 lvalue 참조를 반환한다.
    ReportBuilder& line(const std::string& text) & {
        append_line(text); // const string&는 호출 동안 text를 빌릴 뿐 복사하거나 소유하지 않는다.
        return *this;
    }

    // && 한정 오버로드는 임시 빌더의 fluent chain을 유지한다.
    ReportBuilder&& line(const std::string& text) && {
        append_line(text);
        // std::move<T>(T&&)에서 T는 ReportBuilder&로 추론·붕괴하고 반환은 ReportBuilder&&다.
        // 실제 멤버 이동은 finish() &&에서 일어나며, 이 함수는 *this 상태와 text를 더 바꾸지 않는다.
        return std::move(*this);
    }

    // const &는 lvalue 빌더를 보존해야 하므로 body_를 복사한 새 string prvalue를 반환한다.
    [[nodiscard]] std::string finish() const & { return body_; }

    // &&는 수명이 끝날 builder에서 body_ 버퍼를 이동할 수 있게 xvalue를 반환한다.
    // 반환 string은 호출자의 목적 객체에 직접 초기화될 수 있고, 호출 뒤 body_ 값은 미지정이다.
    [[nodiscard]] std::string finish() && { return std::move(body_); }

private:
    void append_line(const std::string& text) {
        // string::append(const char*)는 "\n"의 첫 포인터를 입력받아 널 전 문자를 body_ 끝에 복사한다.
        // 반환 string&는 연쇄하지 않아 버린다. 성공하면 길이가 1 늘며 재할당 시 기존 관찰자가 무효화된다.
        // 시간은 추가 길이에 선형, 할당/bad_alloc 가능, 실패 시 body_에는 강한 예외 보장이 적용된다.
        body_.append("\n");
        // string::append(const string&)은 text const lvalue를 빌리고 문자를 body_가 복사 소유한다.
        // text와 그 수명은 유지되고 반환 string&는 버린다. 성공 후 길이는 text.size()만큼 늘어난다.
        body_.append(text);
    }

    std::string body_{}; // 빌더가 조립 중인 전체 텍스트를 독점 소유한다.
};

int main() {
    // string(const char*)는 정적 리터럴 "deploy"의 문자를 새 string prvalue가 복사 소유하고, 그 값을 title 매개변수로 이동한다.
    // 문자 수에 선형이며 할당 실패 시 bad_alloc이고 리터럴 자체는 바뀌지 않는다.
    ReportBuilder reusable{std::string{"deploy"}};
    // reusable은 lvalue라 line() &와 finish() const &가 선택되어 빌더 상태를 복사한다.
    // "validate"는 const char*에서 임시 string으로 변환되어 line의 const string&에 전체 식 동안 바인딩된다.
    // line은 그 문자를 body_에 복사하고 임시의 소유권을 얻지 않으며, 반환 ReportBuilder&는 finish 호출에 쓴다.
    const std::string first{reusable.line("validate").finish()};
    // 임시 ReportBuilder prvalue는 line() && 뒤 xvalue가 되고 finish() &&에서 본문 소유권을 넘긴다.
    // "backup"과 "upload"도 각 길이에 선형으로 소유 임시 string을 만들며 할당 실패 가능성이 있다.
    // line의 text 참조는 호출 동안만 빌리고, finish 반환 string prvalue가 second를 직접 초기화해 복사 생략될 수 있다.
    const std::string second{ReportBuilder{std::string{"backup"}}.line("upload").finish()};

    // operator<<는 first/second를 빌려 문자를 쓰며 두 string의 내용·소유권은 유지한다.
    std::cout << first << '\n' << second << '\n';

    // string::size()는 인자 없이 길이를 size_type 값으로 반환하고 수신 문자열을 바꾸지 않는 O(1) 관찰이다.
    return first.size() == 15U && second.size() == 13U ? 0 : 1;
}
