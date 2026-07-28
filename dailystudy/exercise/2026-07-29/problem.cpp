// <iostream>은 연습 결과를 화면에 출력하는 std::cout을 제공한다.
#include <iostream>
// <source_location>은 report 호출 위치를 자동 기록하는 C++20 타입을 제공한다.
#include <source_location>
// <string>은 메시지를 담는 std::string 기본 문자열 타입을 제공한다.
#include <string>
// <utility>는 생성자 매개변수 문자열을 멤버로 이동하는 std::move를 제공한다.
#include <utility>

// class의 기본 접근은 private지만, 아래 public: 뒤의 함수는 외부에서 호출할 수 있다.
class PracticeReporter {
public:
    // 생성자는 반환형이 없고, explicit은 const char*가 객체로 암시 변환되는 실수를 막는다.
    explicit PracticeReporter(std::string name) : name_{std::move(name)} {} // 멤버 초기화 목록으로 멤버 변수를 직접 초기화한다.

    // const 멤버 함수는 name_을 바꾸지 않는다; where의 기본값은 호출 위치에서 만들어지는 prvalue다.
    void report(const std::string& message, std::source_location where = std::source_location::current()) const {
        // TODO: name_, message, where.line()을 << 연산자로 출력한다. const string&는 호출 중인 lvalue 문자열을 빌린다.
        std::cout << name_ << ": " << message << " line=" << where.line() << '\n';
    }

private:
    std::string name_{}; // private 멤버 변수는 Reporter만 바꿀 수 있다.
};

// 반환형 bool은 성공 여부를 호출자에게 준다; 매개변수 int count는 값 복사라 독립적으로 읽을 수 있다.
bool validate_count(int count, const PracticeReporter& reporter) {
    // <= 비교 연산자 결과에 따라 조건 분기한다; 기계 수준의 정확한 명령은 CPU/ABI/최적화에 따라 달라진다.
    if (count <= 0) {
        reporter.report("count는 양수여야 합니다"); // const 참조는 reporter lvalue를 복사 없이 빌린다.
        return false;
    }
    return true;
}

int main() {
    // 직접 초기화 PracticeReporter{"연습"}는 explicit 생성자에서 허용된다.
    const PracticeReporter reporter{"연습"};
    // 반복문은 int 기본 타입 i를 0부터 증가시키며 검증 함수를 호출한다.
    for (int i{0}; i < 2; ++i) {
        if (validate_count(i, reporter)) { std::cout << "통과\n"; }
    }
    return 0;
}
