// 오늘의 실무 패턴: endpoint 이름을 key로 갖는 registry가 설정 객체를 값으로 소유한다.
// std::unordered_map::try_emplace는 같은 이름이 이미 있으면 기존 설정을 보존하고, 없을 때만
// Endpoint를 구성한다. 따라서 재시도될 수 있는 등록 요청을 멱등하게 처리하는 경계를 표현한다.

// <cstddef>는 컨테이너 크기와 개수를 나타내는 부호 없는 기본 타입 별칭 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 출력 객체 std::cout과 형식화 삽입 연산자를 선언한다.
#include <iostream>
// <string>은 문자 저장소를 값으로 소유하는 std::string을 선언한다.
#include <string>
// <unordered_map>은 고유 key와 mapped 값을 node로 소유하는 std::unordered_map을 선언한다.
#include <unordered_map>
// <utility>는 이름 있는 lvalue를 이동 후보인 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>

// using은 새 타입을 만드는 문법이 아니라 기존 기본 타입 int에 업무 의미가 드러나는 별칭을 붙인다.
using EndpointPriority = int;

// struct는 기본 접근 지정자가 public이다. 단순 설정 값인 Endpoint는 주소 문자열을 직접 소유한다.
// class였다면 기본 접근은 private이며, 어느 쪽이든 public/private를 명시해 인터페이스를 나눌 수 있다.
struct Endpoint {
    // std::string의 template 인자는 char이며 allocator 등 나머지 정책은 표준 기본 인자를 사용한다.
    std::string address;
    // 기본 정수는 {}로 값 초기화하면 0이 된다. 중괄호는 위험한 narrowing도 더 엄격히 막는다.
    EndpointPriority priority{};

    // 생성자는 반환형이 없고 두 값 매개변수를 받는다. explicit은 두 인자의 braced 목록이 Endpoint로
    // 뜻하지 않게 암시 변환되는 것을 막고 Endpoint{...} 같은 직접 초기화만 의도적으로 허용한다.
    explicit Endpoint(std::string address_value, EndpointPriority priority_value) noexcept
        // [첫 호출 계약: std::move와 std::string 이동 생성]
        // (1) 자유 함수 move에는 수신 객체가 없다. address_value는 유효한 std::string lvalue이고
        //     목적 멤버 address는 아직 수명이 시작되지 않았다.
        // (2) move<string&>(string&) -> string&&와 basic_string(basic_string&&) noexcept를 선택한다.
        //     문자열의 문자·할당자 template 인자는 각각 char와 기본 allocator다.
        // (3) address_value 식은 lvalue이고 move 결과는 xvalue다. 문자열 저장소를 새 멤버가 소유해도
        //     된다는 뜻이며, priority_value는 별도 기본 int 값을 복사할 뿐이다.
        // (4) move는 string&&를 반환해 address 생성에 즉시 쓰고, 생성자 자체에는 반환값이 없다.
        // (5) 성공 뒤 address가 문자 값을 소유하고 address_value는 유효하지만 내용은 미지정이다.
        //     move 함수 자체는 아무 객체도 바꾸지 않으며 실제 상태 변경은 string 이동 생성이 한다.
        // (6) move는 O(1)·무할당·noexcept이고 수명을 늘리지 않는다. 기본 allocator의 string 이동
        //     생성도 상수 시간·noexcept이며 이 코드는 이전 문자 관찰자를 저장하지 않는다. 멤버는
        //     Endpoint와 함께 파괴되고 공유 변경에는 외부 동기화가 필요하다.
        : address{std::move(address_value)}, priority{priority_value} {}
};

// class는 기본 접근 지정자가 private이다. public API만 등록·값 검증·개수 관찰을 허용하고 저장소는 숨긴다.
class EndpointRegistry {
public:
    EndpointRegistry()
        // [첫 생성 계약: std::unordered_map 기본 생성]
        // (1) 목적 멤버 endpoints_는 아직 존재하지 않으며 외부 원소나 저장소를 받지 않는다.
        // (2) unordered_map() 기본 생성자를 Key=std::string, T=Endpoint와 기본 Hash, KeyEqual,
        //     Allocator template 인자로 선택한다.
        // (3) 명시 인자는 없다. 기본 hash·동등 비교자·allocator 정책 객체도 값으로 구성된다.
        // (4) 생성자는 반환값 없이 size()==0인 유효한 Storage owner의 수명을 시작한다.
        // (5) 성공 뒤 endpoints_는 비어 있고 Endpoint 객체는 아직 없으며 bucket_count는 미지정이다.
        // (6) 상수 시간이다. 이 정확한 std::hash<std::string>, std::equal_to<std::string>,
        //     std::allocator 정책 조합의 기본 생성에서는 예외가 전파되지 않는다. 구현의 내부 저장 방식과
        //     할당 횟수는 단정하지 않는다. 관찰자 무효화·공유 접근은 아직 없고 EndpointRegistry 파괴 시
        //     남은 key와 Endpoint의 수명도 함께 끝난다.
        : endpoints_{} {}

    // 두 std::string 값 매개변수는 호출에서 독립 owner가 된다. const T&는 복사 없이 빌리지만
    // owner 수명을 늘리지 않고, T*는 nullptr일 수 있고 재지정할 수 있다. 이 API는 재시도 실패 뒤에도
    // 호출자 저장소를 빌려 두지 않으려고 값을 받은 뒤 필요할 때 map으로 소유권을 넘긴다.
    [[nodiscard]] bool register_endpoint(std::string endpoint_name,
                                         std::string endpoint_address,
                                         EndpointPriority priority_value) {
        // [첫 호출 계약: std::unordered_map::try_emplace]
        // (1) 수신 endpoints_의 정확한 타입은 유효한 Storage lvalue이며 0개 이상의 고유 이름과
        //     Endpoint node를 소유한다.
        // (2) pair<iterator,bool> try_emplace(key_type&&, Args&&...)의 rvalue key overload를 선택한다.
        //     endpoint_address는 xvalue라 std::string, 이름 있는 priority_value는 lvalue라 int&로 추론되어
        //     Args={std::string, int&}이고, value_type은 pair<const string,Endpoint>이다.
        // (3) std::move(endpoint_name/address)는 두 lvalue를 xvalue로 표시하고 priority_value는 int
        //     lvalue 값이다. forwarding reference는 호출 동안만 빌리며 map이 삽입 성공 시 값을 소유한다.
        // (4) 반환 pair의 iterator는 새/기존 원소, bool은 새 삽입 여부다. prvalue가 insertion을 직접
        //     초기화해 중간 pair 복사·이동은 없고, 이 코드는 bool만 사용해 iterator를 보관하지 않는다.
        // (5) key가 없으면 이름과 Endpoint를 구성해 size가 1 증가하고 두 string 인자는 이동 후 상태가
        //     될 수 있다. key가 있으면 map과 두 인자는 바뀌지 않으며 기존 Endpoint를 덮어쓰지 않는다.
        // (6) 평균 O(1), hash 충돌 최악 O(n)이며 새 node/bucket 할당과 재해시가 가능하다. 재해시는
        //     iterator를 무효화하지만 원소 pointer/reference는 유지하고, erase/파괴는 해당 수명을 끝낸다.
        //     할당·동등 비교·key/mapped 구성 등 사용자 Hash 이외 예외에는 container 삽입 효과가 없다.
        //     사용자 Hash가 던지면 더 강한 no-effect 보장을 가정하지 않으며, 어느 예외도 이미 시작된
        //     호출자 인자의 이동을 원래 값으로 복원해 주지는 않는다. 같은 map의 병행 변경은 동기화한다.
        // 중요: 함수의 모든 인자 식은 호출 전에 평가된다. try_emplace가 중복 key에서 막는 것은
        // Endpoint 구성과 key/생성 인자로부터의 이동이지, 비싼 인자 식 자체의 계산은 아니다.
        const auto insertion{endpoints_.try_emplace(std::move(endpoint_name),
                                                    std::move(endpoint_address),
                                                    priority_value)};

        // if는 bool 값을 load해 조건 분기한다. true 경로와 false 경로 모두 소유 iterator를 반환하지 않는다.
        if (insertion.second) {
            return true;
        }
        return false;
    }

    // const 참조 매개변수는 호출자가 소유한 기대값을 함수 동안만 빌리며 map이나 문자열에 저장하지 않는다.
    [[nodiscard]] bool matches_endpoint(const std::string& endpoint_name,
                                        const std::string& expected_address,
                                        EndpointPriority expected_priority) const {
        // [첫 호출 계약: unordered_map::find/end, const_iterator 비교·operator->, string operator==]
        // (1) 수신 endpoints_의 정확한 타입은 원소 수명이 유지되는 const Storage lvalue다. 두 문자열
        //     인자는 살아 있는 const std::string lvalue이고 같은 map에 병행 쓰기는 없다.
        // (2) const_iterator find(const key_type&) const, const_iterator end() const noexcept,
        //     const_iterator의 operator==와 operator->, bool operator==(const string&,const string&)
        //     overload를 사용한다.
        // (3) endpoint_name과 expected_address는 비소유 const lvalue 참조, expected_priority는 int 값이다.
        //     find는 key를 읽어 빌리고, position lvalue와 end()의 iterator prvalue는 같은 map 소속이다.
        // (4) find의 const_iterator prvalue가 position을 직접 초기화한다. end는 끝 sentinel, iterator
        //     비교는 bool, operator->는 const 원소 pointer, 문자열 비교는 내용 동일 여부 bool을 반환하며
        //     모두 아래 존재·값 검증에 사용한다.
        // (5) 호출 뒤 map, bucket, key, Endpoint와 인자는 변하지 않고 어떤 관찰자도 무효화되지 않는다.
        // (6) find는 평균 O(1)·충돌 최악 O(n), end/iterator 연산은 O(1), 문자열 비교는 최악 O(문자 수)다.
        //     무할당이고 누락 key는 예외 대신 end다. find는 noexcept 선언이 아니며 정책 예외를 전파할 수
        //     있으나 이 Storage에는 사용자 정책 상태가 없다. iterator/pointer는 map 변경·파괴 뒤 dangling이고,
        //     end 역참조는 UB지만 if가 배제한다. 동시 읽기는 가능하되 쓰기와 겹치면 외부 동기화가 필요하다.
        const auto position{endpoints_.find(endpoint_name)};
        // if는 먼저 끝 sentinel 여부를 비교하므로 없는 key에서는 operator->를 평가하지 않는다.
        if (position == endpoints_.end()) {
            return false;
        }
        const Endpoint& endpoint{position->second};
        return endpoint.address == expected_address && endpoint.priority == expected_priority;
    }

    // 뒤의 const는 이 함수가 registry의 논리 상태를 바꾸지 않는다는 뜻이다.
    [[nodiscard]] std::size_t size() const noexcept {
        // [첫 호출 계약: unordered_map::size]
        // (1) 수신 endpoints_는 이 EndpointRegistry가 소유하는 유효한 const 관찰 가능 Storage lvalue다.
        // (2) size_type size() const noexcept를 선택하며 size_type은 std::size_t다.
        // (3) 명시 인자와 소유권 이동은 없다.
        // (4) 현재 node 개수의 독립된 std::size_t prvalue를 반환해 호출자의 count를 초기화한다.
        // (5) map, 원소, bucket, 인자와 모든 관찰자는 바뀌거나 무효화되지 않는다.
        // (6) O(1), 무할당·비투척·무효화 없음이다. 반환 정수의 수명은 map과 독립적이지만 이후
        //     변경 뒤에는 과거 크기일 뿐이다. 같은 객체의 병행 변경과 겹치지 않게 외부 동기화한다.
        return endpoints_.size();
    }

private:
    // 첫 template 인자는 key, 둘째는 mapped 타입이다. map이 key와 Endpoint node의 소유자다.
    using Storage = std::unordered_map<std::string, Endpoint>;
    Storage endpoints_;
};

// 반환 prvalue EndpointRegistry{}가 호출자의 결과 객체를 직접 초기화한다. 이는 C++17의 보장된
// prvalue 복사 생략(넓게 RVO라고 부름)이며, 이름 있는 local을 return하는 선택적 NRVO와 다르다.
[[nodiscard]] EndpointRegistry make_endpoint_registry() {
    return EndpointRegistry{};
}

int main() {
    // factory 결과 prvalue가 registry를 직접 구성하므로 EndpointRegistry 복사/이동은 필요하지 않다.
    // 이름이 생긴 registry 식은 선언 타입과 무관하게 lvalue이며 main scope 끝까지 저장소를 소유한다.
    EndpointRegistry registry{make_endpoint_registry()};

    // [첫 생성 계약: std::string의 C 문자열 생성자]
    // (1) 각 목적 std::string은 아직 없고 문자열 literal 배열은 프로그램 끝까지 살아 있다.
    // (2) basic_string(const char*, const Allocator& = Allocator())를 char와 기본 allocator로 선택한다.
    // (3) literal은 const char[N] lvalue 배열에서 non-null const char*로 변환되며 '\0'로 끝난다.
    //     문자를 읽어 독립 저장소에 복사하고 literal의 소유권이나 pointer를 보관하지 않는다.
    // (4) 생성자는 반환값이 없다. 각 string prvalue는 register_endpoint의 값 매개변수를 직접 만들거나
    //     matches_endpoint의 const 참조에 그 전체 식 동안 바인딩된다.
    // (5) 성공한 문자열은 literal과 독립적으로 같은 문자 열을 소유한다. 아래 인자 식들은 모두 호출
    //     전에 평가되지만 서로 사이의 평가 순서에 의존하지 않는다.
    // (6) 문자 길이에 O(n), 저장에도 O(n)이며 length_error/bad_alloc이 가능하다. null pointer나
    //     종단 없는 범위는 전제 위반이다. 성공한 문자 pointer는 해당 string 변경/파괴 규칙에 묶이고,
    //     여기서는 외부에 저장하지 않으며 객체를 여러 thread와 공유하지 않는다.
    const bool first_insert{registry.register_endpoint(
        std::string{"billing"}, std::string{"https://billing/v1"}, 10)};
    const bool duplicate_insert{registry.register_endpoint(
        std::string{"billing"}, std::string{"https://billing/v2"}, 20)};
    const bool second_insert{registry.register_endpoint(
        std::string{"search"}, std::string{"https://search/v1"}, 30)};

    // size 반환은 소유권 없는 정수 prvalue라 count가 독립적으로 값을 보존한다.
    const std::size_t count{registry.size()};
    const bool first_value_preserved{registry.matches_endpoint(
        std::string{"billing"}, std::string{"https://billing/v1"}, 10)};

    // 두 번째 고유 endpoint와 중복 등록 뒤 최초 값 보존을 함께 확인하되 성공 stdout은 바꾸지 않는다.
    if (!second_insert || !first_value_preserved) {
        return 1;
    }

    // [첫 호출 계약: std::cout의 operator<< 연쇄]
    // (1) 왼쪽 수신은 유효한 std::ostream lvalue std::cout이며 이 프로그램의 한 thread만 쓴다.
    // (2) bool에는 basic_ostream<char>::operator<<(bool), size_t에는 ABI의 대응 unsigned 정수 멤버,
    //     문자에는 operator<<(basic_ostream<char>&, char) overload가 선택된다.
    // (3) 두 bool과 count는 lvalue에서 값으로 읽고 공백/개행은 char prvalue다. 소유권 이동은 없다.
    // (4) 각 호출은 같은 ostream&를 반환해 다음 <<의 수신으로 쓰고 마지막 반환 참조는 버린다.
    // (5) boolalpha가 꺼진 기본 상태라 성공 시 정확히 "1 0 2\n"을 기록하며 registry는 그대로다.
    // (6) 표준은 연쇄 전체의 공통 점근 복잡도나 내부 할당을 고정하지 않는다. locale·buffer·장치
    //     비용이 있고 실패는 상태 bit와 설정된 ios 예외로 보고된다. 다른 객체 관찰자를 무효화하지
    //     않지만 여러 thread의 복합 출력 원자성은 보장하지 않는다.
    std::cout << first_insert << ' ' << duplicate_insert << ' ' << count << '\n';

    // 기계 실행 관점에서 try_emplace는 key 문자 load, hash 계산, bucket 탐색, 문자열 비교와 조건 분기,
    // 성공 시 node allocation 및 pointer/link store를 만들 수 있다. size는 저장된 개수 load가 될 수 있다.
    // 이 타입들에는 virtual 함수가 없어 가상 간접 호출을 요구하지 않지만 라이브러리·allocator 내부까지
    // 특정 호출 형태라고 단정할 수 없다. 실제 명령, 인라이닝, 분기·cache 비용은 CPU, ABI, 표준
    // 라이브러리, compiler와 최적화 옵션에 따라 달라진다.
    return 0;
}
