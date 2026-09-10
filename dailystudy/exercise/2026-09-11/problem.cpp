// <iostream>은 결과를 표준 출력에 쓰는 std::cout과 삽입 연산자를 선언한다.
#include <iostream>
// <ranges>는 filter/transform view와 C++23 ranges::to materialization을 선언한다.
#include <ranges>
// <string>은 저장소 행과 서비스 응답이 이름 문자를 소유하게 한다.
#include <string>
// <utility>는 소유 컨테이너 이동 의도를 표현하는 std::move를 선언한다.
#include <utility>
// <vector>는 저장소와 반환 snapshot의 연속 소유 컨테이너를 선언한다.
#include <vector>

// 연습 목표: repository가 비소유 lazy range를 제공하고 service가 외부 경계에서 소유 vector로 고정한다.
// struct의 멤버는 기본 public이라 단순 행/DTO에 적합하다. 아래 class는 기본 private라 저장 상태를 숨긴다.
struct StockRow {
    int sku;
    std::string title;
    int stock;
    bool published;
};

struct CatalogCard {
    int sku;
    std::string title;
};

// using 별칭은 원래 타입과 완전히 같다. vector의 템플릿 인자가 저장할 원소 타입과 소멸 규칙을 정한다.
using StockRows = std::vector<StockRow>;
using CatalogCards = std::vector<CatalogCard>;

class InventoryRepository {
public:
    // explicit은 StockRows가 repository로 뜻하지 않게 암시 변환되는 것을 막는다. 값 매개변수 rows는
    // 호출자 데이터의 복사본 또는 이동된 값을 함수 경계에서 받고, 멤버 초기화 목록이 rows_를 먼저 만든다.
    // [호출 계약: std::move(rows)와 vector 이동 생성]
    // (1) 입력 rows는 유효한 StockRows lvalue로 행과 문자열을 독점 소유하며 수신 목적 rows_는 아직 생성 전이다.
    // (2) std::move<T>(T&&)에서 T=StockRows&를 선택해 StockRows&&를 만들고 vector(vector&&)가 선택된다.
    // (3) 이름 있는 rows의 lvalue 식을 xvalue로 변환해 버퍼 이전을 허용한다. std::move 자체는 읽거나 옮기지 않는다.
    // (4) std::move의 rvalue reference는 rows_ 생성에 즉시 쓰이고 생성자 반환값은 없다.
    // (5) rows_가 원소 owner가 되고 rows는 유효하지만 내용이 명시되지 않은 moved-from 상태다.
    // (6) 기본 allocator에서는 O(1), 무할당·noexcept 이동이다. 원소 관찰자는 새 owner를 가리킨다.
    //     moved-from vector도 전제조건을 만족하는 연산은 가능하지만 이전 크기·원소를 가정할 수 없다.
    //     공유 쓰기에 대한 동기화는 제공하지 않는다.
    explicit InventoryRepository(StockRows rows) : rows_{std::move(rows)} {}

    [[nodiscard]] auto sellable_rows() const {
        const auto predicate = [](const StockRow& row) {
            return row.published && row.stock > 0;
        };

        // [호출 계약: std::views::filter(rows_, predicate)]
        // (1) rows_는 살아 있는 repository가 소유한 const StockRows lvalue이고 predicate는 무상태 lambda다.
        // (2) views::filter(R&&, Pred&&)에서 R=const StockRows&, Pred=const lambda&를 선택해
        //     ref_view<const StockRows> 기반 filter_view 계열 결과를 만든다.
        // (3) 행 범위는 비소유 참조로 빌리고 predicate 값만 view가 보관한다. 원소 소유권은 이동하지 않는다.
        // (4) lazy view prvalue를 반환 객체에 직접 구성하며 아직 predicate를 실행하지 않는다.
        // (5) rows_는 그대로다. 오늘처럼 새 view의 첫 순회는 그 시점 값을 보지만 forward_range용
        //     filter_view는 첫 통과 반복자를 cache할 수 있어, 시작한 같은 view는 상태 변경 뒤 재사용하지 않는다.
        // (6) 구성 O(1), 무할당·즉시 비무효화다. 순회 O(n)이다. repository 파괴는 항상 dangling이고,
        //     vector 재할당은 이미 얻었거나 cache한 반복자를 무효화한다. 순회와 같은 행의 동시 변경은
        //     data race가 될 수 있으므로 변경 뒤에는 새 query view를 만든다.
        // 대표 문서: ../standard-library/algorithms-and-ranges.md
        return std::views::filter(rows_, predicate);
    }

    void mark_sold_out(const int sku) {
        // [호출 계약: range-for가 숨겨 호출하는 vector::begin/end와 iterator 연산]
        // (1) 수신 rows_는 완성된 StockRows lvalue이고 세 행을 소유한다. 반복 전에 구조 변경은 없고
        //     row는 현재 StockRow를 소유하지 않고 빌리는 StockRow&로 바인딩된다.
        // (2) iterator begin() noexcept/end() noexcept, 같은 vector iterator 비교, operator*, 전위
        //     operator++가 range-for 전개에서 선택된다.
        // (3) 명시 인자는 없고 rows_를 숨은 this로 빌린다. 역참조 결과는 stock 대입에만 쓰며 iterator나
        //     원소 소유권을 loop 밖으로 옮기지 않는다.
        // (4) begin/end는 iterator 값, 비교는 bool 문맥에서 검사할 결과, 역참조는 StockRow&, 전위 증가는
        //     iterator&를 반환해 각각 loop 경계·조건·row reference binding·다음 위치 계산에 사용된다.
        // (5) size/capacity와 모든 iterator는 유지되고, 찾은 한 행의 stock만 0으로 바뀔 수 있다.
        // (6) 각 숨은 연산 O(1), 전체 O(n), 무할당·비무효화다. 과거-끝 역참조나 무효 iterator 사용은
        //     미정의 동작이며, 순회 중 구조 변경과 다른 스레드의 무동기 읽기/쓰기는 금지한다.
        // 대표 문서: ../standard-library/containers-and-views.md
        // StockRow&는 현재 원소의 null 불가능 별칭이다. 포인터처럼 소유하거나 delete하지 않고, 반복이
        // 다음 원소로 넘어가면 이 지역 참조 이름의 사용도 끝난다.
        for (StockRow& row : rows_) {
            if (row.sku == sku) {
                row.stock = 0;
                return;
            }
        }
    }

private:
    StockRows rows_;
};

class CatalogService {
public:
    // reference member는 repository를 소유하지 않는다. 따라서 호출자는 repository 수명이 service보다
    // 길다는 전제조건을 지킨다. const 참조라 service는 공개 query 외 임의 변경을 할 수 없다.
    explicit CatalogService(const InventoryRepository& repository) : repository_{repository} {}

    [[nodiscard]] CatalogCards load_snapshot() const {
        auto sellable = repository_.sellable_rows();

        // [생성 계약: CatalogCard의 std::string copy]
        // (1) 변환 시 row와 row.title은 filter가 가리키는 살아 있는 const lvalue이고 새 card는 아직 없다.
        // (2) CatalogCard aggregate의 title은 basic_string(const basic_string&) 복사 생성자를 선택한다.
        // (3) sku 값과 title 문자를 읽어 새 객체로 복사하고 원본 문자열 소유권은 건드리지 않는다.
        // (4) lambda는 독립 문자열을 가진 CatalogCard prvalue를 반환한다.
        // (5) 원본 행은 그대로이고 반환 card는 원본보다 오래 살아도 된다.
        // (6) title 길이에 선형, 문자 할당 가능, bad_alloc/length_error 가능이다. 실패한 부분 객체는
        //     정리되고 원본은 유지된다. 성공 결과는 rows_ 반복자 무효화와 독립이다.
        const auto make_card = [](const StockRow& row) {
            return CatalogCard{row.sku, row.title};
        };

        // [호출 계약: std::views::transform(sellable, make_card)]
        // (1) sellable은 repository_를 간접 참조하는 유효한 filter_view lvalue, make_card는 무상태 lambda다.
        // (2) views::transform(R&&, F&&)에서 R=decltype(sellable)&, F=const lambda&가 선택된다.
        // (3) 기반 view와 함수 객체를 결과 view 표현에 복사하고 행은 나중에 const 참조로 빌린다.
        // (4) CatalogCard를 아직 만들지 않는 transform_view prvalue를 projected에 직접 초기화한다.
        // (5) 입력과 repository는 변하지 않고 순회 때만 각 통과 행을 card prvalue로 투영한다.
        // (6) 구성 O(1), 무할당이다. 전체 순회 O(n+문자 복사)이고 기반 수명/무효화/동시성 제약을
        //     전달하며 predicate나 복사 생성 예외도 호출자에게 전파한다.
        // 대표 문서: ../standard-library/algorithms-and-ranges.md
        auto projected = std::views::transform(sellable, make_card);

        // [호출 계약: std::ranges::to<CatalogCards>(projected)]
        // (1) projected는 유효한 lazy transform_view lvalue, 목적 C는 정확히 vector<CatalogCard>다.
        // (2) C++23 ranges::to<C>(R&&, Args&&...)에서 C=CatalogCards, R=decltype(projected)&,
        //     빈 추가 인자 pack이며 vector의 from-range 생성 경로가 선택된다.
        // (3) forward_range인 projected를 빌려 거리 계산 뒤 구성 순회를 할 수 있고, 각 card prvalue와
        //     그 문자열을 결과 vector가 소유한다.
        // (4) 모든 통과 행의 독립 snapshot인 CatalogCards prvalue를 반환 객체에 직접 구성한다.
        // (5) repository는 그대로이고 지역 view가 파괴된 뒤에도 결과 vector/string 수명은 계속된다.
        // (6) 시간 O(n+k+총 문자 길이), 공간 O(k+문자)다. vector 원소 저장소 재할당은 없지만 술어는
        //     거리·구성 순회에서 반복 평가될 수 있고 projector는 구성 순회에서 평가된다. 예외 시 부분
        //     결과는 정리된다.
        // 대표 문서: ../standard-library/algorithms-and-ranges.md
        return std::ranges::to<CatalogCards>(projected);
    }

private:
    const InventoryRepository& repository_;
};

int main() {
    // [생성 계약: string C-string 생성과 vector initializer_list 생성]
    // (1) literal 세 개는 null 종료된 정적 char 배열이고 seed/각 string 목적 객체는 아직 없다.
    // (2) basic_string(const char*, allocator)와 vector(initializer_list<StockRow>, allocator)가 선택된다.
    // (3) non-null literal 문자를 각 string이 복사하고 const initializer_list 행들을 vector가 복사한다.
    // (4) 생성자는 반환값이 없으며 seed가 세 행과 title 버퍼를 독점 소유한다.
    // (5) seed는 입력 순서의 size 3 상태이고 literal/임시 list와 수명을 공유하지 않는다.
    // (6) 원소·문자 수에 선형이며 vector/string 할당·길이 예외가 가능하다. 실패 시 이미 만든 원소는
    //     역순 파괴되고 새 객체라 기존 반복자 무효화나 공유 상태 변경은 없다.
    StockRows seed{
        StockRow{101, std::string{"Keyboard"}, 3, true},
        StockRow{102, std::string{"Draft monitor"}, 8, false},
        StockRow{103, std::string{"Mouse"}, 5, true},
    };

    // seed는 lvalue지만 std::move가 xvalue로 바꾼다. 위 이동 계약대로 repository가 최종 owner가 된다.
    InventoryRepository repository{std::move(seed)};
    CatalogService service{repository};

    // 반환 prvalue가 first/second를 직접 초기화한다. 보장 복사 생략으로 중간 vector 이동은 필수가 아니며,
    // 각 snapshot은 자신의 vector와 string 수명을 가진다.
    CatalogCards first{service.load_snapshot()};
    repository.mark_sold_out(101);
    CatalogCards second{service.load_snapshot()};

    // [호출 계약: vector::size()]
    // (1) first/second는 각각 2개/1개 CatalogCard를 소유하는 유효한 vector lvalue다.
    // (2) size_type size() const noexcept overload가 선택되고 명시 인자는 없다.
    // (3) 숨은 this를 읽기만 하며 소유권 입력은 없다.
    // (4) 원소 수 size_type prvalue를 반환해 지역 const 값에 저장한다.
    // (5) 원소·size·capacity와 관찰자는 모두 그대로다.
    // (6) O(1), 무할당·비무효화·noexcept이며 같은 vector의 동시 쓰기는 없어야 한다.
    const auto first_size{first.size()};
    const auto second_size{second.size()};

    // [호출 계약: vector::operator[]]
    // (1) 수신 first/second vector에는 위 size 불변식상 각각 0/1과 0 인덱스가 유효하다.
    // (2) reference operator[](size_type) 비-const overload가 선택된다.
    // (3) 정수 인덱스를 size_type 값으로 변환하고 원소를 빌리기만 한다.
    // (4) CatalogCard&를 반환해 sku int 값을 즉시 읽고 참조는 저장하지 않는다.
    // (5) vector와 원소는 변하지 않는다.
    // (6) O(1), 무할당·비무효화다. 범위 밖이면 검사 없이 미정의 동작이지만 size가 전제조건을 보장한다.
    const int first_sku{first[0].sku};
    const int first_other_sku{first[1].sku};
    const int second_sku{second[0].sku};

    // [호출 계약: ostream 산술/char 삽입 operator<<]
    // (1) 수신 std::cout은 출력 가능한 ostream lvalue이고 모든 숫자 값은 초기화됐다.
    // (2) size_type/int 산술 overload와 operator<<(ostream&, char)가 순서대로 선택된다.
    // (3) 값들을 복사해 형식화하며 vector나 숫자 소유권을 옮기지 않는다.
    // (4) 각 호출은 같은 ostream&를 반환해 연쇄하고 마지막 반환은 버린다.
    // (5) `2 101 103 1 103\n`이 버퍼에 추가되고 snapshot은 변하지 않는다.
    // (6) 출력 문자 수에 선형이며 상태 비트/설정 예외가 가능하다. 컨테이너 관찰자를 무효화하지 않고
    //     동일 stream에 동시 무동기 쓰기를 하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cout << first_size << ' ' << first_sku << ' ' << first_other_sku << ' '
              << second_size << ' ' << second_sku << '\n';

    // 기계 실행 관점에서는 거리 계산 순회와 결과 저장소 확보 뒤 published/stock load·비교·조건 분기,
    // 선택 원소의 string copy/vector store가 일어날 수 있다. vector 원소 저장소 재할당은 없지만 string은
    // 별도 할당할 수 있다. 실제 분기 예측, inlining, SIMD, 명령 수는 CPU·라이브러리·컴파일러·최적화
    // 설정에 따라 달라지므로 특정 어셈블리 형태로 단정하지 않는다.
    return 0;
}
