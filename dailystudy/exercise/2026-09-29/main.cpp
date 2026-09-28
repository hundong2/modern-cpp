#include <cstddef>  // std::size_t: 문자열 길이와 인덱스를 표현하는 부호 없는 크기 타입이다.
#include <iostream> // std::cout: 완성된 로그 키와 제거 문자 수를 표준 출력에 기록한다.
#include <string>   // std::string: 입력과 결과 문자를 연속 저장소에 소유한다.
#include <utility>  // std::move: 이름 있는 문자열을 xvalue로 바꾸어 이동 생성을 선택 가능하게 한다.

// using은 기존 타입에 읽기 쉬운 별칭을 붙일 뿐 새 강한 타입을 만들지 않는다.
using CharacterCount = std::size_t;

// struct의 멤버는 기본 public이다. 계산 결과처럼 불변식이 단순한 데이터 묶음에 알맞다.
struct NormalizationResult {
    std::string value;       // 결과 문자열을 직접 소유하므로 원본 객체가 사라져도 유효하다.
    CharacterCount dropped{}; // 빈 중괄호 초기화는 기본 정수 타입 값을 0으로 만든다.
};

// class의 멤버는 기본 private이다. 원본 문자열과 구분자 정책을 API 뒤에 숨긴다.
// final은 이 class에서 파생 class를 선언하지 못하게 할 뿐, 객체를 불변으로 만들거나 수명을 늘리지 않는다.
class LogKeyNormalizer final {
public:
    // 생성자는 반환형이 없다. explicit은 두 인자 copy-list 초기화처럼 이 생성자를 통한 암시 변환을 막는다.
    explicit LogKeyNormalizer(std::string raw, const char separator)
        // [첫 호출 계약: std::move와 std::string 이동 생성자]
        // (1) std::move는 수신 객체가 없는 함수이고, 목적지 raw_는 아직 생성 전이다. raw는 살아 있는 std::string lvalue다.
        // (2) remove_reference_t<std::string&>&& move(std::string&)가 선택되어 std::string&&를 만들고,
        //     raw_에는 allocator 인자 없는 std::string(std::string&&) 이동 생성자가 선택된다.
        // (3) 인자 raw는 std::string lvalue이고 소유 문자를 넘길 대상으로 허용한다. move 결과는 같은 객체를 가리키는 xvalue다.
        // (4) move는 std::string&&를 반환해 raw_ 초기화에 사용하고, 생성자는 별도 반환값이 없다.
        // (5) 성공하면 raw_가 이동 전 문자열 값을 소유하며 raw는 유효하지만 값이 미지정된 상태다. separator는 값 복사된다.
        // (6) move 자체는 O(1)·무할당·noexcept cast다. 이 allocator 동일 이동 생성은 O(1)·noexcept지만
        //     기존 문자 포인터는 다시 얻어야 한다. 객체를 여러 스레드가 동시에 변경하지 않으며 수명은 각 객체가 관리한다.
        : raw_{std::move(raw)}, separator_{separator} {}

    // 반환형은 독립 소유 결과다. const는 이 호출이 raw_와 separator_를 바꾸지 않는다는 약속이다.
    [[nodiscard]] NormalizationResult normalize() const {
        // [첫 호출 계약: string::size]
        // (1) 수신 객체 raw_의 정확한 타입은 const std::string이며 생성 완료된 유효한 소유 문자열이다.
        // (2) constexpr size_type size() const noexcept가 선택되고 명시 매개변수는 없다.
        // (3) 데이터 인자는 없고 소유권 이전도 없다.
        // (4) string::size_type(여기서는 std::size_t로 받음)을 반환해 쓰기 상한으로 사용한다.
        // (5) raw_의 문자·크기·capacity와 관찰자는 변하지 않는다.
        // (6) O(1), 무할당·무예외·무효화 없음이다. 같은 문자열을 다른 스레드가 변경하지 않을 때 읽기가 안전하다.
        const std::size_t maximum_size{raw_.size()};

        // [첫 생성 계약: std::string 기본 생성자]
        // (1) normalized는 아직 수명이 시작되지 않은 목적 객체다.
        // (2) std::string() 기본 생성자가 선택되며 숨은 문자/traits/allocator 타입 인자를 사용한다.
        // (3) 명시 데이터 인자는 없고 기본 allocator가 사용된다.
        // (4) 생성자는 반환값이 없으며 빈 문자열 객체를 직접 만든다.
        // (5) 성공 뒤 normalized.size()==0이고 문자를 소유할 준비가 된 유효 상태다.
        // (6) 일반 컨테이너 요구상 O(1)이고 기본 allocator에서 noexcept다. 실제 할당/SSO 방식은 고정하지 않으며
        //     관찰자는 아직 없고 normalized는 지역 수명을 따른다. 공유 전 객체라 thread 충돌도 없다.
        std::string normalized{};

        // [첫 호출 계약: std::string::resize_and_overwrite]
        // (1) 수신 normalized는 비어 있는 수정 가능한 std::string이다. 기존 포인터·반복자를 보관하지 않았다.
        // (2) template<class Operation> constexpr void resize_and_overwrite(size_type n, Operation op)가 선택되며
        //     n=maximum_size, Operation은 비소유 this 포인터 값을 복사 캡처한 noexcept lambda의 고유 타입이며,
        //     라이브러리는 std::move(op)(p,m)을 정확히 한 번 평가해 closure를 xvalue로 호출한다.
        // (3) 첫 인자는 std::size_t lvalue를 값으로 복사한 쓰기 상한이다. 같은 string 타입 raw_의 size라 현재
        //     수신의 max_size 이하이다. lambda prvalue는 normalizer를 비소유로 가리킨다. p와 m==n은 값으로
        //     전달되고 const 매개변수라 바뀌지 않는다. 빈 수신이므로 [p,p+n)의 값은 모두 결정되지 않았을 수 있다.
        // (4) 멤버 함수 반환형은 void라 버린 값이 없다. 콜백은 실제 쓴 문자 수 std::size_t를 반환한다.
        // (5) 콜백 반환 r 뒤 normalized는 buffer[0..r)를 소유하고 size()==r이다. raw_와 separator_는 변하지 않는다.
        // (6) 콜백은 던지거나 p/m을 바꾸면 안 되고 0<=r<=m, [p,p+r)는 모두 초기화해야 하며 위반은 UB다.
        //     일반 n>max_size 호출은 length_error, 할당은 bad_alloc 가능하지만 현재 n은 길이 한계 안이다. OP 전
        //     정상 라이브러리 예외에는 수신 변화가 없다. 멤버 전체의 표준 복잡도 상한은 없고 오늘 callback만 O(n)이다.
        //     기존 수신 관찰자와 callback 포인터는 호출 뒤 무효이며 같은 객체의 동시 접근은 외부에서 막는다.
        normalized.resize_and_overwrite(
            maximum_size,
            [this](char* const output, const std::size_t writable_size) noexcept -> std::size_t {
                std::size_t written{};

                // [첫 호출 계약: const string::operator[]]
                // (1) 수신 raw_는 const std::string이며 writable_size==raw_.size()인 동안 바뀌지 않는다.
                // (2) const_reference operator[](size_type position) const가 선택된다.
                // (3) 각 index는 std::size_t lvalue를 값으로 전달하며 루프 조건으로 0<=index<size()가 증명된다.
                // (4) const char&를 반환하지만 즉시 char current에 복사해 콜백 밖에 참조를 보관하지 않는다.
                // (5) raw_와 index는 변하지 않고 소유권 이전·할당·무효화가 없다.
                // (6) 각 접근 O(1)·무예외다. 범위 밖은 전제조건 위반이며 raw_ 동시 변경은 데이터 경쟁이 된다.
                for (std::size_t index{}; index < writable_size; ++index) {
                    const char current{raw_[index]};
                    if (is_ascii_alphanumeric(current)) {
                        // output은 콜백이 빌린 포인터다. written<writable_size이므로 이 저장은 유효 범위 안이다.
                        output[written] = current;
                        ++written;
                    } else if (current == ' ' || current == '/') {
                        output[written] = separator_;
                        ++written;
                    }
                    // 그 밖의 문자는 쓰지 않고 건너뛴다. 읽지 않은 미초기화 영역은 최종 문자열에 포함되지 않는다.
                }
                return written;
            });

        const std::size_t actual_size{normalized.size()};
        const std::size_t dropped{maximum_size - actual_size};

        // normalized는 이름 있는 lvalue다. std::move가 xvalue로 보이게 해 결과 멤버의 문자열 이동 생성을 허용한다.
        // 바깥 NormalizationResult는 prvalue라 호출자의 목적 객체에 직접 구성되어 불필요한 결과 복사/이동이 생략된다.
        return NormalizationResult{std::move(normalized), dropped};
    }

private:
    // bool 반환형은 문자가 허용 집합에 속하는지 나타내고, char 값 매개변수는 소유권과 무관한 작은 값 복사다.
    [[nodiscard]] static bool is_ascii_alphanumeric(const char value) noexcept {
        return (value >= 'a' && value <= 'z') ||
               (value >= 'A' && value <= 'Z') ||
               (value >= '0' && value <= '9');
    }

    std::string raw_; // 입력 문자를 독립적으로 소유한다.
    char separator_{}; // 기본 타입 char를 중괄호로 0 초기화한 뒤 생성자에서 실제 정책 값을 넣는다.
};

int main() {
    // [첫 생성 계약: std::string(const char*)]
    // (1) 임시 std::string은 아직 생성 전이며 문자열 리터럴은 정적 수명의 유효한 NUL 종료 char 배열이다.
    // (2) std::string(const char* source) 생성자가 선택된다.
    // (3) 인자 배열은 const char* prvalue로 변환되어 빌려 읽히고 소유권은 넘어가지 않으며 null이 아니어야 한다.
    // (4) 생성자는 반환값이 없고 첫 NUL 전 13개 문자를 소유하는 임시를 만든다.
    // (5) 임시는 LogKeyNormalizer의 값 매개변수를 직접 초기화하고, 원본 리터럴은 변하지 않는다.
    // (6) 문자 수에 O(n), 저장 할당·길이 오류가 가능하다. 완성 문자열은 리터럴과 독립 수명이고 동시 변경은 없다.
    const LogKeyNormalizer normalizer{std::string{"API v2/West!!"}, '-'};
    const NormalizationResult result{normalizer.normalize()};

    // [첫 호출 계약: std::cout 스트림 삽입 연쇄]
    // (1) 수신 std::cout은 프로그램이 제공하는 살아 있는 std::ostream 객체이며 현재 오류 상태가 없다고 가정한다.
    // (2) const char*·std::string에는 비멤버 operator<<, std::size_t에는 산술 멤버 overload,
    //     '\n'에는 operator<<(std::ostream&, char)가 순서대로 선택된다.
    // (3) 리터럴은 비소유 문자 포인터 prvalue, result.value는 const std::string lvalue,
    //     result.dropped는 std::size_t lvalue, '\n'은 char prvalue이며 어느 인자도 소유권을 넘기지 않는다.
    // (4) 각 호출은 같은 std::ostream&를 반환해 다음 삽입 수신자로 사용하고 마지막 반환 참조는 버린다.
    // (5) result는 변하지 않고 cout의 문자/상태가 갱신된다. '\n'은 flush를 강제하지 않는다.
    // (6) 표준은 형식 출력 연쇄에 단일 점근 복잡도 상한을 두지 않는다. 생성 문자·locale·장치 비용이 들고
    //     내부 버퍼링에서 예외/실패 상태가 가능하다. 참조 무효화는 없지만 동시 연쇄는 레코드 원자성이 없다.
    std::cout << "key=" << result.value << ",dropped=" << result.dropped << '\n';

    // 블록 끝에서는 result.value와 normalizer.raw_의 std::string 소멸자가 역순으로 실행된다.
    // 소멸자는 인자·반환값·예외 없이 소유 문자를 끝내고 저장소를 해제할 수 있어 기존 관찰자는 모두 수명이 끝난다.
    return 0;
}
