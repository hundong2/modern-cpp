// <cstddef>는 컨테이너 크기를 표현하는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 출력 객체 std::cout과 스트림 삽입 연산자를 선언한다.
#include <iostream>
// <ranges>는 C++23 std::views::chunk_by와 그 지연 view/iterator를 선언한다.
#include <ranges>
// <string_view>는 문자를 소유하지 않고 읽기만 하는 std::string_view를 선언한다.
#include <string_view>
// <utility>는 lvalue를 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>
// <vector>는 원소를 연속 메모리에 소유하는 std::vector를 선언한다.
#include <vector>

// enum class는 정수와 암시적으로 섞이지 않는 강한 열거형이다.
// 각 값은 급여 원장이 이미 정렬된 부서 키를 나타낸다.
enum class Department {
    engineering,
    sales,
    support,
};

// struct의 멤버는 기본적으로 public이다. 단순 데이터 묶음인 DTO에 알맞다.
// 중괄호의 {}는 값 초기화다. int는 0, Department는 첫 열거값으로 초기화된다.
struct Employee {
    int id{};
    Department department{};
    int salary{};
};

// 한 인접 부서 구간을 독립적으로 보관하는 소유 결과 DTO다.
struct DepartmentSummary {
    Department department{};
    std::size_t employee_count{};
    long long salary_sum{};
};

// [첫 생성 계약: std::string_view(const char*)]
// (1) 수신 결과는 std::string_view prvalue이며 아직 어떤 문자도 가리키지 않는다.
// (2) 선택 생성자는 constexpr basic_string_view(const char* text)이고 CharT=char,
//     Traits=std::char_traits<char>인 std::string_view 특수화다.
// (3) 각 return의 문자열 리터럴은 const char[N] lvalue이고 const char*로 변환된다.
//     문자는 정적 저장 기간을 가지며 view는 소유권을 얻지 않는다. null 포인터는 허용되지 않는다.
// (4) 반환형은 std::string_view prvalue이고 호출자는 부서 이름을 읽는 데 사용한다.
// (5) 원본 리터럴은 변하지 않고 결과는 끝의 '\0'을 제외한 문자 구간만 관찰한다.
// (6) traits::length 때문에 O(N)이며 할당·무효화가 없다. 표준 생성자 선언 자체에는 noexcept가
//     붙지 않지만 표준 char_traits와 유효한 리터럴 경로는 던지지 않는다. 바깥 함수는 noexcept라
//     뜻밖의 예외가 탈출하면 terminate한다. 리터럴은 정적 수명이고 공유 읽기는 동시에도 안전하다.
[[nodiscard]] constexpr std::string_view department_name(const Department department) noexcept {
    // switch는 열거값을 비교한 뒤 맞는 case로 조건 분기한다.
    switch (department) {
    case Department::engineering:
        return "engineering";
    case Department::sales:
        return "sales";
    case Department::support:
        return "support";
    }
    // 모든 정상 열거값을 위에서 처리했다. 손상된 값에도 빈 정적 문자열 view를 돌려준다.
    return "";
}

// class의 멤버는 기본적으로 private이다. 소유 저장소를 감추고 안전한 연산만 공개한다.
class PayrollSnapshot {
public:
    // using은 긴 template 특수화에 도메인 이름을 붙인다.
    // std::vector의 template 인자 T는 각각 Employee와 DepartmentSummary다.
    using Records = std::vector<Employee>;
    using Summaries = std::vector<DepartmentSummary>;

    // explicit은 Records 하나가 PayrollSnapshot으로 뜻밖에 암시 변환되는 일을 막는다.
    // 매개변수를 값으로 받으면 호출자가 lvalue를 주면 복사, xvalue를 주면 이동해 소유권 경계를 만든다.
    // [첫 호출 계약: std::move(records) 및 std::vector<Employee> 이동 생성]
    // (1) 수신 멤버 records_는 아직 생성 전이고, 매개변수 records는 유효한 Records lvalue다.
    // (2) std::move의 선택 시그니처는 template<class T> remove_reference_t<T>&& move(T&&)
    //     noexcept이고 T=Records&다. 이어 Records(Records&&) 이동 생성자가 선택된다.
    // (3) 식 records의 타입은 Records, 값 범주는 lvalue, std::move(records)는 Records&& xvalue다.
    //     요소 저장소의 소유권을 records_로 넘길 수 있으며 새 소유자는 records_다.
    // (4) std::move는 Records&&를 반환해 생성에 사용하고, vector 생성자는 새 객체 records_를 만든다.
    // (5) records_는 기존 요소를 소유하며 records는 유효하지만 값이 미지정인 moved-from 상태가 된다.
    // (6) 기본 allocator가 같은 vector 이동은 O(1)·noexcept이고 보통 할당하지 않는다. 기존 요소의
    //     참조는 새 vector 원소를 가리키지만 매개변수 자체를 다시 읽어 값에 의존하면 안 된다.
    explicit PayrollSnapshot(Records records) noexcept
        : records_{std::move(records)} {}

    // const 뒤의 &는 const lvalue 객체에서만 이 view를 빌릴 수 있게 한다.
    // 반환 view는 records_를 소유하지 않으므로 PayrollSnapshot보다 오래 살아서는 안 된다.
    [[nodiscard]] auto department_groups() const & {
        // const Employee&는 복사 없이 기존 원소를 빌리고 수정하지 못하게 한다.
        const auto same_department = [](const Employee& left, const Employee& right) noexcept {
            return left.department == right.department;
        };

        // [첫 호출 계약: std::views::chunk_by(records_, same_department)]
        // (1) 수신 기반 범위는 정확히 const Records lvalue인 records_이고 유효한 연속 저장소를 소유한다.
        // (2) adaptor 객체의 구체 operator() 시그니처는 표준화되지 않는다. 표준의 두 인자 식
        //     views::chunk_by(E,F)는 chunk_by_view(E,F)와 expression-equivalent하며, 여기서 deduction 결과는
        //     V=ref_view<const Records>, Pred=decay_t<decltype(same_department)>다.
        // (3) records_는 const lvalue·비소유 대여, same_department도 const lvalue이며 복사 가능한 술어다.
        //     술어는 인접한 두 const Employee lvalue reference에 대해 bool로 변환 가능한 값을 돌려야 한다.
        // (4) 반환형은 지연 view prvalue이고 반환한다. 각 역참조 결과는 같은 부서가 연속한 비소유 subrange다.
        // (5) records_와 원소는 바뀌지 않고 view가 기반 범위 참조와 술어 사본을 보관한다. 아직 순회하지 않는다.
        // (6) 생성은 O(1), 할당 없음이다. 순회 중 각 인접 경계를 한 번 비교한다. 소유 snapshot이 먼저
        //     파괴되거나 vector가 구조 변경되면 iterator/view가 무효화된다. 술어는 noexcept이고 이 읽기만
        //     수행할 때 안전하지만 다른 스레드의 동시 쓰기는 데이터 경쟁이다. 빈 범위도 허용된다.
        return std::views::chunk_by(records_, same_department);
    }

    // 임시 snapshot에서 비소유 view를 꺼내 즉시 dangling시키는 호출은 컴파일 단계에서 금지한다.
    void department_groups() const && = delete;

    // const 멤버 함수이므로 숨은 this 포인터는 const PayrollSnapshot*이며 저장소를 바꾸지 않는다.
    // 반환 vector는 view가 아니라 값을 소유하므로 snapshot보다 오래 살아도 안전하다.
    // 기계 실행 관점에서는 순회가 원소 load, 부서 비교, 경계 조건 분기, 누계 load/store로 나타날 수 있다.
    // 술어는 구체 lambda라 가상 간접 호출이 필요하지 않고 인라인될 수 있지만, 실제 명령·분기 제거·벡터화는
    // CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 형태로 단정하지 않는다.
    [[nodiscard]] Summaries summarize() const & {
        // auto는 오른쪽 prvalue의 실제 chunk_by_view 타입을 보존해 이름 있는 lvalue groups를 만든다.
        auto groups = department_groups();

        // [첫 생성 계약: std::vector<DepartmentSummary>()]
        // (1) 수신 summaries는 아직 생성 전이다.
        // (2) 선택 overload는 constexpr vector() noexcept(noexcept(Allocator()))이며
        //     T=DepartmentSummary, Allocator=std::allocator<DepartmentSummary>다.
        // (3) 명시 인자는 없고 기본 allocator가 값 초기화된다. 외부 저장소를 빌리지 않는다.
        // (4) 생성자는 반환값이 없고 비어 있는 Summaries 객체를 완성한다.
        // (5) summaries.size()==0이고 capacity는 구현이 정한 0 상당이며 records_는 변하지 않는다.
        // (6) O(1), 일반 기본 allocator에서는 할당 없음·noexcept다. 참조/iterator도 아직 없고
        //     소멸 시 자신이 얻은 원소와 저장소만 정리한다. 별도 객체의 동시 사용에는 간섭하지 않는다.
        Summaries summaries{};

        // [첫 숨은 호출 계약: chunk_by_view의 range-for begin/end/iterator 연산]
        // (1) 수신은 이름 있는 chunk_by_view lvalue groups이고 기반 const Records와 snapshot은 살아 있다.
        // (2) vector 기반은 common_range라 groups.begin()/end() 모두 같은 outer iterator를 반환한다.
        //     range-for의 `!=`는 iterator의 operator==에서 재작성되고 operator*, operator++도 선택된다.
        //     begin은 첫 경계를 지연 탐색하며 역참조는 iterator 쌍의 subrange prvalue를 만든다.
        // (3) 명시 인자는 없고 same_department는 인접 const Employee& 두 개만 읽는다. auto&& group은
        //     그 subrange prvalue에 바인딩되어 현재 반복 수명 동안만 유효하다.
        // (4) begin/end는 outer iterator, 비교는 bool, 역참조는 비소유 subrange를 반환해 사용된다.
        //     전위 ++는 outer iterator&를 반환하지만 range-for는 그 반환 참조를 버린다.
        // (5) 첫 begin은 groups 내부 cache에 첫 경계를 기록할 수 있고, 이후 외부 iterator만 전진한다.
        //     records_와 원소·술어 값은 바뀌지 않는다.
        // (6) 전체 외부 순회와 경계 탐색은 합계 O(n), 할당 없음이다. snapshot 수명과 vector iterator
        //     유효성이 전제이며 술어 예외는 전파된다. 같은 view의 최초 순회끼리는 cache 쓰기로 경쟁할 수
        //     있고, 같은 저장소에 동시 쓰기가 있어도 데이터 경쟁이므로 필요한 경우 외부 동기화한다.
        for (auto&& group : groups) {
            Department department{};
            std::size_t employee_count{};
            long long salary_sum{};
            bool is_first{true};

            // [첫 숨은 호출 계약: chunk subrange의 range-for begin/end/iterator 연산]
            // (1) 수신 group은 const Records 안의 비어 있지 않은 한 구간을 가리키는 subrange lvalue다.
            // (2) range-for는 group.begin/end, vector<Employee>::const_iterator의 ==에서 재작성된 !=,
            //     역참조와 전위 ++를 쓴다.
            // (3) 인자는 없고 역참조 결과 const Employee&를 employee가 빌린다. 소유권 이동은 없다.
            // (4) begin/end의 const_iterator와 비교 bool·const Employee&를 사용하며, ++의 iterator&는 버린다.
            // (5) 반복 iterator와 지역 누계만 바뀌며 원소와 vector는 그대로다.
            // (6) 구간 길이를 k라 하면 O(k), 할당 없음이다. iterator 유효성과 소유자 수명이 전제다.
            //     끝 iterator 역참조는 UB지만 range-for는 먼저 비교한다. 읽기끼리는 동시 사용 가능하다.
            for (const Employee& employee : group) {
                if (is_first) {
                    department = employee.department;
                    is_first = false;
                }
                ++employee_count;
                salary_sum += static_cast<long long>(employee.salary);
            }

            // [첫 호출 계약: std::vector<DepartmentSummary>::push_back(DepartmentSummary&&)]
            // (1) 수신 summaries는 유효한 Summaries lvalue이고 size()개 완성 원소를 소유한다.
            // (2) 선택 overload는 void push_back(value_type&&), value_type=DepartmentSummary다.
            // (3) 지정 초기화한 DepartmentSummary 식은 prvalue이고 새 마지막 원소로 이동/직접 구성된다.
            //     값만 전달하며 외부 소유 자원은 없다.
            // (4) 반환형은 void라 반환값을 사용하지 않는다.
            // (5) 성공하면 size가 1 증가하고 누계 사본을 마지막에 소유한다. 재할당 시 기존 iterator·참조가
            //     모두 무효화되지만 이 코드는 summaries의 참조를 보관하지 않으며 group은 다른 vector를 본다.
            // (6) amortized O(1), 재할당 시 현재 size에 선형이고 bad_alloc이면 예외가 전파된다.
            //     이 trivially movable 값에서는 강한 보장을 유지한다. 한 vector의 동시 수정은 안전하지 않다.
            summaries.push_back(DepartmentSummary{
                .department = department,
                .employee_count = employee_count,
                .salary_sum = salary_sum,
            });
        }

        // summaries는 lvalue지만 반환 문맥에서 NRVO 후보다. NRVO가 없으면 암시적 이동이 허용된다.
        return summaries;
    }

private:
    // private 저장소가 Employee의 실제 수명을 소유한다. view와 reference는 이 멤버를 빌릴 뿐이다.
    Records records_;
};

int main() {
    // [첫 생성 계약: std::vector<Employee>(std::initializer_list<Employee>)]
    // (1) 수신 seed는 생성 전이다.
    // (2) 선택 overload는 vector(initializer_list<value_type>, const Allocator&=Allocator())이고
    //     value_type=Employee, Allocator=std::allocator<Employee>다.
    // (3) initializer_list의 8개 Employee는 const 임시 배열 lvalue에서 복사된다. id와 salary는 유효한 int다.
    // (4) 생성자는 값을 반환하지 않고 8개 원소를 소유하는 Records를 완성한다.
    // (5) seed는 입력 순서를 보존하며 원소 수명은 seed 저장소에 속한다.
    // (6) O(8), 한 번의 저장소 할당이 가능하고 bad_alloc이면 이미 만든 원소를 정리해 예외를 전파한다.
    //     재할당 전 참조는 없으며 별도 객체 생성이라 스레드 간 공유 상태가 없다.
    PayrollSnapshot::Records seed{
        {101, Department::engineering, 7000},
        {102, Department::engineering, 7600},
        {103, Department::engineering, 7400},
        {201, Department::sales, 5100},
        {202, Department::sales, 5600},
        {301, Department::support, 4800},
        {302, Department::support, 5000},
        {303, Department::support, 5200},
    };

    // seed는 이름이 있으므로 lvalue이고, std::move(seed)는 xvalue다. 생성자는 값을 받아 소유권을 넘긴다.
    PayrollSnapshot snapshot{std::move(seed)};

    // summarize()가 반환한 vector prvalue가 summaries를 직접 초기화한다. 함수 안 NRVO와 별개로,
    // 반환 prvalue에서 같은 타입 결과 객체로의 마지막 단계에는 C++17 보장 복사 생략이 적용된다.
    const PayrollSnapshot::Summaries summaries = snapshot.summarize();

    // [첫 생성 계약: std::vector<DepartmentSummary> 복사 생성]
    // (1) 수신 copied_summaries는 생성 전이고 summaries는 3개 원소를 소유하는 const Summaries lvalue다.
    // (2) 선택 overload는 vector(const vector& other)이며 allocator는 select_on_container_copy_construction으로 얻는다.
    // (3) other 식 summaries의 타입은 const Summaries, 값 범주는 lvalue이고 원소 값을 복사한다. 소유권은 공유하지 않는다.
    // (4) 생성자는 반환값 없이 독립 저장소를 가진 copied_summaries를 완성한다.
    // (5) 두 vector의 값은 같지만 이후 한쪽의 구조 변경은 다른 쪽 원소·iterator에 영향을 주지 않는다.
    // (6) O(원소 수), 새 할당이 가능하고 bad_alloc 또는 원소 복사 예외가 전파된다. 실패하면 부분 결과를
    //     정리해 원본을 보존한다. 서로 다른 두 vector는 외부 동기화 없이도 각각 수정할 수 있다.
    const PayrollSnapshot::Summaries copied_summaries{summaries};

    // [첫 숨은 호출 계약: const std::vector<DepartmentSummary> range-for]
    // (1) 수신 copied_summaries는 3개 결과를 소유하는 const Summaries lvalue다.
    // (2) range-for가 vector::begin/end const overload와 const_iterator의 operator*, operator++를 쓰며,
    //     `!=` 식은 C++20 비교 재작성에 따라 operator==의 부정으로 bool을 만든다.
    // (3) 인자는 없고 summary는 각 역참조 const DepartmentSummary lvalue를 const&로 빌린다.
    // (4) begin/end의 const_iterator·비교 bool·const DepartmentSummary&를 순회에 사용하고,
    //     전위 ++가 반환하는 const_iterator&는 range-for가 버린다.
    // (5) loop iterator만 이동하며 copied_summaries와 원소는 수정되지 않는다.
    // (6) O(group 수), 할당·예외 없음이다. 순회 중 vector 구조 변경이 없어야 하며 읽기 전용 동시 순회는 안전하다.
    for (const DepartmentSummary& summary : copied_summaries) {
        // [첫 호출 계약: std::ostream 삽입 연산자 체인]
        // (1) 수신 std::cout은 프로그램 시작 때 구성된 std::ostream lvalue이며 출력 가능 상태여야 한다.
        // (2) 선택 overload는 비멤버 operator<<(ostream&, string_view/char), 이 Windows w64devkit에서
        //     basic_ostream::operator<<(unsigned long long)인 size_t overload, operator<<(long long)다.
        // (3) department_name 결과와 문자는 prvalue다. summary.employee_count/salary_sum은 const lvalue이고
        //     정수 멤버 overload에 전달될 때 lvalue-to-rvalue 변환으로 값을 읽는다. 소유권 이동은 없다.
        // (4) 각 연산은 std::ostream&를 반환한다. 중간 반환은 다음 << 수신으로 쓰고 마지막 '\n' 결과는 버린다.
        // (5) 문자가 순서대로 출력 버퍼에 기록되고 실패 시 failbit/badbit가 설정될 수 있다. 인자는 변하지 않는다.
        // (6) 표준은 공통 점근 복잡도나 내부 buffer 할당 여부를 정하지 않는다. exceptions mask에 해당하면
        //     ios_base::failure가 날 수 있다. 표준 stream 동기화를 유지한 동시 출력도 데이터 경쟁은 피하지만
        //     한 레코드로 원자화되지는 않아 문자가 섞일 수 있고, 동기화를 끈다면 외부 동기화가 필요하다.
        std::cout << department_name(summary.department) << ':' << summary.employee_count << ':'
                  << summary.salary_sum << '\n';
    }
}
