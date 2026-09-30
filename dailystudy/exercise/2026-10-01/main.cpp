// <cstddef>는 컨테이너 크기와 순번에 쓰는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 출력 객체 std::cout과 스트림 삽입 연산자를 선언한다.
#include <iostream>
// <ranges>는 C++23 std::views::enumerate와 지연 range/view 기반을 선언한다.
#include <ranges>
// <string>은 문자 저장소를 직접 소유하는 std::string을 선언한다.
#include <string>
// <tuple>은 enumerate 역참조 결과를 구조적 바인딩할 때 쓰는 std::get을 선언한다.
#include <tuple>
// <utility>는 lvalue를 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>
// <vector>는 원소를 연속 저장소에 소유하는 std::vector를 선언한다.
#include <vector>

// enum class는 정수와 암시적으로 섞이지 않는 강한 열거형이다.
enum class StepState {
    ready,
    blocked,
};

// struct의 멤버는 기본적으로 public이다. 단순 데이터 전달 객체(DTO)에 알맞다.
struct ReleaseStep {
    // [첫 생성 계약: std::string 기본 생성자]
    // (1) 수신 name은 ReleaseStep 객체 안에서 아직 생성되지 않은 std::string 멤버다.
    // (2) 선택 생성자는 basic_string()이며 char, char_traits<char>, allocator<char> 특수화다.
    // (3) 명시 인자는 없고 {}는 값 초기화 문법이다. 외부 문자 저장소를 빌리지 않는다.
    // (4) 생성자는 반환값이 없고 유효한 빈 문자열 객체를 완성한다.
    // (5) name은 size()==0인 독립 소유 객체가 되며 다른 멤버나 인자는 변하지 않는다.
    // (6) O(1)이며 std::string의 기본 allocator 특수화에서는 noexcept다. 표준은 작은 문자열 최적화나
    //     내부 저장 전략을 보장하지 않는다. 아직 iterator/reference는 없으며 객체별 동시 사용은 독립적이다.
    std::string name{};

    // 중괄호 {}는 값 초기화다. enum은 첫 열거값 ready로 초기화된다.
    StepState state{};
};

// 외부 계층으로 넘길 결과는 view가 아니라 문자열까지 직접 소유한다.
struct NumberedStep {
    std::size_t number{};
    std::string name{};
    StepState state{};
};

// const char*는 문자를 소유하지 않는 포인터다. null일 수도 있지만 이 함수는 정적 수명의
// 문자열 리터럴 첫 문자를 가리키는 non-null 포인터만 반환한다. 참조와 달리 포인터는 재바인딩 가능하다.
[[nodiscard]] constexpr const char* state_name(const StepState state) noexcept {
    // switch는 state 값을 비교해 정확히 한 case로 조건 분기한다.
    switch (state) {
    case StepState::ready:
        return "ready";
    case StepState::blocked:
        return "blocked";
    }
    return "unknown";
}

// class의 멤버는 기본적으로 private다. owner를 감추고 안전한 관찰 경계만 공개한다.
class ReleasePlan {
public:
    // using은 긴 class-template 특수화에 도메인 이름을 붙일 뿐 새 강한 타입을 만들지 않는다.
    // template 인자 T는 각각 ReleaseStep과 NumberedStep이다.
    using Steps = std::vector<ReleaseStep>;
    using Snapshot = std::vector<NumberedStep>;

    // explicit은 Steps 하나가 ReleasePlan으로 뜻밖에 암시 변환되는 일을 막는다.
    // 값을 받는 매개변수는 lvalue 호출에서는 복사되고 xvalue 호출에서는 이동되어 소유권 경계를 만든다.
    // [첫 호출 계약: std::move(steps) 및 std::vector<ReleaseStep> 이동 생성]
    // (1) 수신 steps_는 아직 생성 전이고, 매개변수 steps는 유효한 Steps lvalue다.
    // (2) move는 template<class T> remove_reference_t<T>&& move(T&&) noexcept에서 T=Steps&가
    //     선택되고, 이어 vector(vector&&) 이동 생성자가 steps_를 완성한다.
    // (3) steps는 Steps lvalue, move(steps)는 Steps&& xvalue다. 저장소 소유권을 넘길 수 있다.
    // (4) move는 xvalue 참조를 반환해 생성에 사용하고, 생성자는 반환값 없이 steps_를 만든다.
    // (5) steps_가 기존 원소를 소유하며 steps는 유효하지만 값이 미지정된 moved-from 상태가 된다.
    // (6) 기본 allocator가 같은 vector 이동은 O(1)·noexcept이고 보통 할당하지 않는다. 기존 원소
    //     참조는 새 owner의 원소를 가리키며, 같은 객체의 동시 이동/접근에는 외부 동기화가 필요하다.
    explicit ReleasePlan(Steps steps) noexcept
        : steps_{std::move(steps)} {}

    // const 뒤의 &는 살아 있는 const lvalue owner에서만 비소유 view를 꺼내도록 제한한다.
    [[nodiscard]] auto indexed_steps() const & {
        // [첫 호출 계약: std::views::enumerate(steps_)]
        // (1) 사용자 데이터 수신 객체는 없고, 기반 steps_는 살아 있는 const Steps lvalue이며
        //     완성된 ReleaseStep 원소를 연속 저장소에 소유한다.
        // (2) adaptor 객체의 구체 operator() 시그니처는 표준화되지 않는다. 표준 식
        //     views::enumerate(E)는 enumerate_view<views::all_t<decltype((E))>>(E)와
        //     expression-equivalent하고, 기반 타입은 ref_view<const Steps>로 추론된다.
        // (3) 유일한 인자 steps_는 const lvalue·비소유 대여다. viewable_range이고, input_range이면서
        //     참조/rvalue-reference 결과가 이동 구성 가능한 요구를 만족한다. 이 vector는 sized_range이기도 하다.
        //     저장소나 원소 소유권은 이동하지 않는다.
        // (4) 반환값은 0-based signed index 값과 const ReleaseStep&를 tuple-like하게 내는 지연 view
        //     prvalue이며 호출자가 반환한다. 아직 원소를 읽지 않는다.
        // (5) steps_의 size/capacity/원소는 그대로이고 view가 owner를 가리키는 작은 기반 view를 보관한다.
        // (6) 구성은 O(1), 원소 할당·복사 없음이다. owner 파괴나 vector 구조 변경 뒤 관찰은 무효다.
        //     기반 view 구성이 던지는 예외는 전파될 수 있고, 읽기끼리는 가능하지만 동시 쓰기는 데이터 경쟁이다.
        return std::views::enumerate(steps_);
    }

    // 임시 ReleasePlan은 이 문장 끝에서 파괴되므로 비소유 view 반환을 컴파일 단계에서 금지한다.
    void indexed_steps() const && = delete;

    // 반환 vector는 문자열까지 소유하므로 ReleasePlan보다 오래 살아도 안전한 boundary DTO다.
    [[nodiscard]] Snapshot make_snapshot() const & {
        // 반환된 enumerate view prvalue가 enumerated 객체를 직접 초기화한다.
        auto enumerated = indexed_steps();

        // [첫 생성 계약: std::vector<NumberedStep>()]
        // (1) 수신 rows는 아직 생성 전이다.
        // (2) 선택 overload는 vector() noexcept(noexcept(Allocator()))이고 T=NumberedStep,
        //     Allocator=allocator<NumberedStep>다.
        // (3) 명시 인자는 없고 기본 allocator가 값 초기화된다. 외부 저장소를 빌리지 않는다.
        // (4) 생성자는 반환값 없이 빈 Snapshot 객체를 완성한다.
        // (5) rows.size()==0이며 steps_와 enumerated는 변하지 않는다.
        // (6) O(1), 일반 기본 allocator에서는 할당 없음·noexcept다. 참조/iterator도 아직 없고
        //     rows를 다른 스레드와 공유하지 않으므로 별도 동기화 상태가 없다.
        Snapshot rows{};

        // [첫 호출 계약: vector::size()와 reserve(count)]
        // (1) size 수신 steps_는 유효한 const Steps lvalue이고 3개 원소를 소유한다. reserve 수신 rows는 빈 Snapshot이다.
        // (2) 선택 overload는 size_type size() const noexcept와 void reserve(size_type new_capacity)다.
        // (3) size에는 인자가 없다. 그 반환 size_type prvalue가 reserve의 값 인자 count가 되며 소유권 이동은 없다.
        // (4) size는 원소 수를 반환해 사용되고 reserve는 void라 반환값이 없다.
        // (5) 성공하면 rows의 size는 0인 채 capacity가 적어도 count가 되고, steps_와 인자는 변하지 않는다.
        // (6) size는 O(1)·무할당·noexcept다. reserve는 현재 원소 수에 선형이고 할당할 수 있으며
        //     length_error/bad_alloc이 가능하다. 재할당 시 rows의 관찰자는 모두 무효지만 아직 없고, 동시 접근은 안전하지 않다.
        rows.reserve(steps_.size());

        // [첫 숨은 호출 계약: enumerate view range-for와 구조적 바인딩]
        // (1) 수신 enumerated의 정확한 타입은 enumerate_view<ref_view<const Steps>>이고, 살아 있는
        //     steps_를 빌리는 유효한 non-const lvalue다. 기반 ref_view는 simple-view다.
        // (2) 그 때문에 begin() const/end() const와 iterator<true>::operator*() const, 전위 operator++()가
        //     선택되고, begin != end는 iterator<true>의 operator== 후보를 부정하도록 재작성된다.
        //     구조적 바인딩은 template<size_t I,class... Ts> tuple_element_t<I,tuple<Ts...>>&&
        //     std::get(tuple<Ts...>&&) noexcept를 I=0,1로 선택한다. auto&& 숨은 객체가 prvalue tuple에
        //     바인딩되어 get에 xvalue로 전달되기 때문이다.
        // (3) begin/end/*/++에는 명시 데이터 인자가 없다. 재작성된 operator==(const iterator<true>& x,
        //     const iterator<true>& y)는 begin 쪽과 end 쪽 iterator lvalue 두 개를 비소유 const 참조로 받는다.
        //     각 std::get<I>의 I는 컴파일 시간 size_t 값이고, 한 함수 인자는 현재 반복
        //     tuple<difference_type,const ReleaseStep&> xvalue다. 어느 호출도 원소 소유권을 얻지 않는다.
        // (4) begin/end는 common range라 iterator<true>, 비교는 bool, 역참조는 위 tuple을 반환해 사용한다.
        //     get<0>은 difference_type&&를 반환해
        //     zero_based가 가리키고, get<1>은 참조 축약으로 const ReleaseStep&를 반환해 step이 가리킨다.
        //     이름 zero_based와 step 자체는 lvalue 식이며 전위 증가의 iterator& 반환은 버린다.
        // (5) iterator와 0-based index만 전진하며 steps_와 ReleaseStep 원소는 바뀌지 않는다.
        // (6) 각 연산 O(1), 전체 O(n), 무할당이며 std::get은 noexcept다. 현재 tuple 임시는 반복 끝까지 산다.
        //     end 역참조, owner 파괴, vector 무효화 뒤 접근은 UB이고 같은 owner의 동시 쓰기는 데이터 경쟁이다.
        for (auto&& [zero_based, step] : enumerated) {
            // [첫 호출 계약: std::vector<NumberedStep>::push_back(NumberedStep&&)와 std::string 복사 생성]
            // (1) 수신 rows는 capacity를 미리 확보한 유효한 Snapshot이고, step.name은 유효한 const string lvalue다.
            // (2) 선택 overload는 void push_back(value_type&&)이고 value_type=NumberedStep다. 지정 초기화 중
            //     name에는 basic_string(const basic_string&) 복사 생성자가 선택된다.
            // (3) NumberedStep{...}는 prvalue다. zero_based는 signed index 값, step.name은 비소유 const lvalue다.
            //     문자열 문자를 새 객체로 깊게 복사하고 원본 소유권은 유지한다.
            // (4) string 생성자는 반환값 없이 멤버를 만들고 push_back은 void라 반환값을 사용하지 않는다.
            // (5) 성공하면 rows.size()가 1 증가하고 새 마지막 원소가 번호·이름·상태를 독립 소유한다.
            // (6) 미리 reserve했으므로 이번 범위에서는 push가 O(1)이고 rows 관찰자를 재할당으로 무효화하지 않는다.
            //     문자열 복사는 O(name 길이), 할당/bad_alloc 가능, 실패 시 vector는 강한 보장을 유지한다.
            //     rows는 지역 객체이고 step은 읽기 전용이다. 같은 객체를 다른 스레드가 수정하려면 외부 동기화가 필요하다.
            rows.push_back(NumberedStep{
                .number = static_cast<std::size_t>(zero_based) + 1U,
                .name = step.name,
                .state = step.state,
            });
        }

        // rows는 이름 있는 lvalue지만 반환 문맥에서 NRVO 후보다. NRVO가 없으면 암시적 이동이 허용된다.
        return rows;
    }

private:
    // 실제 ReleaseStep과 string 수명은 이 vector가 소유하며 enumerate view는 이를 빌릴 뿐이다.
    Steps steps_;
};

int main() {
    // [첫 생성 계약: std::vector<ReleaseStep>(initializer_list)와 std::string(const char*)]
    // (1) 수신 seed와 그 안의 각 string 멤버는 아직 생성 전이다.
    // (2) vector(initializer_list<value_type>, const Allocator&=Allocator())와 각 name의
    //     basic_string(const char*, const Allocator&=Allocator())가 선택된다.
    // (3) 세 문자열 리터럴은 정적 수명의 const char[N] lvalue이며 const char*로 변환된다. null이 아니고
    //     NUL로 끝난다. initializer_list backing array의 const ReleaseStep 원소는 vector로 복사된다.
    // (4) 생성자는 반환값 없이 세 ReleaseStep과 각 문자 저장소를 소유하는 Steps를 완성한다.
    // (5) seed는 입력 순서를 보존하며 리터럴과 소유 문자열은 서로 독립이다.
    // (6) 총 문자 수와 원소 수에 선형이고 동적 할당/bad_alloc이 가능하다. 실패 시 완성 원소를 정리하며
    //     외부 iterator는 아직 없고 생성 중 객체를 다른 스레드와 공유하지 않는다.
    ReleasePlan::Steps seed{
        {"compile", StepState::ready},
        {"test", StepState::blocked},
        {"deploy", StepState::ready},
    };

    // seed는 이름 있는 lvalue, std::move(seed)는 xvalue다. 실제 저장소 이전은 선택된 vector 이동 생성이 한다.
    ReleasePlan plan{std::move(seed)};

    // make_snapshot()이 반환한 vector prvalue가 snapshot을 직접 초기화한다. 함수 안 NRVO가 별개로
    // 적용될 수 있고, 반환 prvalue에서 결과 객체로의 마지막 초기화는 C++17 보장 복사 생략 대상이다.
    const ReleasePlan::Snapshot snapshot = plan.make_snapshot();

    // [첫 생성 계약: std::vector<NumberedStep> 복사 생성]
    // (1) 수신 copied는 생성 전이고 snapshot은 세 소유 원소를 가진 const Snapshot lvalue다.
    // (2) vector(const vector& other)가 선택되고 allocator는 select_on_container_copy_construction으로 얻는다.
    // (3) other 식은 const lvalue이며 각 NumberedStep과 내부 string을 깊게 복사한다. 소유권 공유가 아니다.
    // (4) 생성자는 반환값 없이 독립 저장소를 가진 copied를 완성한다.
    // (5) 두 vector 값은 같고 이후 한쪽 구조 변경은 다른 쪽 참조/iterator에 영향을 주지 않는다.
    // (6) 원소·문자 수에 선형이고 할당/bad_alloc 가능, 실패 시 부분 결과를 정리해 원본을 보존한다.
    //     서로 다른 객체는 각각 수정 가능하지만 같은 객체의 동시 쓰기는 외부 동기화가 필요하다.
    const ReleasePlan::Snapshot copied{snapshot};

    // [첫 숨은 호출 계약: const std::vector<NumberedStep> range-for]
    // (1) 수신 copied는 세 원소를 소유하는 const Snapshot lvalue다.
    // (2) const begin/end와 const_iterator의 비교, 역참조, 전위 증가가 선택된다.
    // (3) 명시 인자는 없고 row는 역참조 const NumberedStep lvalue를 const&로 빌린다.
    // (4) iterator·bool·const reference를 순회에 사용하고 증가 반환 참조는 버린다.
    // (5) loop iterator만 바뀌고 copied와 원소는 변하지 않는다.
    // (6) O(n), 무할당이며 순회 중 구조 변경이 없어야 한다. 읽기끼리는 가능하나 동시 쓰기는 데이터 경쟁이다.
    for (const NumberedStep& row : copied) {
        // [첫 호출 계약: std::ostream 삽입 연산자 체인]
        // (1) 최초 수신 std::cout은 프로그램 시작 때 구성된 std::ostream lvalue이며 출력 가능한 상태여야 한다.
        // (2) size_t 정수 멤버 overload와 비멤버 operator<<(ostream&, char/const char*/const string&)가 선택된다.
        // (3) number/name은 const lvalue에서 값을 읽고, ':'/'\n'은 char prvalue, state_name 결과는
        //     정적 문자열을 가리키는 const char* prvalue다. 어느 인자도 소유권을 넘기지 않는다.
        // (4) 각 삽입은 ostream&를 반환해 다음 <<의 수신으로 쓰고 마지막 반환 참조는 버린다.
        // (5) 문자가 순서대로 버퍼에 기록되고 실패 시 failbit/badbit가 설정될 수 있으며 인자는 그대로다.
        // (6) 비용은 형식화/문자 수에 비례하고 표준은 내부 할당 횟수를 정하지 않는다. 예외 mask에 따라
        //     ios_base::failure가 가능하다. 동시 출력은 레코드 원자성을 보장하지 않아 외부 동기화가 필요하다.
        std::cout << row.number << ':' << row.name << ':' << state_name(row.state) << '\n';
    }
}
