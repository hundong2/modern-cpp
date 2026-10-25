// 오늘의 실무 예제: 1차 저장소에 설정이 없을 때 std::optional::or_else로
// 독립 수명의 기본 Endpoint 값을 만든다. 먼저 두 출력 줄을 종이에 예측해 본다.

// <iostream>은 표준 출력 객체 std::cout과 문자열 삽입 연산을 선언한다.
#include <iostream>
// <optional>은 값의 존재/부재를 함께 표현하는 std::optional과 std::nullopt를 선언한다.
#include <optional>
// <string>은 문자 저장소를 직접 소유하는 std::string을 선언한다.
#include <string>
// <string_view>는 문자를 소유하지 않고 연속 문자 구간을 빌리는 std::string_view를 선언한다.
#include <string_view>
// <utility>는 이름 있는 객체를 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>
// <vector>는 같은 타입 원소를 연속 저장소에 소유하는 std::vector를 선언한다.
#include <vector>

// struct의 기본 접근 지정자는 public이다. 이 타입은 계층 사이를 오가는 단순 소유 DTO다.
struct Endpoint {
    // 두 string이 문자 버퍼를 직접 소유하므로 원본 입력 view보다 오래 안전하게 살 수 있다.
    std::string service;
    std::string url;
};

// class의 기본 접근 지정자는 private다. 저장 방식은 감추고 조회 계약만 공개한다.
class EndpointRepository {
public:
    // explicit은 vector 하나가 저장소 객체로 뜻하지 않게 암시 변환되는 것을 막는다.
    // 값 매개변수 entries는 호출 경계에서 자기 vector를 소유한다.
    explicit EndpointRepository(std::vector<Endpoint> entries)
        // [첫 호출 계약: std::move(entries)와 std::vector<Endpoint> 이동 생성자]
        // (1) entries는 유효한 std::vector<Endpoint> lvalue이며 endpoints_는 아직 수명이
        //     시작되지 않은 같은 타입 멤버다.
        // (2) `remove_reference_t<T>&& move(T&&) noexcept`에서 T=vector<Endpoint>&가
        //     추론되고, 이어 `vector(vector&&) noexcept`가 선택된다.
        // (3) 유일한 인자 entries는 이름 있는 lvalue다. move가 xvalue로 표시하고 뒤 생성자가
        //     원소 저장소 소유권을 endpoints_로 이전한다. move 자체는 아무것도 복사하지 않는다.
        // (4) move는 vector<Endpoint>&&를 반환해 생성 인자로 즉시 사용한다. 생성자는 반환값 없이
        //     endpoints_의 수명을 시작한다.
        // (5) 성공 뒤 endpoints_가 기존 원소를 소유하고 entries는 유효하지만 값이 미지정이다.
        //     기본 allocator가 같은 이동 생성에서는 기존 원소 참조가 새 owner의 같은 원소를 가리킨다.
        // (6) move cast와 기본-allocator vector 이동은 O(1), 무할당, noexcept다. 멤버가 파괴될 때
        //     원소와 저장소도 정리된다. 같은 vector를 병행 변경하면 외부 동기화가 필요하다.
        : endpoints_{std::move(entries)} {}

    // 반환형 optional<Endpoint>는 조회 성공 시 독립 복사본을, 실패 시 정상적인 빈 상태를 표현한다.
    [[nodiscard]] std::optional<Endpoint> find(std::string_view service) const {
        // [첫 숨은 호출 계약: const vector의 range-for begin/end, iterator 비교·역참조·증가]
        // (1) endpoints_는 살아 있는 const vector<Endpoint>이며 반복 중 구조를 바꾸지 않는다.
        // (2) `const_iterator begin() const noexcept`, `const_iterator end() const noexcept`와 표준이
        //     implementation-defined로 둔 const_iterator의 `!=`, `*`, 전위 `++` 연산이 선택된다.
        // (3) begin/end의 수신자는 endpoints_ const lvalue이고 데이터 인자는 없다. 비교의 두 피연산자는
        //     현재 begin/end iterator lvalue, 역참조와 증가는 현재 begin iterator lvalue를 수신자로 쓴다.
        //     iterator와 endpoint 참조는 원소를 빌릴 뿐 vector 소유권을 넘기지 않는다.
        // (4) begin/end는 iterator 값을, 비교는 bool을, 역참조는 const Endpoint&를, 증가는 iterator&를
        //     반환한다. 이 결과들은 loop 제어와 현재 endpoint binding에 모두 사용된다.
        // (5) `++begin`만 현재 iterator를 다음 위치로 바꾸고 end·vector·원소·service는 그대로다.
        //     endpoint 참조 변수는 매 iteration body 동안 현재 원소에 바인딩된다.
        // (6) 각 iterator 연산은 O(1), 전체 순회는 O(N)·무할당이다. owner 파괴·재할당 뒤 iterator와
        //     reference는 무효이며 end 역참조·유효 범위 밖 증가는 전제 위반이다. 동시 구조 변경은
        //     데이터 경쟁이고, 이 const 순회 자체는 외부 원소 변경이 없을 때만 안전하다.
        for (const Endpoint& endpoint : endpoints_) {
            // [첫 호출 계약: std::string과 std::string_view의 operator==]
            // (1) endpoint.service는 유효한 const std::string lvalue, service는 호출 동안 유효한
            //     std::string_view lvalue이며 각 data/size가 읽을 수 있는 문자 범위를 가리킨다.
            // (2) C++20의 string/string_view 비교 연산 후보가 선택되어 두 문자 시퀀스를 비교한다.
            // (3) 두 피연산자는 읽기 전용 lvalue다. 문자를 복사하거나 소유권·수명을 넘기지 않는다.
            // (4) 반환 bool prvalue는 if 조건으로 사용된다.
            // (5) 성공/실패 어느 쪽에서도 두 문자열과 저장소는 바뀌지 않는다.
            // (6) 길이와 첫 불일치까지 O(min(n,m)), 무할당이다. view가 dangling이 아니어야 하며
            //     같은 문자 저장소를 다른 thread가 쓰는 동안 비교하면 안 된다.
            if (endpoint.service == service) {
                // [첫 생성 계약: optional<Endpoint> converting value constructor]
                // (1) endpoint는 vector가 소유하는 살아 있는 const Endpoint lvalue이고 반환 optional의
                //     내부 저장소는 아직 비어 있다.
                // (2) `template<class U=T> constexpr optional(U&&)`에서 U=const Endpoint&가 추론되어
                //     Endpoint 복사 생성자를 optional 저장소 안에서 선택한다.
                // (3) 인자는 endpoint const lvalue다. 두 string 값을 깊게 복사하며 원본을 빌려 두지 않는다.
                // (4) 생성자는 별도 반환값 없이 engaged optional prvalue를 만들고 return 값으로 사용된다.
                // (5) 결과가 독립 Endpoint를 소유하고 repository 원소는 그대로다.
                // (6) 전체 문자열 길이에 선형이며 각 string에서 별도 할당과 bad_alloc이 가능하다
                //     (SSO라면 실제 heap 할당이 없을 수도 있다). 예외 시 완성되지 않은
                //     결과는 정리되고 저장소는 유지된다. 원본 iterator/reference 무효화는 없다.
                return endpoint;
            }
        }

        // [첫 생성 계약: std::optional<Endpoint>(std::nullopt_t)]
        // (1) 반환 목적 optional은 생성 전이고 repository 검색은 끝까지 일치 항목을 찾지 못했다.
        // (2) `constexpr optional(nullopt_t) noexcept`가 선택된다.
        // (3) 인자는 정적 수명의 std::nullopt const 객체에서 복사되는 std::nullopt_t 태그 값이며
        //     Endpoint나 다른 소유 자원을 전달하지 않는다.
        // (4) 생성자는 별도 반환값 없이 disengaged optional prvalue를 만들어 return에 사용한다.
        // (5) 결과에는 Endpoint 수명이 시작되지 않고 repository와 service는 그대로다.
        // (6) O(1)·무할당·noexcept이고 관찰자를 무효화하지 않는다. 빈 상태는 정상 값이며
        //     나중에 검사 없는 operator*는 UB, value()는 bad_optional_access가 될 수 있다.
        return std::nullopt;
    }

private:
    // vector가 Endpoint 원소의 객체 수명과 각 string의 문자 저장소를 소유한다.
    std::vector<Endpoint> endpoints_;
};

// Resolver는 repository를 소유하지 않고 빌리므로 repository가 resolver보다 오래 살아야 한다.
class EndpointResolver {
public:
    // reference 매개변수는 null이 될 수 없는 별칭이고, fallback_url 값 매개변수는 독립 소유 입력이다.
    EndpointResolver(const EndpointRepository& primary, std::string fallback_url)
        : primary_{primary},
          // [첫 생성 계약: std::string 이동 생성자]
          // (1) fallback_url은 이 생성자가 소유하는 유효한 string lvalue, fallback_url_은 생성 전이다.
          // (2) std::move가 string&&를 만들고 기본 allocator `basic_string(basic_string&&) noexcept`를 고른다.
          // (3) 이름 있는 lvalue 하나의 자원을 새 멤버로 이전하며 관찰 reference를 보관하지 않는다.
          // (4) 생성자는 반환값 없이 멤버 수명을 시작하고 move 반환 참조는 즉시 소비된다.
          // (5) 멤버가 원래 값을 소유하고 매개변수는 유효하지만 값이 미지정이다.
          // (6) 보통 O(1)·무할당·noexcept지만 작은 문자열은 구현 내부에서 문자를 복사할 수 있다.
          //     기존 문자 pointer의 목적지 추종을 일반 보장으로 가정하지 않으며 동시 접근은 없다.
          fallback_url_{std::move(fallback_url)} {}

    // const 멤버 함수는 resolver의 reference와 fallback string을 바꾸지 않는다.
    [[nodiscard]] std::optional<Endpoint> resolve(std::string_view service) const {
        // [첫 호출 계약: rvalue std::optional<Endpoint>::or_else(F&&) &&]
        // (1) 수신자는 primary_.find(service)가 반환한 유효한 optional<Endpoint> prvalue다. 성공이면
        //     독립 Endpoint를 소유하고 실패이면 disengaged다. this와 service는 호출 동안 유효하다.
        // (2) 선택 overload는 `template<class F> constexpr optional or_else(F&&) &&`이며 F는
        //     `[this, service]` lambda의 고유 타입이다. Constraints는 invocable<F>와
        //     move_constructible<Endpoint>이고, Mandates는
        //     `is_same_v<remove_cvref_t<invoke_result_t<F>>, optional<Endpoint>>`가 true인 것이다.
        // (3) lambda prvalue가 forwarding reference에 바인딩된다. 캡처한 this는 비소유 pointer,
        //     service는 string_view 값 복사다. callable을 저장하지 않고 빈 경로에서만 1회 호출한다.
        // (4) 반환형 optional<Endpoint> prvalue를 resolve의 반환값으로 사용한다. engaged 수신자는
        //     contained Endpoint를 결과로 이동하고, 빈 수신자는 lambda가 만든 fallback을 반환한다.
        // (5) primary 성공이면 lambda는 실행되지 않는다. 실패이면 std::string{service}가 조회 key의
        //     문자를 새로 소유하고 fallback_url_은 복사되어 결과가 resolver와 입력 view에 독립한다.
        //     engaged 경로에서만 rvalue 수신자의 Endpoint가 이동된다. disengaged 경로는 수신 값을
        //     이동하지 않고 f() 결과를 직접 반환한다. 어느 쪽이든 수신 임시는 전체 식 끝에 파괴된다.
        // (6) 상태 분기는 O(1), callable은 최대 1회다. 전체 비용은 Endpoint 이동 또는 두 string
        //     구성 비용이며 할당/bad_alloc 가능하다. optional wrapper 자체는 별도 heap을 요구하지
        //     않는다. dangling service/primary_/this, 동시 repository 변경은 허용되지 않는다.
        return primary_.find(service).or_else([this, service] {
            // [첫 생성 계약 A: std::string의 string-view 호환 생성자]
            // (1) 첫 string 목적 객체는 생성 전이고 service는 호출 동안 유효한 string_view lvalue다.
            // (2) `template<class T> explicit basic_string(const T&, const Allocator& = Allocator())`에서
            //     T=string_view인 view 호환 overload와 기본 allocator<char>가 선택된다.
            // (3) 인자 service는 비소유 const lvalue 입력이며 data()/size() 범위의 문자를 빌린다.
            //     allocator 기본 인자는 값으로 쓰이고 source 문자 소유권은 이동하지 않는다.
            // (4) 생성자는 별도 반환값 없이 소유 string prvalue를 만들며 Endpoint::service 초기값으로
            //     사용된다. 같은 타입 prvalue라 중간 string 복사/이동 없이 멤버를 직접 초기화할 수 있다.
            // (5) 새 string이 문자를 독립 소유하고 service와 원본 범위는 그대로다. 결과는 view 수명과
            //     분리되며 Endpoint가 파괴될 때 문자 저장소도 정리된다.
            // (6) 문자 수에 선형이고 SSO가 아니면 할당한다. 길이 초과 length_error와 bad_alloc이
            //     가능하며 예외 시 완성 string이 없다. 유효하지 않은 view 범위를 읽으면 UB이고,
            //     source를 다른 thread가 동시에 쓰면 데이터 경쟁이다. source 관찰자는 무효화되지 않는다.

            // [첫 생성 계약 B: std::string 복사 생성자]
            // (1) fallback_url_은 resolver가 소유하는 살아 있는 const string lvalue이고 두 번째 string
            //     목적 멤버는 생성 전이다.
            // (2) `basic_string(const basic_string&)`가 같은 char/traits/allocator 타입으로 선택된다.
            // (3) 유일한 인자 fallback_url_은 읽기 전용 lvalue다. 문자를 깊게 복사하고 원본 소유권과
            //     allocator/storage 수명을 빌려 두지 않는다.
            // (4) 생성자는 별도 반환값 없이 Endpoint::url의 수명을 시작한다.
            // (5) 성공 뒤 두 string은 같은 문자 값을 독립 소유하고 fallback_url_은 변하지 않는다.
            // (6) 문자 수에 선형이고 SSO가 아니면 할당/bad_alloc 가능이다. 예외 시 미완성 Endpoint의
            //     먼저 만든 service가 파괴되어 누수되지 않는다. 원본 iterator/reference는 무효화되지
            //     않으며, 원본 동시 쓰기와 복사는 데이터 경쟁이다.

            // [첫 생성 계약 C: optional<Endpoint> converting value constructor]
            // (1) optional 목적 저장소는 생성 전이고 Endpoint aggregate prvalue는 두 소유 string 멤버가
            //     모두 살아 있는 완성 값이다.
            // (2) `template<class U=T> constexpr optional(U&&)`에서 T=Endpoint, U=Endpoint가 추론되고
            //     contained value에는 Endpoint의 암시적 이동 생성자가 선택된다. in_place overload가 아니다.
            // (3) Endpoint prvalue가 U&&에 바인딩된 뒤 xvalue로 contained storage에 이동된다. 두 string
            //     저장소 소유권은 새 Endpoint로 넘어가며 외부 객체를 참조로 보관하지 않는다.
            // (4) 생성자는 별도 반환값 없이 engaged optional<Endpoint> prvalue를 만들어 lambda가 반환한다.
            // (5) 결과가 두 string을 소유하고 이동된 임시 Endpoint는 유효한 멤버 상태로 곧 파괴된다.
            // (6) 완성된 인자를 담는 wrapper 작업은 O(1)·별도 할당 없음이고 두 기본 allocator string을
            //     가진 Endpoint 이동도 noexcept다. 다만 표준의 `optional(U&&)` 선언 자체에는 noexcept
            //     명세가 없어 portable `noexcept(이 생성식)`이 true라고 요구할 수 없고, 구현은 이를 더
            //     강하게 보장할 수 있다. 표준상 선택된 T 생성자가 던진 예외만 전파한다. 아직 wrapper
            //     관찰자 무효화 대상은 없고 결과 공유 시 별도 동기화가 필요하다. optimizer가 이동을
            //     없앨 수 있어도 언어 수준의 converting-value 경로와 in_place 경로는 구별한다.
            return std::optional<Endpoint>{
                Endpoint{std::string{service}, fallback_url_}};
        });
    }

private:
    const EndpointRepository& primary_;
    std::string fallback_url_;
};

// const reference는 결과 Endpoint를 출력 동안만 빌리고 소유권이나 수명을 늘리지 않는다.
void print_endpoint(const Endpoint& endpoint) {
    // [첫 호출 계약: std::cout operator<< 연쇄]
    // (1) std::cout은 정상 상태인 std::ostream lvalue이고 두 string은 호출 동안 살아 있다.
    // (2) 선택된 특수화 기준 `ostream& operator<<(ostream&, const string&)` 두 번과
    //     `ostream& operator<<(ostream&, char)` 두 번이다. string overload의 template 인자는
    //     CharT=char, Traits=char_traits<char>, Allocator=allocator<char>이고 char overload의 Traits도
    //     char_traits<char>다. 여기서 ostream/string은 그 기본 template 특수화의 별칭이다.
    // (3) 각 free operator의 첫 인자는 최초 cout 또는 앞 호출이 반환한 ostream lvalue이고, 둘째는
    //     endpoint의 두 const string lvalue 또는 ' '/'\n' char prvalue다. 호출 동안만 읽고 소유권을
    //     넘기지 않는다.
    // (4) 각 호출은 같은 ostream&를 반환해 다음 호출이 사용하고 마지막 반환 참조는 버린다.
    // (5) 성공 시 출력 buffer/위치만 바뀌며 endpoint는 그대로다. 실패 시 failbit/badbit가 가능하다.
    // (6) 표준 점근 상한은 별도 지정되지 않고 비용·내부 할당은 문자 수와 streambuf/locale에 따른다.
    //     exception mask가 켜졌다면 ios_base::failure가 가능하다. 기본 동기화 상태의 표준 stream은
    //     문자 단위 데이터 경쟁을 막지만 네 삽입 전체의 레코드 원자성은 보장하지 않는다.
    std::cout << endpoint.service << ' ' << endpoint.url << '\n';
}

// 운영체제가 호출하는 진입점이다. int 0은 정상 종료 상태를 뜻한다.
int main() {
    // [첫 생성 계약: vector initializer_list와 Endpoint/string 생성]
    // (1) 수신자에 해당하는 임시 vector 목적 객체와 그 안의 두 Endpoint/string 목적 객체는
    //     아직 생성 전이며 repository 값 매개변수로 전달될 예정이다.
    // (2) `vector(initializer_list<Endpoint>, const Allocator& = Allocator())`가 선택되고 각
    //     Endpoint의 string은 `basic_string(const char*, allocator)`로 리터럴을 복사한다.
    // (3) vector 생성 인자 initializer_list의 원소는 const Endpoint lvalue로 보이며 vector가 각
    //     원소를 복사 소유한다. string 생성 인자 네 리터럴은 정적 수명의 const char 배열이고
    //     null 종료 포인터 prvalue로 변환된다.
    // (4) 생성자 반환값은 없고 vector prvalue가 EndpointRepository 값 매개변수를 직접 초기화한다.
    // (5) repository가 최종 vector와 문자들을 소유하며 임시 initializer_list 수명에 의존하지 않는다.
    // (6) 원소 수/문자 수에 선형, vector와 string 할당/bad_alloc 가능이다. 예외 시 이미 만든 원소는
    //     역순 파괴되고 누수되지 않는다. 완성 전 관찰자나 병행 접근은 없다.
    const EndpointRepository repository{std::vector<Endpoint>{
        Endpoint{std::string{"billing"}, std::string{"https://billing.internal"}},
        Endpoint{std::string{"catalog"}, std::string{"https://catalog.internal"}},
    }};

    // Resolver는 repository를 빌리고 fallback_url 문자를 자기 string으로 복사한 뒤 멤버로 이동한다.
    const EndpointResolver resolver{repository, std::string{"https://fallback.internal"}};

    // [첫 생성 계약: std::string_view(const char*)와 resolve 값 전달]
    // (1) 각 목적 string_view는 생성 전이고 문자열 리터럴 배열은 정적 수명으로 살아 있다.
    // (2) `constexpr basic_string_view(const char*)`가 char_traits::length로 길이를 구하는 형태로
    //     선택되고, 그 prvalue가 resolve의 값 매개변수를 직접 초기화한다.
    // (3) 인자는 null 종료 const char 배열 lvalue가 변환된 const char* prvalue다. view는 pointer와
    //     길이만 복사하고 문자를 소유하거나 수명을 늘리지 않는다.
    // (4) 생성자는 별도 반환값 없이 string_view prvalue를 만들고 resolve 인자로 사용한다.
    // (5) 리터럴과 resolver는 그대로이며 resolve 결과 optional은 found/missing을 직접 초기화한다.
    // (6) 길이 측정에 문자 수만큼 시간, 무할당·constexpr이며 null pointer나 종료 문자가 없는
    //     범위는 전제조건을 어긴다. 리터럴 수명 덕분에 호출 동안 dangling이 아니며 thread 쓰기도 없다.
    const std::optional<Endpoint> found{resolver.resolve("billing")};
    const std::optional<Endpoint> missing{resolver.resolve("search")};

    // [첫 호출 계약: optional::operator*() const &]
    // (1) found와 missing은 모두 살아 있는 const optional<Endpoint> lvalue이며 이 예제 불변식상 engaged다.
    // (2) 각 호출은 `constexpr const T& operator*() const & noexcept`, T=Endpoint를 선택한다.
    // (3) 데이터 인자는 없고 수신 객체만 읽는다. 소유권과 수명은 optional에 남는다.
    // (4) 반환 const Endpoint&를 print_endpoint의 const reference 인자로 즉시 사용한다.
    // (5) optional/Endpoint는 변하지 않고 반환 reference는 각 optional의 값 수명에 묶인다.
    // (6) O(1)·무할당·noexcept지만 disengaged 수신자 역참조는 UB다. 오늘 resolve의 postcondition이
    //     전제조건을 세운다. 같은 객체의 무동기 reset/대입과 겹치면 데이터 경쟁이다.
    print_endpoint(*found);
    print_endpoint(*missing);

    // 지역 객체는 선언의 역순으로 파괴된다. resolver가 먼저 사라져 reference가 dangling이 되기 전에
    // repository가 그 뒤까지 살아 있으므로 비소유 reference 수명 조건을 만족한다.
    return 0;
}
