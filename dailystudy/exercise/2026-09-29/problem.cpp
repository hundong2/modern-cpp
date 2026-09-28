#include <cstddef>  // std::size_t: 문자열 길이, 인덱스, 제거 개수를 표현한다.
#include <iostream> // std::cout: 연습 결과를 표준 출력에 기록한다.
#include <string>   // std::string: 원본 일련번호와 압축 결과를 값으로 소유한다.
#include <utility>  // std::move: 값 매개변수의 소유 문자열을 멤버로 이전한다.

// using 별칭은 std::size_t와 같은 타입이며 단위가 다른 새 강한 타입을 만들지 않는다.
using CharacterCount = std::size_t;

// struct의 멤버는 기본 public이다. 결과처럼 불변식이 단순한 값 묶음을 호출자가 바로 읽게 할 때 적합하다.
// 직접 해보기: 숫자뿐 아니라 마지막 영문 검사 문자 하나를 보존하도록 술어와 기대 출력을 바꿔 보자.
struct CompactResult {
    std::string digits;
    CharacterCount dropped{};
};

// class의 멤버는 기본 private이고 public: 뒤만 외부 API다. final은 파생 class 선언을 컴파일 단계에서
// 막지만 객체의 변경이나 수명 연장을 뜻하지 않는다.
class SerialNumber final {
public:
    // explicit은 문자열 하나가 SerialNumber로 의도 없이 암시 변환되는 것을 막는다.
    explicit SerialNumber(std::string raw)
        // [첫 호출 계약: std::move와 std::string 이동 생성자]
        // (1) 수신 없는 std::move가 살아 있는 std::string lvalue raw를 보고, 멤버 raw_는 생성 전이다.
        // (2) move<std::string&>가 std::string&&를 반환하고 allocator 인자 없는 문자열 이동 생성자가 선택된다.
        // (3) raw는 이름 있는 lvalue이자 현재 문자열 소유자이고, xvalue로 소유 이전을 허용한다.
        // (4) move의 반환 참조는 raw_ 초기화에 사용되며 생성자에는 반환값이 없다.
        // (5) raw_가 이동 전 값을 소유하고 raw는 유효하지만 값이 미지정된다.
        // (6) cast와 이동은 O(1)·noexcept이며 기존 문자 관찰자는 다시 얻는다. 공유 변경 없이 객체 수명을 지킨다.
        : raw_{std::move(raw)} {}

    // CompactResult는 반환형, 빈 ()는 데이터 매개변수가 없다는 뜻이고 뒤 const는 raw_를 바꾸지 않는 약속이다.
    [[nodiscard]] CompactResult compact_digits() const {
        // [첫 호출 계약: string::size]
        // (1) raw_는 살아 있는 const std::string 수신 객체이며 호출 전 "SN-20 26-A"를 소유한다.
        // (2) size_type size() const noexcept가 선택되고 데이터 인자는 없다.
        // (3) 매개변수·소유권 이전은 없다.
        // (4) 문자 수를 값으로 반환하며 std::size_t maximum_size에 사용한다.
        // (5) raw_와 관찰자는 변하지 않는다.
        // (6) O(1), 무할당·무예외·무효화 없음이며 동시 변경이 없어야 한다.
        const std::size_t maximum_size{raw_.size()};

        // [첫 생성 계약: std::string 기본 생성자]
        // (1) digits는 아직 생성 전인 지역 목적 객체다.
        // (2) std::string()이 선택되고 명시 인자는 없다.
        // (3) 기본 allocator 정책을 쓰며 외부 소유권을 빌리지 않는다.
        // (4) 반환값 없이 빈 소유 문자열을 생성한다.
        // (5) 성공 뒤 size()==0인 유효 객체다.
        // (6) 일반 컨테이너 요구상 O(1)이고 기본 allocator에서 noexcept다. 빈 표현/실제 할당은 구현 세부이며
        //     digits는 지역 수명을 따르고 아직 공유되지 않아 thread 충돌이 없다.
        std::string digits{};

        // [첫 호출 계약: std::string::resize_and_overwrite]
        // (1) 수신 digits는 빈 수정 가능 std::string이며 기존 관찰자가 없다.
        // (2) template<class Operation> constexpr void resize_and_overwrite(size_type, Operation)에서 Operation=lambda가 선택되고,
        //     라이브러리는 std::move(op)(p,m)을 정확히 한 번 평가해 closure를 xvalue로 호출한다.
        // (3) maximum_size는 값으로 전달하는 쓰기 상한이고 같은 string raw_의 size라 max_size 이하이다. lambda
        //     prvalue는 비소유 this 포인터 값을 복사 캡처한다. p와 m==n은 const 값 매개변수이며 호출 중 바뀌지
        //     않고, 빈 수신이므로 [p,p+n)의 값은 모두 결정되지 않았을 수 있어 쓰기 전 읽지 않는다.
        // (4) 멤버 함수는 void, 콜백은 실제 초기화한 숫자 문자 수를 std::size_t로 반환한다.
        // (5) 성공 후 digits는 반환 길이의 문자만 소유하고 raw_는 변하지 않는다.
        // (6) 콜백은 noexcept이고 0<=r<=m, [p,p+r) 전부 초기화 조건을 지키며 위반은 UB다. 일반 n>max_size는
        //     length_error, 할당은 bad_alloc 가능하지만 현재 n은 범위 안이고 OP 전 정상 예외에는 수신 변화가 없다.
        //     멤버 전체의 표준 복잡도 상한은 없고 오늘 callback만 O(n)이다. 기존 관찰자와 p는 호출 뒤 무효이며
        //     같은 객체의 동시 접근은 외부에서 막는다.
        digits.resize_and_overwrite(
            maximum_size,
            [this](char* const output, const std::size_t writable_size) noexcept -> std::size_t {
                std::size_t written{};

                // [첫 호출 계약: const string::operator[]]
                // (1) 수신 raw_는 const std::string이고 콜백 동안 크기와 문자가 고정된다.
                // (2) const_reference operator[](size_type) const overload가 선택된다.
                // (3) index는 값 전달되는 std::size_t lvalue이고 루프가 index<size()를 보장한다.
                // (4) const char&를 반환해 비교하고, 참조를 저장하지 않는다.
                // (5) 수신/인자/소유권은 변하지 않는다.
                // (6) O(1)·무예외·무할당이다. 범위 밖 접근은 전제조건 위반이고 동시 수정은 금지된다.
                // for는 0부터 쓰기 상한 전까지 증가하고 if는 현재 문자가 숫자인 경로만 선택한다.
                for (std::size_t index{}; index < writable_size; ++index) {
                    const char current{raw_[index]};
                    if (current >= '0' && current <= '9') {
                        output[written] = current;
                        ++written;
                    }
                }
                return written;
            });

        const std::size_t actual_size{digits.size()};
        const std::size_t dropped{maximum_size - actual_size};
        // 결과 prvalue는 호출자 목적 객체에 직접 구성된다. 문자열 멤버만 xvalue에서 이동 소유한다.
        return CompactResult{std::move(digits), dropped};
    }

private:
    std::string raw_;
};

int main() {
    // [첫 생성 계약: std::string(const char*)]
    // (1) 목적 임시는 생성 전이고 리터럴은 정적 수명의 유효한 NUL 종료 char 배열이다.
    // (2) std::string(const char*) 생성자가 선택된다.
    // (3) 인자 배열에서 변환된 const char* prvalue를 빌려 읽고 null은 허용하지 않으며 소유권은 넘지 않는다.
    // (4) 생성자는 반환값 없이 리터럴 문자를 복사 소유하는 임시를 만든다.
    // (5) 임시는 SerialNumber 값 매개변수를 직접 초기화하고 원본 배열은 그대로다.
    // (6) O(n), 할당·길이 실패 가능, 완성 문자열 수명은 리터럴과 독립이다.
    const SerialNumber serial{std::string{"SN-20 26-A"}};
    const CompactResult result{serial.compact_digits()};

    // [첫 호출 계약: std::cout 스트림 삽입 연쇄]
    // (1) 수신 std::cout은 살아 있는 std::ostream이며 정상 상태라고 가정한다.
    // (2) const char*·std::string 비멤버 삽입, std::size_t 산술 멤버 삽입, char 비멤버 삽입이 선택된다.
    // (3) 리터럴/결과 문자열/제거 개수/'\n'은 모두 읽기 입력이고 소유권을 넘기지 않는다.
    // (4) 각 호출은 같은 std::ostream&를 다음 호출에 반환하며 마지막 반환은 버린다.
    // (5) result는 유지되고 cout 버퍼와 상태만 갱신되며 '\n'은 강제 flush가 아니다.
    // (6) 표준은 형식 출력 연쇄에 단일 점근 복잡도 상한을 두지 않는다. 생성 문자·locale·장치 비용과
    //     실패 상태·설정 시 예외가 가능하다. 동시 연쇄 출력의 레코드 원자성은 없다.
    std::cout << "digits=" << result.digits << ",dropped=" << result.dropped << '\n';
    return 0;
}
