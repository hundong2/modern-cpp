#include <functional> // std::bind_back: 뒤쪽 인자를 값으로 보관하는 호출 래퍼를 만든다.
#include <iostream>   // std::cout: 계산 결과를 표준 출력 스트림에 기록한다.
#include <string>     // std::string: 주문·정책 이름의 문자와 수명을 소유한다.
#include <utility>    // std::move: 이름 있는 객체를 이동 후보인 xvalue 식으로 바꾼다.

// struct는 기본 접근이 public이다. 입력 데이터처럼 불변식을 별도 함수가 지키는 값 묶음에 알맞다.
struct Order {
    // std::string은 basic_string<char, ...>의 표준 별칭이다. 숨은 템플릿 인자는 문자 특성과 할당자다.
    std::string id;
    int subtotal{}; // int는 원 단위 금액을 담고, {}는 먼저 0으로 초기화한다.
};

// 결과도 값으로 소유한다. 호출 래퍼나 입력 Order가 사라져도 두 문자열은 독립적으로 살아 있다.
struct Quote {
    std::string order_id;
    std::string policy_name;
    int total{};
};

// class는 기본 접근이 private이다. 정책 이름과 세율을 생성자 검문소 뒤에 감춘다.
class PricingPolicy {
public:
    // explicit은 `PricingPolicy p = {name, rate}` 같은 copy-list 암시 변환을 막고 직접 구성을 요구한다.
    // 값 매개변수 name은 호출자가 복사 또는 이동해 준 문자열을 함수 안에서 소유한다.
    explicit PricingPolicy(std::string name, int basis_points)
        // [호출 계약: std::move와 string 이동 생성]
        // (1) 수신 대상 name_은 아직 생성 전이고, name은 유효한 std::string lvalue다.
        // (2) move<std::string&>(std::string&) -> std::string&&와
        //     basic_string(basic_string&&) 생성자를 선택한다.
        // (3) name 식은 lvalue이고 move 결과는 xvalue다. 소유 문자를 name_으로 이전해도 된다는
        //     뜻이며 move 자체는 복사·할당하지 않는다. basis_points는 int 값 매개변수다.
        // (4) move는 std::string&&를 반환해 생성자 입력으로 쓰고, 생성자는 반환값이 없다.
        // (5) name_이 정책 이름을 소유한다. name은 유효하지만 값이 미지정인 상태로 소멸한다.
        // (6) 기본 allocator가 같은 string 이동 생성은 상수 시간·noexcept다. 기존 name의 문자
        //     참조·포인터·반복자는 이동으로 영향을 받을 수 있으므로 이후 사용하지 않는다. 공유 문자열을
        //     동시에 변경하려면 별도 동기화가 필요하다.
        : name_{std::move(name)}, basis_points_{basis_points} {}

    // const 멤버 함수는 정책 상태를 바꾸지 않는다. 반환 참조는 *this가 소유한 문자열을 빌린다.
    [[nodiscard]] const std::string& name() const noexcept {
        return name_;
    }

    // subtotal은 값으로 복사된다. 10,000 basis points가 100%이므로 정수식으로 가산한다.
    [[nodiscard]] int apply(int subtotal) const noexcept {
        return subtotal + (subtotal * basis_points_) / 10'000;
    }

private:
    std::string name_;  // 정책 객체가 문자를 소유하며 정책과 함께 파괴된다.
    int basis_points_{}; // 1,000은 10%를 뜻한다. {}는 기본값 0을 보장한다.
};

// using은 새 타입을 만들지 않고 긴 함수 포인터 타입에 읽기 쉬운 별칭을 붙인다.
// 포인터는 함수를 소유하지 않는다. make_quote 함수 코드는 프로그램 실행 내내 호출 가능하다.
using QuoteFunction = Quote (*)(Order, const PricingPolicy&);

// Order는 값 매개변수라 호출 인자의 주문을 소유하고, policy는 const 참조로 호출 동안만 빌린다.
// 반환형 Quote는 두 문자열과 계산 결과를 독립적으로 소유한다.
[[nodiscard]] Quote make_quote(Order order, const PricingPolicy& policy) {
    // [생성 계약: std::string 이동·복사와 Quote prvalue 반환]
    // (1) Quote의 두 string 대상은 아직 생성 전이다. order.id는 유효한 string lvalue이고
    //     policy.name()은 bind_back 래퍼가 소유한 정책 안의 const string lvalue를 돌려준다.
    // (2) 첫 필드는 basic_string(basic_string&&), 둘째는 basic_string(const basic_string&)를 택한다.
    // (3) move(order.id)는 xvalue라 주문 ID 소유권을 이전하고, const 참조 정책 이름은 깊게 복사한다.
    // (4) 두 생성자는 별도 반환값이 없고, Quote{...} prvalue가 함수 반환값으로 사용된다.
    // (5) 결과가 두 문자열을 소유한다. order.id는 유효하지만 값 미지정이고 정책 이름은 그대로다.
    // (6) 첫 이동은 상수 시간, 복사는 문자 수에 선형이며 복사에는 할당·bad_alloc 가능성이 있다.
    //     C++17부터 같은 타입 prvalue는 반환 목적 객체에 직접 구성된다. 참조·반복자를 반환하지 않는다.
    return Quote{std::move(order.id), policy.name(), policy.apply(order.subtotal)};
}

// main은 운영체제에 종료 상태를 돌려주는 int 반환 함수다. 매개변수 없이 예제를 조립한다.
int main() {
    // [생성 계약: 문자열 리터럴에서 std::string 생성]
    // (1) 임시 string 대상은 아직 없고 "standard"는 정적 수명의 const char[9] 배열이다.
    // (2) basic_string(const char*, const Allocator& = Allocator())에서 char 특수화를 선택한다.
    // (3) 배열은 const char*로 변환되고 첫 NUL 전 8자를 복사한다. 리터럴 소유권은 이전하지 않는다.
    // (4) 생성자는 반환값이 없고 임시 string이 PricingPolicy의 값 매개변수를 직접 초기화한다.
    // (5) 정책 생성 뒤 멤버 name_이 문자를 소유하며 리터럴은 프로그램 끝까지 그대로다.
    // (6) 문자 수에 선형이고 할당 시 bad_alloc이 가능하다. 유효한 NUL 종료 배열이 전제다.
    PricingPolicy policy{std::string{"standard"}, 1'000};
    QuoteFunction quote_function{&make_quote};

    // [호출 계약: std::move와 std::bind_back]
    // (1) 수신 객체 없는 자유 함수 호출이다. quote_function은 make_quote를 가리키는 유효한
    //     QuoteFunction lvalue이고 policy는 유효한 PricingPolicy lvalue다.
    // (2) bind_back<F, Args...>(F&&, Args&&...)에서 F=QuoteFunction&, Args={PricingPolicy}로
    //     추론한다. move<PricingPolicy&>(policy)는 PricingPolicy&&를 만든다.
    // (3) 첫 인자는 함수 포인터를 decay-copy하고, 둘째 xvalue는 정책을 래퍼 내부 decay 타입으로
    //     이동 구성한다. 반환 래퍼가 정책을 소유하며 외부 객체를 빌리지 않는다.
    // (4) 구현 지정 호출 래퍼 prvalue를 반환하며 quote_with_policy를 직접 초기화한다.
    // (5) 래퍼는 앞으로 받은 Order 뒤에 저장 정책을 붙여 make_quote를 호출한다. policy는
    //     유효하지만 값 미지정이고, quote_function 포인터 값은 변하지 않는다.
    // (6) decay 함수 포인터와 정책은 각 입력에서 구성 가능하고 MoveConstructible이어야 하며 여기서는
    //     만족한다. 표준은 일반 복잡도·무할당 상한을 약속하지 않고 저장 초기화 예외는 전파된다.
    //     래퍼 파괴 시 저장 정책과 그 string도 파괴된다. 이동된 policy.name_의 기존 관찰자는 영향을
    //     받을 수 있으나 여기서는 만들지 않았고, 래퍼는 자체 스레드 동기화를 제공하지 않는다.
    auto quote_with_policy{std::bind_back(quote_function, std::move(policy))};

    // [호출 계약: bind_back 반환 래퍼의 operator()]
    // (1) 수신 quote_with_policy는 정책과 함수 포인터를 보유한 non-const lvalue 래퍼다.
    // (2) 반환 타입의 operator() 선언 형태는 구현 지정이다. 이 non-const lvalue 래퍼는 저장
    //     함수 포인터와 정책을 QuoteFunction&, PricingPolicy&로 전달한다. 호출 위치 Order prvalue는
    //     임시를 materialize해 전달 참조에 바인딩되고 래퍼 안에서 Order&& xvalue로 forward된다.
    // (3) 그 Order xvalue에서 대상의 값 매개변수를 이동 구성한다. 저장 정책은 래퍼가 계속 소유한
    //     lvalue이고 const PricingPolicy&에 바인딩되어 대상 호출이 정책을 이동 소비하지 않는다.
    // (4) make_quote의 Quote prvalue를 그대로 반환하며 result를 직접 초기화한다.
    // (5) 래퍼와 저장 정책은 유지되고 임시 Order는 전체 식 끝에 파괴된다. result는 독립 문자열을 소유한다.
    // (6) 표준은 래퍼 호출의 일반 복잡도·할당 상한을 따로 약속하지 않으며 실제 비용/예외는 대상
    //     make_quote와 string 복사 계약을 따른다. 함수 포인터는 non-null이어야 하며 여기서는 &make_quote다.
    //     일반적으로 래퍼 lvalue/const lvalue/rvalue/const rvalue는 저장 정책을 각각 T&/const T&/
    //     T&&/const T&&로 전달하며 모두 이 const 참조 대상에는 바인딩된다. 참조를 래퍼보다 오래
    //     보관하면 안 되고 호출 불가능한 인자 조합은 컴파일 오류다.
    Quote result{quote_with_policy(Order{std::string{"A-42"}, 10'000})};

    // [호출 계약: ostream 문자열·정수·문자 삽입]
    // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 result의 문자열과 int가 살아 있다.
    // (2) operator<<(ostream&, const string&), operator<<(ostream&, char),
    //     ostream::operator<<(int)를 왼쪽부터 연쇄한다.
    // (3) 두 string lvalue와 int lvalue를 읽기만 하고 ':', '=', '\n' char prvalue를 값으로 보낸다.
    // (4) 각 호출은 같은 ostream&를 반환해 다음 삽입에 쓰며 마지막 반환 참조는 버린다.
    // (5) 출력 버퍼와 스트림 상태 비트만 변하고 result는 변하지 않는다.
    // (6) 표준은 일반 복잡도·할당 상한을 명시하지 않는다. 이 식은 표시된 문자들을 내보내며 실패는
    //     상태 비트 또는 exception mask에 따른 예외로 드러난다. 같은 스트림을 여러 스레드에서 복합
    //     레코드로 쓸 때는 동기화가 필요하다.
    std::cout << result.order_id << ':' << result.policy_name << '=' << result.total << '\n';

    // 기계 실행 관점: 정수 곱셈·나눗셈, 문자열 주소/길이 로드, 간접 함수 포인터 호출과
    // 출력 분기가 남을 수 있다. bind_back은 가상 기반 타입 소거가 아니므로 구체 래퍼를 인라인할 수도 있다.
    // 실제 명령·할당·간접 호출 제거 여부는 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 다르다.
}
