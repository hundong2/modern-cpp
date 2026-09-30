// <cstddef>는 1-based 표시 순번에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 결과를 쓰는 std::cout과 삽입 연산자를 선언한다.
#include <iostream>
// <ranges>는 C++23 인덱스 결합 view std::views::enumerate를 선언한다.
#include <ranges>
// <string>은 이름 문자를 소유하는 std::string을 선언한다.
#include <string>
// <tuple>은 enumerate 역참조 결과를 구조적 바인딩할 때 쓰는 std::get을 선언한다.
#include <tuple>
// <utility>는 소유 컨테이너 이동에 쓰는 std::move를 선언한다.
#include <utility>
// <vector>는 항목과 결과를 연속 저장하는 std::vector를 선언한다.
#include <vector>

// 직접 해보기: 승인되지 않은 항목만 원래의 1-based 위치와 함께 소유 결과로 만든다.
// struct는 멤버가 기본 public이므로 단순 입력/출력 DTO에 알맞다.
struct ReviewItem {
    // [첫 생성 계약: std::string 기본 생성자]
    // (1) 수신 name은 아직 생성 전인 std::string 멤버다.
    // (2) 선택 overload는 basic_string()이고 char/char_traits/allocator 특수화다.
    // (3) 명시 인자는 없으며 {}는 값 초기화다. 외부 문자를 빌리지 않는다.
    // (4) 반환값 없는 생성자가 유효한 빈 소유 문자열을 완성한다.
    // (5) name.size()==0이고 approved나 외부 상태는 바뀌지 않는다.
    // (6) O(1)이며 std::string의 기본 allocator 특수화에서는 noexcept다. 작은 문자열 최적화나
    //     내부 저장 전략은 보장되지 않고 아직 무효화할 iterator/reference는 없다. 생성 중 객체는 공유하지
    //     않으며 완성 뒤 같은 string을 한 스레드가 수정하고 다른 스레드가 접근하려면 외부 동기화가 필요하다.
    std::string name{};
    bool approved{}; // bool{}는 false 값 초기화다.
};

struct PendingReview {
    std::size_t position{};
    std::string name{};
};

// class는 저장소를 private으로 감추고, 원본 view 대신 안전한 소유 결과만 공개한다.
class ReviewChecklist {
public:
    // using은 template 특수화의 별칭이다. Items와 Pending은 서로 다른 vector 타입이다.
    using Items = std::vector<ReviewItem>;
    using Pending = std::vector<PendingReview>;

    // explicit은 Items에서 ReviewChecklist로의 의도하지 않은 암시 변환을 막는다.
    // [첫 호출 계약: std::move(items)와 std::vector<ReviewItem> 이동 생성]
    // (1) 수신 items_는 생성 전이고 값 매개변수 items는 유효한 Items lvalue다.
    // (2) move<T>에서 T=Items&가 선택되어 Items&&를 만들고 vector(vector&&)가 저장소를 받는다.
    // (3) items는 lvalue, move(items)는 xvalue이며 원소 저장소 소유권을 넘길 수 있다.
    // (4) move 반환 참조는 생성에 사용되고 vector 생성자는 반환값 없이 items_를 완성한다.
    // (5) items_가 원소를 소유하며 items는 유효하지만 값이 미지정된 moved-from 상태다.
    // (6) 기본 allocator vector 이동은 O(1)·noexcept이고 보통 할당하지 않는다. 기존 원소 참조는
    //     새 owner를 가리키며 같은 객체에 대한 동시 이동/접근은 외부 동기화가 필요하다.
    explicit ReviewChecklist(Items items) noexcept
        : items_{std::move(items)} {}

    // 반환값이 문자열까지 독립 소유하고 내부 view는 반환 전에 모두 소비되므로 lvalue/rvalue owner 모두 안전하다.
    [[nodiscard]] Pending pending_items() const {
        // [첫 호출 계약: std::views::enumerate(items_)]
        // (1) 사용자 데이터 수신 객체는 없고, 기반 items_는 살아 있는 const Items lvalue이며 완성 원소 세 개를 소유한다.
        // (2) views::enumerate(E)는 enumerate_view<views::all_t<decltype((E))>>(E)와
        //     expression-equivalent하고 여기서는 ref_view<const Items> 기반이 선택된다.
        //     adaptor의 구현 operator() 선언은 비공개 세부다.
        // (3) items_는 const lvalue·비소유 대여이고 viewable_range다. input_range이면서 참조와
        //     rvalue-reference 결과가 이동 구성 가능한 요구를 만족하고, 이 vector는 sized_range이기도 하다.
        // (4) 0-based signed index 값과 const ReviewItem&를 지연 생성하는 view prvalue를 반환해 사용한다.
        // (5) items_는 바뀌지 않고 결과 view는 아직 어떤 원소도 순회하지 않는다.
        // (6) O(1)·원소 할당 없음이다. owner 파괴나 vector 구조 변경 뒤 iterator/reference는 무효다.
        //     기반 view 구성 예외는 전파 가능하고, 읽기와 동시 쓰기는 데이터 경쟁이다.
        auto indexed = std::views::enumerate(items_);

        // [첫 생성 계약: std::vector<PendingReview>()]
        // (1) 수신 result는 아직 생성 전이다.
        // (2) vector()와 기본 allocator가 선택되며 value_type=PendingReview다.
        // (3) 명시 인자는 없고 외부 저장소를 빌리지 않는다.
        // (4) 반환값 없이 빈 Pending을 완성한다.
        // (5) size는 0이고 items_/indexed는 변하지 않는다.
        // (6) O(1), 일반 allocator에서 할당 없음·noexcept다. 아직 관찰자가 없고 지역 result는 공유되지 않는다.
        //     완성 뒤 같은 vector를 한 스레드가 수정하며 다른 스레드가 접근하려면 외부 동기화가 필요하다.
        Pending result{};

        // [첫 호출 계약: vector::size()와 reserve(count)]
        // (1) size 수신 items_는 const Items lvalue, reserve 수신 result는 빈 Pending이다.
        // (2) size_type size() const noexcept와 void reserve(size_type)가 차례로 선택된다.
        // (3) size 인자는 없고 반환한 size_type prvalue가 reserve의 허용 count 값이 된다.
        // (4) size 반환값은 사용하며 reserve는 void라 반환값이 없다.
        // (5) 성공하면 result.size()==0, capacity()>=items_.size()이고 원본은 그대로다.
        // (6) size O(1)·noexcept; reserve는 현재 원소 수에 선형이며 할당, length_error/bad_alloc이
        //     가능하다. 재할당 시 result 관찰자가 무효지만 아직 없고 같은 vector 동시 수정은 안전하지 않다.
        result.reserve(items_.size());

        // [첫 숨은 호출 계약: enumerate view range-for와 구조적 바인딩]
        // (1) 수신 indexed의 정확한 타입은 enumerate_view<ref_view<const Items>>이고, 살아 있는 items_를
        //     빌리는 유효한 non-const lvalue다. 기반 ref_view는 simple-view다.
        // (2) 그 때문에 begin() const/end() const와 iterator<true>::operator*() const, 전위 operator++()가
        //     선택되고, begin != end는 iterator<true>의 operator== 후보를 부정하도록 재작성된다. 구조적 바인딩은
        //     template<size_t I,class... Ts> tuple_element_t<I,tuple<Ts...>>&& std::get(tuple<Ts...>&&) noexcept를
        //     I=0,1로 선택한다. auto&& 숨은 객체가 prvalue tuple에 바인딩되어 get에 xvalue로 전달되기 때문이다.
        // (3) begin/end/*/++에는 명시 데이터 인자가 없다. 재작성된 operator==(const iterator<true>& x,
        //     const iterator<true>& y)는 begin 쪽과 end 쪽 iterator lvalue 두 개를 비소유 const 참조로 받는다.
        //     각 std::get<I>의 I는 컴파일 시간 size_t 값이고, 한 함수 인자는 현재 반복
        //     tuple<difference_type,const ReviewItem&> xvalue다. 어느 호출도 원소 소유권을 얻지 않는다.
        // (4) begin/end는 common range라 iterator<true>, 비교는 bool, 역참조는 위 tuple을 반환해 사용한다.
        //     get<0>은 difference_type&&를 반환해 zero_based가 가리키고,
        //     get<1>은 참조 축약으로 const ReviewItem&를 반환해 item이 가리킨다. 두 이름은 lvalue 식이며
        //     전위 증가의 iterator& 반환은 버린다.
        // (5) iterator/index만 전진하고 items_와 ReviewItem은 바뀌지 않는다.
        // (6) 각 연산 O(1), 전체 O(n), 무할당이며 std::get은 noexcept다. tuple 임시는 반복 끝까지 산다.
        //     end 역참조/owner 파괴/무효화 뒤 접근은 UB이며 같은 owner의 동시 쓰기는 데이터 경쟁이다.
        for (auto&& [zero_based, item] : indexed) {
            // if는 bool 멤버를 load해 조건 분기한다. 승인 항목은 결과에 넣지 않는다.
            if (item.approved) {
                continue;
            }

            // [첫 호출 계약: std::vector<PendingReview>::push_back(PendingReview&&)와 string 복사]
            // (1) 수신 result는 미리 capacity를 확보한 유효 vector이고 item.name은 const string lvalue다.
            // (2) void push_back(value_type&&)와 name의 basic_string(const basic_string&)가 선택된다.
            // (3) PendingReview 지정 초기화 식은 prvalue다. index는 비음수 signed 값이고 name은 빌린 lvalue다.
            // (4) string 생성자는 멤버를 완성하고 push_back 반환형 void는 사용하지 않는다.
            // (5) result 크기가 1 증가하고 새 결과가 위치와 문자열을 독립 소유하며 item은 그대로다.
            // (6) push는 확보 용량 안에서 O(1), 문자열 복사는 O(길이)이며 bad_alloc 가능하다. 실패하면
            //     vector는 강한 보장을 유지하고, 이 범위에서는 재할당이 없어 기존 result 참조도 유지된다.
            //     result는 지역 객체이고 item은 읽기 전용이다. 같은 객체의 동시 수정/접근에는 외부 동기화가 필요하다.
            result.push_back(PendingReview{
                .position = static_cast<std::size_t>(zero_based) + 1U,
                .name = item.name,
            });
        }

        // NRVO가 적용되면 result가 호출자 결과 객체로 직접 만들어지고, 아니면 이동할 수 있다.
        return result;
    }

private:
    Items items_;
};

int main() {
    // [첫 생성 계약: std::vector<ReviewItem>(initializer_list)와 std::string 리터럴 생성]
    // (1) 수신 seed와 각 name string은 생성 전이다.
    // (2) vector(initializer_list<ReviewItem>)와 basic_string(const char*)가 선택된다.
    // (3) 세 리터럴은 정적 const char 배열 lvalue에서 포인터로 변환되며 null이 아니다. 목록 원소는 복사된다.
    // (4) 반환값 없는 생성자가 세 원소와 문자를 독립 소유하는 Items를 만든다.
    // (5) seed는 parser/cache/docs 순서를 보존하고 리터럴은 변하지 않는다.
    // (6) 원소·문자 수에 선형, 할당/bad_alloc 가능, 실패 시 부분 원소를 정리한다. 외부 관찰자는 아직 없고
    //     생성 중 seed는 공유되지 않는다. 완성 뒤 같은 객체의 동시 수정/접근에는 외부 동기화가 필요하다.
    ReviewChecklist::Items seed{
        {"parser", true},
        {"cache", false},
        {"docs", false},
    };

    // 이름 있는 seed는 lvalue이고 move(seed)는 xvalue다. 생성자가 vector 저장소를 items_로 옮긴다.
    const ReviewChecklist checklist{std::move(seed)};

    // 반환 prvalue가 pending을 직접 초기화하며 결과 문자열은 checklist 수명에 의존하지 않는다.
    const ReviewChecklist::Pending pending = checklist.pending_items();

    // [첫 숨은 호출 계약: const std::vector<PendingReview> range-for]
    // (1) 수신 pending은 두 결과를 소유하는 const Pending lvalue다.
    // (2) const begin/end와 const_iterator 비교/역참조/전위 증가가 선택된다.
    // (3) row는 역참조 const PendingReview lvalue를 const&로 빌리고 소유권을 얻지 않는다.
    // (4) iterator·bool·const reference를 사용하고 증가 반환 참조는 버린다.
    // (5) 반복자만 전진하며 pending과 원소는 바뀌지 않는다.
    // (6) O(n), 무할당이다. 순회 중 구조 변경은 금지되고 읽기끼리는 가능하지만 동시 쓰기는 데이터 경쟁이다.
    for (const PendingReview& row : pending) {
        // [첫 호출 계약: std::ostream 삽입 연산자 체인]
        // (1) 수신 std::cout은 출력 가능한 std::ostream lvalue다.
        // (2) size_t 멤버 overload와 operator<<(ostream&, char/const string&)가 선택된다.
        // (3) position/name은 const lvalue에서 읽고 ':'/'\n'은 char prvalue다. 소유권 이동은 없다.
        // (4) 각 ostream& 반환은 다음 삽입에 쓰고 마지막 반환은 버린다.
        // (5) 출력 버퍼와 실패 상태만 바뀌고 row는 그대로다.
        // (6) 문자 수에 비례하며 실패 시 상태 비트, 예외 mask에 따라 ios_base::failure가 가능하다.
        //     한 레코드 단위 원자성은 없으므로 여러 스레드 출력에는 외부 동기화가 필요하다.
        std::cout << row.position << ':' << row.name << '\n';
    }
}
