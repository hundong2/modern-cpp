#include <iostream> // std::cout: 복사·이동 결과를 두 줄의 표준 출력으로 검증한다.
#include <string>   // std::string: "trace" 문자를 독립적으로 소유하는 값 타입을 제공한다.
#include <utility>  // std::move와 C++23 std::forward_like를 제공한다.

// struct는 기본 접근이 public이라 연습 결과 두 값을 단순히 전달하는 aggregate에 알맞다.
// 각 멤버 string은 자기 문자 저장소를 소유하며 CopyMoveResult 소멸 때 RAII로 함께 정리된다.
struct CopyMoveResult {
    std::string copied;
    std::string moved;
};

// class는 기본 접근이 private이므로 저장한 값을 외부에서 임의로 바꾸지 못하게 하고
// content()라는 하나의 정책 경계만 공개한다. T는 호출자가 정하는 타입 템플릿 인자다.
template <class T>
class Box {
public:
    // using은 T와 같은 타입에 도메인 문맥의 이름을 붙일 뿐 새로운 타입을 정의하지 않는다.
    using Value = T;

    // explicit은 Value 하나가 Box로 뜻밖에 암시 변환되는 것을 막는다.
    // 값 매개변수는 lvalue 입력에는 복사 한 번, rvalue 입력에는 이동을 허용하는 소유권 sink다.
    // [호출 계약: std::move(value)와 Value 이동 생성]
    // (1) 수신 결과 value_는 아직 수명이 시작되지 않은 Value 멤버이고, 값 매개변수 value는 살아 있으며
    //     이 생성자 안에서 이름 있는 lvalue 식이다.
    // (2) `std::move<U>(U&&) noexcept`에서 U=Value&로 추론되어 Value&&를 반환하고,
    //     그 xvalue로 Value의 이동 생성자를 선택한다. 오늘 specialization의 Value는 std::string이다.
    // (3) 유일한 인자 value는 Value lvalue이고 멤버가 소유권을 이어받도록 소비 의도로 전달한다.
    //     빈 문자열도 허용되며 문자열 내용 자체에 별도 전제는 없다.
    // (4) std::move는 Value&&를 반환해 value_ 직접 초기화에 사용한다. 이동 생성자와 Box 생성자는 반환값이 없다.
    // (5) 성공 뒤 value_는 매개변수의 이동 전 값을 가지고 value는 유효하지만 값은 미지정 상태다.
    //     오늘의 string specialization에서는 과거 문자 포인터·참조·반복자가 무효화될 수 있으므로 다시 얻으며,
    //     특정 문자열 버퍼 주소가 그대로 이전된다고 단정하지 않는다.
    //     std::move만 평가한 직후에는 상태 변화가 없고 뒤 이동 생성이 실제 소유권을 바꾼다.
    // (6) std::move는 별도 표준 Complexity 항목 없이 참조 cast를 반환하고 새 소유 저장소를 요구하지 않으며
    //     noexcept다. 일반 T의 이동 비용/예외는 T 계약에 따르지만 오늘의 allocator 없는 string 이동은
    //     상수 시간·noexcept다. 같은 value를 동시에 관찰·이동하면 데이터 경쟁이다.
    explicit Box(Value value)
        : value_{std::move(value)} {}

    // 명시적 객체 매개변수 Self는 Box를 호출한 식의 const성과 lvalue/rvalue 성질을 함께 기억한다.
    template <class Self>
    [[nodiscard]] decltype(auto) content(this Self&& self) noexcept {
        // [호출 계약: std::forward_like<Self>(self.value_)]
        // (1) 자유 함수라 수신 객체는 없고 self가 가리키는 Box와 value_가 살아 있어야 한다.
        //     이름 있는 self는 항상 lvalue 식이므로 self.value_도 Value lvalue다.
        // (2) `forward_like<Self, U>(U&&) noexcept`에서 Self는 explicit-object argument로부터 정해진다.
        //     mutable Box에서는 U=Value&, const Box에서는 U=const Value&로 추론된다. Self가 const Box&면
        //     const Value&, Box면 Value&& 결과가 된다.
        // (3) 유일한 인자는 self.value_ lvalue이며 이 호출 자체는 문자열을 복사하거나 소유권을 넘기지 않는다.
        // (4) 반환형은 Self와 같은 const/ref 성질을 입힌 Value 참조이고 decltype(auto)가 이를 보존한다.
        //     호출부는 const lvalue 결과를 복사 입력으로, rvalue 결과를 이동 입력으로 사용한다.
        // (5) Box와 value_는 그대로이며 반환 참조는 동일한 subobject를 별칭한다.
        // (6) 표준은 별도 Complexity 항목을 두지 않지만 지정 결과는 참조 cast이고 새 소유 저장소를 요구하지
        //     않으며 noexcept·직접 무효화 없음이다. 반환 참조는 Box 수명을 연장하지 않고 임시 Box에서 꺼낸
        //     참조를 full-expression 밖에 보관하면 dangling이다. 동시 변경은 동기화가 필요하다.
        return std::forward_like<Self>(self.value_);
    }

private:
    // value_가 실제 문자열 저장소를 소유한다. Box가 파괴되면 저장한 Value도 함께 파괴된다.
    Value value_;
};

// 함수 반환형의 Box<std::string>에서 std::string은 Box의 타입 템플릿 인자다.
[[nodiscard]] Box<std::string> make_trace_box() {
    // [생성 계약: std::string(const char*)]
    // (1) Box 생성자의 Value 매개변수로 쓰일 std::string prvalue는 아직 생성 전이고,
    //     "trace"는 프로그램 끝까지 살아 있는 const char[6] lvalue다.
    // (2) `basic_string(const char*, const Allocator& = Allocator())`가 선택되고 문자/traits/allocator는
    //     std::string 별칭의 char, 표준 문자 traits, 기본 allocator다.
    // (3) 배열-포인터 변환된 const char* prvalue를 전달하며 NUL 종료와 유효한 문자 범위가 전제다.
    //     리터럴을 빌려 길이를 센 뒤 문자열이 별도 문자 값을 소유한다.
    // (4) 생성자는 반환값이 없고 길이 5인 소유 문자열 prvalue를 만들어 Box 생성 인자로 사용한다.
    // (5) 리터럴은 바뀌지 않고 Box 내부 value_가 자기 수명 동안 "trace"를 소유한다.
    // (6) O(5), 구현의 작은 문자열 최적화 여부와 무관하게 할당/길이 관련 예외 가능성을 계약에 포함한다.
    //     생성 중 실패하면 완성 객체가 없고 누출도 없다. 외부 참조·반복자와 동시 접근은 아직 없다.
    // 같은 타입의 Box prvalue가 함수 결과와 호출자 변수를 직접 구성하므로 C++17 보장 복사 생략 대상이다.
    return Box<std::string>{std::string{"trace"}};
}

// int는 운영체제에 성공 0 또는 검증 실패 1을 반환하는 기본 정수 타입이다.
int main() {
    // make_trace_box() 호출은 Box<std::string> prvalue이고 auto는 참조가 아닌 그 값 타입으로 추론된다.
    // 중괄호 직접 초기화로 box가 바로 만들어져 중간 Box 복사·이동은 필요 없다.
    auto box{make_trace_box()};

    // 포인터는 객체를 소유하지 않고 주소만 보관한다. &box는 살아 있는 lvalue의 주소를 얻으며 null이 아니다.
    // pointee가 const라 content()의 Self는 const Box<std::string>&로 추론된다.
    const Box<std::string>* const observer{&box};

    // aggregate의 initializer-clause는 선언 순서로 평가된다. 따라서 첫 멤버가 먼저 문자열을 복사한 뒤
    // 둘째 멤버가 원본 Box 문자열을 이동해도 copied는 독립된 "trace"를 계속 소유한다.
    // [호출 계약: string 복사 생성, std::move(box), string 이동 생성]
    // (1) 결과의 copied/moved std::string 멤버는 아직 생성 전이다. observer->content()는 "trace"를 소유한
    //     Box subobject의 const std::string lvalue를 가리키고 box는 아직 이동되지 않은 유효한 lvalue다.
    // (2) copied에는 `basic_string(const basic_string&)` 복사 생성자를 고른다. 이어
    //     `std::move<U>(U&&) noexcept`에서 U=Box<std::string>&가 되어 Box&&를 반환하고,
    //     rvalue content()가 내놓은 std::string&&에는 allocator 없는 string 이동 생성자를 고른다.
    // (3) 첫 인자는 const string lvalue라 문자 값을 복사하며 원본 소유권을 유지한다. std::move의 인자는
    //     box lvalue이고 rvalue accessor 결과는 이동 가능한 xvalue다. null 포인터나 수명 종료 입력은 허용되지 않는다.
    // (4) 복사/이동 생성자는 반환값이 없다. std::move의 Box&&와 content의 string&&는 둘째 멤버 초기화에
    //     즉시 사용되고 따로 저장되지 않으며 aggregate 전체가 result lvalue가 된다.
    // (5) 성공 뒤 result.copied와 result.moved가 각각 독립적으로 "trace"를 소유한다. box는 살아 있지만
    //     내부 string은 유효·미지정 상태이고 과거 문자 관찰자는 무효화될 수 있다. observer는 여전히 source
    //     Box를 가리키며 destination으로 재바인딩되지 않으므로 이후 content를 읽지 않는다.
    // (6) 복사는 문자 수 O(n)이며 할당 실패가 가능하고 실패 시 source는 유지된다. exact string 이동은 O(1),
    //     noexcept다. source의 과거 포인터·참조·반복자는 보관하지 말고 동시 접근도 금지한다.
    const CopyMoveResult result{
        observer->content(),
        std::move(box).content(),
    };

    // [호출 계약: std::cout의 const-char/string/char 삽입 연산자]
    // (1) 수신자는 정상 상태를 기대하는 정확한 std::ostream 타입의 std::cout lvalue이고,
    //     result.copied는 "trace"를 소유하는 const std::string lvalue다.
    // (2) const char*용, std::string용, char용 basic_ostream 비멤버/멤버 삽입 overload가 순서대로 선택된다.
    // (3) "copied="는 정적 수명 리터럴에서 변환된 const char* prvalue, result.copied는 빌린 lvalue,
    //     '\n'은 char prvalue다. 출력은 어느 문자열 소유권도 받지 않는다.
    // (4) 각 <<는 같은 std::ostream&를 반환해 다음 <<의 수신자로 사용하고 마지막 반환 참조만 버린다.
    // (5) result는 변하지 않고 성공 시 `copied=trace`와 줄바꿈이 출력 버퍼에 기록된다.
    // (6) 비용은 출력 문자 수·locale·장치에 의존한다. 실패는 상태 비트, exception mask가 켜졌다면 예외로
    //     보고될 수 있다. 기본 synchronized std::cout의 형식 출력은 동시 호출해도 data race는 없지만 문자가
    //     섞일 수 있어 레코드 원자성은 없다. 입력 string 참조는 호출 동안 유효하다.
    std::cout << "copied=" << result.copied << '\n';

    // [호출 계약: 두 번째 std::cout 삽입 연산자 연쇄]
    // (1) std::cout는 첫 줄 뒤의 std::ostream lvalue이며 result.moved는 유효한 const std::string lvalue다.
    // (2) const char*와 std::string용 operator<< overload를 차례로 고른다.
    // (3) "moved=" 리터럴 포인터와 result.moved lvalue를 읽기만 하고 마지막 newline 인자는 의도적으로 없다.
    // (4) 첫 반환 std::ostream&는 다음 삽입에 사용하고 마지막 반환 참조는 버린다.
    // (5) 성공하면 `moved=trace`가 두 번째 줄에 기록되며 result의 두 소유 문자열은 그대로다.
    // (6) 문자 수에 따른 비용, 상태 비트/설정된 예외, 외부 장치와 동시 stream 접근 계약은 앞 출력과 같다.
    std::cout << "moved=" << result.moved;

    // [호출 계약: 두 std::string과 문자열 리터럴의 비교]
    // (1) result의 두 수신 문자열은 모두 "trace"를 소유하는 const std::string lvalue다.
    // (2) 각 `!=`는 C++20 동등 비교 재작성 규칙을 포함한 basic_string/const-char 비교로 bool을 만든다.
    // (3) 오른쪽 "trace"는 유효한 NUL 종료 const char[6] lvalue이고 소유권 이동은 없다.
    // (4) bool 결과는 `||`와 if 분기에 사용하며 첫 비교가 true면 두 번째는 단락 평가로 실행되지 않는다.
    // (5) 두 string과 리터럴은 바뀌지 않고 포인터·참조·반복자도 무효화되지 않는다.
    // (6) 각 비교는 최악 O(n), 무할당이다. 유효한 NUL 종료 입력과 겹치는 쓰기 없음이 전제이며
    //     실패 시 1, 둘 다 같으면 0을 운영체제에 반환한다.
    if (result.copied != "trace" || result.moved != "trace") {
        return 1;
    }

    return 0;
}
