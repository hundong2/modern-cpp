#include <array>       // std::array: 템플릿 크기의 고정 문자 저장소를 값으로 소유한다.
#include <cstddef>     // std::size_t: 용량과 기록된 문자 수의 타입을 제공한다.
#include <iostream>    // std::cout: 완성된 연습 결과를 표준 출력으로 보낸다.
#include <optional>    // std::optional: 포맷 성공 뷰 또는 용량 부족을 표현한다.
#include <span>        // std::span: array 메모리를 소유하지 않는 연속 구간으로 빌린다.
#include <spanstream>  // std::ospanstream: 외부 고정 버퍼에 스트림 형식으로 기록한다.
#include <string_view> // std::string_view: 이름과 결과 문자열을 복사 없이 관찰한다.

// struct는 기본 public인 단순 입력 묶음에 사용한다. name은 문자를 소유하지 않는다.
struct MetricSample {
    // [생성 계약: std::string_view 기본 생성]
    // (1) 수신 결과는 아직 수명이 시작되지 않은 name 멤버이며 정확한 타입은 std::string_view다.
    // (2) constexpr basic_string_view() noexcept 기본 생성자가 선택되고 문자 타입은 char다.
    // (3) 명시 인자와 문자 소유권 입력은 없다. aggregate가 name을 생략한 경우에만 이 기본 멤버 초기화를 평가한다.
    // (4) 생성자는 반환값이 없고 name을 data()==nullptr, size()==0인 빈 뷰로 만든다.
    // (5) 외부 객체는 바뀌지 않으며 아래 sample처럼 name을 명시하면 이 기본 초기화는 평가되지 않는다.
    // (6) O(1), 무할당·noexcept·무효화 없음이다. 빈 뷰는 역참조할 수 없고 객체별 동시 접근 규칙을 따른다.
    std::string_view name{};
    int value{};
};

// 직접 해보기: Capacity를 16, 8로 바꾸고 어떤 입력부터 nullopt가 되는지 확인한다.
// 이 템플릿은 힙 할당 없이 "name=value" 한 줄을 만드는 실무형 고정 버퍼 어댑터다.
template <std::size_t Capacity>
class MetricLineFormatter {
public:
    // 함수는 sample을 const 참조로 빌리고, 성공 시 storage_를 가리키는 뷰를 반환한다.
    // 다음 format 호출은 같은 저장소를 덮어쓰므로 이전 뷰의 주소는 살아 있어도 내용은 바뀐다.
    [[nodiscard]] std::optional<std::string_view> format(const MetricSample& sample) {
        // [생성 계약: std::span<char>과 std::ospanstream]
        // (1) storage_는 std::array<char, Capacity> 타입의 살아 있는 소유 lvalue이고, stream은 아직 만들어지지 않았다.
        // (2) span의 array lvalue 생성자와 explicit basic_ospanstream(std::span<char>, ios_base::out 기본값)를 차례로 선택한다.
        // (3) storage_는 소유권을 넘기지 않는 lvalue, 만들어진 span은 포인터·Capacity를 값 복사해 전달한다. Capacity는 0도 타입상 가능하나 이 예제에서는 양수다.
        // (4) 생성자에는 반환값이 없다. buffer는 storage_ 전체를 가리키고 stream의 put position은 0에서 시작한다.
        // (5) storage_ 주소/크기는 유지되고 이후 stream 삽입이 문자만 갱신한다. stream은 함수 반환 전에 파괴된다.
        // (6) 두 생성 모두 O(1), 문자용 동적 할당 없음이다. storage_가 stream보다 오래 살아야 하고 같은 formatter의 동시 format은 데이터 경쟁이다. iostream/locale 구성 예외 가능성은 구현 상태에 따른다.
        const std::span<char> buffer{storage_};
        // [바로 위 계약 적용: std::ospanstream] 묶음의 (1)~(6) 중 두 번째 생성자 계약이 이 첫 호출에 적용된다.
        std::ospanstream stream{buffer};

        // [호출 계약: ospanstream/ostream 삽입 연산자]
        // (1) 수신자는 정상 상태인 std::ospanstream stream lvalue이며 Capacity만큼만 쓸 수 있다.
        // (2) string_view용 비멤버 operator<<, char용 비멤버 operator<<, int용 basic_ostream::operator<<(int)가 차례로 선택된다.
        // (3) sample.name은 비소유 string_view lvalue, '='는 char prvalue, sample.value는 int lvalue다. 입력은 읽기만 하고 소유권을 옮기지 않는다.
        // (4) 각 호출은 동일한 std::ostream&를 반환해 다음 삽입의 수신자로 쓰며 마지막 반환은 버린다.
        // (5) 성공하면 storage_ 앞에 name=value가 기록되고 put position이 전진한다. sample은 바뀌지 않는다.
        // (6) O(이름 길이+숫자 자릿수), 문자 버퍼 무할당이다. 용량 초과는 스트림 실패 상태를 만들고 예외 마스크 설정 시 ios_base::failure가 가능하다. 이전 결과 뷰의 내용은 덮일 수 있다.
        stream << sample.name << '=' << sample.value;

        // [호출 계약: std::basic_ios<char>::operator bool() const]
        // (1) stream은 완전 기록 또는 용량 초과 상태다.
        // (2) explicit operator bool() const 결과를 !로 부정하며 인자는 없다.
        // (3) 상태만 읽고 소유권 이동은 없다.
        // (4) 실패가 없으면 true인 bool을 반환하며, 그 값을 부정해 실패 분기에 사용한다.
        // (5) stream/storage_는 바뀌지 않으며 실패 때 부분 출력은 공개하지 않는다.
        // (6) 통상 O(1), 무할당·무효화 없음이다. 표준 시그니처 자체에는 noexcept가 없어 이를 일반 보장으로 단정하지 않으며, 동시 쓰기 안전성은 제공하지 않는다.
        if (!stream) {
            // [생성 계약: std::optional<std::string_view>(std::nullopt_t)]
            // (1) 수신 함수 결과 객체는 아직 생성 전인 optional<string_view>이고 stream은 실패 상태다. std::nullopt는 const std::nullopt_t 상수 객체다.
            // (2) constexpr optional(nullopt_t) noexcept 생성자가 반환 문맥에서 선택된다.
            // (3) std::nullopt lvalue 태그를 값으로 전달하고 문자·버퍼 소유권은 넘기지 않는다.
            // (4) 생성자는 별도 반환값이 없고 format 결과를 disengaged optional로 직접 구성한다.
            // (5) stream과 storage_는 그대로이고 부분 출력은 외부 뷰로 공개되지 않는다.
            // (6) O(1), 무할당·noexcept·무효화 없음이다. 빈 결과의 operator*/value 접근은 각각 UB/예외이므로 먼저 상태를 검사한다.
            return std::nullopt;
        }

        // [호출 계약: ospanstream::span, span::data, span::size, string_view와 optional 값 생성]
        // (1) stream은 성공 상태이고 storage_에 완성된 문자가 있다. written·string_view·반환 optional은 아직 생성 전이다.
        // (2) span() const noexcept, data() const noexcept, size() const noexcept, string_view(const char*,size_t)와 U=string_view인 optional(U&&)를 사용한다. 뒤의 두 생성자 선언에는 noexcept가 없다.
        // (3) 멤버 호출 인자는 없고 생성자에는 유효한 포인터 lvalue·길이 lvalue와 string_view prvalue를 준다. 널 종료나 소유권 이전은 없다.
        // (4) span 값이 written을 만들고 data/size가 string_view를 만들며, 변환 생성된 engaged optional이 format 반환 객체가 된다.
        // (5) storage_는 바뀌지 않고 optional 안의 뷰가 같은 범위를 가리킨다. 별도 string_view 이동 생성자가 없어 포인터·길이는 복사된다.
        // (6) 모두 O(1)·무할당이고 span/data/size만 noexcept다. 현재 char/string_view 구성에서 던질 작업은 없지만 생성자 선언의 일반 noexcept 보장은 없다. 포인터 범위가 유효해야 하며, 다음 format은 같은 주소의 내용을 바꾸고 소유자 파괴 뒤 뷰 사용은 UB다.
        const std::span<char> written{stream.span()};
        // [바로 위 계약 적용: span::data] 묶음의 (1)~(6) 중 data() 계약이 이 첫 호출에 적용된다.
        const char* const first{written.data()};
        // [바로 위 계약 적용: span::size] 묶음의 (1)~(6) 중 size() 계약이 이 첫 호출에 적용된다.
        const std::size_t count{written.size()};
        // [바로 위 계약 적용: string_view 포인터-길이 생성자] 묶음의 (1)~(6) 중 생성자 계약이 이 첫 호출에 적용된다.
        return std::string_view{first, count};
    }

private:
    // private 접근 지정자는 호출자가 버퍼를 직접 깨뜨리지 못하게 한다.
    std::array<char, Capacity> storage_{};
};

int main() {
    MetricLineFormatter<32> formatter{};
    // [생성 계약: std::string_view(const char*) 문자열 리터럴 변환]
    // (1) 수신 name 멤버는 아직 생성 전인 std::string_view이고 "latency_ms"는 정적 수명의 const char[11] lvalue다.
    // (2) 배열-포인터 변환 뒤 constexpr basic_string_view(const char*) 생성자가 선택되고 traits_type은 std::char_traits<char>다.
    // (3) NUL 종료된 첫 문자 주소 const char* prvalue를 전달하며 문자를 빌릴 뿐 복사·소유하지 않는다.
    // (4) 생성자는 반환값이 없고 길이 10인 name 뷰를 만들어 MetricSample의 첫 멤버를 초기화한다.
    // (5) 리터럴은 바뀌지 않고 sample.name이 프로그램 끝까지 유효한 정적 문자 범위를 가리킨다.
    // (6) 유효한 NUL 종료 배열이 전제다. traits::length 때문에 O(문자 수), 무할당·무효화 없음이다. std::char_traits<char>::length는 비투척 요구를 따르지만 생성자 선언 자체에는 noexcept가 없다.
    const MetricSample sample{"latency_ms", 37};

    // format(sample)의 optional 반환 prvalue가 line을 직접 초기화한다(RVO/복사 생략 가능 경계).
    const auto line{formatter.format(sample)};

    // [호출 계약: optional 상태 검사와 operator*]
    // (1) line은 const std::optional<std::string_view> lvalue이며 engaged여야 역참조할 수 있다.
    // (2) explicit operator bool() const noexcept와 const& operator*() const& noexcept가 선택되고 인자는 없다.
    // (3) 상태/내부 뷰를 읽기만 하며 소유권을 옮기지 않는다.
    // (4) bool은 오류 분기, operator*의 const string_view&는 출력과 비교에 사용된다.
    // (5) line과 formatter 저장소는 바뀌지 않는다.
    // (6) 둘 다 O(1), 무할당이다. operator*는 disengaged 상태면 UB이므로 먼저 검사한다. 반환 참조 수명은 line, 문자는 formatter에 종속되며 동시 수정은 안전하지 않다.
    if (!line) {
        return 1;
    }

    // [호출 계약: std::cout 삽입 연산자]
    // (1) 수신자는 정상 상태를 기대하는 std::ostream 타입 std::cout lvalue다.
    // (2) string_view와 char용 비멤버 operator<<가 선택된다.
    // (3) *line은 const string_view lvalue, '\n'은 char prvalue이며 소유권 이동은 없다.
    // (4) 각 호출은 같은 ostream&를 반환해 연결하고 마지막 반환은 버린다.
    // (5) name=value와 줄바꿈을 출력하며 입력 객체는 바뀌지 않는다.
    // (6) 비용은 문자 수/locale/장치에 의존한다. 실패는 상태 비트 또는 설정된 예외로 보고되고 레코드 단위 동시 출력은 보장되지 않는다.
    std::cout << *line << '\n';

    // 작은 저장소는 "latency_ms=37"을 모두 담지 못한다. 실패가 값으로 표현되는지 검증한다.
    MetricLineFormatter<8> tiny{};
    const auto rejected{tiny.format(sample)};

    // [호출 계약: std::string_view 비교와 optional 상태 검사]
    // (1) *line은 유효한 완성 뷰이고 rejected는 disengaged여야 한다.
    // (2) string_view operator==와 optional explicit operator bool이 선택된다.
    // (3) 문자열 리터럴은 정적 수명이고 모든 피연산자는 읽기 전용이며 소유권 이동이 없다.
    // (4) 두 bool 결과를 &&로 결합해 성공 종료 코드를 정한다.
    // (5) 객체 상태와 저장 문자는 바뀌지 않는다.
    // (6) 비교는 O(n), 상태 검사는 O(1), 둘 다 무할당이다. line이 engaged라는 앞 분기가 역참조 전제조건을 만족시키고, 같은 버퍼 동시 수정은 없어야 한다.
    return *line == "latency_ms=37" && !rejected ? 0 : 2;
}
