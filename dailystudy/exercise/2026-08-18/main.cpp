// <iostream>은 표준 출력 객체 std::cout과 삽입 연산자 선언을 제공한다.
#include <iostream>
// <memory>는 독점 소유 포인터 std::unique_ptr와 std::make_unique를 제공한다.
#include <memory>
// <string>은 문자 버퍼를 소유하는 std::string과 문자열 결합 연산자를 제공한다.
#include <string>
// <utility>는 lvalue를 xvalue로 바꾸는 std::move를 제공한다.
#include <utility>

// MessageFormatter는 공개 헤더에 둘 수 있는 작은 인터페이스이고 구현 세부사항은 Impl 뒤에 숨긴다.
class MessageFormatter {
public:
    // 생성자는 클래스와 이름이 같고 반환형이 없다. explicit은 문자열 하나가 MessageFormatter로
    // 뜻밖에 암시 변환되는 일을 막고 MessageFormatter{"[prod] "} 직접 초기화는 허용한다.
    explicit MessageFormatter(std::string prefix);
    // unique_ptr가 가리키는 Impl이 완전한 타입인 위치에서 소멸자를 정의해야 안전하게 delete할 수 있다.
    ~MessageFormatter();

    // unique_ptr는 복사할 수 없으므로 이 공개 타입도 복사를 명시적으로 금지한다.
    MessageFormatter(const MessageFormatter&) = delete;
    MessageFormatter& operator=(const MessageFormatter&) = delete;
    // 이동 생성자와 이동 대입은 독점 소유권을 옮기며 noexcept는 컨테이너도 안전하게 이동을 선택하게 한다.
    MessageFormatter(MessageFormatter&&) noexcept;
    MessageFormatter& operator=(MessageFormatter&&) noexcept;

    // const 멤버 함수는 논리 상태를 바꾸지 않고 새 문자열 값을 반환한다.
    [[nodiscard]] std::string format(const std::string& message) const;

private:
    // class의 기본 접근은 private이다. 전방 선언만으로 공개부가 구현 멤버를 몰라도 된다.
    class Impl;
    // unique_ptr<Impl>은 Impl 객체 하나를 독점 소유하고 MessageFormatter와 수명을 함께한다.
    // 이 멤버는 생성자의 make_unique 반환값으로 직접 초기화되므로 빈 기본 상태를 거치지 않는다.
    std::unique_ptr<Impl> impl_;
};

// 구현 클래스 변경은 MessageFormatter의 공개 데이터 배치를 바꾸지 않는 Pimpl 컴파일 방화벽이 된다.
class MessageFormatter::Impl {
public:
    // 생성자 매개변수는 문자열 값을 소유해 받고 멤버 초기화 목록에서 실제 멤버로 이동한다.
    explicit Impl(std::string prefix)
        // std::move<T>(T&&)의 T는 std::string으로 추론된다. prefix는 이름 때문에 lvalue지만
        // xvalue로 변환되어 prefix_의 이동 생성자가 버퍼를 넘겨받을 수 있고, 원본은 유효하지만 값은 미지정이다.
        : prefix_{std::move(prefix)} {}

    // class의 public 영역에 둔 멤버지만 Impl 자체는 외부에 보이지 않는다. {}만 사용했다면 인자 없는
    // string 기본 생성자가 할당 없는 빈 문자열을 만들지만, 실제 생성 경로에서는 이동 생성자로 초기화된다.
    std::string prefix_{};
};

MessageFormatter::MessageFormatter(std::string prefix)
    // std::make_unique<Impl>(std::move(prefix))는 Impl 템플릿 인자를 명시하고 xvalue 문자열 한 개를 전달한다.
    // Impl을 동적 할당해 생성한 뒤 unique_ptr<Impl> prvalue를 반환하며, 실패하면 bad_alloc 등이 발생하고
    // 부분 객체를 남기지 않는다. 반환 포인터가 impl_을 직접 초기화해 독점 소유권을 갖는다.
    : impl_{std::make_unique<Impl>(std::move(prefix))} {}

// Impl 정의 뒤의 default 소멸자는 unique_ptr 소멸을 통해 Impl을 정확히 한 번 파괴한다.
MessageFormatter::~MessageFormatter() = default;
// default 이동 생성자는 impl_ 포인터 소유권을 새 객체로 옮기고 원본 포인터를 빈 상태로 만든다.
MessageFormatter::MessageFormatter(MessageFormatter&&) noexcept = default;
// default 이동 대입은 대상의 기존 Impl을 먼저 정리하고 원본의 소유권을 넘겨받는다.
MessageFormatter& MessageFormatter::operator=(MessageFormatter&&) noexcept = default;

std::string MessageFormatter::format(const std::string& message) const {
    // 표준 string operator+(const string&, const string&) 계열은 impl_->prefix_와 message를 변경하지 않고
    // 두 문자를 복사한 새 std::string prvalue를 반환한다. 시간·추가 공간은 결과 길이에 선형이며 할당 실패가
    // 예외가 될 수 있다. 반환 prvalue는 C++17 이후 호출자의 목적 객체를 직접 초기화하도록 복사 생략된다.
    return impl_->prefix_ + message;
}

int main() {
    // 문자열 리터럴은 std::string(const char*) 생성자의 포인터 값 인자에서 null 종료 문자를 선형 복사해
    // 값 매개변수를 만들고, 새 문자열이 저장소를 소유한다. null 포인터는 금지되고 할당 실패는 예외가 된다.
    // 이어 explicit MessageFormatter 생성자를 직접 호출하므로 암시 변환 금지 규칙을 어기지 않는다.
    MessageFormatter formatter{"[prod] "};
    // format의 string 반환 prvalue가 result를 직접 초기화하고 result가 문자 버퍼를 소유한다.
    const std::string result{formatter.format("ready")};

    // std::move<MessageFormatter&>(formatter)는 formatter를 xvalue로 바꿀 뿐 직접 이동하지 않는다.
    // 다음 이동 생성자가 impl_ 소유권을 deployed로 옮기며 formatter는 파괴 가능한 빈 상태가 된다.
    MessageFormatter deployed{std::move(formatter)};

    // std::cout의 정확한 타입은 std::ostream이며 operator<<는 result의 문자를 스트림 버퍼에 기록하고
    // std::ostream&를 반환해 '\n' 삽입을 연쇄한다. result와 deployed는 바뀌지 않고 반환 참조는 저장하지 않는다.
    // 출력 비용은 문자열 길이에 선형이고 I/O 실패는 스트림 상태 비트를 설정하며 예외 마스크가 없으면 던지지 않는다.
    std::cout << result << '\n';

    // string operator==(const string&, const char*) 계열은 result와 null 종료 리터럴을 선형 비교해 bool을
    // 반환하고 두 입력을 바꾸지 않는다. 리터럴은 유효한 포인터여야 하며 반환값을 if 조건 분기에 사용한다.
    if (result == "[prod] ready") {
        return 0;
    }

    // 객체 생성·간접 접근·함수 호출·분기는 기계 수준에서 load/store/call 등을 만들 수 있으나 실제 명령은
    // CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.
    return 1;
}
