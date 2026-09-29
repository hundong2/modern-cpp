// <cstddef>는 원소 개수 타입 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 결과를 출력할 std::cout을 선언한다.
#include <iostream>
// <ranges>는 C++23 인접 그룹 view std::views::chunk_by를 선언한다.
#include <ranges>
// <utility>는 소유권 이동 의도를 나타내는 std::move를 선언한다.
#include <utility>
// <vector>는 연속 소유 컨테이너 std::vector를 선언한다.
#include <vector>

// 직접 해보기: 연속한 같은 상태를 하나의 장애 구간으로 압축한다.
// struct는 멤버가 기본 public이라 값 객체·입출력 DTO에 간결하다.
struct HealthSample {
    int minute{};
    bool healthy{};
};

struct HealthRun {
    bool healthy{};
    int first_minute{};
    int last_minute{};
    std::size_t sample_count{};
};

class HealthTimeline {
public:
    // template 인자 HealthSample/HealthRun이 각 vector의 원소 타입을 결정한다.
    using Samples = std::vector<HealthSample>;
    using Runs = std::vector<HealthRun>;

    // explicit은 Samples에서 HealthTimeline으로의 암시 변환을 막는다.
    // [첫 호출 계약: std::move(samples) 및 vector 이동 생성]
    // (1) 수신 samples_는 생성 전이고 값 매개변수 samples는 유효한 Samples lvalue다.
    // (2) move는 template<class T> remove_reference_t<T>&& move(T&&) noexcept에서 T=Samples&이고,
    //     이어 vector(vector&&)가 samples_의 멤버 초기화 목록에서 선택된다.
    // (3) samples는 lvalue, move 결과는 Samples&& xvalue이며 저장소 소유권 이전을 허용한다.
    // (4) move의 반환 reference는 vector 생성에 쓰이고 생성자는 samples_ 객체를 완성한다.
    // (5) samples_가 원소를 소유하고 매개변수는 유효하지만 값이 미지정인 이동 후 상태다.
    // (6) 기본 allocator에서는 O(1)·noexcept·추가 할당 없음이다. 기존 원소 참조는 새 저장소를
    //     가리킬 수 있으나 이 코드는 보관하지 않는다. 별도 객체의 생성은 공유 상태를 만들지 않는다.
    explicit HealthTimeline(Samples samples) noexcept
        : samples_{std::move(samples)} {}

    [[nodiscard]] Runs compress() const & {
        const auto same_health = [](const HealthSample& left, const HealthSample& right) noexcept {
            return left.healthy == right.healthy;
        };

        // [첫 호출 계약: std::views::chunk_by(samples_, same_health)]
        // (1) 기반 수신은 const Samples lvalue samples_이고 모든 원소와 저장소가 살아 있다.
        // (2) adaptor 객체의 구체 operator() 시그니처는 표준화되지 않는다. 표준의 두 인자 식
        //     views::chunk_by(E,F)는 chunk_by_view(E,F)와 expression-equivalent하며, deduction 결과는
        //     V=ref_view<const Samples>, Pred=decay_t<decltype(same_health)>다.
        // (3) samples_는 빌린 lvalue, 술어는 복사 가능한 lvalue다. 술어는 인접 const reference 두 개를 받는다.
        // (4) 비소유 지연 view prvalue를 반환해 이름 있는 groups가 보관한다.
        // (5) 입력은 변하지 않고 view가 기반 참조와 술어 사본을 보관하며 아직 원소를 읽지 않는다.
        // (6) 생성 O(1), 할당 없음이고 순회 전체는 O(n)이다. owner 수명·iterator 유효성이 전제다.
        //     술어 예외는 전파되며 이 noexcept 술어는 던지지 않는다. 동시 쓰기는 데이터 경쟁이다.
        auto groups = std::views::chunk_by(samples_, same_health);

        // [첫 생성 계약: std::vector<HealthRun>()]
        // (1) 수신 answer는 생성 전이다.
        // (2) 선택 overload는 vector()이고 T=HealthRun, Allocator=std::allocator<HealthRun>다.
        // (3) 인자 없이 기본 allocator를 사용하며 외부 객체를 빌리지 않는다.
        // (4) 반환값 없이 빈 Runs를 완성한다.
        // (5) size는 0이고 소유 원소가 없다.
        // (6) O(1), 보통 할당 없음·noexcept다. iterator 무효화 대상이 없고 소멸 시 자기 저장소만 정리한다.
        Runs answer{};

        // [첫 숨은 호출 계약: chunk_by_view와 subrange의 중첩 range-for]
        // (1) groups는 살아 있는 const Samples를 빌린 view lvalue이고 각 group은 비어 있지 않은 subrange다.
        // (2) vector 기반이라 두 range의 begin/end는 각각 같은 종류 iterator 쌍을 돌려준다. range-for의
        //     `!=`는 operator==에서 재작성되고 역참조·전위 증가 연산도 선택된다.
        // (3) 명시 인자는 없고 auto&& group은 subrange prvalue, sample은 원소 const lvalue를 빌린다.
        // (4) iterator·bool·subrange·const HealthSample&는 반복에 쓰이고 각 전위 ++의 iterator&는 버린다.
        // (5) 첫 begin은 groups 내부 첫 경계 cache를 채울 수 있고 반복 iterator와 지역 누계가 변한다.
        //     samples_의 원소와 술어 값은 불변이다.
        // (6) 모든 원소에 합계 O(n), 할당 없음이다. owner 파괴·vector 구조 변경 뒤 사용은 UB이고,
        //     같은 view의 최초 순회끼리는 cache 쓰기로 경쟁할 수 있고, 같은 저장소의 동시 쓰기도 데이터
        //     경쟁이므로 공유할 때 외부 동기화가 필요하다.
        for (auto&& group : groups) {
            bool state{};
            int first{};
            int last{};
            std::size_t count{};
            bool is_first{true};

            for (const HealthSample& sample : group) {
                if (is_first) {
                    state = sample.healthy;
                    first = sample.minute;
                    is_first = false;
                }
                last = sample.minute;
                ++count;
            }

            // [첫 호출 계약: vector<HealthRun>::push_back(HealthRun&&)]
            // (1) 수신 answer는 유효한 Runs lvalue이고 지금까지의 구간을 소유한다.
            // (2) 선택 overload는 void push_back(value_type&&), value_type=HealthRun이다.
            // (3) HealthRun 지정 초기화 식은 prvalue이며 값들이 새 마지막 원소로 전달된다.
            // (4) void라 반환값은 없다.
            // (5) 성공 시 size가 1 늘고, 재할당이면 answer의 모든 기존 iterator/reference가 무효화된다.
            // (6) amortized O(1), 재할당은 O(size)·할당 가능, bad_alloc 전파다. 이 값 타입에서는
            //     강한 예외 보장을 유지하며 한 answer에 대한 동시 수정은 안전하지 않다.
            answer.push_back(HealthRun{
                .healthy = state,
                .first_minute = first,
                .last_minute = last,
                .sample_count = count,
            });
        }

        // 이름 있는 answer는 NRVO 후보이며, 적용되지 않아도 반환 문맥의 암시적 이동이 가능하다.
        return answer;
    }

    // 결과는 소유 vector지만 구현 중 view 안전성 규칙을 단순하게 유지하려고 임시 수신 호출을 막는다.
    Runs compress() const && = delete;

private:
    // class의 private 멤버가 실제 원소 수명을 소유한다. const&는 이 소유자를 빌리는 참조다.
    Samples samples_;
};

int main() {
    // [첫 생성 계약: vector<HealthSample>(initializer_list)]
    // (1) 수신 samples는 생성 전이다.
    // (2) vector(initializer_list<value_type>, allocator)에서 value_type=HealthSample이다.
    // (3) 7개 aggregate 값은 임시 const 배열에서 복사되며 minute는 증가 순서다.
    // (4) 반환값 없이 7개 원소를 소유하는 Samples를 완성한다.
    // (5) 입력 순서와 값이 저장되고 원소 수명은 vector가 관리한다.
    // (6) O(7), 저장소 할당 시 bad_alloc이 가능하고 실패 시 부분 원소를 정리한다. 기존 참조는 없다.
    HealthTimeline::Samples samples{
        {0, true}, {1, true}, {2, false}, {3, false}, {4, false}, {5, true}, {6, true},
    };

    HealthTimeline timeline{std::move(samples)};
    const HealthTimeline::Runs runs = timeline.compress();

    // [첫 숨은 호출 계약: const vector<HealthRun> range-for]
    // (1) 수신 runs는 3개 압축 결과를 소유하는 const Runs lvalue다.
    // (2) vector::begin/end const와 const_iterator의 역참조·증가가 선택되고, range-for의 `!=` 식은
    //     C++20 비교 재작성에 따라 operator==의 부정으로 bool을 만든다.
    // (3) 인자 없이 run이 각 const HealthRun lvalue를 const reference로 빌린다.
    // (4) begin/end iterator·비교 bool·const reference를 반복 제어와 본문에서 사용하고,
    //     전위 ++가 반환하는 iterator&는 range-for가 버린다.
    // (5) iterator만 이동하고 vector와 원소는 그대로다.
    // (6) O(구간 수), 할당·예외 없음이다. 순회 중 구조 변경 금지, 읽기끼리 동시 사용 가능하다.
    for (const HealthRun& run : runs) {
        // [첫 호출 계약: std::cout 스트림 삽입 체인]
        // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 출력 상태를 가진다.
        // (2) basic_ostream::operator<<(bool)이 boolalpha가 꺼진 상태에서 0/1을 쓰고,
        //     이어 int, 이 Windows w64devkit의 size_t인 unsigned long long, 비멤버 char overload가 선택된다.
        // (3) run.healthy/first_minute/last_minute/sample_count는 const lvalue이고 by-value overload에
        //     전달될 때 lvalue-to-rvalue 변환으로 읽힌다. 구두점 문자는 prvalue이며 소유권 이동은 없다.
        // (4) 각 <<는 std::ostream&를 반환한다. 중간 반환은 다음 삽입에 쓰고 마지막 '\n' 결과는 버린다.
        // (5) 문자가 순서대로 버퍼에 기록되고 실패 시 상태 비트가 바뀔 수 있다.
        // (6) 표준은 공통 점근 복잡도나 내부 buffer 할당 여부를 정하지 않는다. 설정된 exceptions mask면
        //     ios_base::failure가 날 수 있다. 표준 stream 동기화를 유지한 동시 출력도 한 줄로 원자화되지는
        //     않아 문자가 섞일 수 있고, 동기화를 끈다면 같은 stream 접근에 외부 동기화가 필요하다.
        std::cout << run.healthy << ':' << run.first_minute << '-' << run.last_minute << ':'
                  << run.sample_count << '\n';
    }
}
