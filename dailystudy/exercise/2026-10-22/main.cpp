// 오늘의 실무 패턴: 저장소가 돌려준 `std::optional<ServiceConfig>`를 C++23
// `optional::transform`으로 외부 계층이 소유하는 `LaunchPlan` DTO로 투영한다.
// 값이 없으면 lambda를 부르지 않고 빈 optional을 그대로 전파하므로, 소스에 직접 쓰는 분기와 중간
// 변환 객체를 줄일 수 있다. 실제 기계 분기·임시 객체 제거 여부는 컴파일러와 최적화에 달려 있다.
// 동시에 "없음"을 예외나 null pointer가 아닌 타입으로 표현한다.

// <iostream>은 표준 출력 객체 std::cout과 ostream 삽입 연산을 제공한다.
#include <iostream>
// <optional>은 값 하나가 있거나 없는 상태를 직접 소유하는 std::optional과 std::nullopt를 제공한다.
#include <optional>
// <string>은 문자 버퍼를 소유하는 std::string을 제공한다.
#include <string>
// <string_view>는 문자를 소유하지 않고 연속 문자 구간만 빌리는 std::string_view를 제공한다.
#include <string_view>
// <utility>는 식을 xvalue로 바꾸는 std::move를 제공한다.
#include <utility>
// <vector>는 같은 타입의 값을 연속 저장하는 동적 배열 std::vector를 제공한다.
#include <vector>

// struct는 기본 접근 지정자가 public이라 단순 데이터 묶음(DTO)에 알맞다.
// class는 기본 접근이 private이라 뒤의 ConfigRepository처럼 불변식을 감추는 데 알맞다.
struct ServiceConfig {
    // 멤버 선언 자체는 객체를 만들지 않는다. 오늘의 모든 aggregate 식이 service를 명시해 소유 string을
    // 직접 생성한다. std::string은 문자 메모리를 소유하므로 입력 buffer의 수명에 매이지 않는다.
    std::string service;
    // {}는 기본 타입 int를 0으로 값 초기화해 초기화되지 않은 값을 막는다.
    int replicas{};
};

// transform의 결과 타입이다. 저장소 내부 객체를 참조하지 않고 필요한 값을 새 객체로 소유한다.
struct LaunchPlan {
    // 오늘의 aggregate 식이 이동할 service를 명시하므로 별도의 선행 기본 생성이나 대입은 없다.
    std::string service;
    int replicas{};
};

// class의 private 기본 접근으로 저장 vector를 감추고 읽기 전용 질의만 public으로 연다.
class ConfigRepository {
public:
    // 생성자는 반환형이 없다. explicit은 vector 한 개가 저장소로 뜻하지 않게 암시 변환되는 것을 막는다.
    explicit ConfigRepository(std::vector<ServiceConfig> configs)
        // [첫 호출 계약: std::move(configs)]
        // (1) 자유 함수라 수신 객체는 없다. configs의 정확한 타입은 이 생성자가 값으로 소유한
        //     std::vector<ServiceConfig> lvalue이고, configs_ subobject는 아직 수명이 시작되지 않았다.
        // (2) remove_reference_t<T>&& move(T&&) noexcept에서 T=std::vector<ServiceConfig>&로 추론되어
        //     std::vector<ServiceConfig>&&를 반환한다.
        // (3) 유일한 인자 식 configs는 이름 있는 lvalue다. xvalue로 바꾸어 원소 buffer 소유권을
        //     configs_에 이전해도 된다는 의도를 표시하며, move 함수 자체는 아직 아무것도 옮기지 않는다.
        // (4) 같은 객체를 가리키는 xvalue reference를 반환하고 vector 이동 생성의 인자로 즉시 사용한다.
        // (5) std::move 자체로 configs 상태는 변하지 않으며, 바로 뒤 선택된 이동 생성자가 상태를 바꾼다.
        // (6) O(1) 형 변환, 무할당, noexcept이고 iterator를 직접 무효화하지 않는다. 반환 reference는
        //     configs보다 오래 살지 않으며 이 생성 중 다른 thread가 configs에 접근해서는 안 된다.
        // [같은 식의 첫 생성 계약: std::vector<ServiceConfig> 이동 생성]
        // (1) source configs는 유효한 vector xvalue이고 목적 configs_는 아직 생성 전이다.
        // (2) vector(vector&&) noexcept, 같은 allocator를 쓰는 비-allocator overload를 선택한다.
        // (3) source xvalue가 동적 buffer와 원소 소유권을 넘긴다. 외부 reference를 새로 빌리지 않는다.
        // (4) 생성자는 반환값 없이 configs_의 수명을 시작한다.
        // (5) configs_가 기존 원소를 소유하고 source는 유효하지만 unspecified 상태가 된다. 이 overload 뒤
        //     기존 원소 pointer/reference/iterator는 목적 vector의 같은 원소를 계속 가리킨다.
        // (6) 상수 시간, 별도 할당 없음, noexcept다. configs_ 파괴 때 원소도 파괴된다. 생성 중 source를
        //     병행 접근하지 않으며 완성 repository의 병행 읽기만 data race 없이 허용된다.
        // 멤버 초기화 목록은 함수 본문보다 먼저, 실제 멤버 선언 순서대로 실행된다.
        : configs_{std::move(configs)} {}

    // 반환형 optional<ServiceConfig>는 찾은 설정의 독립 소유 snapshot 또는 빈 상태다.
    // 매개변수 string_view는 비소유 참조이므로 호출 동안 원본 문자 범위가 살아 있어야 한다.
    // 뒤 const는 이 함수가 configs_를 바꾸지 않는다는 뜻이다.
    [[nodiscard]] std::optional<ServiceConfig> find(std::string_view service) const {
        // [첫 호출 계약: std::vector<ServiceConfig> 범위 순회]
        // (1) 수신 configs_는 완성된 repository가 소유하는 유효한 const std::vector<ServiceConfig>
        //     lvalue다. 함수 동안 크기와 저장 위치가 바뀌지 않는다.
        // (2) ServiceConfig와 기본 allocator를 쓰는 vector 특수화에서
        //     `const_iterator begin() const noexcept`, `const_iterator end() const noexcept`를 고른다.
        //     숨은 cursor/sentinel의 정확한 별칭은 vector<ServiceConfig>::const_iterator다. 구현별 반복자
        //     클래스 이름은 표준이 고정하지 않지만, 반복에는 bool로 판정되는 동등/부등 비교(언어 규칙상
        //     `!=`가 `==` 후보에서 재작성될 수 있음), `const_iterator& operator++()`,
        //     `const ServiceConfig& operator*() const`에 해당하는 연산이 필요하다.
        // (3) begin/end의 명시 데이터 인자는 없고 숨은 수신자는 configs_다. 반복자 비교는 cursor와
        //     sentinel lvalue, 증가는 cursor lvalue, 역참조는 cursor const lvalue를 관찰한다. 원소
        //     소유권은 이동하지 않고 const로 빌린다.
        // (4) begin/end는 첫 원소와 one-past-end 반복자를 값으로 돌려 순회 제어에 사용한다. 비교 결과
        //     bool은 반복 조건에, prefix 증가는 갱신된 cursor reference로 다음 반복에, 역참조 결과
        //     const ServiceConfig&는 config 바인딩에 사용된다.
        // (5) vector와 원소는 변하지 않는다. 각 config 참조는 이번 반복 동안 같은 원소를 가리킨다.
        // (6) begin/end와 각 반복자 연산은 O(1), 전체 순회는 O(N), 무할당이다. vector 재할당·파괴가
        //     없다는 수명 전제가 필요하며 같은 vector의 병행 변경은 data race다.
        for (const ServiceConfig& config : configs_) {
            // [첫 호출 계약: std::string과 std::string_view의 operator==]
            // (1) 왼쪽 config.service는 유효한 const std::string lvalue이고, 오른쪽 service는 호출자가
            //     빌려 준 유효한 const std::string_view lvalue다.
            // (2) `template<class CharT, class Traits> constexpr bool operator==(
            //     basic_string_view<CharT, Traits>, type_identity_t<basic_string_view<CharT, Traits>>) noexcept`를
            //     CharT=char, Traits=char_traits<char>로 선택한다. 원래 인자 순서에서는 첫 string 인자로
            //     template 인자를 추론할 수 없으므로, C++20 동등 비교의 reversed rewritten candidate가
            //     service를 첫 parameter로 받아 타입을 추론한다. config.service는 두 번째 non-deduced
            //     parameter에서 자신의 const noexcept string_view 변환 연산자로 비소유 view가 된다.
            // (3) 두 인자는 문자 구간을 읽기만 하며 소유권 이동이 없다. string_view의 data가 size만큼
            //     유효해야 하고 null 종료는 요구하지 않는다.
            // (4) 길이와 각 문자가 같으면 true인 bool prvalue를 반환해 if 조건 분기에 사용한다.
            // (5) 두 문자열 객체와 backing storage는 바뀌지 않는다.
            // (6) 길이가 같을 때 최대 문자 수에 선형인 O(L), 무할당이며 예외를 던지지 않는다. 참조·반복자
            //     무효화가 없고, 비교 중 어느 문자 저장소도 병행 변경하지 않아야 한다.
            if (config.service == service) {
                // [첫 생성 계약: const ServiceConfig&에서 std::optional<ServiceConfig> 값 생성]
                // (1) 목적 optional은 아직 없고 config는 vector가 소유한 유효한 const ServiceConfig lvalue다.
                // (2) template<class U=T> constexpr optional(U&&)에서 T=ServiceConfig,
                //     U=const ServiceConfig&인 converting value constructor를 선택한다.
                // (3) config를 const lvalue로 받아 ServiceConfig와 내부 string을 복사한다. 저장소 원소의
                //     소유권은 유지되고 반환 snapshot이 자기 문자열 buffer를 소유한다.
                // (4) 생성자는 별도 반환값 없이 engaged optional prvalue를 만들어 함수 반환값으로 쓴다.
                // (5) 원본 config는 그대로이고 새 optional 안 ServiceConfig의 수명이 시작된다.
                // (6) ServiceConfig/string 복사 비용은 문자열 길이에 선형이고 할당 실패가 날 수 있다.
                //     실패하면 완성 optional이 없고 원본은 보존된다. vector 반복자는 무효화되지 않으며
                //     반환값은 repository 파괴 뒤에도 독립적으로 유효하다.
                // [같은 식 내부 첫 생성 계약: std::string 복사 생성]
                // (1) source는 config 안의 살아 있는 const string lvalue이고 snapshot의 목적 string
                //     subobject는 아직 수명 시작 전이다. (2) `basic_string(const basic_string&)` overload를
                //     char/char_traits<char>/기본 allocator 특수화로 선택한다. (3) source의 문자 값을 읽는
                //     const lvalue 하나를 받고 소유권은 원본에 남는다. (4) 생성자는 반환값 없이 목적
                //     string의 수명을 시작한다. (5) 두 string은 같은 값을 가진 독립 owner가 되고 source는
                //     변하지 않는다. (6) 문자 수에 선형이며 할당과 bad_alloc 가능성이 있다. 실패하면 목적
                //     수명은 시작되지 않고 원본은 보존된다. 원본 reference/iterator는 무효화되지 않고 새
                //     string의 reference 수명은 snapshot까지이며, 복사 중 source 병행 변경은 허용되지 않는다.
                return config;
            }
        }

        // [첫 생성 계약: std::optional<ServiceConfig>(std::nullopt_t)]
        // (1) 목적 optional은 아직 없고 std::nullopt는 표준의 불변 std::nullopt_t 상수 객체다.
        // (2) constexpr optional(nullopt_t) noexcept를 T=ServiceConfig로 선택한다.
        // (3) nullopt는 빈 상태를 고르는 값 인자이며 데이터나 소유권을 전달하지 않는다.
        // (4) 생성자는 반환값 없이 disengaged optional prvalue를 만들어 함수 반환값으로 쓴다.
        // (5) ServiceConfig는 생성되지 않고 repository도 변하지 않는다.
        // (6) O(1), 무할당, noexcept이며 참조 무효화나 공유 상태가 없다.
        return std::nullopt;
    }

private:
    // 멤버 선언 자체는 vector를 생성하지 않는다. 유일한 생성자의 mem-initializer가 configs_를 곧바로
    // 이동 생성하므로 빈 vector를 먼저 만들고 대입하는 단계가 없다.
    std::vector<ServiceConfig> configs_;
};

// 저장소 snapshot을 API/실행 계층의 DTO로 바꾸는 application service다.
// 반환값 자체가 owner이므로 repository보다 오래 보관해도 dangling reference가 생기지 않는다.
[[nodiscard]] std::optional<LaunchPlan> make_launch_plan(
    const ConfigRepository& repository,
    std::string_view service) {
    // 이름 있는 found는 lvalue이며 optional 안에 ServiceConfig를 직접 소유한다.
    std::optional<ServiceConfig> found{repository.find(service)};

    // [첫 호출 계약: std::move(found)]
    // (1) 수신 객체 없는 함수이고 found는 engaged 또는 disengaged인 유효한
    //     std::optional<ServiceConfig> lvalue다.
    // (2) remove_reference_t<T>&& move(T&&) noexcept, T=std::optional<ServiceConfig>&를 선택해
    //     std::optional<ServiceConfig>&&를 반환한다.
    // (3) found lvalue를 인자로 받아 소유 상태를 rvalue-qualified transform에서 소비해도 된다는
    //     의도를 표시한다. std::move 자체는 contained 값을 이동하지 않는다.
    // (4) found를 가리키는 xvalue reference를 반환해 바로 transform의 수신 객체로 사용한다.
    // (5) 이 호출 자체 뒤 found는 그대로이고 실제 이동 여부는 transform의 callable이 결정한다.
    // (6) O(1), 무할당, noexcept다. reference 수명은 found까지이며 같은 객체를 병행 접근하지 않는다.
    // [첫 호출 계약: std::optional::transform]
    // (1) 수신 객체는 std::move가 만든 optional<ServiceConfig> xvalue다. 값이 있으면 그 contained
    //     ServiceConfig가 살아 있고, 없으면 callable을 실행하지 않아야 하는 유효한 빈 상태다.
    // (2) template<class F> constexpr auto transform(F&&) && overload를 선택한다. F는 아래 lambda의
    //     고유 closure 타입, 호출 결과 U는 LaunchPlan이며 반환형은 std::optional<LaunchPlan>이다.
    // (3) lambda 식은 소유 상태가 없는 prvalue이고 F&&에 결합된다. engaged일 때만 contained 값을
    //     ServiceConfig&&로 전달한다. reference를 결과에 저장하지 않고 필요한 string 소유권만 옮긴다.
    // (4) engaged면 lambda 결과로 직접 초기화한 LaunchPlan을 가진 optional, 아니면 빈 optional을
    //     값으로 반환하고 이 함수의 반환값으로 그대로 사용한다.
    // (5) 빈 입력에는 상태 변화가 없다. 값이 있으면 found는 여전히 engaged지만 lambda가 service
    //     문자열을 옮겨 contained ServiceConfig가 유효한 moved-from 상태가 될 수 있다. 결과가 새 owner다.
    // (6) 존재 검사 1회와 최대 callable 1회다. optional 자체는 별도 heap 할당을 요구하지 않는다.
    //     일반 transform은 callable/U 생성 예외를 잡지 않지만 member 시그니처 자체도 noexcept는 아니다.
    //     이 특수화의 noexcept lambda는 string 이동 생성과 int 복사만 하므로 정상 실행에 할당·throw가
    //     필요하지 않는다. noexcept callable 밖으로 예외가 나오면 전파가 아니라 terminate다. 원본 값의
    //     참조는 수명은 남아도 이동 후 상태일 수 있고 같은 optional의 병행 접근은 동기화해야 한다.
    return std::move(found).transform([](ServiceConfig&& config) noexcept {
        // [첫 호출 계약: std::move(config.service)]
        // (1) 수신 객체 없는 함수이고 config.service는 transform이 건넨 살아 있는 std::string lvalue다.
        // (2) move(T&&) noexcept에서 T=std::string&로 추론되어 std::string&&를 반환한다.
        // (3) 이름 있는 member lvalue의 문자열 값과 자원을 새 LaunchPlan 구성에 사용해도 된다는 뜻이다.
        // (4) 같은 string을 가리키는 xvalue reference를 반환해 아래 string 이동 생성에 쓴다.
        // (5) move 자체는 상태를 바꾸지 않고 뒤 이동 생성이 config.service를 유효한 moved-from 상태로 만든다.
        // (6) O(1), 무할당, noexcept 형 변환이며 반환 reference는 config.service보다 오래 살지 않는다.
        // [같은 식의 첫 생성 계약: std::string 이동 생성]
        // (1) source는 유효한 string xvalue이고 LaunchPlan::service 목적 subobject는 아직 없다.
        // (2) basic_string(basic_string&&) noexcept, allocator를 따로 받지 않는 overload를 선택한다.
        // (3) source의 문자열 값과 재사용 가능한 자원을 목적 멤버로 옮긴다. 외부 view를 보관하지 않는다.
        // (4) 생성자는 반환값 없이 목적 string 수명을 시작한다.
        // (5) 목적이 서비스 이름을 소유하고 source는 유효하지만 unspecified 상태가 된다.
        // (6) 상수 시간, 이 overload는 noexcept이다. 구현은 SSO 문자를 복사할 수도 있으므로 heap buffer
        //     탈취를 보장하지 않는다. source의 기존 iterator/reference가 목적을 가리킨다고 기대하지 말고,
        //     두 객체를 다른 thread가 동시에 변경하지 않는다.
        // aggregate 중괄호 초기화는 멤버 선언 순서대로 string과 int를 초기화하고 narrowing을 막는다.
        return LaunchPlan{std::move(config.service), config.replicas};
    });
}

int main() {
    // [첫 생성 계약: 문자열 literal에서 std::string 생성과 initializer_list vector 생성]
    // (1) 목적 string 세 개와 vector는 아직 없고 literal들은 프로그램 전체 수명의 const char 배열이다.
    // (2) 각 string은 basic_string(const char*, const Allocator& = Allocator())를 선택하고,
    //     vector는 vector(initializer_list<ServiceConfig>, const Allocator& = Allocator())를 선택한다.
    // (3) 각 pointer는 null 종료된 유효 문자 배열을 가리킨다. initializer_list 원소는 const라 vector는
    //     세 ServiceConfig를 복사해 소유하며 literal이나 임시의 수명을 빌리지 않는다.
    // (4) 생성자들은 반환값 없이 세 string, 세 aggregate 원소, vector의 수명을 차례로 시작한다.
    // (5) vector가 크기 3의 원소와 독립 문자열 buffer를 소유한다. 식 끝의 initializer_list 임시는 끝난다.
    // (6) 전체 문자 수와 원소 수에 선형이고 동적 할당/예외가 가능하다. 실패하면 완성된 하위 객체를
    //     역순 정리한다. 생성 전 iterator는 없고 이 main thread에서만 객체를 구성한다.
    // vector prvalue는 ConfigRepository의 값 매개변수를 직접 초기화하고, 생성자 안에서 configs_로
    // 이동한다. C++17 prvalue 보장 복사 생략과 실제 vector 이동을 구분한다.
    // [가까운 생성 계약: std::string literal 생성]
    // (1) 각 목적 string은 생성 전이다. (2) `basic_string(const char*, const Allocator&)` overload를 선택한다.
    // (3) 입력 pointer는 null 종료된 literal의 lvalue-to-pointer 변환 결과이고 소유권을 빌리지 않는다.
    // (4) 생성자는 반환값 없이 새 string을 만든다. (5) 결과가 문자를 독립 소유한다. (6) 복잡도는
    // 문자 수에 선형이고 할당 실패 예외가 가능하며, 성공한 string의 reference 수명은 그 owner까지다.
    ConfigRepository repository{std::vector<ServiceConfig>{
        ServiceConfig{std::string{"billing"}, 3},
        ServiceConfig{std::string{"search"}, 5},
        ServiceConfig{std::string{"worker"}, 2},
    }};

    // [첫 생성 계약: std::string_view의 C 문자열 생성자]
    // (1) make_launch_plan의 service 값 매개변수는 아직 생성 전이고, "billing"은 프로그램 전체
    //     수명의 const char[8] lvalue다. (2) `constexpr basic_string_view(const char*)`
    //     overload를 char/char_traits<char>로 선택한다. (3) 배열에서 변환된 non-null const char*
    //     prvalue가 유일한 인자이며 view는 소유권을 받지 않는다. (4) 생성자는 반환값 없이 매개변수
    //     string_view를 만든다. (5) data는 literal 첫 문자를, size는 null 전 7자를 나타내고 원본은
    //     바뀌지 않는다. (6) Traits::length 때문에 복잡도 O(문자 수), 무할당·무효화 없음이다. 표준
    //     시그니처는 noexcept가 아니므로 Traits::length가 던질 수 있는 특수화라면 예외가 전파된다.
    //     pointer가 null이거나 null 전 범위가 유효하지 않으면 전제조건 위반이다. 불변 literal은 병행
    //     읽을 수 있고 view 수명은 문자를 연장하지 않지만 여기서는 literal이 프로그램 끝까지 살아 안전하다.
    // 함수가 반환한 optional<LaunchPlan> prvalue가 launch를 직접 초기화하므로 optional 복사/이동은
    // 필요 없다. 이 보장 복사 생략은 이름 있는 local의 선택적 NRVO와 다른 규칙이다.
    const std::optional<LaunchPlan> launch{make_launch_plan(repository, "billing")};
    const std::optional<LaunchPlan> missing{make_launch_plan(repository, "unknown")};

    // [첫 호출 계약: std::optional<LaunchPlan>::operator bool]
    // (1) 수신 launch는 engaged가 기대되지만 타입상 빈 상태도 허용되는 유효한 const optional lvalue다.
    // (2) constexpr explicit operator bool() const noexcept를 선택한다.
    // (3) 명시 인자와 소유권 이동은 없고 contained 객체 존재 bit만 읽는다.
    // (4) 값이 있으면 true인 bool prvalue를 반환해 if 조건 분기에 사용한다.
    // (5) launch와 contained LaunchPlan은 바뀌지 않는다.
    // (6) O(1), 무할당, noexcept, 무효화 없음이다. 같은 optional에 병행 쓰기가 없어야 한다.
    if (launch) {
        // [첫 호출 계약: std::optional<LaunchPlan>::operator->]
        // (1) 수신 launch는 직전 분기로 engaged가 증명된 const optional<LaunchPlan> lvalue다.
        // (2) constexpr const T* operator->() const noexcept, T=LaunchPlan overload를 선택한다.
        // (3) 명시 인자·소유권 이동 없이 contained object 주소를 관찰한다.
        // (4) const LaunchPlan*를 반환해 두 멤버 접근에 즉시 사용한다.
        // (5) optional과 값은 바뀌지 않고 pointer를 따로 저장하지 않는다.
        // (6) O(1), 무할당, noexcept다. 빈 optional에 쓰면 전제조건 위반이지만 앞 분기가 막는다. 반환
        //     pointer는 launch의 수명과 다음 reset/대입보다 오래 쓰면 안 되며 병행 변경이 없어야 한다.
        // [첫 호출 계약: std::cout operator<< 연쇄]
        // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 main thread만 이 레코드를 쓴다.
        // (2) string에는 operator<<(ostream&, const string&), int에는 ostream::operator<<(int),
        //     '\n'에는 operator<<(ostream&, char)가 순서대로 선택된다.
        // (3) service/replicas는 const lvalue에서 읽고 문자들은 prvalue다. 소유권 이동이 없다.
        // (4) 각 삽입은 같은 ostream&를 반환해 다음 호출의 수신으로 쓰며 마지막 반환 참조는 버린다.
        // (5) 정확히 "billing 3\n"을 출력하려 시도하고 launch는 변하지 않는다. stream 상태 bit는
        //     장치 성공/실패에 따라 바뀔 수 있다.
        // (6) 표준은 장치 I/O의 단일 복잡도/할당 상한을 정하지 않는다. 실패는 상태 bit 또는 설정된
        //     ios 예외로 보고된다. 다른 객체 참조는 무효화하지 않으며 복합 출력의 thread 원자성은 없다.
        std::cout << launch->service << ' ' << launch->replicas << '\n';
    }

    // [첫 호출 계약: std::optional<LaunchPlan>::has_value]
    // (1) 수신 missing은 유효한 const optional<LaunchPlan> lvalue이며 이번 입력에서는 비어 있다.
    // (2) constexpr bool has_value() const noexcept를 선택한다.
    // (3) 명시 인자·소유권 이동 없이 존재 상태를 읽는다.
    // (4) contained 값이 있으면 true를 반환하지만 여기서는 false이며 ostream 입력으로 사용한다.
    // (5) missing 상태는 변하지 않는다.
    // (6) O(1), 무할당, noexcept, 무효화 없음이며 같은 객체의 병행 변경은 허용되지 않는다.
    // 같은 ostream 삽입 계약을 재사용한다. 기본 bool 숫자 형식에서 false는 0으로 출력된다.
    std::cout << missing.has_value() << '\n';

    // 기계 실행 관점: 최적화 전 개념 모델에서는 optional의 상태 load, 문자열 길이/문자 load와 비교,
    // 조건 분기, string 값/자원과 관련된 load·store가 보일 수 있다. 실제 명령·분기·할당·복사 생략 여부는 CPU,
    // ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 합쳐지거나 사라질 수 있으므로 특정
    // 어셈블리 한 줄과 소스 한 줄이 대응한다고 단정하지 않는다. 이 예제에는 virtual 함수가 없어
    // 가상 간접 호출은 없고, transform의 lambda도 보통 정적 호출 대상으로 최적화할 수 있다.

    // 0은 운영체제에 정상 종료를 알리는 int prvalue다.
    return 0;
}
