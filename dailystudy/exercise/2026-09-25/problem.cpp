// <generator>는 재고 항목을 필요할 때 한 건씩 내보내는 std::generator를 선언한다.
#include <generator>
// <iostream>은 std::cout과 출력 스트림 삽입 연산을 선언한다.
#include <iostream>
// <string>은 항목 이름을 소유하는 std::string을 선언한다.
#include <string>
// <utility>는 소유권 이전 후보를 표시하는 std::move를 선언한다.
#include <utility>
// <vector>는 동적 연속 저장소와 reserve/push_back/size를 선언한다.
#include <vector>

// struct는 기본 public인 단순 데이터 묶음이다. 두 int 멤버의 {}는 0 값 초기화다.
struct StockItem {
    std::string sku;
    int on_hand{};
    int minimum{};
};

// class는 저장소를 private에 감춰 소유권 경계를 명시한다.
class InventorySnapshot final {
public:
    // explicit은 vector에서 snapshot으로의 의도하지 않은 암시 변환을 막는다.
    // [첫 호출 계약: std::move(items)와 vector 이동 생성]
    // (1) 수신 items_는 생성 중인 std::vector<StockItem>, items는 유효한 값 매개변수 owner다.
    // (2) std::move<T>(T&&)에서 T=vector<StockItem>&이고 vector(vector&&) noexcept가 선택된다.
    // (3) 이름 있는 items lvalue를 xvalue로 바꾸어 저장소 이전을 허용하며 별도 허용값 제한은 없다.
    // (4) std::move 반환 vector&&는 멤버 초기화에 소비되고 생성자 자체 반환값은 없다.
    // (5) 성공하면 items_가 원소를 소유하고 items는 유효하지만 내용 미지정인 이동 후 상태다.
    // (6) 표준 allocator 이동은 상수 시간·noexcept이며 기존 원소 관찰자는 새 owner에 이어진다.
    //     snapshot 수명이 저장소 수명이고 동기화는 제공하지 않는다.
    explicit InventorySnapshot(std::vector<StockItem> items) noexcept
        : items_{std::move(items)} {}

    InventorySnapshot(const InventorySnapshot&) = delete;
    InventorySnapshot& operator=(const InventorySnapshot&) = delete;
    InventorySnapshot(InventorySnapshot&&) noexcept = default;
    InventorySnapshot& operator=(InventorySnapshot&&) noexcept = default;

    [[nodiscard]] const std::vector<StockItem>& items() const & noexcept {
        return items_;
    }

    [[nodiscard]] const std::vector<StockItem>& items() const && = delete;

private:
    // [첫 생성 계약: private vector 기본 멤버 초기화식]
    // (1) 수신 items_는 이 초기화식을 사용하는 생성자에서 아직 수명이 시작되지 않은 vector 멤버다.
    // (2) std::vector<StockItem>::vector()와 그 specialization의 기본 allocator_type 구성이 선택된다.
    // (3) 데이터 인자는 없고 allocator는 기본값이며 외부 소유권을 받지 않는다.
    // (4) 반환값 없이 빈 vector 멤버를 생성한다.
    // (5) 적용되면 size=0이고 원소 수명은 시작하지 않는다. 오늘 생성자는 items_{std::move(items)}를 직접
    //     지정하므로 이 기본 멤버 초기화식은 그 생성 경로에서는 평가되지 않는다.
    // (6) 표준 allocator 조합은 상수 시간·noexcept다. 빈 상태에 원소 저장소가 필요하지 않지만 구현 내부
    //     할당을 표준 계약 이상으로 단정하지 않는다. 아직 무효화·공유 원소·동기화 대상은 없다.
    std::vector<StockItem> items_{};
};

// 긴 "읽기 전용 StockItem 참조를 지연 생산하는 generator" 타입에 별칭을 붙인다.
using ReorderStream = std::generator<const StockItem&>;

// snapshot 값 매개변수는 코루틴 프레임이 소유하므로 호출자의 지역 vector 수명에 기대지 않는다.
// [첫 생성/호출 계약: std::generator coroutine protocol 전체]
// (1) 사용자 수신 객체는 없고 컴파일러가 promise와 InventorySnapshot 매개변수를 담는 frame을 만든다.
// (2) 필요한 frame operator new(size_t), get_return_object() noexcept, initial_suspend() const noexcept,
//     정상 종료의 return_void() const noexcept/final_suspend() noexcept, 예외의 unhandled_exception(),
//     generator 소멸과 frame delete가 선택되는 숨은 protocol이다.
// (3) allocation 크기는 구현이 정하며 snapshot은 이동/복사되어 frame이 소유한다. raw frame 소유권은
//     호출자에게 노출되지 않는다.
// (4) get_return_object 반환 generator는 호출부가 저장하고 awaiter 반환은 상태 전이에 사용한다.
//     void protocol과 소멸/해제 결과는 직접 사용하지 않는다.
// (5) 호출 직후 initial suspend, 정상 끝에는 final suspend다. 미처리 예외는 promise가 처리해 재개 지점에
//     전달하고 generator 소멸 시 중단 frame과 snapshot이 함께 파괴된다.
// (6) frame 할당 실패가 가능하고 전체 본문은 O(N)이다. frame과 산출 참조 수명은 generator에 묶이며,
//     같은 frame의 동시 resume·이동·파괴를 동기화하지 않는다.
[[nodiscard]] ReorderStream items_to_reorder(InventorySnapshot snapshot) {
    // [첫 호출 계약: const vector range-for의 숨은 반복 연산]
    // (1) 수신 범위는 frame의 snapshot이 소유한 const std::vector<StockItem>&다.
    // (2) const_iterator begin() const noexcept, end() const noexcept, const_reference operator*() const,
    //     const_iterator& operator++(), bool 비교(operator!= 또는 ==에서 합성)가 선택된다.
    // (3) begin/end/*/++에는 명시 데이터 인자가 없다. 숨은 begin!=end 비교는 현재/끝 const_iterator lvalue
    //     두 개를 비소유 피연산자로 쓰고, item은 원소를 빌리는 const StockItem& lvalue 참조다.
    // (4) 반복자/참조/비교 bool 반환은 컴파일러가 만든 루프가 소비한다.
    // (5) 읽기만 하므로 원본 크기·용량·원소는 유지된다.
    // (6) begin/end와 각 iterator 연산은 O(1)·무할당이고 begin/end는 noexcept다. frame/vector가 살아 있고
    //     구조 변경이 없어야 한다. past-the-end는 역참조·증가할 수 없고 무효 iterator 사용은 미정의 동작이다.
    //     전체 O(N)이며 동시 쓰기는 데이터 경쟁이다.
    for (const StockItem& item : snapshot.items()) {
        if (item.on_hand < item.minimum) {
            // [첫 호출 계약: generator promise yield_value]
            // (1) 수신 promise와 item의 owner snapshot은 실행 중인 같은 코루틴 frame 안에 있다.
            // (2) suspend_always yield_value(const StockItem&) noexcept가 선택된다.
            // (3) item은 const lvalue이며 소유권을 넘기지 않는 참조 인자다.
            // (4) suspend_always 반환은 co_yield 중단에 사용되고 직접 저장하지 않는다.
            // (5) 현재 원소 주소가 노출되고 frame은 다음 iterator 증가까지 중단된다.
            // (6) 상수 시간·무할당·noexcept다. frame 파괴 뒤 참조 사용과 동시 재개는 안전하지 않다.
            co_yield item;
        }
    }
}

int main() {
    // [첫 생성 계약: vector<StockItem> 기본 생성]
    // (1) items는 아직 생성 전이다. (2) vector()와 기본 allocator가 선택된다.
    // (3) 인자는 없다. (4) 반환값 없이 빈 owner를 만든다. (5) size=0이 된다.
    // (6) 이 기본 allocator specialization은 상수 시간·noexcept다. 빈 상태는 원소 저장소를 요구하지 않지만
    //     구현 내부 할당 여부를 표준 계약 이상으로 단정하지 않으며 공유 원소도 없다.
    std::vector<StockItem> items{};

    // [첫 호출 계약: vector::reserve]
    // (1) 수신 items는 빈 vector다. (2) void reserve(size_type)를 선택한다.
    // (3) int prvalue 4가 size_type으로 변환되며 소유권 의미는 없다. (4) 반환값은 void다.
    // (5) 성공하면 size=0, capacity>=4다. (6) 재할당 시 선형, bad_alloc/length_error 가능,
    //     기존 관찰자 전부 무효화이며 자체 스레드 동기화는 없다.
    items.reserve(4);

    // [첫 호출 계약: string 생성과 vector::push_back(StockItem&&)]
    // (1) 수신 items는 size=0, capacity>=4다. (2) string(const char*)와 push_back(T&&)를 선택한다.
    // (3) "ssd" 배열 lvalue를 깊게 복사한 StockItem prvalue의 소유권을 vector로 이동한다.
    // (4) 생성자와 push_back은 반환값이 없다. (5) 성공 뒤 size=1이고 원소가 이름을 소유한다.
    // (6) 문자 수 선형+상각 O(1), 할당 예외 가능이다. 예약 범위라 vector 재할당은 없고 기존 원소 관찰자는
    //     유지되지만 과거 end 반복자는 무효화된다. 자체 동기화는 없다.
    items.push_back(StockItem{std::string{"ssd"}, 2, 5});
    items.push_back(StockItem{std::string{"cable"}, 15, 10});
    items.push_back(StockItem{std::string{"adapter"}, 1, 4});
    items.push_back(StockItem{std::string{"mouse"}, 8, 8});

    InventorySnapshot snapshot{std::move(items)};

    // [첫 호출 계약: items_to_reorder와 generator 구성]
    // (1) 사용자 수신 객체는 없고 snapshot은 네 원소 owner인 lvalue다.
    // (2) ReorderStream items_to_reorder(InventorySnapshot)가 선택된다.
    // (3) std::move(snapshot)은 xvalue이며 frame 값 매개변수를 이동 구성한다.
    // (4) std::generator<const StockItem&> prvalue가 stream을 직접 초기화해 순회에 사용된다.
    // (5) snapshot은 이동 후 유효·내용 미지정이고 frame이 저장소를 소유하며 본문은 아직 실행되지 않는다.
    // (6) 초기 호출은 상수 작업이나 frame 할당 실패가 가능하다. generator는 이동 전용·단일 통과이고
    //     같은 객체 begin 두 번은 미정의 동작이며 자체 동기화가 없다.
    ReorderStream stream{items_to_reorder(std::move(snapshot))};

    // [첫 생성 계약: std::vector<std::string> 기본 생성]
    // (1) 수신 copied_skus는 아직 생성 전이다.
    // (2) std::vector<std::string>::vector()와 기본 allocator 구성이 선택된다.
    // (3) 인자는 없다. (4) 반환값 없이 빈 SKU owner 목록을 만든다. (5) 성공 뒤 size=0이다.
    // (6) 이 기본 allocator 조합은 상수 시간·noexcept다. 빈 vector는 string 원소 저장소를 요구하지 않지만
    //     구현 내부 할당 여부를 표준 계약 이상으로 단정하지 않으며 관찰자·공유 원소는 없다.
    std::vector<std::string> copied_skus{};
    int total_shortage{};

    // [첫 호출 계약: generator range-for]
    // (1) 수신 stream은 initial suspend 상태의 유효한 generator lvalue다.
    // (2) iterator begin(), default_sentinel_t end() const noexcept,
    //     reference iterator::operator*() const noexcept(is_nothrow_copy_constructible_v<reference>),
    //     iterator& operator++(), friend bool operator==(const iterator&, default_sentinel_t)가 선택된다.
    // (3) begin/end/*/++에는 명시 데이터 인자가 없다. 끝 비교는 현재 iterator const lvalue를 빌리고 end가
    //     만든 default_sentinel_t는 by-value 매개변수로 복사하며 item은 역참조한 const lvalue를 빌린다.
    // (4) iterator·sentinel·참조·bool 반환은 숨은 루프가 사용한다.
    // (5) 루프가 frame 실행 위치를 전진시키고 끝에는 final suspend에 도달한다.
    // (6) 전체 O(N)이고 결과 원소 컨테이너는 만들지 않는다. begin은 initial-suspend 상태에서 한 번만, ++는
    //     아직 끝나지 않은 유효 iterator에서만 쓴다. 종료 iterator의 역참조/증가, owner 파괴 뒤 iterator·item
    //     사용은 미정의 동작이다. begin은 active-stack bookkeeping/최초 재개에서 실패할 수 있고 ++는 재개된
    //     본문 예외를 전파할 수 있다. 같은 generator의 동시 순회·파괴는 안전하지 않다.
    for (const StockItem& item : stream) {
        // [첫 호출 계약: vector<string>::push_back(const string&)]
        // (1) 수신 copied_skus는 앞선 SKU 복사본을 소유한다. (2) const string& 복사 overload를 선택한다.
        // (3) item.sku는 frame의 const lvalue이며 문자를 빌려 새 string에 깊게 복사한다. (4) 반환형 void다.
        // (5) 성공하면 size가 1 증가하고 새 이름은 frame 수명과 독립이다. (6) 상각 O(1)+문자 수 선형,
        //     vector/string 할당 실패가 가능하다. 재할당 시 모든 copied_skus 관찰자가 무효화되고, 재할당이 없어도
        //     과거 end 반복자는 무효화된다. 이 copy-insertable 타입은 실패 시 효과가 없고 동기화는 없다.
        copied_skus.push_back(item.sku);
        total_shortage += item.minimum - item.on_hand;
    }

    // [첫 호출 계약: vector::size]
    // (1) 수신 copied_skus는 두 독립 문자열을 소유한다. (2) size_type size() const noexcept를 선택한다.
    // (3) 인자는 없다. (4) 원소 수 값을 반환해 alert_count 초기화에 사용한다. (5) 상태 변화는 없다.
    // (6) 상수 시간·무할당·noexcept, 무효화 없음이며 동시 구조 변경은 데이터 경쟁이다.
    const auto alert_count{copied_skus.size()};

    // [첫 호출 계약: std::cout operator<<]
    // (1) 수신 std::cout은 유효한 std::ostream이다. (2) const char*, size_type, int, char overload를 잇는다.
    // (3) 리터럴은 빌리고 숫자/개행은 값으로 읽는다. (4) 각 반환형은 ostream&이고 같은 stream을
    //     다음 호출에 쓰며 마지막 반환 참조는 버린다.
    // (5) 스트림 버퍼/상태만 바뀌고 피연산자는 유지된다. (6) 형식 변환·locale·버퍼 비용은 들지만 표준이
    //     단순한 "문자 수 선형" 복잡도를 보장하지는 않는다. 실패는 상태 비트/설정 예외로 나타나며 참조
    //     무효화는 없고 여러 스레드의 한 레코드 원자성은 보장하지 않는다.
    std::cout << "alerts=" << alert_count << ",shortage=" << total_shortage << '\n';
}
