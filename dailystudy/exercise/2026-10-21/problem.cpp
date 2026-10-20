// 직접 해보기: 작업 완료 통지를 처리하는 agent를 factory로 공동 소유하고, scheduler에 넘길 callback에는
// agent의 weak handle만 넣는다. callback 실행 중 lock 성공 시에만 강한 owner를 잠깐 확보한다.

// <functional>은 bool(int)처럼 정해진 signature의 callable을 소유하는 std::function을 선언한다.
#include <functional>
// <iostream>은 한 줄 검증 결과를 쓰는 std::cout과 stream 삽입 연산자를 선언한다.
#include <iostream>
// <memory>는 공동/약한 소유권과 std::enable_shared_from_this, std::make_shared를 선언한다.
#include <memory>

// using 별칭은 새 타입이 아니다. std::function의 template 인자 bool(int)는 int 하나를 받아 bool을
// 돌려준다는 호출 규약이고, 이 값 객체가 내부 callable의 수명을 소유한다.
using CompletionCallback = std::function<bool(int)>;

// class의 기본 접근은 private, struct의 기본 접근은 public이다. 상태 불변식을 감추려고 class를 쓰며
// public 상속한 enable_shared_from_this<TaskAgent>가 최초 shared owner의 control block을 관찰한다.
class TaskAgent final : public std::enable_shared_from_this<TaskAgent> {
private:
    // 타입 이름을 숨기는 것만으로는 `TaskAgent{{}, 1}`을 막지 못한다. 기본 생성자까지 private으로 두고
    // enclosing TaskAgent만 key를 만들 수 있게 friend로 선언한다.
    class FactoryKey final {
        friend class TaskAgent;

        FactoryKey() noexcept = default;

    public:
        // make_shared가 이미 만들어진 key를 값 매개변수로 전달할 수 있게 복사·이동 생성만 공개한다.
        // 외부 호출자는 최초 key를 만들 수 없으므로 이 공개 특수 멤버로 생성 권한을 얻지 못한다.
        FactoryKey(const FactoryKey&) noexcept = default;
        FactoryKey(FactoryKey&&) noexcept = default;
    };

public:
    // explicit 생성자는 의도하지 않은 copy-list 변환을 막는다. 반환형은 없고 두 값 매개변수를 받으며,
    // 멤버 초기화 목록은 함수 본문 전에 base와 멤버 수명을 선언 순서대로 시작한다.
    explicit TaskAgent(FactoryKey, int accepted_job) noexcept
        // [첫 생성 계약: std::enable_shared_from_this<TaskAgent> 기본 생성]
        // (1) 목적 base subobject는 아직 없고 어떤 shared control block과도 연결되지 않았다.
        // (2) protected constexpr enable_shared_from_this() noexcept를 T=TaskAgent로 선택한다.
        // (3) 명시 인자가 없으며 pointer/reference나 외부 owner를 저장하지 않는다.
        // (4) 생성자는 반환값 없이 비어 있는 weak 관찰 상태의 base 수명을 시작한다.
        // (5) 생성 직후 base만으로는 owner가 늘지 않는다. make_shared가 반환 owner를 만들면서 이 접근
        //     가능한 기반 부분을 같은 control block과 연결한다.
        // (6) O(1), 무할당·noexcept·무효화 없음이다. 완성 전 self 관찰을 하지 않고, base는 파생
        //     객체와 함께 파괴된다. 동일 객체의 생성/파괴를 동시에 수행하지 않는다.
        : std::enable_shared_from_this<TaskAgent>{}, accepted_job_{accepted_job} {}

    // enable_shared_from_this 기반을 가진 객체 자체를 복사·이동하면 새 객체가 기존 shared control block에
    // 연결되지 않은 채 생길 수 있다. 공개 factory를 유일한 생성 경로로 유지하도록 네 특수 멤버를
    // 명시적으로 삭제하고, shared_ptr handle만 복사·이동하게 한다.
    TaskAgent(const TaskAgent&) = delete;
    TaskAgent& operator=(const TaskAgent&) = delete;
    TaskAgent(TaskAgent&&) = delete;
    TaskAgent& operator=(TaskAgent&&) = delete;

    // 반환 shared_ptr의 template 인자 TaskAgent는 소유 대상 타입이다. static이라 this 수신자는 없다.
    [[nodiscard]] static std::shared_ptr<TaskAgent> create(int accepted_job) {
        // [첫 호출 계약: std::make_shared<TaskAgent>(FactoryKey{}, accepted_job)]
        // (1) 자유 함수라 수신 객체가 없고 TaskAgent/control block은 아직 생성되지 않았다.
        // (2) shared_ptr<TaskAgent> make_shared<TaskAgent, FactoryKey, int&>(FactoryKey&&, int&)를 선택한다.
        // (3) FactoryKey{}는 비소유 prvalue, accepted_job은 이름 있는 int lvalue다. forwarding reference는
        //     값 범주를 보존해 생성자에 전달하며 모든 int 값이 허용된다.
        // (4) 새 객체를 소유하는 shared_ptr<TaskAgent> prvalue를 반환해 caller의 owner를 만든다.
        // (5) 성공 뒤 TaskAgent와 control block 수명이 시작되고 기반 self observer가 연결된다. 두 인자는
        //     자원 owner가 아니며 호출 뒤 값 변화가 없다.
        // (6) 고정 크기 객체 구성은 O(1)이고 동적 저장소가 필요하다. 객체/control block 공동 배치 같은
        //     세부는 구현에 따르며 할당 또는 생성 실패 예외 시 완성 owner가 없다. 반환된 서로 다른
        //     smart handle의 control-block 연산은 thread-safe하지만 한 handle 객체의 동시 변경은 금지다.
        // 이 반환 shared_ptr prvalue가 호출자 결과를 직접 구성한다. C++17 보장 복사 생략이며 이름 있는
        // local을 return할 때 허용될 뿐인 NRVO와 다르다.
        return std::make_shared<TaskAgent>(FactoryKey{}, accepted_job);
    }

    // 함수 반환형은 CompletionCallback 값이다. callback은 agent owner가 아니라 weak observer를 소유한다.
    [[nodiscard]] CompletionCallback completion_callback() {
        // [첫 호출 계약: TaskAgent::weak_from_this]
        // (1) 수신 *this는 factory가 shared_ptr로 소유 중인 살아 있는 TaskAgent lvalue다.
        // (2) std::weak_ptr<TaskAgent> weak_from_this() noexcept 비-const overload를 선택한다.
        // (3) 인자는 없고 this pointer는 호출 동안만 빌린다. factory 생성 뒤라는 불변식이 허용 상태다.
        // (4) 같은 control block을 관찰하는 weak_ptr<TaskAgent> prvalue를 반환해 observer를 초기화한다.
        // (5) strong count/TaskAgent 값은 그대로이고 weak bookkeeping만 증가할 수 있다. 아직 연결되지 않은
        //     객체라면 empty 결과지만 factory key가 정상 경로에서 그 상태를 배제한다.
        // (6) O(1), 무할당·noexcept·무효화 없음이다. weak 결과는 대상 파괴 뒤에도 안전하게 만료를
        //     표현한다. 서로 다른 handle control-block 연산은 thread-safe이고 같은 handle 변경은 동기화한다.
        const std::weak_ptr<TaskAgent> observer{weak_from_this()};

        // [첫 생성 계약: lambda의 std::weak_ptr<TaskAgent> copy-capture]
        // (1) observer는 현재 non-empty인 유효한 const weak_ptr lvalue이고 closure 멤버는 아직 없다.
        // (2) weak_ptr(const weak_ptr&) noexcept를 T=TaskAgent로 선택한다.
        // (3) observer lvalue를 읽어 weak 관찰 권한만 복사하며 strong 소유권은 전달하지 않는다.
        // (4) 생성자는 반환값 없이 closure 내부에 독립 weak handle을 만든다.
        // (5) 원본과 복사본이 같은 control block을 관찰하고 agent 수명/strong count는 늘지 않는다.
        // (6) O(1), 무할당·noexcept·무효화 없음이다. control block은 모든 weak/shared handle 종료까지
        //     남으며 각 handle 객체의 병행 변경은 피한다.
        // [이 식에서의 첫 생성 계약: closure 이동에 따른 std::weak_ptr<TaskAgent> 이동 생성]
        // (1) function에 전달될 임시 closure는 weak handle 멤버를 가지며 저장 target은 아직 없다.
        // (2) closure의 암시적 move constructor가 weak_ptr(weak_ptr&&) noexcept를 호출한다.
        // (3) 임시 closure는 xvalue로 전달되고 weak 관찰 handle의 상태를 target 멤버로 넘긴다.
        // (4) weak_ptr 생성자는 반환값 없이 target 안의 weak 멤버 수명을 시작한다.
        // (5) target이 같은 control block을 관찰하고 source weak_ptr은 유효하지만 empty가 된다.
        // (6) O(1), 무할당·noexcept·무효화 없음이고 strong count는 불변이다. source는 식 끝에
        //     안전하게 파괴되며 target handle의 병행 변경은 동기화해야 한다.
        // [첫 생성 계약: std::function<bool(int)>의 callable 대상 생성]
        // (1) 생성자는 기존 수신 객체에 대한 멤버 호출이 아니다. 목적 CompletionCallback은 아직 없고,
        //     source observer는 살아 있는 non-empty const weak_ptr<TaskAgent> lvalue다.
        // (2) template<class F> function(F&&)에서 F는 아래 이름 없는 closure 타입으로 추론되고,
        //     완성 타입/호출 signature의 template 인자는 정확히 std::function<bool(int)>다.
        // (3) 유일한 명시 인자 식 `[observer](int completed_job) noexcept { ... }`는 closure prvalue라
        //     F&&에 결합된다. int literal 입력은 이 생성 호출에는 없고 callback 호출 때 따로 전달된다.
        //     capture가 weak handle을 복사하고 function은 closure 값을 소유하지만 strong owner는 얻지 않는다.
        // (4) 생성자 반환형/반환값은 없다. 완성된 non-empty function 객체가 CompletionCallback 반환
        //     prvalue의 결과 객체가 되어 호출자 callback을 직접 초기화한다.
        // (5) source observer는 copy-capture 뒤 그대로이고 function target이 독립 weak handle 수명을 맡는다.
        //     임시 closure는 full-expression 끝에 끝나며 TaskAgent의 strong count는 변하지 않는다.
        // (6) target은 bool(int)로 호출 가능하고 CopyConstructible이어야 한다. 오늘 고정 크기 target 처리
        //     시간은 O(1)이며 small-object 최적화 여부에 따라 동적 할당/실패 예외가 가능하다. 예외 시
        //     완성 function은 없고 임시가 정리된다. 외부 iterator 무효화는 없으며 callback 객체의 병행
        //     변경과 target이 공유하는 상태 접근에는 동기화가 필요하다.
        return CompletionCallback{
            [observer](int completed_job) noexcept {
                // [첫 호출 계약: std::weak_ptr<TaskAgent>::lock]
                // (1) 수신 observer는 closure가 소유한 유효한 const weak_ptr lvalue이며 대상은 생존 또는
                //     expired 상태다.
                // (2) std::shared_ptr<TaskAgent> lock() const noexcept를 선택한다.
                // (3) 인자·선행 소유권 이동은 없고 control block의 strong count 증가를 시도한다.
                // (4) 생존 시 공동 owner, 만료 시 empty shared_ptr prvalue를 반환해 pinned를 직접 만든다.
                // (5) 성공하면 호출 동안 strong count가 하나 늘고 pinned 소멸 때 줄며, 실패하면 agent에
                //     접근하지 않는다. observer 자체는 두 경로 모두 그대로다.
                // (6) O(1), 무할당·noexcept이며 생존 판정과 strong count 확보는 원자적이다. iterator
                //     무효화는 없다. 별도 shared handle의 reset과 경합해도 둘 중 안전한 결과 하나를 얻는다.
                const std::shared_ptr<TaskAgent> pinned{observer.lock()};

                // [첫 호출 계약: std::shared_ptr<TaskAgent>::operator bool]
                // (1) 수신 pinned는 lock이 만든 유효한 const shared_ptr lvalue이며 empty일 수 있다.
                // (2) explicit operator bool() const noexcept와 반환 bool에 대한 built-in !를 사용한다.
                // (3) 명시 인자나 소유권 이동 없이 stored pointer를 읽는다.
                // (4) non-null이면 true인 bool prvalue를 if 조건에 사용한다.
                // (5) pointer, count, target과 observer는 변하지 않는다.
                // (6) O(1), 무할당·noexcept·무효화 없음이다. 결과 bool은 수명을 늘리지 않고 분기가
                //     empty pointer의 멤버 접근을 차단한다. 같은 pinned 객체의 동시 변경은 없다.
                if (!pinned) {
                    return false;
                }

                // [첫 호출 계약: std::shared_ptr<TaskAgent>::operator->]
                // (1) 수신 pinned는 위 조건 뒤 non-empty인 const shared_ptr<TaskAgent> lvalue다.
                // (2) element_type* operator->() const noexcept에서 element_type=TaskAgent다.
                // (3) 명시 인자와 소유권 이전은 없다.
                // (4) 유효한 TaskAgent*를 반환해 사용자 함수 matches_job의 this로 즉시 쓴다.
                // (5) shared_ptr/count/TaskAgent 값은 operator-> 자체로 바뀌지 않는다.
                // (6) O(1), 무할당·noexcept·무효화 없음이다. null pointer로 멤버를 부르면 UB지만 guard가
                //     배제하고, 반환 raw pointer를 pinned보다 오래 저장하지 않는다. 병행 쓰기도 없다.
                return pinned->matches_job(completed_job);
            }};
    }

private:
    // const 멤버 함수는 accepted_job_을 바꾸지 않는다. 값 매개변수 int는 독립 복사라 수명 조건이 없다.
    [[nodiscard]] bool matches_job(int completed_job) const noexcept {
        // 기본 정수 비교는 STL 호출이 아니며 결과 bool prvalue를 반환한다.
        return completed_job == accepted_job_;
    }

    // 기본 타입 int를 {}로 값 초기화하면 0이고 narrowing을 막는 데도 도움이 된다.
    int accepted_job_{};
};

int main() {
    // 42는 int prvalue이고 factory 반환 prvalue가 owner를 직접 초기화한다. 이름 있는 owner는 lvalue다.
    // shared_ptr은 TaskAgent 수명을 소유하지만 T*는 null 가능 비소유 주소, T&는 null 불가 비소유 별칭이다.
    std::shared_ptr<TaskAgent> owner{TaskAgent::create(42)};

    // operator->의 첫 표준 계약은 lambda 구현 가까이에 적혀 있다. 현재 owner는 factory 불변식상
    // non-empty이고, 반환 function prvalue가 callback을 직접 초기화해 별도 function 복사/이동이 없다.
    CompletionCallback callback{owner->completion_callback()};

    // [첫 호출 계약: std::function<bool(int)>::operator()]
    // (1) 수신 callback은 weak-capturing lambda를 소유한 non-empty CompletionCallback lvalue다.
    // (2) bool function<bool(int)>::operator()(int) const를 선택한다.
    // (3) 42는 허용된 int prvalue로 값 전달되며 agent 소유권은 인자로 이동하지 않는다.
    // (4) target의 bool 반환을 while_alive 초기화에 사용한다.
    // (5) lock 성공으로 임시 pinned가 호출 동안만 strong count를 늘렸다가 반환 때 해제된다. 장기
    //     owner와 callback 상태는 그대로다.
    // (6) type-erased dispatch와 오늘 target은 O(1), 호출 중 무할당이다. non-empty가 전제이고 empty면
    //     예외가 난다. target은 noexcept라 사용자 예외가 없고 관찰자 무효화도 없다. 동시 호출은
    //     target이 접근하는 상태의 thread-safety까지 만족해야 한다.
    const bool while_alive{callback(42)};

    // [첫 호출 계약: std::shared_ptr<TaskAgent>::reset]
    // (1) 수신 owner는 유효한 non-empty shared_ptr lvalue이고 현재 마지막 strong owner다. 이전 호출의
    //     pinned는 끝났으며 callback은 weak handle만 가진다.
    // (2) void reset() noexcept 무인자 overload를 선택한다.
    // (3) 인자나 새 owner는 없다.
    // (4) 반환형 void이며 뒤 callback 결과로 파괴 후 동작을 확인한다.
    // (5) owner는 empty가 되고 strong count 0에서 TaskAgent 수명이 끝난다. control block만 weak handle
    //     때문에 남고 모든 이전 raw pointer/reference는 dangling이 된다.
    // (6) count 처리 자체 O(1)에 상수 크기 소멸이 더해지고 무할당·noexcept다. weak observer는
    //     무효 pointer가 아니라 expired 상태가 된다. 같은 owner 객체의 병행 reset/read는 data race다.
    owner.reset();

    // 같은 function 호출 규약을 재사용하며 이제 lock 실패 경로가 false를 돌려준다.
    const bool after_destroy{callback(42)};

    // [첫 호출 계약: std::cout의 operator<< 연쇄]
    // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 main thread 하나가 사용한다.
    // (2) bool에는 basic_ostream<char>::operator<<(bool), char에는 비멤버 operator<< overload가 선택된다.
    // (3) 두 const bool lvalue를 값으로 읽고 ' ', '\n' char prvalue를 전달하며 소유권 이동은 없다.
    // (4) 각 ostream& 반환을 다음 삽입 수신으로 사용하고 마지막 참조는 버린다.
    // (5) 기본 bool 숫자 형식으로 정확히 "1 0\n"을 기록하며 callback은 그대로다.
    // (6) stream/locale/device를 합친 단일 이식 가능 복잡도·내부 할당 횟수는 표준이 고정하지 않는다.
    //     실패는 상태 bit 또는 설정된 예외로 보고된다. 다른 관찰자 무효화는 없고 복합 출력의
    //     thread 간 원자성은 보장하지 않는다.
    std::cout << while_alive << ' ' << after_destroy << '\n';

    // 값 범주와 복사/이동: owner/callback 같은 이름 있는 식은 lvalue, 42와 factory/lock 결과는 prvalue다.
    // std::move(owner)를 사용했다면 xvalue가 되어 shared ownership handle을 이동할 수 있지만 여기서는
    // reset 시점을 명확히 보여 주려고 이동하지 않는다. weak handle은 lambda capture에서 복사되고 closure가
    // function target으로 이동될 수 있다. 반환 prvalue는 보장 복사 생략되며 named-local NRVO와 다르다.
    //
    // 기계 실행 관점에서 lock은 control block count load/원자 갱신, 만료 비교와 조건 분기를 만들 수 있다.
    // reset은 마지막 owner 비교와 객체 파괴 store를 만들 수 있다. std::function type erasure는 간접 호출이
    // 될 수 있지만 virtual 함수 호출이라고 단정할 수 없고 optimizer가 제거할 수도 있다. 실제 명령,
    // cache/분기 비용은 CPU, ABI, compiler, 표준 라이브러리와 최적화 옵션에 따라 달라진다.
    //
    // 초보자 검증: callback이 raw pointer를 캡처했다면 reset 뒤 접근은 UB가 될 수 있다. weak_ptr은
    // 대상 수명을 늘리지 않으면서 lock 실패를 값으로 표현한다. 직접 해보기로 첫 인자를 7로 바꾸면
    // 생존 중에도 false이므로 출력이 "0 0"인지 확인해 보자.
    return 0;
}
