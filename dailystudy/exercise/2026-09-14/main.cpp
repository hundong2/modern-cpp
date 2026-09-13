#include <array>       // std::array: 크기가 타입에 포함되는 고정 길이 연속 저장소를 제공한다.
#include <cstddef>     // std::size_t: 배열 크기와 메모리 크기를 나타내는 부호 없는 타입을 제공한다.
#include <iostream>    // std::cout: 학습 결과를 표준 출력에 쓰는 전역 스트림 객체를 제공한다.
#include <optional>    // std::optional: 직렬화 성공 뷰 또는 정상적인 실패(없음)를 한 타입으로 표현한다.
#include <span>        // std::span: 문자를 소유하지 않고 연속 메모리 구간만 빌려 보는 뷰를 제공한다.
#include <spanstream>  // std::ospanstream: 호출자가 준 span을 출력 버퍼로 사용하는 C++23 스트림을 제공한다.
#include <string_view> // std::string_view: 문자열 문자를 복사·소유하지 않는 읽기 전용 뷰를 제공한다.
#include <utility>     // std::move: lvalue 식을 xvalue 식으로 바꾸는 값 범주 변환을 제공한다.

// struct는 기본 접근이 public이라 데이터 전달 객체(DTO)에 알맞다.
// 각 {}는 값 초기화다. int는 0, string_view는 빈 뷰, bool은 false가 된다.
// string_view는 actor 문자를 소유하지 않으므로 원본 문자가 사용하는 동안 살아 있어야 한다.
struct AuditEvent {
    int sequence{};

    // [생성 계약: std::string_view 기본 생성]
    // (1) 수신 결과는 아직 수명이 시작되지 않은 actor 멤버이며 정확한 타입은 std::string_view다.
    // (2) constexpr basic_string_view() noexcept 기본 생성자가 선택되고 템플릿 문자는 char다.
    // (3) 명시 인자와 문자 소유권 입력이 없다. 이 기본 멤버 초기화는 aggregate가 actor를 생략할 때 사용된다.
    // (4) 생성자는 반환값이 없고 actor를 data()==nullptr, size()==0인 빈 뷰로 만든다.
    // (5) 외부 객체는 바뀌지 않으며, 현재 아래 event처럼 actor를 명시하면 이 기본 초기화는 평가되지 않는다.
    // (6) O(1), 무할당·noexcept·무효화 없음이다. 빈 뷰는 역참조할 수 없고 객체별 동시 접근 규칙을 따른다.
    std::string_view actor{};
    bool allowed{};
};

// 템플릿 인자 Capacity는 컴파일 시간 상수다. AuditLineEncoder<48>과 <64>는 서로 다른 타입이다.
// class는 기본 접근이 private이므로 저장소를 감추고 유효한 encode 연산만 공개하기 좋다.
template <std::size_t Capacity>
class AuditLineEncoder {
public:
    // using은 새 타입을 만드는 문법이 아니라 긴 타입에 읽기 좋은 별칭을 붙인다.
    // 성공 상태는 buffer_를 가리키는 string_view, 실패 상태는 값 없음이다.
    using Result = std::optional<std::string_view>;

    // explicit은 "gateway" 같은 값이 AuditLineEncoder로 뜻밖에 암시 변환되는 것을 막는다.
    // 매개변수 channel은 값으로 복사되는 비소유 뷰이며 문자열 리터럴처럼 충분히 긴 원본을 요구한다.
    // 멤버 초기화 목록은 함수 본문보다 먼저 channel_을 직접 구성한다.
    // [생성 계약: std::string_view 복사 생성]
    // (1) 수신 channel_은 아직 수명이 시작되지 않은 std::string_view 멤버이고 channel은 유효한 string_view 값 매개변수다.
    // (2) constexpr basic_string_view(const basic_string_view&) noexcept = default 복사 생성자가 선택된다.
    // (3) 이름 있는 channel은 std::string_view lvalue이며 포인터와 길이만 복사하고 문자 소유권은 넘기지 않는다.
    // (4) 생성자는 반환값이 없고 channel_이 channel과 같은 문자 범위를 보게 한다.
    // (5) channel과 원본 문자는 바뀌지 않으며 channel_은 encoder 수명 동안 포인터·길이를 보관한다.
    // (6) O(1), 무할당·noexcept·무효화 없음이다. 원본 문자는 channel_의 모든 사용보다 오래 살아야 하고 동시 쓰기는 동기화해야 한다.
    explicit AuditLineEncoder(std::string_view channel) noexcept
        : channel_{channel} {}

    // 반환형 Result의 [[nodiscard]]는 직렬화 실패를 호출자가 무시하면 컴파일러가 경고할 수 있게 한다.
    // const AuditEvent&는 event를 복사·수정하지 않고 이름 있는 원본(lvalue)을 잠시 빌린다.
    [[nodiscard]] Result encode(const AuditEvent& event) {
        // [생성 계약: std::span<char>의 std::array 변환 생성]
        // (1) 수신 결과 타입은 동적 extent의 std::span<char>; buffer_는 std::array<char, Capacity> lvalue이며 이 객체가 소유한다.
        // (2) 선택되는 것은 template<size_t N> span(array<element_type, N>&) noexcept 계열이고 N은 Capacity다.
        // (3) 유일한 인자 buffer_의 타입은 std::array<char, Capacity>, 값 범주는 lvalue, 소유권은 이동하지 않으며 N과 extent가 호환돼야 한다.
        // (4) 생성자는 반환값이 없고, 새 span 값은 buffer_.data()부터 Capacity개 char를 가리킨다.
        // (5) buffer_는 바뀌지 않고 span만 포인터·길이를 복사한다. encoder가 파괴되면 span과 이를 통해 만든 뷰는 매달린다.
        // (6) 전제조건은 유효한 array lvalue이며 후조건은 size()==Capacity다. O(1), 무할당, 무효화 없음, noexcept이고 동시 공유 쓰기는 별도 동기화 없이는 데이터 경쟁이다.
        const std::span<char> storage{buffer_};

        // [생성 계약: std::basic_ospanstream<char>]
        // (1) 수신 결과 타입은 std::ospanstream(=basic_ospanstream<char>); storage는 쓰기 가능한 Capacity개 char를 가리키는 유효한 비소유 span이다.
        // (2) 선택 오버로드는 explicit basic_ospanstream(std::span<char>, ios_base::openmode = ios_base::out)이며 기본 openmode를 쓴다.
        // (3) storage는 const lvalue 식이지만 span 값 자체가 복사되어 전달된다. 문자 소유권은 encoder에 남고 허용 범위는 buffer_ 전체다.
        // (4) 생성자는 반환값이 없으며 output은 put position 0에서 시작하는 출력 스트림이 된다.
        // (5) storage와 buffer_의 크기/주소는 그대로이고 이후 output 쓰기만 buffer_의 문자를 바꾼다. output은 이 함수 끝에서 먼저 파괴된다.
        // (6) O(1), 자체 문자 버퍼 할당 없음이다. span 메모리는 output보다 오래 살아야 하며 같은 문자를 동시에 읽고 쓰면 데이터 경쟁이다. iostream 구성 과정 예외 가능성은 구현/locale 상태에 따른다.
        std::ospanstream output{storage};

        // [호출 계약: ospanstream/ostream 삽입 연산자 묶음]
        // (1) 최초 수신자는 정상 상태인 std::ospanstream output lvalue이고, 각 호출이 돌려준 std::ostream&가 다음 삽입의 수신자가 된다.
        // (2) string_view 비멤버 operator<<, char/const char* 비멤버 operator<<, int용 basic_ostream::operator<<(int) 오버로드가 피연산자 타입에 따라 선택된다.
        // (3) channel_/event.actor는 비소유 string_view lvalue, '|'는 char prvalue, sequence는 int lvalue, 조건식 결과는 정적 수명의 const char* prvalue다. 어느 문자 원본의 소유권도 넘기지 않는다.
        // (4) 각 호출은 같은 std::ostream&를 반환하고 체인의 다음 호출에 사용한다. 마지막 반환 참조만 버린다.
        // (5) 성공하면 "채널|번호|행위자|결정" 문자가 buffer_에 순서대로 기록되고 put position이 전진한다. event와 문자열 원본은 바뀌지 않는다.
        // (6) 출력 문자 수에 선형이며 동적 문자 할당은 없다. 용량 초과/변환 실패는 상태 비트를 실패로 만들 수 있고 예외 마스크 설정 시 ios_base::failure가 가능하다. 기존 뷰는 다음 encode 쓰기로 내용이 덮일 수 있고 동시 호출은 안전하지 않다.
        output << channel_ << '|' << event.sequence << '|' << event.actor << '|'
               << (event.allowed ? "ALLOW" : "DENY");

        // [호출 계약: std::basic_ios<char>::operator bool() const]
        // (1) 수신자는 쓰기를 마친 output lvalue이며 good 상태이거나 용량 초과 등으로 fail/bad 상태일 수 있다.
        // (2) if의 !output은 explicit operator bool() const의 결과를 논리 부정하며 인자는 없다.
        // (3) 위치 인자와 소유권 이동은 없고 output 수명 안에서 상태만 관찰한다.
        // (4) operator bool은 fail()==false를 bool로 반환하며 여기서는 부정해 실패 분기에 사용한다.
        // (5) output과 buffer_를 바꾸지 않는다. 실패 때 부분 문자열은 남을 수 있으나 외부에 공개하지 않는다.
        // (6) 통상 O(1), 무할당·무효화 없음이다. 표준 시그니처 자체에는 noexcept가 없으므로 이를 일반 보장으로 단정하지 않으며, 동일 스트림의 동시 접근은 외부 동기화가 필요하다.
        if (!output) {
            // [생성 계약: std::optional<std::string_view>(std::nullopt_t)]
            // (1) 수신 함수 결과 객체는 아직 생성 전인 Result이고 output은 실패 상태다. std::nullopt는 const std::nullopt_t 상수 객체다.
            // (2) constexpr optional(nullopt_t) noexcept 생성자가 반환 문맥에서 선택되며 T는 std::string_view다.
            // (3) std::nullopt lvalue의 태그 값을 복사해 전달할 뿐 문자·버퍼 소유권 인자는 없다.
            // (4) 생성자는 별도 반환값이 없고 encode의 반환 객체를 disengaged optional로 직접 구성한다.
            // (5) output과 buffer_는 그대로이고, 실패 중 기록됐을 수 있는 부분 문자열은 외부 뷰로 공개되지 않는다.
            // (6) O(1), 무할당·noexcept·무효화 없음이다. 빈 결과의 operator*/value 접근은 각각 UB/예외이므로 호출자가 먼저 상태를 검사해야 한다.
            return std::nullopt;
        }

        // [호출 계약: std::basic_ospanstream<char>::span() const noexcept]
        // (1) 수신자는 성공 상태이며 buffer_ 앞부분에 완성된 레코드를 가진 output lvalue다.
        // (2) 선택 시그니처는 std::span<char> span() const noexcept이고 인자가 없다.
        // (3) 전달 인자/소유권 이동은 없다. output이 제어하는 현재 출력 시퀀스를 관찰한다.
        // (4) 반환형 std::span<char> 값은 실제 기록된 문자 구간을 가리키며 written을 직접 초기화하는 데 사용한다.
        // (5) output과 buffer_는 바뀌지 않고 written이 같은 문자 구간을 비소유로 가리킨다.
        // (6) O(1), 무할당, noexcept다. buffer_가 파괴되거나 다음 encode가 덮어쓰면 내용 수명이 끝나거나 바뀐다. 동시 접근은 보장되지 않는다.
        const std::span<char> written{output.span()};

        // [호출 계약: std::span<char>::data()와 size()]
        // (1) 수신자는 완성된 기록 구간을 가리키는 유효한 const std::span<char> written이다.
        // (2) data() const noexcept -> char*, size() const noexcept -> size_type 오버로드가 각각 선택되며 둘 다 인자가 없다.
        // (3) 인자와 소유권 이동은 없고 span의 포인터/길이만 읽는다.
        // (4) data() 반환은 string_view 시작 주소, size() 반환은 문자 수로 모두 사용된다. 빈 span이면 data는 비교만 가능한 값일 수 있으나 길이는 0이다.
        // (5) written, output, buffer_ 모두 바뀌지 않는다. 생성될 string_view도 문자를 소유하지 않는다.
        // (6) 두 호출 모두 O(1), 무할당, noexcept, 반복자/참조 무효화 없음이다. 반환 포인터 사용 수명은 buffer_에 종속되고 동시 쓰기는 안전하지 않다.
        const char* const first{written.data()};
        // [바로 위 계약 적용: span::size] 묶음의 (1)~(6)이 이 첫 size() 호출에도 각각 적용된다.
        const std::size_t count{written.size()};

        // [생성 계약: std::string_view(const char*, size_type)와 std::optional 값 생성]
        // (1) 안쪽 수신 결과는 아직 생성 전인 std::string_view, 바깥 수신 결과는 encode의 아직 생성 전인 Result다. [first, first+count)는 buffer_ 안에서 유효하다.
        // (2) 안쪽은 constexpr basic_string_view(const char*, size_type), 바깥은 U=std::string_view인 optional(U&&) 값 생성자를 선택한다. 두 표준 선언 자체에는 noexcept가 없다.
        // (3) first는 const char* lvalue, count는 size_t lvalue, 안쪽 결과는 std::string_view prvalue다. 널 종료나 문자 소유권 이전은 요구하지 않는다.
        // (4) 생성자 자체 반환값은 없다. 바깥 생성자가 engaged Result를 만들고 그 안의 string_view가 기록 범위를 가리킨다.
        // (5) buffer_는 그대로이며 Result는 주소와 길이만 보관한다. string_view에는 별도 이동 생성자가 없어 전달된 prvalue에서도 포인터·길이 복사가 선택된다.
        // (6) 포인터 범위가 유효해야 하며 위반은 UB다. 둘 다 O(1)·무할당이고 현재 char/string_view 구성에서 던질 작업은 없지만, 선언에 없는 noexcept를 일반 보장으로 확대하지 않는다. 다음 encode는 같은 주소의 문자를 덮어쓰며 encoder 파괴 뒤 뷰 사용은 UB다.
        // 이 prvalue Result는 반환 객체를 직접 구성할 수 있어 불필요한 optional 복사를 요구하지 않는다(보장된 복사 생략).
        return Result{std::string_view{first, count}};
    }

private:
    // 배열이 실제 문자를 소유한다. 멤버는 선언 역순으로 파괴되지만 두 멤버 모두 단순 값이다.
    std::array<char, Capacity> buffer_{};
    // channel_은 외부 문자를 관찰한다. 생성자에 임시 std::string을 넘겨서는 안 된다.
    std::string_view channel_{};
};

// int 반환형은 운영체제에 프로그램 성공(0) 또는 학습 검증 실패(0이 아님)를 전달한다.
int main() {
    // 48은 비타입 템플릿 인자다. encoder는 48바이트 배열을 값으로 소유한다.
    // 문자열 리터럴 "gateway"는 정적 수명이므로 channel_ string_view보다 오래 산다.
    // [생성 계약: std::string_view(const char*) 문자열 리터럴 변환]
    // (1) 수신 channel 매개변수는 아직 생성 전인 std::string_view 값이고 "gateway"는 정적 수명의 const char[8] lvalue다.
    // (2) 배열-포인터 변환 뒤 constexpr basic_string_view(const char*) 생성자가 선택되고 traits_type은 std::char_traits<char>다.
    // (3) NUL 종료된 첫 문자 주소 const char* prvalue를 전달한다. 문자를 빌릴 뿐 복사하거나 소유하지 않는다.
    // (4) 생성자는 반환값이 없고 길이 7인 channel 값을 만들어 AuditLineEncoder 생성자 인자로 사용한다.
    // (5) 리터럴은 바뀌지 않고 channel/channel_이 같은 정적 문자 범위를 가리킨다.
    // (6) 포인터가 유효한 NUL 종료 배열이어야 한다. traits::length 때문에 O(문자 수), 무할당·무효화 없음이다. std::char_traits<char>::length는 비투척 요구를 따르지만 생성자 선언 자체에는 noexcept가 없다.
    AuditLineEncoder<48> encoder{"gateway"};
    // 중괄호 집계 초기화는 세 public 멤버를 선언 순서대로 초기화한다.
    // [바로 위 계약 적용: string_view(const char*)] "kim"도 const char[4]에서 길이 3인 비소유 뷰를 같은 전제·사후·복잡도·수명 규칙으로 만든다.
    const AuditEvent event{17, "kim", true};

    // event는 이름 있는 const lvalue다. encode(event)가 반환한 Result는 prvalue이며 encoded를 직접 초기화한다.
    auto encoded{encoder.encode(event)};

    // [호출 계약: std::optional<std::string_view>::operator bool() const]
    // (1) 수신자는 encode 결과를 소유하는 encoded lvalue이고 성공이면 engaged, 용량 부족이면 disengaged다.
    // (2) if의 !encoded는 explicit operator bool() const noexcept 결과를 부정하며 인자는 없다.
    // (3) 인자/소유권 이동 없이 활성 상태 태그만 읽는다.
    // (4) bool 반환값은 실패 분기에 사용된다.
    // (5) encoded와 내부 string_view 상태는 바뀌지 않는다.
    // (6) O(1), 무할당, noexcept, 무효화 없음이다. 같은 optional을 동시에 수정하며 읽으면 데이터 경쟁이다.
    if (!encoded) {
        return 1;
    }

    // [호출 계약: std::move와 optional::value() &&]
    // (1) encoded의 정확한 타입은 engaged std::optional<std::string_view> lvalue이며 내부 뷰는 encoder.buffer_를 가리킨다.
    // (2) std::move<optional<string_view>&>(encoded) -> optional<string_view>&& 뒤에, value() && -> string_view&& 오버로드가 선택된다.
    // (3) std::move 인자는 encoded lvalue 참조이고 xvalue로만 바꾼다. value에는 인자가 없으며 문자 소유권은 이동하지 않는다.
    // (4) value가 돌려준 string_view&&로 published를 구성한다. string_view에는 별도 이동 생성자가 없어 defaulted 복사 생성자가 포인터와 길이를 복사하며, 반환 참조는 따로 저장하지 않는다.
    // (5) encoded는 여전히 engaged이고 내부 string_view의 포인터·길이도 그대로다. published는 같은 buffer_ 문자를 관찰한다.
    // (6) 두 연산은 O(1), 무할당이다. value는 disengaged면 std::bad_optional_access를 던지지만 앞 분기가 전제조건을 증명한다. buffer_ 수명·다음 encode의 내용 변경 규칙은 유지되며 스레드 안전 보장은 없다.
    // 이름 있는 encoded는 lvalue, std::move(encoded)는 xvalue, encode(event)의 반환값은 prvalue다. xvalue라도 대상 타입에 이동 생성자가 없으면 복사가 선택될 수 있다.
    const std::string_view published{std::move(encoded).value()};

    // [호출 계약: std::cout의 string_view/char 삽입]
    // (1) 최초 수신자는 프로그램 시작부터 존재하는 std::ostream 타입의 std::cout lvalue이며 정상 상태를 기대한다.
    // (2) operator<<(ostream&, string_view)와 operator<<(ostream&, char)가 순서대로 선택된다.
    // (3) published는 유효한 비소유 string_view lvalue, '\n'은 char prvalue이고 소유권 이동은 없다.
    // (4) 각 호출은 같은 ostream&를 반환해 다음 호출에 사용하고 마지막 반환은 버린다.
    // (5) 성공하면 정확한 레코드와 줄바꿈이 출력 버퍼에 기록된다. published와 encoder는 바뀌지 않는다.
    // (6) 비용은 문자 수와 locale/장치에 의존한다. 실패는 상태 비트, 예외 마스크가 켜졌다면 ios_base::failure로 보고된다. 전역 스트림 공유 시 레코드 단위 원자성은 보장되지 않는다.
    std::cout << published << '\n';

    // [호출 계약: std::string_view 비교 operator==]
    // (1) 왼쪽은 buffer_를 가리키는 유효한 published lvalue, 오른쪽은 정적 수명의 문자열 리터럴에서 변환된 string_view다.
    // (2) basic_string_view<char>의 동등 비교가 선택되며 두 문자 시퀀스를 읽는다.
    // (3) 어느 쪽도 소유권을 넘기지 않고 두 범위 모두 비교 동안 유효해야 한다.
    // (4) 길이와 모든 문자가 같으면 bool true를 반환하며 종료 코드 선택에 사용한다.
    // (5) 피연산자와 문자 저장소는 바뀌지 않는다.
    // (6) 최악 O(n), 무할당, 예외 없음이며 dangling view를 넘기면 UB다. 동시 수정이 없다면 읽기 비교가 가능하다.
    return published == "gateway|17|kim|ALLOW" ? 0 : 2;
}
