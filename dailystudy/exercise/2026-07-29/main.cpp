// <iostream>은 콘솔 출력 객체 std::cout과 스트림 삽입 연산자 <<를 제공한다.
#include <iostream>
// <memory>는 단독 소유권을 나타내는 std::unique_ptr와 std::make_unique를 제공한다.
#include <memory>
// <source_location>은 호출한 파일·줄·함수 정보를 담는 C++20 타입을 제공한다.
#include <source_location>
// <string>은 문자 데이터를 소유하는 std::string 타입을 제공한다.
#include <string>
// <utility>는 lvalue를 이동 가능한 xvalue로 표현하는 std::move를 제공한다.
#include <utility>

// struct의 기본 접근은 public이다. 진단을 외부 세계로 내보내는 포트(추상 경계)다.
struct DiagnosticSink {
    // 가상 소멸자는 기반 포인터로 파생 객체를 지울 때 올바른 소멸을 보장한다.
    virtual ~DiagnosticSink() = default;
    // void는 반환값 없음, const string&는 문자열을 복사하지 않고 호출 동안만 빌리는 매개변수다.
    virtual void write(const std::string& message, std::source_location where) const = 0;
};

// using은 긴 템플릿 타입에 읽기 쉬운 별칭을 붙인다. unique_ptr의 템플릿 인자는 소유할 기반 타입이다.
using DiagnosticPtr = std::unique_ptr<DiagnosticSink>;

// class의 기본 접근은 private다. 이 어댑터는 포트를 콘솔 출력으로 연결한다.
class ConsoleDiagnosticSink final : public DiagnosticSink {
public:
    // 생성자는 반환형이 없다. explicit은 문자열 하나가 어댑터로 암시 변환되는 것을 막는다.
    explicit ConsoleDiagnosticSink(std::string channel)
        // 멤버 초기화 목록은 본문 전에 멤버를 직접 초기화한다; std::move는 매개변수 lvalue를 xvalue로 만든다.
        : channel_{std::move(channel)} {}

    // override는 기반 가상 함수의 시그니처가 정확히 일치하는지 컴파일러가 확인하게 한다.
    void write(const std::string& message, std::source_location where) const override {
        // source_location의 file_name()/function_name()은 인자 없이 구현 보유 C 문자열 포인터를, line()은 줄 번호 정수를 반환한다.
        // where는 바뀌지 않고 반환 문자 포인터는 source_location 계약 수명에 의존하므로 직접 delete하지 않는다.
        std::cout << '[' << channel_ << "] " << message << " @ "
                  << where.file_name() << ':' << where.line() << " (" << where.function_name() << ")\n";
    }

private:
    std::string channel_{}; // 멤버 변수는 어댑터가 소유하는 채널 이름이며 private으로 숨긴다.
};

// struct는 결과 데이터에 적합하다. bool과 int는 각각 참/거짓과 정수 기본 타입이다.
struct PlaceResult { bool accepted{}; int stored_quantity{}; };

class OrderService {
public:
    // 생성자 매개변수는 소유권을 옮길 unique_ptr 값이다; 직접 초기화 OrderService{std::move(sink)}가 올바른 사용이다.
    explicit OrderService(DiagnosticPtr sink) : sink_{std::move(sink)} {}

    // 기본 인자의 current()는 이 함수를 호출한 위치에서 source_location prvalue를 만든다.
    // current()는 명시 인자 없이 호출 지점 정보를 담은 source_location 값을 반환한다. 기본 인자는 place 호출 위치에서 평가된다.
    [[nodiscard]] PlaceResult place(int quantity, std::source_location where = std::source_location::current()) const {
        // if는 비교 결과에 따라 분기한다. 일반적으로 비교·조건 분기·호출은 구현되지만 정확한 명령은 환경마다 다르다.
        if (quantity <= 0) {
            // ->는 포인터가 가리키는 객체의 멤버를 부른다; 가상 간접 호출 여부와 인라이닝은 ABI/최적화에 따라 달라진다.
            sink_->write("수량은 1 이상이어야 합니다", where);
            return PlaceResult{false, 0}; // 중괄호 초기화한 prvalue는 반환 목적 객체에 직접 구성되어 복사 생략될 수 있다.
        }
        return PlaceResult{true, quantity};
    }

private:
    DiagnosticPtr sink_; // 서비스가 sink의 유일한 소유자다; 서비스 수명 종료 시 자동으로 해제된다.
};

int main() { // main의 반환형 int는 운영체제에 상태 코드를 돌려준다.
    // make_unique의 결과 prvalue로 단독 소유 포인터를 만들고, 중괄호로 직접 초기화한다.
    // make_unique는 "order"를 생성자 인자로 넘겨 객체를 만들고 unique_ptr<ConsoleDiagnosticSink> prvalue를 반환한다.
    DiagnosticPtr sink{std::make_unique<ConsoleDiagnosticSink>("order")};
    // std::move는 sink lvalue를 xvalue로 바꿔 소유권 이동을 요청한다; 이동 뒤 sink는 비어 있을 수 있다.
    const OrderService service{std::move(sink)};
    // result는 이름이 있으므로 lvalue다. const는 이후 값을 바꾸지 않겠다는 약속이다.
    const PlaceResult result{service.place(0)};
    // &&는 왼쪽이 거짓이면 오른쪽을 평가하지 않는 논리 AND 연산자다.
    return (!result.accepted && result.stored_quantity == 0) ? 0 : 1;
}
