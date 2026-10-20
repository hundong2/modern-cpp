// 직접 연습: 배치 job ID를 등록하는 서비스가 queue 설정을 값으로 소유한다. 같은 배포 요청이
// 반복되어도 최초 JobSpec을 유지하도록 std::unordered_map::try_emplace를 사용한다.

// <cstddef>는 컨테이너 원소 수를 표현할 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 출력 객체 std::cout과 operator<< 형식 출력을 선언한다.
#include <iostream>
// <string>은 job ID와 queue 이름을 소유하는 std::string을 선언한다.
#include <string>
// <unordered_map>은 해시 기반 고유 key registry인 std::unordered_map을 선언한다.
#include <unordered_map>
// <utility>는 lvalue를 xvalue 식으로 바꾸는 std::move를 선언한다.
#include <utility>

// 별칭은 새 강한 타입이 아니며 int와 같은 연산·변환 규칙을 갖는다.
using RetryLimit = int;

// struct의 기본 접근은 public이다. JobSpec은 queue 문자열과 재시도 횟수를 한 값으로 묶어 소유한다.
struct JobSpec {
    std::string queue_name;
    RetryLimit retry_limit{}; // {}는 기본 int를 0으로 값 초기화하고 narrowing을 막는 데 도움을 준다.

    // explicit은 braced 두 값이 매개변수의 JobSpec으로 묵시 변환되는 경로를 닫는다.
    // 멤버 초기화 목록은 생성자 본문보다 먼저, 실제 멤버 선언 순서대로 두 멤버를 만든다.
    explicit JobSpec(std::string queue_value, RetryLimit retry_value) noexcept
        // [첫 호출 계약: std::move와 std::string 이동 생성]
        // (1) 자유 함수 move에는 수신자가 없다. queue_value는 문자를 소유한 std::string lvalue이고
        //     queue_name은 아직 생성되지 않은 목적 멤버다.
        // (2) move<string&>(string&) -> string&&와 basic_string(basic_string&&) noexcept를 선택한다.
        // (3) queue_value lvalue가 move에 들어가고 반환 xvalue가 queue_name 생성에 쓰인다. 소유권을
        //     넘길 수 있다는 표시이며 retry_value는 RetryLimit(int) 값으로 복사된다.
        // (4) move 반환 참조는 즉시 소비하고 저장하지 않는다. string 생성자와 JobSpec 생성자는 별도
        //     반환값이 없으며 완성된 멤버가 queue 문자를 소유한다.
        // (5) queue_name은 이전 문자를 가지며 queue_value는 유효하지만 내용 미지정이다. move 자체는
        //     상태를 바꾸지 않고 선택된 string 이동 생성이 소유 상태를 바꾼다.
        // (6) move와 기본 allocator string 이동은 상수 시간·무할당·noexcept다. 참조 수명을 연장하지
        //     않고, 저장한 문자 관찰자는 없다. JobSpec 파괴 시 문자열도 파괴되며 공유 변경은 동기화한다.
        : queue_name{std::move(queue_value)}, retry_limit{retry_value} {}
};

// class의 기본 접근은 private이다. public 등록 API를 통하지 않고 내부 map 불변식을 바꿀 수 없다.
class JobRegistry {
public:
    JobRegistry()
        // [첫 생성 계약: std::unordered_map 기본 생성]
        // (1) 목적 jobs_ 멤버는 아직 수명이 없고 외부 job 객체나 bucket을 빌리지 않는다.
        // (2) unordered_map()을 Key=std::string, T=JobSpec 및 기본 Hash, KeyEqual, Allocator로 선택한다.
        // (3) 명시 인자는 없고 기본 정책 객체들이 map 안에 값으로 구성된다.
        // (4) 생성자는 반환값 없이 size()==0인 유효한 Storage owner를 만든다.
        // (5) 성공 뒤 jobs_는 비어 있고 JobSpec은 없으며 bucket_count는 미지정이다.
        // (6) 상수 시간이다. 이 정확한 std::hash<std::string>, std::equal_to<std::string>,
        //     std::allocator 정책 조합의 기본 생성에서는 예외가 전파되지 않는다. 구현의 내부 저장 방식과
        //     할당 횟수는 단정하지 않는다. iterator 무효화·공유 접근은 아직 없고 JobRegistry 파괴가
        //     모든 node 수명을 끝낸다.
        : jobs_{} {}

    // 값 매개변수는 job_id와 queue_value를 함수가 독립적으로 소유한다. const std::string&는 기존
    // 문자열을 빌리는 lvalue 참조라 null일 수 없지만 owner보다 오래 살 수 없고, const std::string*
    // 관찰 pointer라면 nullptr 가능성과 재지정 가능성이 있다. 이 API는 어느 관찰자도 저장하지 않는다.
    [[nodiscard]] bool register_job(std::string job_id,
                                    std::string queue_value,
                                    RetryLimit retry_value) {
        // [첫 호출 계약: std::unordered_map::try_emplace]
        // (1) 수신 jobs_는 유효한 Storage lvalue이며 지금까지 성공 등록된 고유 job node를 소유한다.
        // (2) pair<iterator,bool> try_emplace(key_type&&, Args&&...)의 rvalue key overload를 선택한다.
        //     queue_value는 xvalue라 std::string, 이름 있는 retry_value는 lvalue라 int&로 추론되어
        //     Args={std::string, int&}이고, value_type은 pair<const string,JobSpec>이다.
        // (3) std::move(job_id/queue_value)는 string lvalue를 xvalue로 표시하고 retry_value는 int
        //     lvalue 값이다. forwarding reference는 잠시 빌리고 삽입 성공 때 map node가 값을 소유한다.
        // (4) 반환 pair prvalue의 iterator는 새/기존 node, bool은 삽입 여부다. insertion이 결과를
        //     직접 초기화해 pair 복사·이동은 없고, iterator는 역참조하거나 호출 뒤 보관하지 않는다.
        // (5) 새 key면 size가 1 늘고 두 string은 이동 후 상태일 수 있다. 중복이면 기존 JobSpec,
        //     map 크기와 두 인자가 그대로라 최초 queue/retry 정책을 덮어쓰지 않는다.
        // (6) 평균 O(1), 충돌 최악 O(n)이다. node/bucket 할당과 재해시는 iterator를 무효화하되
        //     원소 pointer/reference는 유지한다. 할당·동등 비교·key/mapped 구성 등 사용자 Hash 이외
        //     예외에는 container 삽입 효과가 없다. 사용자 Hash가 던지면 더 강한 no-effect 보장을 가정하지
        //     않고, 어느 예외도 이미 시작된 호출자 인자의 이동을 원래 값으로 복원하지 않는다. erase/owner
        //     파괴 뒤 관찰자는 dangling이며 같은 map의 병행 접근은 외부 동기화가 필요하다.
        // 모든 인자 식은 호출 전 평가된다. 중복 key에서 mapped 구성과 인자로부터의 이동만 생략되며,
        // 비싼 queue 계산 식 자체를 지연하려면 별도의 find/factory 설계가 필요하다.
        const auto insertion{jobs_.try_emplace(std::move(job_id),
                                               std::move(queue_value),
                                               retry_value)};

        // if는 insertion.second를 읽어 등록 성공/중복 두 제어 경로 중 하나만 실행한다.
        if (insertion.second) {
            return true;
        }
        return false;
    }

    // const 참조 매개변수는 호출자 문자열을 함수 동안만 빌리고 저장하지 않는 읽기 전용 관찰 API다.
    [[nodiscard]] bool matches_job(const std::string& job_id,
                                   const std::string& expected_queue,
                                   RetryLimit expected_retry) const {
        // [첫 호출 계약: unordered_map::find/end, const_iterator 비교·operator->, string operator==]
        // (1) 수신 jobs_의 정확한 타입은 원소 수명이 유지되는 const Storage lvalue다. 두 문자열 인자는
        //     살아 있는 const std::string lvalue이고 같은 map에 병행 쓰기는 없다.
        // (2) const_iterator find(const key_type&) const, const_iterator end() const noexcept,
        //     const_iterator의 operator==와 operator->, bool operator==(const string&,const string&)
        //     overload를 사용한다.
        // (3) job_id와 expected_queue는 비소유 const lvalue 참조, expected_retry는 int 값이다. find는
        //     key를 읽어 빌리고 position lvalue와 end()의 iterator prvalue는 같은 map 소속이다.
        // (4) find의 const_iterator prvalue가 position을 직접 초기화한다. end는 끝 sentinel, iterator
        //     비교는 bool, operator->는 const 원소 pointer, 문자열 비교는 내용 동일 여부 bool을 반환하며
        //     모두 아래 존재·값 검증에 사용한다.
        // (5) 호출 뒤 map, bucket, key, JobSpec과 인자는 변하지 않고 어떤 관찰자도 무효화되지 않는다.
        // (6) find는 평균 O(1)·충돌 최악 O(n), end/iterator 연산은 O(1), 문자열 비교는 최악 O(문자 수)다.
        //     무할당이고 누락 key는 예외 대신 end다. find는 noexcept 선언이 아니며 정책 예외를 전파할 수
        //     있으나 이 Storage에는 사용자 정책 상태가 없다. iterator/pointer는 map 변경·파괴 뒤 dangling이고,
        //     end 역참조는 UB지만 if가 배제한다. 동시 읽기는 가능하되 쓰기와 겹치면 외부 동기화가 필요하다.
        const auto position{jobs_.find(job_id)};
        // if의 실패 경로는 즉시 반환하므로 끝 iterator를 operator->로 역참조하지 않는다.
        if (position == jobs_.end()) {
            return false;
        }
        const JobSpec& job{position->second};
        return job.queue_name == expected_queue && job.retry_limit == expected_retry;
    }

    // const 멤버 함수는 jobs_를 관찰만 하며 반환 size_t는 registry와 독립된 기본 정수 값이다.
    [[nodiscard]] std::size_t size() const noexcept {
        // [첫 호출 계약: unordered_map::size]
        // (1) 수신 jobs_는 살아 있는 const 관찰 가능 Storage lvalue이며 유효한 node를 소유한다.
        // (2) size_type size() const noexcept를 선택하고 size_type은 std::size_t다.
        // (3) 명시 인자, 참조 저장이나 소유권 이동은 없다.
        // (4) 현재 job 수를 std::size_t prvalue로 반환해 count를 직접 초기화한다.
        // (5) map과 모든 key/mapped 값, bucket과 관찰자는 그대로다.
        // (6) O(1), 무할당·noexcept·무효화 없음이다. 값은 map 파괴 뒤에도 독립적이지만 이후 등록 뒤
        //     최신 크기를 자동 반영하지 않는다. 병행 변경과 겹치지 않도록 호출자가 동기화한다.
        return jobs_.size();
    }

private:
    // template 인자 std::string은 key, JobSpec은 mapped 값이며 map이 둘의 수명을 소유한다.
    using Storage = std::unordered_map<std::string, JobSpec>;
    Storage jobs_;
};

// 임시 JobRegistry prvalue를 직접 반환하므로 호출자의 결과 객체에 바로 생성된다. C++17 보장 복사
// 생략이며 흔히 RVO라고 부른다. `JobRegistry local; return local;`의 선택적 NRVO와 구별한다.
[[nodiscard]] JobRegistry make_job_registry() {
    return JobRegistry{};
}

int main() {
    // 결과 prvalue가 registry를 직접 초기화해 사용자 정의 복사/이동은 실행되지 않는다.
    // 이름 있는 registry는 lvalue이고 main이 끝날 때까지 map과 모든 job을 소유한다.
    JobRegistry registry{make_job_registry()};

    // [첫 생성 계약: std::string의 C 문자열 생성자]
    // (1) 각 목적 string은 아직 없고 literal 배열은 정적 저장 기간 동안 유효하다.
    // (2) basic_string(const char*, const Allocator& = Allocator())를 char/default allocator로 선택한다.
    // (3) const char[N] lvalue 배열이 non-null, '\0' 종단 const char*로 변환된다. string은 문자를
    //     복사해 소유하며 literal pointer나 수명을 빌려 저장하지 않는다.
    // (4) 생성자는 반환값이 없다. string prvalue는 register_job의 값 매개변수를 직접 초기화하거나
    //     matches_job의 const 참조에 그 전체 식 동안 바인딩된다.
    // (5) 완성된 값은 literal과 독립이다. 한 호출의 인자 식은 모두 먼저 평가되지만 인자들 사이
    //     평가 순서에는 의존하지 않는다.
    // (6) 시간·공간 O(문자 수), allocation과 length_error/bad_alloc 가능이다. null/비종단 pointer는
    //     전제 위반이다. 문자 관찰자는 string 변경·파괴 규칙에 묶이지만 여기서는 저장하지 않는다.
    const bool first_insert{registry.register_job(
        std::string{"nightly-backup"}, std::string{"batch"}, 3)};
    const bool duplicate_insert{registry.register_job(
        std::string{"nightly-backup"}, std::string{"urgent"}, 9)};
    const bool second_insert{registry.register_job(
        std::string{"thumbnail-refresh"}, std::string{"media"}, 2)};

    const std::size_t count{registry.size()};
    const bool first_value_preserved{registry.matches_job(
        std::string{"nightly-backup"}, std::string{"batch"}, 3)};
    if (!second_insert || !first_value_preserved) {
        return 1;
    }

    // [첫 호출 계약: std::cout의 operator<< 연쇄]
    // (1) 왼쪽 수신 std::cout은 유효한 std::ostream lvalue이며 단일 main thread가 사용한다.
    // (2) bool, ABI에 대응하는 size_t의 unsigned 정수, char 삽입 overload가 차례로 선택된다.
    // (3) bool 두 개와 count는 lvalue 값 입력, ' '와 '\n'은 char prvalue이며 소유권 전달은 없다.
    // (4) 각 호출은 ostream&를 반환해 다음 삽입의 수신으로 쓰고 마지막 참조는 의도적으로 버린다.
    // (5) 기본 bool 형식으로 정확히 "1 0 2\n"을 기록하며 registry 상태는 변하지 않는다.
    // (6) 이식 가능한 단일 점근 복잡도·무할당 보장은 없고 locale/buffer/device 비용이 더해진다.
    //     실패는 stream 상태 bit와 설정된 예외로 드러난다. 관찰자 무효화는 없지만 동시 레코드
    //     원자성도 없다.
    std::cout << first_insert << ' ' << duplicate_insert << ' ' << count << '\n';

    // 구현은 job ID 문자 load와 hash, bucket pointer 추적, equality 비교·조건 분기, 성공 경로의
    // allocation과 node link store를 수행할 수 있다. 가상 함수가 없어 가상 간접 호출은 요구되지
    // 않지만 allocator와 라이브러리 내부 호출 모양까지 고정되지는 않는다. CPU·ABI·compiler·표준
    // 라이브러리·최적화 옵션에 따라 명령 선택, 인라인, cache·분기 비용이 달라진다.
    return 0;
}
