#include <iostream> // std::cout: 감사용 관찰값과 게시된 스냅숏을 표준 출력에 기록한다.
#include <string>   // std::string: 서비스 이름 문자를 직접 소유하는 문자열 타입을 제공한다.
#include <utility>  // std::move, std::forward_like: 이동 의도를 표시하고 수신 객체의 const·값 범주를 적용한다.
#include <vector>   // std::vector<int>: 가변 개수의 포트 번호를 연속 저장소에 소유한다.

// using은 새 타입을 만들지 않고 긴 기존 타입에 문맥이 드러나는 별칭을 붙인다.
// 템플릿 인자 int는 각 포트가 기본 정수 타입이라는 뜻이며 PortList의 실제 타입은 std::vector<int>다.
using PortList = std::vector<int>;

// struct는 기본 접근이 public이므로 계층 사이를 오가는 단순 결과 DTO에 알맞다.
// 이 스냅숏은 string과 vector를 값으로 가지므로 두 자원의 수명과 해제를 스스로 책임진다.
struct DeploymentSnapshot {
    std::string service;
    PortList ports;
};

// class는 기본 접근이 private이다. 따라서 소유 상태를 감추고 의도한 접근 함수만 공개하기 좋다.
// DeploymentPlan은 서비스 이름과 포트 목록을 값으로 소유하는 변경 계획이다.
class DeploymentPlan {
public:
    // explicit은 `DeploymentPlan plan = {name, ports};` 같은 copy-list 초기화를 막고,
    // 호출자가 `DeploymentPlan{...}`로 도메인 객체 생성을 명시하게 한다.
    // 매개변수를 값으로 받은 뒤 멤버로 이동하는 sink-by-value 패턴이라 lvalue 호출자는 복사하고
    // rvalue 호출자는 이동할 수 있다. 멤버 초기화 목록은 본문 대입 전에 두 멤버를 직접 구성한다.
    // [호출 계약: std::move와 string/vector 이동 생성]
    // (1) 수신 결과 service_와 ports_는 아직 수명이 시작되지 않은 std::string/PortList 멤버이고,
    //     값 매개변수 service와 ports는 각각 유효한 소유 객체이며 이 생성자 안에서는 이름 있는 lvalue다.
    // (2) 각 식은 `std::move<U>(U&&) noexcept`에서 U가 각각 std::string&, PortList&로 추론되는
    //     instantiation을 고른다. 반환된 xvalue로 각 타입의 allocator 없는 이동 생성자를 선택한다.
    // (3) service와 ports는 lvalue 식이며 값 자체의 소유권을 멤버로 넘길 의도로 전달한다.
    //     허용값에는 빈 문자열과 빈 vector도 포함되고 포트 유효성 검사는 이 작은 예제의 범위 밖이다.
    // (4) 두 std::move 호출은 각각 std::string&&와 PortList&&를 반환해 멤버 초기화에 사용한다.
    //     이동 생성자는 반환값이 없으며 DeploymentPlan 생성자도 별도 값을 반환하지 않는다.
    // (5) 성공 뒤 service_와 ports_는 두 매개변수의 이동 전 값을 가지며, 매개변수는 유효하지만 값은
    //     미지정 상태다. vector 저장소 이전과 달리 string 버퍼 주소가 그대로 이전된다고 단정하지 않는다.
    //     std::move 자체는 상태를 바꾸지 않고, 바로 뒤 선택된 이동 생성이 소유 상태를 바꾼다.
    // (6) std::move는 별도 표준 Complexity 항목 없이 참조 cast를 반환하고 새 소유 저장소를 요구하지 않으며
    //     noexcept다. 이 정확한 string/vector 이동 생성은 상수 시간이고 noexcept다. 이동 전 vector 원소 관찰자는 과거 end를 제외하면
    //     destination 원소를 계속 가리키지만, string 문자 관찰자는 무효화될 수 있어 다시 얻어야 한다.
    //     source 객체 자체의 참조는 source에 남는다. 같은 객체의 동시 이동·관찰은 데이터 경쟁이다.
    explicit DeploymentPlan(std::string service, PortList ports)
        : service_{std::move(service)}, ports_{std::move(ports)} {}

    // `this Self&&`는 C++23 명시적 객체 매개변수다. 호출한 객체의 const와 값 범주가 Self에 추론된다.
    // 하나의 함수 몸체가 과거의 `&`, `const &`, `&&`, `const &&` 네 overload 역할을 맡는다.
    template <class Self>
    [[nodiscard]] decltype(auto) service(this Self&& self) noexcept {
        // [호출 계약: std::forward_like<Self>(self.service_)]
        // (1) 자유 함수라 수신 객체는 없다. self가 가리키는 DeploymentPlan은 살아 있고 service_도 유효하다.
        //     이름 있는 self 식은 선언 타입이 `Self&&`여도 lvalue이므로 self.service_ 역시 std::string lvalue다.
        // (2) 선택되는 C++23 함수 템플릿은 `forward_like<Self, U>(U&&) noexcept`이며 명시 인자는 Self다.
        //     mutable owner에서는 U=std::string&, const owner에서는 U=const std::string&로 추론된다.
        //     Self는 호출자가 lvalue/const lvalue/rvalue인지에 따라 참조 포함 여부와 const성이 달라진다.
        // (3) 유일한 인자 self.service_는 std::string lvalue이고 소유권을 즉시 넘기지 않는다.
        //     실제 service_가 살아 있어야 하며 잘못된 this나 수명 종료 객체를 통한 호출은 허용되지 않는다.
        // (4) 반환형은 Self의 const성과 lvalue/rvalue 종류를 적용한 std::string 참조다. const lvalue 호출이면
        //     const std::string&, mutable rvalue 호출이면 std::string&&이고 const rvalue면 const std::string&&다.
        //     decltype(auto)는 그 참조를 그대로 반환한다.
        // (5) plan과 service_는 바뀌지 않는다. 반환 참조는 같은 subobject를 별칭하며 후속 복사/이동의 입력이 된다.
        // (6) 표준은 별도 Complexity 항목을 두지 않지만 지정 결과는 참조 cast이고 새 소유 저장소를 요구하지
        //     않으며 noexcept·직접 무효화 없음이다. 참조는 plan 수명을 넘지 못하고 임시 plan에서 얻은 것을
        //     full-expression 뒤 저장하면 dangling이다. 겹치는 변경에는 외부 동기화가 필요하다.
        return std::forward_like<Self>(self.service_);
    }

    template <class Self>
    [[nodiscard]] decltype(auto) ports(this Self&& self) noexcept {
        // [호출 계약: std::forward_like<Self>(self.ports_)]
        // (1) 자유 함수라 수신 객체는 없다. self의 DeploymentPlan과 그 PortList subobject는 살아 있다.
        // (2) `forward_like<Self, U>(U&&) noexcept`에서 Self는 명시적 객체로부터 추론된다. 이름 있는
        //     self.ports_는 lvalue이며 mutable owner면 U=PortList&, const owner면 U=const PortList&다.
        // (3) 유일한 인자는 유효한 PortList lvalue다. 포트 원소나 버퍼 소유권을 이 호출만으로 옮기지 않는다.
        // (4) const lvalue plan이면 const PortList&, mutable rvalue면 PortList&&, const rvalue면
        //     const PortList&&를 반환하며 호출부가 그대로 사용한다.
        // (5) ports_의 size/capacity/원소는 변하지 않고 반환 참조가 같은 vector subobject를 별칭한다.
        // (6) 표준은 별도 Complexity 항목을 두지 않지만 지정 결과는 참조 cast이고 새 소유 저장소를 요구하지
        //     않으며 noexcept·원소/반복자 직접 무효화 없음이다. 반환 참조는 plan 수명과 변경에 종속되고,
        //     같은 vector를 다른 스레드가 동시에 변경하면 읽기나 이동과 데이터 경쟁이 된다.
        return std::forward_like<Self>(self.ports_);
    }

private:
    // 두 멤버는 생성자의 멤버 초기화 목록에서 선언 순서대로 구성되고 객체 소멸 때 역순으로 정리된다.
    std::string service_;
    PortList ports_;
};

// DeploymentPublisher는 소비 가능한 계획에서 외부로 독립 반출할 값 스냅숏을 만든다.
// `DeploymentPlan&&`는 그 자체로 소유자가 아니라 호출자의 객체를 가리키는 참조지만, 이 API는 내용을 소비한다.
class DeploymentPublisher {
public:
    [[nodiscard]] DeploymentSnapshot publish(DeploymentPlan&& plan) const {
        // plan의 선언 타입은 rvalue reference여도 이름 있는 식 `plan` 자체는 lvalue다.
        // 따라서 std::move로 xvalue를 복원해야 explicit-object accessor가 rvalue 경로를 고른다.
        // [호출 계약: std::move, deducing-this accessor, string/vector 이동 생성]
        // (1) 수신 결과 snapshot의 service/ports 멤버는 아직 생성 전이다. plan은 호출자의 살아 있는
        //     DeploymentPlan을 참조하며 현재 service와 ports를 소유한다.
        // (2) 두 `std::move<DeploymentPlan&>(plan)`은 DeploymentPlan&&를 반환한다. 이어 service()/ports()의
        //     Self는 DeploymentPlan으로 추론되어 각각 std::string&&/PortList&&를 반환하고,
        //     aggregate 멤버에는 각 타입의 allocator 없는 이동 생성자가 선택된다.
        // (3) 두 std::move의 인자는 같은 이름 있는 plan lvalue다. 첫 accessor는 service만, 둘째는 ports만
        //     노출하므로 선언 순서대로 서로 다른 subobject의 소유권을 소비한다. plan은 null일 수 없는 참조다.
        // (4) std::move와 accessor 반환 참조는 즉시 각 멤버 생성에 사용해 따로 보관하지 않는다.
        //     이동 생성자는 반환값이 없고 return 식의 DeploymentSnapshot prvalue가 함수 결과를 직접 구성한다.
        // (5) 성공 뒤 반환 snapshot은 두 멤버의 이동 전 값을 독립 소유하고 호출자의 plan은 살아 있지만
        //     두 source 멤버 값은 미지정 상태다. 과거 ports_ 원소 관찰자는 end를 제외하면 destination 원소를
        //     가리키지만 service_ 문자 관찰자는 무효화될 수 있다. plan 자체의 참조는 재바인딩되지 않는다.
        // (6) cast/accessor에는 별도 표준 Complexity 항목이 없지만 참조 cast만 지정되고 새 소유 저장소를
        //     요구하지 않으며 noexcept다. 두 exact move 생성은 상수 시간·noexcept다.
        //     반환 prvalue는 보장된 복사 생략 대상으로 별도 snapshot 이동이 필요 없다. 소비 중 동시 접근은 금지다.
        return DeploymentSnapshot{
            std::move(plan).service(),
            std::move(plan).ports(),
        };
    }
};

// 반환형은 DeploymentPlan 값이다. 매개변수가 없고 호출자에게 새 소유 계획을 전달한다.
[[nodiscard]] DeploymentPlan make_plan() {
    // [생성 계약: std::string(const char*)와 std::vector<int>(initializer_list)]
    // (1) DeploymentPlan의 두 값 매개변수는 아직 생성 전이다. 문자열 리터럴은 정적 수명의
    //     const char[11] lvalue이고 PortList prvalue도 아직 생성 전이다.
    // (2) 문자열은 `basic_string(const char*, const Allocator& = Allocator())`, 포트는
    //     `vector(initializer_list<int>, const Allocator& = Allocator())`가 선택되며 vector 원소 타입은 int다.
    // (3) "search-api"는 배열-포인터 변환된 const char* prvalue로 NUL 종료가 전제다.
    //     `{8080, 8443}`의 두 int prvalue는 initializer_list가 빌린 임시 const int 배열에서 복사된다.
    // (4) 생성자는 반환값이 없다. 각각 문자 10개를 소유한 string과 원소 2개를 소유한 vector를 만든 뒤
    //     DeploymentPlan 생성자의 값 매개변수 및 멤버 초기화에 사용한다.
    // (5) 리터럴은 바뀌지 않고 임시 initializer_list 배열은 full-expression 뒤 사라진다.
    //     최종 service_/ports_는 자기 저장소를 소유하므로 두 입력의 수명과 독립적이다.
    // (6) 문자열은 O(10), vector는 O(2)이고 allocator가 동적 할당할 수 있다. 길이/할당 실패 예외가 가능하며
    //     완성되지 않은 객체는 누출 없이 정리된다. vector 생성 중 외부 반복자가 없으므로 무효화 대상도 없다.
    // 이 DeploymentPlan prvalue는 함수 반환 객체를 직접 구성하며 호출자의 변수까지 보장된 복사 생략으로 이어진다.
    return DeploymentPlan{"search-api", PortList{8080, 8443}};
}

// int 반환형은 0을 운영체제에 돌려 성공을, 0이 아닌 값을 자체 검증 실패로 알린다.
int main() {
    // make_plan()은 DeploymentPlan prvalue다. auto는 참조가 아닌 DeploymentPlan 값으로 추론되고
    // prvalue가 plan을 직접 초기화하므로 중간 DeploymentPlan 복사나 이동은 필요 없다.
    auto plan{make_plan()};

    // 이름 있는 plan은 lvalue다. const lvalue 참조와 포인터는 소유권을 얻거나 수명을 연장하지 않고
    // main 끝까지 살아 있는 plan을 관찰한다. `const DeploymentPlan* const`는 pointee와 포인터 값 모두 const다.
    const DeploymentPlan& audit_view{plan};
    const DeploymentPlan* const audit_source{&audit_view};
    const std::string& audited_service{audit_source->service()};
    const PortList& audited_ports{audit_source->ports()};

    // [호출 계약: PortList::size와 std::cout 삽입 연산자 연쇄]
    // (1) size의 수신자는 두 원소를 가진 const PortList lvalue다. 출력 수신자는 정상 상태를 기대하는
    //     정확한 std::ostream 타입의 전역 std::cout lvalue다.
    // (2) `vector<int>::size() const noexcept -> size_type`과, const char*, std::string, char,
    //     구현의 size_type 기반 부호 없는 정수에 맞는 basic_ostream 삽입 overload들이 왼쪽부터 선택된다.
    // (3) size에는 인자가 없다. 출력 인자는 정적 수명 리터럴, const string lvalue, char prvalue,
    //     size가 돌려준 unsigned prvalue이며 어떤 소유권도 cout으로 이전하지 않는다.
    // (4) size는 2를 값으로 반환해 마지막 수치 삽입 인자가 된다. 각 <<는 같은 std::ostream&를 반환해
    //     다음 삽입의 수신자로 쓰고 마지막 newline 삽입 반환 참조만 버린다.
    // (5) plan과 vector는 바뀌지 않는다. 성공하면 `audit=search-api:2`와 줄바꿈이 출력 버퍼에 순서대로 기록된다.
    // (6) size는 O(1)·무할당·noexcept·무효화 없음이다. 출력은 문자 수/locale/장치에 의존하고 실패 시
    //     stream 상태 비트가 바뀌며 exception mask 설정 시 예외가 가능하다. 기본 synchronized std::cout의
    //     형식 출력은 여러 스레드가 호출해도 data race는 없지만 문자가 섞일 수 있어 레코드 원자성은 없다.
    std::cout << "audit=" << audited_service << ':' << audited_ports.size() << '\n';

    DeploymentPublisher publisher{};

    // [호출 계약: std::move(plan)와 rvalue-reference 바인딩]
    // (1) 자유 함수라 수신자는 없다. plan은 살아 있는 DeploymentPlan lvalue이며 아직 두 값을 소유한다.
    // (2) `std::move<U>(U&&) noexcept`에서 U=DeploymentPlan&로 추론되어 DeploymentPlan&&를 반환한다.
    //     그 결과가 publish의 DeploymentPlan&& 매개변수에 직접 바인딩되고 객체를 새로 만들지는 않는다.
    // (3) 유일한 인자는 plan lvalue다. xvalue로 노출해 소비를 허용하지만 호출 자체는 소유권을 옮기지 않는다.
    // (4) 반환 reference는 publish 인자 바인딩에 사용하고 저장하지 않는다. publish가 돌려준 snapshot prvalue는
    //     published를 직접 초기화해 보장된 복사 생략이 적용된다.
    // (5) std::move 직후에는 plan이 그대로지만 publish 본문이 두 멤버를 이동한 뒤에는 plan이 유효·미지정 상태다.
    //     audit_view/audit_source/audited_*는 source 쪽 객체를 계속 가리키므로 아래에서 다시 읽지 않는다.
    // (6) std::move는 별도 표준 Complexity 항목 없이 참조 cast를 반환하고 새 소유 저장소를 요구하지 않으며
    //     noexcept다. 반환 참조는 plan 수명을 넘길 수 없고 같은 plan의 동시 관찰/소비는 안전하지 않다.
    //     실제 이동 보장은 publish 내부 선택 연산에 따른다.
    const DeploymentSnapshot published{publisher.publish(std::move(plan))};

    // [호출 계약: 게시 결과의 PortList::size와 std::cout 삽입 연산자]
    // (1) published는 독립 소유 snapshot const lvalue이고 ports에는 두 원소가 있다. std::cout는 앞 출력 뒤
    //     계속 사용할 수 있는 std::ostream lvalue이며 실패 상태라면 이후 삽입도 실패를 전파할 수 있다.
    // (2) `vector<int>::size() const noexcept`와 const char*, std::string, char, size_type용 출력 overload를 고른다.
    // (3) 인자들은 `"published="`, published.service lvalue, ':' prvalue, size()의 unsigned prvalue다.
    //     모두 빌려 읽으며 마지막 줄에는 의도적으로 newline 인자를 주지 않는다.
    // (4) size 반환값 2는 출력에 사용한다. 각 <<가 반환한 std::ostream&는 다음 호출에 연쇄되고 마지막 반환은 버린다.
    // (5) snapshot 내용은 유지되고 성공 시 정확히 `published=search-api:2`가 첫 줄 다음에 기록된다.
    // (6) size는 O(1)·무할당·noexcept이며 무효화가 없다. 출력 비용·오류·예외·동시성 계약은 앞 연쇄와 같고,
    //     프로그램 정상 종료 때 표준 stream이 버퍼를 내보내지만 외부 장치 성공까지 보장하지는 않는다.
    std::cout << "published=" << published.service << ':' << published.ports.size();

    // [호출 계약: std::string과 const char* 비교, PortList::size 관찰]
    // (1) 왼쪽 문자열은 "search-api"를 소유한 const std::string lvalue, ports는 두 원소의 const PortList다.
    // (2) C++20 이후 `published.service != "search-api"`는 basic_string/const-char 문자열 동등 비교의
    //     부정으로 bool을 만들고, size는 앞과 같은 `size() const noexcept` overload를 고른다.
    // (3) 오른쪽 리터럴은 NUL 종료된 정적 const char[11] lvalue다. size에는 인자가 없고 소유권 이동도 없다.
    // (4) 비교 bool과 size 값은 `||`의 자체 검증 분기에 사용한다. 문자열이 다르면 단락 평가로 size를 부르지 않는다.
    // (5) 문자열/vector/리터럴 어느 것도 바뀌지 않고 반복자·참조도 무효화되지 않는다.
    // (6) 문자열 비교는 최악 O(n), size는 O(1)이며 둘 다 무할당이다. 유효한 NUL 종료 포인터가 전제이고
    //     같은 객체에 동시 쓰기가 없어야 한다. 실패면 1, 모두 맞으면 0을 반환한다.
    if (published.service != "search-api" || published.ports.size() != 2U) {
        return 1;
    }

    return 0;
}
