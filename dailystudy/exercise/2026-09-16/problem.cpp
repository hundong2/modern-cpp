#include <any>      // std::any/any_cast: 타입을 지운 값을 소유하고 안전한 포인터 조회를 제공한다.
#include <iostream> // std::cout: 복사·이동·명시적 reset 결과를 세 줄로 출력한다.
#include <string>   // std::string: "trace" 문자를 독립적으로 소유하는 실제 payload 타입이다.
#include <utility>  // std::move: 이름 있는 any를 이동 가능한 xvalue로 표현한다.

// 작은 연습 목표:
// 1) any 복사는 payload를 독립 복사한다.
// 2) any 이동 뒤 source가 비었다고 가정하면 안 된다. source는 유효하지만 상태가 명시적으로 비어 있다고 보장되지 않는다.
// 3) 빈 상태가 필요하면 reset()을 호출하고 has_value()로 확인한다.
// 4) pointer-form any_cast는 타입 불일치에 예외 대신 nullptr를 반환한다.

// struct는 기본 public이라 단순 데이터 묶음에, class는 기본 private이라 소유 상태를 숨기는 경계에 알맞다.
// 여기서는 payload_에 대한 접근 정책을 보여 주려고 class와 public/private 접근 지정자를 사용한다.
class AnySlot {
public:
    // using 별칭은 새 타입이 아니며 Value와 std::any는 정확히 같은 타입이다.
    using Value = std::any;

    // explicit은 any 하나가 AnySlot으로 뜻밖에 암시 변환되는 것을 막는다.
    // 값 매개변수는 호출자가 준 any를 이 객체가 이어받는 ownership sink이고,
    // 멤버 초기화 목록은 생성자 본문 전에 payload_를 직접 구성한다.
    explicit AnySlot(Value payload)
        // [호출 계약: std::move와 std::any 이동 생성자]
        // (1) 생성될 수신 payload_의 정확한 타입은 std::any이고 아직 수명이 시작되지 않았다.
        //     매개변수 payload는 유효한 Value 객체이며 이름 있는 식이므로 lvalue다.
        // (2) `move<U>(U&&) noexcept`에서 U=Value&로 추론되어 Value&&를 반환하고,
        //     그 xvalue로 `any(any&& other) noexcept` 이동 생성자를 선택한다.
        // (3) 유일한 move 인자는 payload lvalue이며 멤버에 소유 값을 넘기려는 의도다. 빈 any도 허용된다.
        // (4) move는 any&&를 반환해 멤버 초기화에 즉시 사용한다. any/AnySlot 생성자는 반환값이 없다.
        // (5) 성공 뒤 payload_는 이동 전 값을 가진다. source payload는 유효하지만 비었는지는 보장되지 않으며
        //     값이 남았다면 그 contained object는 moved-from 상태일 수 있다. 함수 종료 때 source 매개변수는 파괴된다.
        // (6) any 이동 생성자는 noexcept지만 표준은 별도 복잡도 절을 두지 않는다. 구현은 저장 객체 자체 또는
        //     관리 구조를 이전할 수 있고, 실제 비용은 payload와 저장 전략에 따른다.
        //     과거 source payload 내부 포인터는 보존된다고 가정하지 않는다. payload_ 수명은 AnySlot과 같고,
        //     같은 any에 대한 동시 이동/관찰은 데이터 경쟁이므로 외부 동기화가 필요하다.
        : payload_{std::move(payload)} {}

    // & 반환은 기존 any를 복사하지 않는 lvalue reference다. null일 수 없고 AnySlot 수명을 연장하지 않는다.
    [[nodiscard]] Value& value() noexcept {
        return payload_;
    }

private:
    Value payload_;
};

// 기계 실행 관점에서 any는 보통 타입 식별 정보와 type-erased manager 주소를 함께 관리한다.
// 복사는 payload 복사와 할당을, 이동은 포인터/버퍼 이전 또는 payload 이동을, any_cast는 타입 비교와 조건 분기를
// 만들 수 있다. 표준은 특정 vtable·가상 간접 호출·작은 객체 최적화·heap 배치나 명령어를 강제하지 않으므로
// 실제 load/store/비교/분기/간접 호출은 CPU, 라이브러리 구현, 컴파일러와 최적화 옵션에 따라 달라진다.

// int는 운영체제에 성공 0 또는 자체 검증 실패 1을 돌려주는 기본 정수 타입이다.
int main() {
    // [생성 계약: std::string(const char*)와 std::any 값 생성자]
    // (1) string과 이를 담을 any 수신 객체는 아직 수명이 시작되지 않았다. "trace"는 정적 수명의 const char[6] lvalue다.
    // (2) `basic_string(const char*, const Allocator& = Allocator())`와
    //     `template<class ValueType> any(ValueType&&)`에서 ValueType=string인 overload가 선택된다.
    // (3) 문자열 생성 인자는 NUL 종료 배열에서 변환된 const char* prvalue다. any 인자는 string prvalue라
    //     이동 가능한 값이며 any가 decay된 string을 소유한다. string은 복사 생성 가능한 타입이어야 한다.
    // (4) 두 생성자는 반환값이 없다. 완성된 any prvalue는 AnySlot의 Value 값 매개변수를 직접 초기화한다.
    // (5) 최종 source_slot.payload_가 "trace" string을 소유해 리터럴 및 임시의 수명과 독립적이다.
    // (6) 문자열 길이에 선형이고 any 저장 전략은 구현별이라 문자열/관리 저장소 할당 실패 예외가 가능하다.
    //     생성 실패 시 완성 AnySlot이 없고 이미 완성된 임시는 정리된다. 외부 관찰자·무효화·동시 접근은 아직 없다.
    AnySlot source_slot{std::any{std::string{"trace"}}};

    // source는 source_slot 안 payload_를 별칭하는 non-const lvalue reference다. 소유권과 수명은 source_slot에 남는다.
    // prvalue any가 위 값 매개변수를 직접 초기화하므로 불필요한 중간 any 복사는 없다. 함수의 named local을
    // 반환하지 않으므로 NRVO는 이 파일에 적용되지 않으며, 이 직접 구성은 RVO와 구분되는 보장된 prvalue 규칙이다.
    std::any& source{source_slot.value()};

    // [호출 계약: std::any 복사 생성자]
    // (1) 생성될 수신 copied는 아직 수명이 시작되지 않은 std::any이고 source는 "trace" string을 가진 any lvalue다.
    // (2) 선택 시그니처는 `any(const any& other)`이며 contained string의 복사 생성 경로를 사용한다.
    // (3) 인자 source는 non-const lvalue지만 const any&에 바인딩되어 빌려 읽고 소유권을 넘기지 않는다.
    // (4) 생성자는 반환값이 없고 copied가 독립 payload를 가진 완성 객체가 된다.
    // (5) 성공 뒤 source와 copied가 각각 별도의 "trace" string을 소유하므로 한쪽 reset이 다른 쪽에 영향 주지 않는다.
    // (6) 표준은 any 복사 생성에 별도 복잡도 상한을 두지 않으며 비용은 contained string 복사와 저장 전략에
    //     따른다. any/string 저장소 할당 예외가 가능하다. 실패하면 copied는 완성되지 않고 source는 유지된다.
    //     source 내부의 기존 관찰자는 무효화되지 않으며 동시 변경은 허용되지 않는다.
    std::any copied{source};

    // [호출 계약: std::move(source)와 std::any 이동 생성자]
    // (1) 생성될 수신 moved는 아직 수명 전이고 source는 살아 있는 std::any lvalue로 payload를 갖는다.
    // (2) `move<U>(U&&) noexcept`에서 U=any&로 추론되어 any&&를 반환하고 `any(any&&) noexcept`가 선택된다.
    // (3) move 인자는 source lvalue이며 xvalue로 바꾸어 소비를 허용하지만 std::move 자체는 상태를 바꾸지 않는다.
    // (4) 반환 any&&는 moved 초기화에 즉시 사용하며 생성자는 반환값이 없다.
    // (5) 성공 뒤 moved는 "trace" 값을 소유한다. source는 여전히 유효하지만 빈 상태라고 보장되지 않고,
    //     값이 남아 있다면 그 string은 moved-from이므로 내용을 가정하지 않는다.
    // (6) any 이동 생성자는 noexcept이고 별도 실패 예외가 없지만 표준 복잡도 절은 없다. 실제 비용은
    //     contained object와 저장 전략에 따른다. source payload의 과거 포인터 수명/주소는 보장되지 않는다.
    //     source와 moved는 별도 객체지만 이동 중 동시 source 접근에는 동기화가 필요하다.
    std::any moved{std::move(source)};

    // [호출 계약: std::any_cast<std::string>(std::any*)]
    // (1) 수신 객체 없는 자유 함수이며 copied/moved는 각각 string payload를 가진 살아 있는 mutable any lvalue다.
    // (2) 두 식 모두 `template<class T> T* any_cast(any*) noexcept`에서 T=string을 선택한다.
    // (3) 인자는 copied/moved 주소인 any* prvalue이고 null이 아니며 소유권을 넘기지 않는다.
    // (4) contained type과 typeid(string)이 일치하면 mutable string*, 아니면 nullptr를 반환한다. typeid 비교는
    //     최상위 cv를 구별하지 않지만 산술·사용자 정의 변환은 하지 않는다. 반환값은 각 const string*
    //     변수로 안전하게 qualification conversion되어 이 예제에서 payload 변경을 막는다.
    // (5) 두 any와 string은 바뀌지 않으며 반환 포인터가 각각의 contained object를 비소유 별칭한다.
    // (6) 별도 표준 복잡도 절은 없고, 무할당·noexcept라 bad_any_cast를 던지지 않는다. 포인터는 해당
    //     any의 reset/교체/파괴 전까지만 유효하고 다른 스레드가 같은 any를 변경하면 데이터 경쟁이다.
    const std::string* const copied_text{std::any_cast<std::string>(&copied)};
    const std::string* const moved_text{std::any_cast<std::string>(&moved)};

    // [호출 계약: std::any::reset]
    // (1) 수신 객체는 이동 뒤에도 유효한 정확한 std::any 타입의 source lvalue이며, 값 보유 여부는 미지정이다.
    // (2) 선택 시그니처는 `void reset() noexcept`이고 인자나 함수 템플릿 인자가 없다.
    // (3) 매개변수는 없다. 수신 any가 가진 payload가 있다면 그 소유 수명을 끝내라는 명시적 요청이다.
    // (4) 반환형은 void라 사용할 값이 없다.
    // (5) 호출 뒤 source.has_value()는 반드시 false이며 남아 있던 moved-from string도 있었다면 파괴된다.
    // (6) reset은 noexcept지만 별도 표준 복잡도 절은 없고, 비용은 저장 타입 소멸과 구현의 저장 전략에
    //     따른다. 새 할당은 하지 않는다. source payload 포인터/참조는 무효화된다. copied/moved와 그 관찰자는
    //     독립이라 유지되고, 같은 source에 대한 동시 접근에는 외부 동기화가 필요하다.
    source.reset();

    // [호출 계약: std::any::has_value]
    // (1) 수신 객체는 reset 직후 비어 있는 const 관찰 가능한 std::any source lvalue다.
    // (2) 선택 시그니처는 `bool has_value() const noexcept`이고 인자는 없다.
    // (3) 매개변수나 소유권 이전이 없으며 수신 객체는 살아 있어야 한다.
    // (4) payload가 있으면 true, 없으면 false를 반환하고 논리 부정 결과를 source_reset 초기화에 사용한다.
    // (5) source 상태는 비어 있는 채 유지되고 다른 any 및 포인터도 변하지 않는다.
    // (6) 별도 표준 복잡도 절은 없고 무할당·noexcept·무효화 없음이다. source 수명 안에서만 호출하며
    //     동시 변경은 금지한다.
    const bool source_reset{!source.has_value()};

    // null 포인터를 역참조하면 미정의 동작이다. pointer-form any_cast 결과를 먼저 검사하는 것이 실무 안전 패턴이다.
    if (copied_text == nullptr || moved_text == nullptr || !source_reset) {
        return 1;
    }

    // [호출 계약: std::cout의 문자열/문자 삽입 operator<<]
    // (1) 정확한 수신 타입은 std::ostream인 전역 std::cout lvalue다. 두 포인터는 살아 있는 const string을
    //     가리키고 source_reset은 true다.
    // (2) const char*, const string&, char에 맞는 basic_ostream 삽입 overload가 왼쪽부터 선택된다.
    // (3) 리터럴/삼항식은 정적 수명의 const char* 값, 역참조 결과는 const string lvalue, '\n'은 char prvalue다.
    //     어느 입력도 stream으로 소유권을 넘기지 않는다.
    // (4) 각 operator<<는 같은 ostream&를 반환해 연쇄의 다음 수신자로 쓰고 마지막 반환 참조만 버린다.
    // (5) 성공하면 `copied=trace`, `moved=trace`, `source-reset=true`와 마지막 개행이 순서대로 기록된다.
    // (6) 출력 문자 수/locale/장치에 따른 비용이며 버퍼 할당, 상태 비트 오류, exception mask에 따른 예외가 가능하다.
    //     포인터 수명은 출력 끝까지 유효해야 한다. 표준 stream 동시 출력은 문자가 섞일 수 있어 레코드 원자성이 없다.
    std::cout << "copied=" << *copied_text << '\n'
              << "moved=" << *moved_text << '\n'
              << "source-reset=" << (source_reset ? "true" : "false") << '\n';

    return 0;
}
