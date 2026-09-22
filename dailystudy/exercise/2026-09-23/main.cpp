#include <cstddef>  // std::size_t: 열 개수처럼 음수가 아닌 크기를 표현한다.
#include <iostream> // std::cout와 출력 스트림 삽입 연산자를 선언한다.
#include <optional> // std::optional/nullopt: 검증 성공 또는 거부를 값으로 표현한다.
#include <ranges>   // std::views::zip: 여러 열을 복사 없이 행처럼 함께 순회한다.
#include <string>   // std::string: 서비스 이름과 결과 이름을 독립 소유한다.
#include <tuple>    // std::get: zip 반복자가 돌려준 tuple-like 행의 세 참조를 구조적 바인딩한다.
#include <utility>  // std::move: 소유 열을 새 객체로 옮길 수 있는 xvalue 식을 만든다.
#include <vector>   // std::vector: 같은 타입의 열 데이터를 연속 동적 저장소에 소유한다.

// using은 새 타입을 만드는 문법이 아니라 긴 기존 타입에 읽기 쉬운 별칭을 붙인다.
// vector의 템플릿 인자는 각각 string과 int이며, int는 오늘의 작은 밀리초 값에 충분한 기본 정수 타입이다.
using ServiceNames = std::vector<std::string>;
using Milliseconds = std::vector<int>;

// 도메인 상한을 두면 아래의 int 뺄셈이 [-10^9, 10^9] 안이라 부호 있는 오버플로가 없다.
constexpr int maximum_latency_ms{1'000'000'000};

// 반환형 bool은 열 전체가 허용 범위인지 나타내고, const& 매개변수는 vector를 읽기만 빌린다.
[[nodiscard]] bool valid_latency_column(const Milliseconds& values) noexcept {
    // [첫 호출 계약: const std::vector<int> range-for의 숨은 begin/end/비교/역참조/증가]
    // (1) 수신 객체 values는 살아 있는 const vector<int> lvalue이고 순회 동안 구조를 바꾸지 않는다.
    // (2) const begin()/end(), const_iterator의 operator*, 전위 operator++가 선택되고, 루프 조건의
    //     `begin != end`는 C++20 비교 재작성으로 operator== 후보를 부정해 평가한다.
    // (3) 숨은 range는 const 참조로 빌리고 역참조한 const int&의 값을 int value로 복사한다.
    // (4) begin/end는 const_iterator 값을, operator==는 부정될 bool을, 역참조는 복사에 쓰는 const int&를,
    //     전위 ++는 const_iterator&를 반환한다. 비교 bool만 분기에 쓰고 증가식의 반환 참조는 버린다.
    // (5) iterator만 전진하고 vector의 크기·용량·원소·소유권은 그대로다.
    // (6) 전체 O(N), 무할당이다. begin/end는 noexcept이고 여기의 int 비교도 던지지 않지만 일반 반복자
    //     연산 전체를 무조건 noexcept라 일반화할 수는 없다. owner 수명과 iterator 유효성이 전제이며,
    //     같은 vector의 동시 쓰기는 데이터 경쟁이다.
    for (const int value : values) {
        if (value < 0 || value > maximum_latency_ms) {
            return false;
        }
    }
    return true;
}

// struct는 기본 접근이 public이다. 검증 전 세 열을 전달하는 단순 값 묶음이므로 struct가 알맞다.
struct LatencyColumns {
    ServiceNames service_names;
    Milliseconds observed_ms;
    Milliseconds budget_ms;
};

// class는 기본 접근이 private이다. 길이가 같은 열만 존재한다는 불변식을 factory 뒤에 감춘다.
class LatencyTable {
public:
    // 반환형 optional은 실패를 예외나 특별한 포인터 값이 아니라 "값 없음"으로 명시한다.
    // 매개변수를 값으로 받아 factory가 성공하면 세 vector의 소유권을 결과로 이전할 수 있다.
    [[nodiscard]] static std::optional<LatencyTable> create(LatencyColumns columns) {
        // [첫 호출 계약: 세 std::vector 특수화의 size]
        // (1) 수신 객체는 살아 있는 vector<string> 하나와 vector<int> 둘이며 각자 0개 이상의 원소를 소유한다.
        // (2) 각 호출은 해당 특수화의 size_type size() const noexcept 오버로드를 인자 없이 선택한다.
        // (3) 명시적 인자는 없고 const 관찰만 하므로 원소·버퍼의 소유권이나 값 범주를 바꾸지 않는다.
        // (4) 각 원소 수를 size_t prvalue로 반환하며 세 지역 상수에 저장해 폭 비교에 사용한다.
        // (5) 호출 뒤 세 vector의 크기·용량·원소·관찰자는 모두 그대로다.
        // (6) 각 호출은 O(1), 무할당·무효화·무예외다. 살아 있는 수신 객체가 전제이며 동시 구조 변경은 금지다.
        const std::size_t name_count{columns.service_names.size()};
        const std::size_t observed_count{columns.observed_ms.size()};
        const std::size_t budget_count{columns.budget_ms.size()};

        // if는 bool 조건을 비교해 폭 불일치인 실패 경로와 성공 경로 중 하나로 분기한다.
        if (name_count != observed_count || name_count != budget_count ||
            !valid_latency_column(columns.observed_ms) ||
            !valid_latency_column(columns.budget_ms)) {
            // [첫 호출 계약: std::optional<LatencyTable>의 nullopt 생성]
            // (1) 반환 optional 객체는 아직 없고, 표준 nullopt 태그 객체는 프로그램 동안 유효하다.
            // (2) optional(nullopt_t) 생성자를 선택하며 contained LatencyTable은 생성하지 않는다.
            // (3) std::nullopt는 복사되는 태그 값이고 열 저장소나 소유권을 받지 않는다.
            // (4) 생성자는 별도 반환값이 없고 disengaged optional prvalue가 create의 반환값이 된다.
            // (5) 길이 불일치나 음수/10^9 초과 값을 가진 columns는 이동되지 않고 호출자는 거부 상태를 받는다.
            // (6) O(1), 무할당·noexcept이며 참조 무효화가 없다. 서로 다른 optional 사용에는 동기화가 필요 없다.
            return std::nullopt;
        }

        // [첫 호출 계약: std::move와 optional의 값 변환 생성]
        // (1) 함수에는 표준 수신 객체가 없고 columns는 검증을 통과한 LatencyColumns lvalue이며 반환 optional은 아직 없다.
        // (2) move<LatencyColumns&>(columns)는 LatencyColumns&&를 만든다. 반환 변환은 대표적으로
        //     template<class U=T> explicit(!is_convertible_v<U,T>) optional(U&&)에서 U=LatencyTable인 생성자다.
        // (3) columns 식은 lvalue, move 결과는 xvalue다. 세 vector를 소비해도 된다는 뜻이지 move 자체가
        //     바이트를 옮기는 것은 아니다. 이어 LatencyTable prvalue가 optional의 소유 값으로 이동한다.
        // (4) move는 xvalue 참조를 반환해 private 생성자에 사용되고, create는 engaged optional 값을 반환한다.
        // (5) 성공 결과가 세 열을 독점 소유하며 지역 columns와 중간 table은 유효하지만 이동 후 상태가 된다.
        // (6) 기본 allocator vector 이동은 O(1)·무할당이고 여기의 table 이동도 noexcept다. optional 값
        //     생성은 그 이동의 보장을 따른다. 외부 관찰자는 아직 없고 같은 source에 동시 접근하지 않는다.
        return LatencyTable{std::move(columns)};
    }

    [[nodiscard]] std::size_t row_count() const noexcept {
        // 앞에서 설명한 vector<string>::size()와 같은 무변경 O(1) 관찰이며 반환값을 그대로 전달한다.
        return service_names_.size();
    }

    // const& 참조 한정자는 수명이 확인된 lvalue table에서만 비소유 행 view를 만들게 한다.
    [[nodiscard]] auto rows() const & {
        // [첫 호출 계약: std::views::zip]
        // (1) 함수 객체에 사용자 수신자는 없고, 세 수신 열은 같은 길이인 const vector lvalue로 살아 있다.
        // (2) 대표 operator()<viewable_range... Rs>(Rs&&...)에서 Rs는 const ServiceNames&,
        //     const Milliseconds&, const Milliseconds&다. 반환 특수화는 정확히 세 ref_view를 가진
        //     zip_view<ref_view<const ServiceNames>, ref_view<const Milliseconds>, ref_view<const Milliseconds>>다.
        // (3) 세 인자는 const lvalue라 저장소를 넘기지 않고 ref-view 형태로 빌린다. factory 불변식상 길이가 같다.
        // (4) 세 현재 원소를 tuple-like 참조로 투영하는 zip view prvalue를 반환해 range-for가 즉시 사용한다.
        // (5) 원본 열은 변하지 않고 반환 view만 비소유 참조를 보관한다. 가장 짧은 열에서 끝나지만 길이는 같다.
        // (6) 고정된 세 범위 포장은 O(1)·무할당이고 이 입력에는 던지는 하위 연산이 없지만 공개 호출 전체가
        //     표준상 무조건 noexcept인 것은 아니다. table 파괴/재할당 뒤 사용은 dangling이고 동시 쓰기는 금지다.
        return std::views::zip(service_names_, observed_ms_, budget_ms_);
    }

    // 임시 table에서 얻은 view는 문장 끝에 곧바로 dangling이 되므로 non-const/const rvalue를 모두 막는다.
    void rows() && = delete;
    void rows() const && = delete;

    LatencyTable(const LatencyTable&) = default;
    LatencyTable& operator=(const LatencyTable&) = default;
    LatencyTable(LatencyTable&&) noexcept = default;
    LatencyTable& operator=(LatencyTable&&) noexcept = default;

private:
    // explicit은 LatencyColumns 하나가 뜻하지 않게 LatencyTable로 암시 변환되는 것을 막는다.
    explicit LatencyTable(LatencyColumns&& columns)
        // [첫 호출 계약: std::move와 세 std::vector 이동 생성]
        // (1) 각 목적 멤버는 아직 생성 전이고 columns의 세 source vector는 유효한 lvalue다.
        // (2) 각 move<T&>(source)는 T&&를 반환하고 allocator 인자 없는 vector(vector&&) 생성자를 선택한다.
        // (3) 세 source 식은 lvalue, move 결과는 xvalue이며 각 동적 저장소의 독점 소유권 이전을 허용한다.
        // (4) move의 반환 참조는 각 멤버 초기화에 사용하고 vector 생성자는 별도 반환값이 없다.
        // (5) 세 멤버가 이전 값을 소유하고 source vector는 파괴·재대입 가능한 유효한 값 미지정 상태가 된다.
        // (6) 기본 allocator에서는 각 이동이 O(1)·noexcept·무할당이다. 기존 source 원소 관찰자는
        //     destination 원소를 가리키지만 이 코드에는 노출된 관찰자가 없고 동시 접근도 없다.
        : service_names_{std::move(columns.service_names)},
          observed_ms_{std::move(columns.observed_ms)},
          budget_ms_{std::move(columns.budget_ms)} {}

    ServiceNames service_names_;
    Milliseconds observed_ms_;
    Milliseconds budget_ms_;
};

// 결과는 원본 table의 string 참조가 아니라 독립 소유 string을 담으므로 table보다 오래 살 수 있다.
struct Breach {
    std::string service;
    int over_budget_ms{}; // 빈 중괄호 초기화는 기본값을 0으로 만든다.
};

using BreachList = std::vector<Breach>;

// 반환형은 소유 vector 값이고 매개변수 const&는 table을 복사하지 않고 호출 동안 읽기만 빌린다.
[[nodiscard]] BreachList collect_breaches(const LatencyTable& table) {
    // [첫 호출 계약: std::vector<Breach> 기본 생성]
    // (1) breaches 객체와 원소 저장소는 아직 존재하지 않는다.
    // (2) vector() 기본 생성자를 선택하며 템플릿 인자는 Breach, allocator는 기본값이다.
    // (3) 명시 인자나 이전 저장소 소유권은 없다.
    // (4) 생성자는 별도 반환값 없이 size 0인 vector 객체를 완성한다.
    // (5) 빈 소유 컨테이너가 생기고 아직 Breach 객체의 수명은 시작하지 않는다.
    // (6) 기본 allocator에서는 O(1)·무할당·무예외다. 무효화할 관찰자는 없고 함수 지역 수명이다.
    BreachList breaches{};

    // [첫 호출 계약: std::vector<Breach>::reserve]
    // (1) 수신 객체 breaches는 비어 있는 유효한 vector lvalue이고 table.row_count()는 최대 필요한 원소 수다.
    // (2) void reserve(size_type new_capacity)를 선택한다.
    // (3) row_count의 size_t prvalue를 값으로 전달하며 table이나 그 열의 소유권은 빌리지 않는다.
    // (4) 반환형은 void라 저장할 값이 없다.
    // (5) 성공하면 capacity가 최소 요청값이고 size는 0이다. 기존 원소나 iterator는 아직 없다.
    // (6) O(N) 상한과 O(N) 저장 공간, length_error/bad_alloc 가능성이 있다. 재할당이면 기존 관찰자는
    //     모두 무효화되지만 여기에는 없고, 실패 시 기존 vector는 유지된다. 동시 접근은 하지 않는다.
    breaches.reserve(table.row_count());

    // [첫 호출 계약: zip view range-for의 숨은 begin/end/비교/역참조/증가]
    // (1) table은 살아 있는 const lvalue이고 rows()가 만든 임시 zip view는 루프 전체로 수명이 연장된다.
    // (2) 숨은 `auto&& range`가 비-const 이름이어도 모든 기반 ref_view가 simple-view라 `begin() const`와
    //     `end() const`, iterator<true>::operator*() const와 전위 operator++()가 선택된다. 루프의
    //     `begin != end`는 C++20 비교 재작성으로 iterator<true>의 operator== 후보를 부정한다.
    // (3) 숨은 range 변수는 view prvalue를 &&로 붙잡는다. 각 I=0/1/2는 컴파일 시간 size_t 값이고,
    //     ADL로 고른 std::get<I>에는 역참조가 만든 tuple<const string&,const int&,const int&> xvalue가 간다.
    // (4) begin/end는 이 세 vector가 common+sized random-access라 같은 iterator<true> 값을 반환한다.
    //     get<I>(tuple&&)의 T&& 반환은 참조 원소 타입과 축약되어 각 const 원소 lvalue 참조가 된다.
    // (5) 전위 ++는 iterator&를 반환해 세 기반 iterator만 전진한다. proxy tuple은 반복 끝에 사라지지만
    //     table 원소와 바인딩 참조는 바뀌지 않고, 세 구조적 바인딩 이름은 현재 반복 안에서만 사용한다.
    // (6) 고정 세 열의 각 숨은 연산과 get은 O(1)·무할당이고 get은 noexcept다. 다른 공개 연산까지
    //     무조건 noexcept는 아니다. 범위 밖 I는 컴파일 실패이며 owner 파괴·재할당·동시 수정 뒤 참조
    //     사용은 dangling/미정의 동작 또는 데이터 경쟁을 일으킨다.
    for (auto&& [service, observed, budget] : table.rows()) {
        // if와 continue는 예산 이하 행에서 현재 반복의 나머지만 건너뛴다.
        if (observed <= budget) {
            continue;
        }

        // [첫 호출 계약: std::string 복사 생성과 std::vector<Breach>::push_back]
        // (1) 수신 객체 breaches는 예약된 vector lvalue이고 service는 table이 소유한 const string lvalue다.
        // (2) basic_string(const basic_string&)로 Breach의 이름을 만들고 void push_back(Breach&&)를 선택한다.
        // (3) service는 읽기만 빌려 깊게 복사한다. Breach{...}는 prvalue라 새 원소로 이동되며 table 소유권은 없다.
        // (4) 두 생성에는 별도 반환값이 없고 push_back도 void라 호출 결과는 저장하지 않는다.
        // (5) 성공하면 끝에 독립 이름/양의 초과량이 생겨 size가 1 증가한다. 전달 prvalue는 이동 후 파괴된다.
        // (6) 이름 길이에 선형, push_back은 분할 상환 O(1)이다. reserve 범위라 재할당은 없어서 기존 원소
        //     참조는 유지되지만 기존 past-the-end iterator는 무효다. string 복사는 bad_alloc을 던질 수 있고
        //     실패 시 기존 원소는 유지된다. 같은 breaches 객체의 동시 읽기/쓰기는 외부 동기화가 필요하다.
        // factory가 두 값을 [0,10^9]로 제한했고 이 분기는 observed>budget이므로 뺄셈은 int에 안전하다.
        breaches.push_back(Breach{service, observed - budget});
    }

    // [첫 호출 계약: 반환 시 std::vector<Breach>의 선택적 이동 생성]
    // (1) 수신 결과 객체는 아직 없고 지역 breaches는 모든 결과를 소유한 유효한 vector lvalue다.
    // (2) NRVO가 적용되지 않으면 allocator 인자 없는 vector(vector&&) noexcept 이동 생성자를 선택한다.
    // (3) return의 이동 적격 지역식은 overload 선택에서 xvalue처럼 취급되어 저장소 소유권을 넘긴다.
    // (4) 생성자는 별도 반환값이 없고 함수 호출의 vector prvalue가 호출자의 결과 객체를 초기화한다.
    // (5) 목적 vector가 저장소를 소유하며 source는 파괴 가능한 유효한 값 미지정 상태가 된 뒤 소멸한다.
    // (6) 기본 allocator에서 O(1)·무할당·noexcept다. 기존 원소 관찰자는 목적 저장소를 계속 가리킬 수
    //     있지만 밖에 노출되지 않았다. NRVO이면 이 호출 자체가 생략되고, 함수는 동시 접근을 만들지 않는다.
    return breaches;
}

int main() {
    // [첫 호출 계약: std::string C 문자열 생성과 std::vector initializer_list 생성]
    // (1) 세 vector와 그 원소는 아직 없고 문자열 리터럴은 정적 수명의 NUL 종료 배열이다.
    // (2) basic_string(const char*) 세 번과 vector(initializer_list<T>) 세 번을 선택한다.
    // (3) 리터럴 포인터는 문자를 빌려 복사하고, initializer_list 원소는 const라 string/int를 vector로 복사한다.
    // (4) 생성자는 별도 반환값 없이 각 임시 소유 vector를 완성해 LatencyColumns 필드가 되게 한다.
    // (5) 세 vector가 3개씩의 값을 독점 소유하며 리터럴과 정수 원본은 변하지 않는다.
    // (6) 전체 원소/문자 수에 선형이고 length_error/bad_alloc 가능성이 있다. 실패 시 이미 만든 임시는
    //     정리되며 외부 관찰자는 없다. 같은 객체를 여러 스레드에서 만들거나 수정하지 않는다.
    const std::optional<LatencyTable> table_result{LatencyTable::create(LatencyColumns{
        ServiceNames{std::string{"auth"}, std::string{"billing"}, std::string{"cache"}},
        Milliseconds{90, 132, 60},
        Milliseconds{100, 120, 50}})};

    // [첫 호출 계약: std::optional<LatencyTable>::has_value]
    // (1) 수신 table_result는 살아 있는 const optional lvalue이며 factory 결과 한 개를 소유하거나 비어 있다.
    // (2) constexpr bool has_value() const noexcept를 인자 없이 선택한다.
    // (3) 인자·소유권 이동이 없고 contained table을 생성·파괴하지 않는다.
    // (4) engaged 여부 bool prvalue를 반환해 !와 if 조건에서 즉시 사용한다.
    // (5) optional과 contained 객체의 상태·수명은 그대로다.
    // (6) O(1), 무할당·무효화·무예외다. 같은 optional의 동시 쓰기 중 관찰은 허용되지 않는다.
    if (!table_result.has_value()) {
        return 1;
    }

    // [첫 호출 계약: std::optional<LatencyTable>::value const&]
    // (1) 앞의 검사로 table_result가 engaged인 const optional lvalue임이 보장된다.
    // (2) const T& value() const&에서 T=LatencyTable인 overload를 선택한다.
    // (3) 인자나 소유권 이전은 없고 optional 내부 객체를 읽기 전용으로 빌린다.
    // (4) const LatencyTable&를 반환해 table 참조에 저장한다.
    // (5) optional과 table은 변하지 않고 참조는 table_result보다 먼저 사용을 끝낸다.
    // (6) O(1)·무할당이다. 비어 있으면 bad_optional_access지만 사전 검사로 배제했다. reset/파괴 시 참조가
    //     무효가 되며, 같은 객체의 동시 변경은 외부 동기화가 필요하다.
    const LatencyTable& table{table_result.value()};

    // 함수 반환 prvalue가 breaches의 결과 객체를 직접 초기화할 수 있고, 함수 안 지역에는 NRVO가 가능하다.
    const BreachList breaches{collect_breaches(table)};

    // [첫 호출 계약: std::vector<Breach>::size와 ostream 문자열/size_t 삽입]
    // (1) breaches는 완성된 const vector lvalue이고 std::cout은 출력 가능한 ostream lvalue다.
    // (2) size_type size() const noexcept, 비멤버 operator<<(ostream&, const char*) 및 이 w64devkit에서
    //     size_t가 unsigned long long이므로 basic_ostream<char>::operator<<(unsigned long long)를 선택한다.
    // (3) size에는 인자가 없고 리터럴은 NUL 종료 문자를 빌리며 size prvalue는 값으로 서식화한다.
    // (4) size는 원소 수를 반환해 출력에 사용하고 각 삽입은 같은 ostream&를 반환해 다음 호출로 잇는다.
    // (5) breaches는 그대로이고 성공한 문자만 출력 버퍼/위치에 추가된다.
    // (6) size는 O(1)·무예외다. 스트림 비용은 문자 수/장치에 따르며 실패는 상태 비트나 설정된 예외로
    //     나타난다. vector 관찰자는 무효화되지 않고 복합 출력의 동시 사용에는 외부 동기화가 필요하다.
    std::cout << "breaches=" << breaches.size();

    // [첫 호출 계약: std::vector<Breach> range-for의 숨은 연산]
    // (1) breaches는 루프 동안 살아 있고 구조 변경이 없는 const vector lvalue다.
    // (2) vector<Breach>::begin() const/end() const와 const_iterator 역참조·전위 증가를 선택한다.
    //     `begin != end`는 C++20 비교 재작성으로 operator== 후보를 부정해 bool을 만든다.
    // (3) 숨은 range는 const 참조로 빌리고 각 역참조 결과 const Breach&를 breach에 바인딩한다.
    // (4) begin/end는 const_iterator 값, operator*는 const Breach&, 전위 ++는 const_iterator&를 반환한다.
    // (5) iterator만 전진하며 vector/원소/용량은 변하지 않고 breach 참조는 현재 반복 동안 유효하다.
    // (6) begin/end는 O(1), 전체는 O(N), 무할당이다. 표준 vector 관찰 연산은 이 경로에서 던지지 않는다.
    //     owner/iterator 수명이 전제이며 같은 vector의 동시 구조 변경은 무효화와 데이터 경쟁을 만든다.
    for (const Breach& breach : breaches) {
        // [첫 호출 계약: ostream char/string/int 삽입]
        // (1) 수신 객체 std::cout은 같은 유효한 ostream lvalue이고 breach와 그 소유 string이 살아 있다.
        // (2) operator<<(ostream&, char), operator<<(ostream&, const string&), 멤버 operator<<(int)를 고른다.
        // (3) ','와 ':'는 char prvalue, service는 const string lvalue, 초과량은 int lvalue 값이며 모두 빌리거나 복사한다.
        // (4) 각 호출은 같은 ostream&를 반환해 체인에 쓰고 마지막 반환 참조는 버린다.
        // (5) 출력 버퍼만 `,이름:초과량`만큼 바뀌고 breach는 변하지 않는다.
        // (6) 출력 비용은 문자 수/장치에 따르고 할당 여부는 구현 버퍼에 달렸다. 실패는 상태 비트/설정 예외로
        //     보고되며 참조 무효화는 없다. 같은 스트림의 복합 출력은 외부 동기화 없이 섞지 않는다.
        std::cout << ',' << breach.service << ':' << breach.over_budget_ms;
    }
    // 앞에서 설명한 char 삽입과 같은 overload이며 반환 참조는 버린다.
    std::cout << '\n';

    /*
    값 범주·수명·소유권·기계 실행 관점:
    table_result, table, breaches는 이름 있는 lvalue다. LatencyColumns{...}, factory/collect의 값 반환은
    prvalue이고 std::move(columns)는 xvalue다. zip view는 세 vector를 소유하지 않으므로 table보다 오래
    보관하면 안 되며, Breach의 string 깊은 복사가 결과 수명을 원본에서 분리한다. factory의 optional과
    collect의 vector 반환은 결과 객체 직접 구성/NRVO와 이동 대상이지만, 이름 있는 객체가 언제나 자동으로
    이동되는 것은 아니다. raw pointer는 쓰지 않는다. 참조도 비소유 관찰자이므로 owner 수명이 먼저 끝나야 한다.

    실행 시 열 길이 load·비교와 조건 분기, zip iterator의 병렬 주소 전진, 지연 시간 load/비교, 결과 string
    복사와 vector store, 스트림 서식화가 나타날 수 있다. 사용자 정의 도메인 타입에는 virtual 함수가 없어
    그 객체에 대한 가상 간접 호출은 필요하지 않지만, iostream 내부의 streambuf 경로에는 구현에 따라
    가상 호출이 나타날 수 있다. 실제 명령, 인라이닝, 복사 생략, 분기 제거와 메모리 배치는 CPU·ABI·표준
    라이브러리·컴파일러·최적화 옵션에 따라 달라지므로 특정 어셈블리로 단정하지 않는다.
    */

    return 0;
}
