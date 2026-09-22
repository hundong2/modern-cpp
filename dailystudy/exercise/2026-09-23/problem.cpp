#include <cstddef>  // std::size_t: 열 너비와 결과 개수에 쓰는 부호 없는 크기 타입이다.
#include <iostream> // std::cout와 여러 출력 삽입 연산자를 선언한다.
#include <optional> // std::optional/nullopt: 열 검증 성공과 실패를 타입으로 구분한다.
#include <ranges>   // std::views::zip: 별도 Row 복사 없이 같은 위치의 열 원소를 묶는다.
#include <string>   // std::string: 품목 코드가 저장 문자를 소유하게 한다.
#include <tuple>    // std::get: zip 행의 tuple-like 참조를 구조적 바인딩할 때 선택된다.
#include <utility>  // std::move: 이름 있는 소유 객체를 이동 가능한 xvalue로 표시한다.
#include <vector>   // std::vector: 열과 보충 항목을 동적 연속 저장소에 소유한다.

// 직접 해보기: 세 재고 열을 검증한 뒤 부족 행만 독립 소유 보고서로 수집한다.
// using 별칭은 새 타입을 만들지 않는다. 각 vector의 템플릿 인자는 string 또는 int다.
using ItemNames = std::vector<std::string>;
using Quantities = std::vector<int>;

constexpr int maximum_quantity{1'000'000'000};
constexpr std::size_t maximum_rows{1'000'000U};

// 반환형 bool은 모든 수량이 허용 범위인지 나타내고 const&는 vector를 복사하지 않는다.
[[nodiscard]] bool valid_quantity_column(const Quantities& values) noexcept {
    // [첫 호출 계약: const std::vector<int> range-for의 숨은 begin/end/비교/역참조/증가]
    // (1) 수신 객체 values는 살아 있는 const vector<int> lvalue이고 순회 중 구조 변경이 없다.
    // (2) const begin()/end(), const_iterator의 operator*, 전위 operator++를 선택하고, 루프 조건의
    //     `begin != end`는 C++20 비교 재작성으로 operator== 후보를 부정해 평가한다.
    // (3) 숨은 range는 const 참조로 빌리고 역참조한 const int&를 int value로 복사한다.
    // (4) begin/end는 const_iterator 값을, operator==는 부정될 bool을, 역참조는 복사에 쓰는 const int&를,
    //     전위 ++는 const_iterator&를 반환한다. 비교 bool만 분기에 쓰고 증가식의 반환 참조는 버린다.
    // (5) iterator만 전진하고 vector 크기·용량·원소·관찰자는 모두 그대로다.
    // (6) 전체 O(N), 무할당이다. begin/end는 noexcept이고 여기의 int 비교도 던지지 않지만 일반 반복자
    //     연산 전체를 무조건 noexcept라 일반화할 수는 없다. owner 수명과 iterator 유효성이 전제이며,
    //     같은 vector의 동시 쓰기는 데이터 경쟁이다.
    for (const int value : values) {
        if (value < 0 || value > maximum_quantity) {
            return false;
        }
    }
    return true;
}

// struct는 기본 public이라 검증 전 DTO처럼 단순한 값 묶음에 적합하다.
struct StockColumns {
    ItemNames item_names;
    Quantities on_hand;
    Quantities minimum_required;
};

// class는 기본 private이다. 생성 경로를 factory 하나로 제한해 세 열의 같은 길이를 불변식으로 만든다.
class StockTable {
public:
    // 반환형 optional은 너비 오류를 예외 대신 명시적인 빈 값으로 돌려준다.
    [[nodiscard]] static std::optional<StockTable> create(StockColumns columns) {
        // [첫 호출 계약: 세 std::vector 특수화의 size]
        // (1) 수신은 살아 있는 vector<string> 하나와 vector<int> 둘이며 각 열은 독립 저장소를 소유한다.
        // (2) 각 특수화의 size_type size() const noexcept를 인자 없이 선택한다.
        // (3) 명시 인자나 소유권 이동이 없고 수신 열을 const 방식으로 관찰한다.
        // (4) 각 원소 수 size_t prvalue를 반환해 세 지역 상수에 저장한다.
        // (5) 열의 원소·크기·용량과 기존 관찰자는 바뀌지 않는다.
        // (6) 각 호출은 O(1), 무할당·무효화·무예외다. 살아 있는 수신과 동시 구조 변경 없음이 전제다.
        const std::size_t names_size{columns.item_names.size()};
        const std::size_t stock_size{columns.on_hand.size()};
        const std::size_t minimum_size{columns.minimum_required.size()};

        if (names_size != stock_size || names_size != minimum_size || names_size > maximum_rows ||
            !valid_quantity_column(columns.on_hand) ||
            !valid_quantity_column(columns.minimum_required)) {
            // [첫 호출 계약: std::optional<StockTable>의 nullopt 생성]
            // (1) 반환 optional은 아직 없고 표준 nullopt 태그는 유효하다.
            // (2) optional(nullopt_t) 생성자가 선택되어 StockTable contained object를 만들지 않는다.
            // (3) std::nullopt 태그를 값으로 전달하며 어떤 열 저장소도 빌리거나 소유하지 않는다.
            // (4) 생성자는 별도 반환값이 없고 disengaged optional prvalue가 함수 결과가 된다.
            // (5) 너비/행 수/수량 범위를 어긴 columns는 이동되지 않고 호출자는 거부 상태를 소유한다.
            // (6) O(1), 무할당·noexcept이며 무효화가 없다. 같은 optional을 동시에 수정하지 않는다.
            return std::nullopt;
        }

        // [첫 호출 계약: std::move와 optional 값 변환 생성]
        // (1) 함수에는 표준 수신 객체가 없고 columns는 길이 검증을 통과한 StockColumns lvalue이며 결과 optional은 아직 없다.
        // (2) move<StockColumns&>(columns) -> StockColumns&&를 만든다. 반환 변환은 대표적으로
        //     template<class U=T> explicit(!is_convertible_v<U,T>) optional(U&&)에서 U=StockTable인 생성자다.
        // (3) columns를 xvalue로 표시해 세 vector를 소비하도록 허용하고, table prvalue를 optional 값으로 이동한다.
        // (4) move 결과는 private 생성자에 쓰이고 함수는 engaged optional 값을 반환한다.
        // (5) 반환 결과가 열을 독점 소유하며 지역 source는 유효하지만 이동 후 값 미지정 상태가 된다.
        // (6) 기본 allocator vector/table 이동은 O(1)·무할당·noexcept이고 optional 값 생성도 그 보장을
        //     따른다. 외부 관찰자는 아직 없으며 같은 source를 다른 스레드가 동시에 접근하지 않는다.
        return StockTable{std::move(columns)};
    }

    [[nodiscard]] std::size_t row_count() const noexcept {
        // 앞에서 설명한 vector<string>::size() 계약을 재사용하며 반환값을 그대로 전달한다.
        return item_names_.size();
    }

    [[nodiscard]] auto rows() const & {
        // [첫 호출 계약: std::views::zip]
        // (1) 함수 객체에 사용자 수신자는 없고 세 멤버는 같은 길이의 살아 있는 const vector lvalue다.
        // (2) 대표 operator()<viewable_range... Rs>(Rs&&...)에서 Rs는 const ItemNames&,
        //     const Quantities&, const Quantities&다. 반환 특수화는 정확히
        //     zip_view<ref_view<const ItemNames>, ref_view<const Quantities>, ref_view<const Quantities>>다.
        // (3) 세 인자는 읽기 전용으로 빌리며 동적 저장소 소유권은 table에 남는다.
        // (4) 같은 위치의 세 원소를 tuple-like 참조로 노출하는 zip view prvalue를 반환한다.
        // (5) 원본 열은 그대로이고 view는 ref-view 성격의 비소유 연결만 저장한다.
        // (6) 고정된 세 참조 포장은 O(1)·무할당이고 이 입력에는 던지는 하위 연산이 없지만 공개 호출 전체가
        //     표준상 무조건 noexcept인 것은 아니다. owner 파괴/재할당 뒤 사용은 dangling이고 동시 수정은 금지다.
        return std::views::zip(item_names_, on_hand_, minimum_required_);
    }

    // 임시 StockTable의 멤버를 가리키는 view가 생기지 않도록 non-const/const rvalue를 모두 막는다.
    void rows() && = delete;
    void rows() const && = delete;

    StockTable(const StockTable&) = default;
    StockTable& operator=(const StockTable&) = default;
    StockTable(StockTable&&) noexcept = default;
    StockTable& operator=(StockTable&&) noexcept = default;

private:
    // explicit은 StockColumns 한 개가 table로 뜻하지 않게 암시 변환되는 것을 막는다.
    explicit StockTable(StockColumns&& columns)
        // [첫 호출 계약: std::move와 세 std::vector 이동 생성]
        // (1) 목적 멤버는 아직 생성 전이고 source vector 셋은 유효한 lvalue다.
        // (2) move<T&>(source)가 T&&를 만들고 각 vector(vector&&) 생성자를 선택한다.
        // (3) source는 lvalue, move 결과는 xvalue이며 저장소 소유권 이전을 허용한다.
        // (4) move 반환 참조는 멤버 초기화에 쓰고 각 생성자 자체에는 반환값이 없다.
        // (5) 목적 멤버가 열 값을 소유하고 source는 파괴·대입 가능한 값 미지정 상태가 된다.
        // (6) 기본 allocator에서는 O(1)·무할당·noexcept다. 공개된 iterator가 없어 무효화 피해가 없고
        //     생성 중 같은 source에 동시 접근하지 않는다.
        : item_names_{std::move(columns.item_names)},
          on_hand_{std::move(columns.on_hand)},
          minimum_required_{std::move(columns.minimum_required)} {}

    ItemNames item_names_;
    Quantities on_hand_;
    Quantities minimum_required_;
};

// 각 결과 string은 원본 열을 빌리지 않고 품목 코드를 깊게 복사해 소유한다.
struct RestockItem {
    std::string item_name;
    long long quantity{};
};

struct RestockReport {
    // [첫 생성 계약: RestockReport의 std::vector<RestockItem> 멤버 기본 생성]
    // (1) 수신 객체 items와 저장소는 아직 없고 enclosing report 객체가 생성되는 중이다.
    // (2) vector<RestockItem>::vector() 기본 생성자를 선택하며 allocator는 기본 템플릿 인자다.
    // (3) 생성자에는 명시 인자나 이전 저장소 소유권이 없다.
    // (4) 생성자는 반환값 없이 size 0인 멤버 객체의 수명을 시작한다.
    // (5) 빈 vector가 report에 소유되고 RestockItem 원소 수명은 아직 시작하지 않는다.
    // (6) 기본 allocator에서 O(1)·무할당·무효화·무예외다. report보다 먼저 파괴되며 동시 접근은 없다.
    std::vector<RestockItem> items{};
    long long total_quantity{};
};

// const& 매개변수는 table을 소유하지 않고 호출 동안만 읽는다. 반환 report는 완전한 소유 값이다.
[[nodiscard]] RestockReport build_restock_report(const StockTable& table) {
    // [첫 호출 계약: std::vector<RestockItem> 기본 생성]
    // (1) 생성될 수신 객체 items와 원소 저장소는 아직 없다.
    // (2) vector() 기본 생성자를 선택하며 원소 템플릿 인자는 RestockItem이다.
    // (3) 명시 인자나 이전 소유 저장소가 없다.
    // (4) 생성자는 별도 반환값 없이 size 0인 vector를 완성한다.
    // (5) 빈 소유 컨테이너가 생기며 RestockItem 원소 수명은 아직 시작하지 않는다.
    // (6) 기본 allocator에서 O(1)·무할당·무예외다. 무효화할 관찰자는 없고 지역 수명이다.
    std::vector<RestockItem> items{};

    // [첫 호출 계약: std::vector<RestockItem>::reserve]
    // (1) 수신 객체 items는 비어 있는 유효한 vector lvalue이고 row_count는 가능한 최대 결과 수다.
    // (2) void reserve(size_type new_capacity)를 선택한다.
    // (3) size_t prvalue를 값으로 받고 table의 저장소나 소유권은 건드리지 않는다.
    // (4) void라 반환값은 없다.
    // (5) 성공하면 capacity가 요청 이상이고 size는 여전히 0이다.
    // (6) O(N) 상한, length_error/bad_alloc 가능성이 있다. 재할당 시 기존 관찰자는 무효화되지만 현재는
    //     없고 실패하면 vector는 유지된다. 같은 items에 대한 동시 접근은 없다.
    items.reserve(table.row_count());

    // 최대 10^6행 * 행당 10^9라 합은 10^15 이하이고 long long 범위 안이다.
    long long total_quantity{};

    // [첫 호출 계약: zip view range-for의 숨은 begin/end/비교/역참조/증가]
    // (1) table은 살아 있는 const lvalue이고 임시 zip view는 range-for 끝까지 수명이 연장된다.
    // (2) 숨은 `auto&& range`가 비-const 이름이어도 모든 기반 ref_view가 simple-view라 `begin() const`와
    //     `end() const`, iterator<true>::operator*() const와 전위 operator++()가 선택된다. 루프의
    //     `begin != end`는 C++20 비교 재작성으로 iterator<true>의 operator== 후보를 부정한다.
    // (3) 숨은 range 변수는 view를 &&로 붙잡는다. I=0/1/2는 컴파일 시간 size_t 값이고, ADL로 고른
    //     std::get<I>에는 역참조가 만든 tuple<const string&,const int&,const int&> xvalue가 전달된다.
    // (4) begin/end는 같은 iterator<true> 값을 반환한다. get<I>(tuple&&)의 T&& 반환은 참조 원소 타입과
    //     축약되어 각 const 원소 lvalue 참조가 되고 구조적 바인딩 이름에 연결된다.
    // (5) 전위 ++는 iterator&를 반환해 세 기반 iterator만 전진한다. proxy tuple은 반복 끝에 파괴되지만
    //     table 원소와 참조는 변하지 않으며 구조적 바인딩 이름은 현재 반복 안에서만 사용한다.
    // (6) 고정 세 열의 각 연산과 get은 O(1)·무할당이고 get은 noexcept, 전체 순회는 O(N)이다. 그 밖의
    //     공개 연산을 무조건 noexcept라 단정하지 않는다. 범위 밖 I는 컴파일 실패이며 owner 파괴·재할당·
    //     동시 쓰기 뒤 참조 사용은 dangling/미정의 동작 또는 데이터 경쟁을 만든다.
    for (auto&& [item_name, on_hand, minimum] : table.rows()) {
        if (on_hand >= minimum) {
            continue;
        }

        // 검증된 int를 먼저 long long으로 승격해 차를 계산하므로 부호 있는 int 오버플로가 없다.
        const long long needed{
            static_cast<long long>(minimum) - static_cast<long long>(on_hand)};
        // [첫 호출 계약: std::string 복사 생성과 vector::emplace_back]
        // (1) item_name은 원본 열의 const string lvalue이고 items는 충분한 용량을 예약한 vector lvalue다.
        // (2) basic_string(const basic_string&)로 aggregate 이름을 만들고 template<class... Args>
        //     reference emplace_back(Args&&...)에서 Args={RestockItem}인 overload를 선택한다.
        // (3) item_name 문자는 깊게 복사한다. RestockItem prvalue는 rvalue로 전달되어 원소 위치에 이동 생성된다.
        // (4) string 생성자는 반환값이 없고 emplace_back은 새 원소 참조를 반환하지만 여기서는 버린다.
        // (5) 성공하면 독립 이름/수량 원소가 생겨 size가 1 늘고 전달 prvalue는 이동 후 파괴된다.
        // (6) 이름 복사는 문자 수에 선형, emplace_back은 분할 상환 O(1)이다. 예약 범위라 재할당은 없어
        //     기존 원소 참조는 유지되지만 past-the-end iterator는 무효다. string 할당 실패 시 새 원소는
        //     남지 않고 기존 원소는 유효하다. 같은 items 객체의 동시 읽기/쓰기는 외부 동기화가 필요하다.
        items.emplace_back(RestockItem{item_name, needed});
        total_quantity += needed;
    }

    // [첫 호출 계약: std::move(items)와 std::vector<RestockItem> 이동 생성]
    // (1) 수신 report의 items 멤버는 아직 없고 지역 items는 결과 원소를 소유한 vector lvalue다.
    // (2) move<vector<RestockItem>&>와 allocator 인자 없는 vector(vector&&) noexcept 생성자를 선택한다.
    // (3) items는 xvalue 참조로 변환되어 동적 저장소 소유권을 새 멤버에 넘기며 합계는 값으로 복사된다.
    // (4) move는 rvalue 참조를 반환해 멤버 초기화에 쓰고 생성자는 별도 반환값이 없다. report prvalue는 반환된다.
    // (5) 결과 report가 저장소를 소유하고 지역 items는 유효한 값 미지정 상태가 된 뒤 파괴된다.
    // (6) 기본 allocator에서 이동은 O(1)·무할당·noexcept이고 같은 타입 prvalue는 결과 목적지에 직접
    //     구성된다. 기존 관찰자는 없으며 함수는 같은 vector에 대한 동시 접근을 만들지 않는다.
    return RestockReport{std::move(items), total_quantity};
}

int main() {
    // [첫 호출 계약: std::string C 문자열 생성과 std::vector initializer_list 생성]
    // (1) valid 입력의 vector/원소는 아직 없고 세 리터럴은 정적 수명의 NUL 종료 char 배열이다.
    // (2) basic_string(const char*) 세 번과 vector(initializer_list<T>) 세 번을 선택한다.
    // (3) 리터럴 문자를 string이 복사하고 const initializer_list 원소를 각 vector가 복사해 소유한다.
    // (4) 생성자는 별도 반환값 없이 세 임시 vector와 StockColumns 값을 완성한다.
    // (5) 각 열이 3개 값을 독점 소유하며 리터럴과 정수 원본은 바뀌지 않는다.
    // (6) 원소/문자 수에 선형이고 length_error/bad_alloc 가능성이 있다. 실패 시 완성된 임시는 정리되며
    //     아직 외부 iterator가 없어 무효화 대상이 없다. 생성 중 동시 접근은 없다.
    const std::optional<StockTable> valid_result{StockTable::create(StockColumns{
        ItemNames{std::string{"bolt"}, std::string{"filter"}, std::string{"paint"}},
        Quantities{3, 7, 10},
        Quantities{8, 13, 9}})};

    // 같은 생성 계약을 재사용한다. 이 입력은 열 길이를 1, 2, 1로 일부러 다르게 만든다.
    const std::optional<StockTable> invalid_width_result{StockTable::create(StockColumns{
        ItemNames{std::string{"orphan"}}, Quantities{1, 2}, Quantities{4}})};

    // 열 길이는 같지만 음수 재고는 도메인 계약을 어기므로 별도의 빈 optional이 된다.
    const std::optional<StockTable> invalid_domain_result{StockTable::create(StockColumns{
        ItemNames{std::string{"unsafe"}}, Quantities{-1}, Quantities{4}})};

    // [첫 호출 계약: std::optional<StockTable>::has_value]
    // (1) valid_result는 factory가 돌려준 살아 있는 const optional lvalue다.
    // (2) constexpr bool has_value() const noexcept를 인자 없이 선택한다.
    // (3) 인자·소유권 이전 없이 contained 상태만 관찰한다.
    // (4) engaged 여부 bool prvalue를 반환해 !와 if 조건에서 사용한다.
    // (5) optional과 contained table의 값·수명은 그대로다.
    // (6) O(1), 무할당·무효화·무예외다. 같은 optional을 다른 스레드가 동시에 수정하지 않는다.
    if (!valid_result.has_value()) {
        return 1;
    }

    // [첫 호출 계약: std::optional<StockTable>::value const&]
    // (1) 검사 결과 valid_result는 engaged인 const optional lvalue다.
    // (2) const T& value() const&에서 T=StockTable인 overload를 선택한다.
    // (3) 인자나 소유권 이동 없이 contained table을 const 참조로 빌린다.
    // (4) const StockTable&를 반환하고 build 함수의 인자로 즉시 사용한다.
    // (5) optional/table은 변하지 않고 report는 그 안의 문자열을 별도로 소유한다.
    // (6) O(1)·무할당이다. empty이면 bad_optional_access지만 앞 검사로 배제했다. optional 파괴 뒤 반환
    //     참조 사용은 dangling이며 같은 객체의 동시 변경은 외부 동기화가 필요하다.
    const RestockReport report{build_restock_report(valid_result.value())};

    // [첫 호출 계약: 결과 vector::size, optional::has_value, ostream C문자열/size_t/long long/char 삽입]
    // (1) report와 두 invalid optional은 살아 있는 const 객체이고 std::cout은 유효한 ostream lvalue다.
    // (2) size() const noexcept, 앞과 같은 has_value(), operator<<(ostream&, const char*), char 삽입과
    //     basic_ostream<char>::operator<<(unsigned long long) 및 operator<<(long long)를 선택한다.
    //     이 w64devkit target에서 size_t의 기반 타입이 unsigned long long이기 때문이다.
    // (3) size/has_value에는 인자가 없다. 리터럴/삼항 결과는 NUL 종료 const char*로 빌리고 수치는 값으로 쓴다.
    // (4) size와 has_value 반환값은 각각 출력/삼항 분기에 쓰며 각 삽입의 ostream&는 체인 후 버린다.
    // (5) report·optional은 변하지 않고 출력 버퍼에 한 줄이 추가된다.
    // (6) 관찰은 O(1)·무할당·무예외이고 출력 비용은 문자 수/장치에 따른다. 스트림 실패는 상태 비트나
    //     설정 예외로 나타난다. 참조 무효화는 없고 같은 stream의 복합 동시 출력은 외부 동기화가 필요하다.
    std::cout << "valid=" << report.items.size()
              << ",total=" << report.total_quantity
              << ",width=" << (invalid_width_result.has_value() ? "accepted" : "rejected")
              << ",domain=" << (invalid_domain_result.has_value() ? "accepted" : "rejected")
              << '\n';

    /*
    초보자·Modern C++ 확인:
    int와 bool은 기본 타입이고 `{}`는 0/false 값 초기화다. 함수 이름 앞 RestockReport는 반환형,
    괄호 안 const StockTable&는 매개변수다. const는 이 경로의 쓰기를 막고 &는 소유하지 않는 참조다.
    raw pointer는 쓰지 않았지만 zip view와 참조도 대상 수명을 늘리지 않는 비소유 관찰자라는 점은 같다.
    for는 반복, if는 조건 분기, continue는 현재 반복의 남은 문장만 건너뛴다. public/private 접근 지정자는
    factory만 불변식을 만들게 하고, 멤버 초기화 목록은 생성자 본문 전에 세 vector를 직접 구성한다.

    valid_result, 두 invalid 결과, report는 lvalue다. StockColumns{...}와 함수 반환 결과는 prvalue,
    std::move(columns)는 xvalue다. table은 열 저장소를, report는 결과 저장소와 string을 각각 소유한다.
    return RestockReport{...}는 같은 타입 prvalue를 결과 목적지에 직접 구성할 수 있고, 이름 있는 지역을
    반환하는 구현으로 바꾸면 선택적 NRVO 대상이 될 수 있다. 이동 뒤 source는 유효하지만 값은 미지정이다.

    기계 관점에서는 세 size load와 비교/조건 분기, zip iterator의 주소 전진, 재고 비교와 정수 load/store,
    string 복사·vector 원소 구성, 스트림 출력이 생길 수 있다. 사용자 정의 도메인 타입에는 virtual 함수가
    없어 그 객체에 대한 가상 간접 호출은 필요하지 않지만, iostream 내부의 streambuf 경로에는 구현에 따라
    가상 호출이 나타날 수 있다. 실제 명령·인라이닝·복사 생략·분기 제거·메모리 배치는 CPU·ABI·표준
    라이브러리·컴파일러·최적화 옵션에 따라 달라 특정 어셈블리로 단정하지 않는다.
    */

    return 0;
}
