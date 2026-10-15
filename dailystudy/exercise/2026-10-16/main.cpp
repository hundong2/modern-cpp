// 오늘의 실무 패턴: 여러 필드를 단계적으로 바꾸는 작업에 작은 ScopeExit 롤백 가드를 두어
// 모든 검증을 통과한 경우에만 commit한다. 중간 return이나 앞으로 추가될 예외 경로에서도
// 가드가 살아 있으면 원상 복구되므로, 정상 경로와 실패 정리 경로를 한곳에 모을 수 있다.

// <cstddef>는 객체 크기와 컨테이너 색인을 나타내는 부호 없는 기본 별칭 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 출력 객체 std::cout과 형식화 삽입 연산을 선언한다.
#include <iostream>
// <utility>는 객체 자체를 옮기지 않고 식을 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>
// <vector>는 연속 메모리를 소유하는 동적 배열 std::vector를 선언한다.
#include <vector>

// using은 새 정수 타입을 만드는 것이 아니라 긴 타입에 문맥이 드러나는 별칭을 붙인다.
using AccountId = std::size_t;

// 표준 <scope>가 없는 현재 실습 도구 모음에서도 원리를 직접 확인하도록 만든 최소 RAII 가드다.
// class template의 ExitFunction 인자는 종료 때 실행할 lambda/함수 객체의 정확한 타입이 된다.
template <class ExitFunction>
class ScopeExit {
public:
    // 값 매개변수는 호출 가능 객체를 먼저 소유하고, 멤버 초기화 목록에서 다시 멤버로 이동한다.
    explicit ScopeExit(ExitFunction exit_function)
        // [첫 호출 계약: std::move<ExitFunction&>]
        // (1) 자유 함수라 수신 객체는 없고, exit_function은 살아 있는 ExitFunction lvalue다.
        // (2) std::move<ExitFunction&>(ExitFunction&) overload가 선택되어 인스턴스화되고
        //     반환형은 ExitFunction&&다.
        // (3) 인자는 이름 있는 값 매개변수 lvalue이며 함수 객체를 exit_function_에 넘길 예정이다.
        // (4) 반환 xvalue는 멤버의 이동 생성에 즉시 사용하고 별도 객체를 만들지 않는다.
        // (5) move 자체는 상태를 바꾸지 않으며 뒤의 ExitFunction 이동 생성이 소유 상태를 옮긴다.
        // (6) move는 constexpr/noexcept/상수 시간/무할당이고 수명을 늘리지 않는다. 실제 멤버 이동의
        //     예외·비용·이동 후 상태는 ExitFunction 계약에 달렸고 이 예제 lambda 이동은 무할당이다.
        : exit_function_{std::move(exit_function)} {}

    // 가드를 복사하면 rollback이 두 번 실행될 수 있으므로 복사/대입을 명시적으로 금지한다.
    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit(ScopeExit&&) = delete;
    ScopeExit& operator=(ScopeExit&&) = delete;

    // 소멸자는 active일 때만 호출 가능 객체를 실행한다. 이 예제에서는 callback이 반드시 noexcept다.
    ~ScopeExit() noexcept {
        if (active_) {
            exit_function_();
        }
    }

    // commit 뒤에는 소멸자가 rollback을 실행하지 않도록 상태를 내린다.
    void release() noexcept {
        active_ = false;
    }

private:
    ExitFunction exit_function_;
    bool active_{true};
};

// struct의 기본 접근 지정자는 public이다. 이 작은 owner는 잔액 저장소의 초기 소유권만 받는다.
struct Ledger {
    // vector의 템플릿 인자 int는 각 계좌 잔액의 원소 타입이다. vector가 원소 저장소를 소유한다.
    std::vector<int> balances;

    // explicit은 vector가 뜻하지 않게 Ledger로 암시 변환되는 것을 막는다.
    explicit Ledger(std::vector<int> initial) noexcept
        // [첫 생성 계약: std::vector<int> 이동 생성]
        // (1) 목적 멤버 balances는 아직 수명이 시작되지 않았고 initial은 두 잔액을 소유한
        //     살아 있는 std::vector<int> lvalue다.
        // (2) vector(vector&&) noexcept 이동 생성자 overload가 선택되며 T=int,
        //     Allocator=std::allocator<int>다. 안쪽 std::move 계약은 앞에서 설명했다.
        // (3) std::move(initial)이 만든 vector<int>&& xvalue 하나가 인자다. initial의 버퍼 소유권을
        //     balances로 이전해도 된다는 뜻이며 외부 참조를 빌리지 않는다.
        // (4) 생성자는 반환값이 없고 balances가 initial의 저장소를 소유하게 한다.
        // (5) balances는 두 원소를 갖고 initial은 유효하지만 내용이 미지정인 이동 후 상태가 된다.
        // (6) 같은 기본 allocator의 이동 생성은 상수 시간·무할당·noexcept다. 원소 pointer/reference는
        //     새 owner 아래 계속 유효하지만 initial 자체의 상태에 기대면 안 된다. Ledger 수명 종료 때
        //     저장소를 해제하고, 구성 중 다른 thread와 공유하지 않는다.
        : balances{std::move(initial)} {}
};

// 집합 초기화 가능한 명령 값이다. 중괄호 초기화는 멤버 선언 순서대로 값을 넣는다.
struct TransferCommand {
    AccountId from{}; // {}는 부호 없는 기본 타입을 0으로 값 초기화한다.
    AccountId to{};
    int amount{}; // int는 이 예제의 작고 음수가 될 수도 있는 기본 정수 입력을 표현한다.
};

// class의 기본 접근 지정자는 private이다. 불변식을 지키는 public 함수만 외부에 공개한다.
class LedgerService {
public:
    // explicit은 Ledger 하나가 LedgerService로 암시 변환되는 것을 막는다.
    // 매개변수 Ledger&는 null일 수 없는 비소유 lvalue 참조다. 포인터와 달리 재지정할 수 없고,
    // service보다 ledger가 오래 살아야 한다. 멤버 초기화 목록은 함수 몸체 전에 참조를 결합한다.
    explicit LedgerService(Ledger& ledger) noexcept
        : ledger_{ledger} {}

    // [[nodiscard]]는 성공 여부를 무시하기 쉬운 호출자에게 컴파일러 진단을 요청한다.
    // const TransferCommand&는 명령을 복사하지 않고 읽기만 하며 소유권을 가져오지 않는다.
    [[nodiscard]] bool apply(const TransferCommand& command) {
        // [첫 호출 계약: std::vector<int>::size]
        // (1) 수신 ledger_.balances의 정확한 타입은 살아 있는 std::vector<int>이며 현재 원장을 소유한다.
        // (2) size_type size() const noexcept 오버로드가 선택되고 size_type은 std::size_t다.
        // (3) 명시 인자는 없다. command.from/to는 아래 비교에서 소유권 없는 정수 값으로 읽힌다.
        // (4) 반환은 현재 원소 수의 prvalue이며 두 범위 비교에 사용한다.
        // (5) vector, command, 참조/반복자는 전혀 바뀌지 않는다.
        // (6) 상수 시간, 무할당, 예외 없음, 무효화 없음이다. 같은 vector의 동시 쓰기와 겹치지 않아야 한다.
        const AccountId count{ledger_.balances.size()};

        // ||는 왼쪽부터 단락 평가한다. 잘못된 명령은 복사·할당이나 원장 변경 전에 거부한다.
        if (command.from >= count || command.to >= count || command.from == command.to ||
            command.amount <= 0) {
            return false;
        }

        // [첫 호출 계약: std::vector<int>::operator[]]
        // (1) 수신 ledger_.balances는 count개 int를 소유하는 non-const std::vector<int> lvalue다.
        // (2) reference operator[](size_type position) noexcept 오버로드가 선택되며 reference는 int&다.
        // (3) 첫 인자 command.from과 뒤의 command.to는 AccountId(std::size_t) lvalue에서 값으로
        //     읽히며 위에서 둘 다 count보다 작음을 보장했다. 소유권 이전은 없다.
        // (4) 반환 int&는 잔액 비교와 뒤의 -=/+= 저장에서 해당 원소를 지정하는 데 사용한다.
        // (5) 이 읽기 자체는 vector나 인자를 바꾸지 않고, 뒤 복합 대입은 해당 int만 수정한다.
        // (6) 전제조건인 유효 색인을 어기면 미정의 동작이다. 여기서는 충족하므로 상수 시간, 무할당,
        //     예외/재할당/반복자 무효화 없음이다. 참조는 vector 수명과 재할당 전까지만 유효하고
        //     외부 동기화 없는 동시 쓰기는 데이터 경쟁이다.
        if (ledger_.balances[command.from] < command.amount) {
            return false;
        }

        // [첫 생성 계약: std::vector<int> 복사 생성]
        // (1) 목적 객체 before는 아직 없고, 수신 원본 ledger_.balances의 정확한 타입은
        //     살아 있는 std::vector<int> lvalue이며 유효한 int 원소들을 소유한다.
        // (2) vector(const vector&) 오버로드가 선택되고 템플릿 인자는 T=int,
        //     Allocator=std::allocator<int>다.
        // (3) 인자 ledger_.balances는 const vector<int>&에 결합되는 lvalue다. 원본 소유권은
        //     유지되고 각 int 값만 새 저장소로 복사된다.
        // (4) 생성자는 반환값이 없으며 독립 저장소를 소유하는 before를 완성한다.
        // (5) 성공 뒤 원본은 그대로이고 before.size()==ledger_.balances.size()이며 원소 값이 같다.
        // (6) 시간과 추가 공간은 원소 수 O(n)이다. 할당 실패 시 bad_alloc 등이 가능하지만
        //     아직 원장을 바꾸지 않았으므로 강한 예외 보장을 얻는다. 성공한 복사는 원본의
        //     참조/반복자를 무효화하지 않고 두 저장소의 수명은 독립이다. 한 스레드에서만 호출한다.
        std::vector<int> before{ledger_.balances};

        // [RAII 타입 생성 계약: ScopeExit<rollback-closure>]
        // (1) 목적 객체 rollback은 아직 없고 ledger_, before와 현재 LedgerService가 모두 살아 있다.
        // (2) CTAD가 lambda의 고유 closure 타입을 ExitFunction으로 정해 위 생성자를 선택한다.
        // (3) 인자는 [this, &before]로 만든 lambda prvalue다. this 포인터는 service를 비소유로,
        //     before는 lvalue 참조로 캡처한다. 둘 다 guard보다 오래 살아야 한다.
        // (4) 생성자는 반환값이 없고 호출 가능 객체를 값으로 소유하는 active guard를 만든다.
        // (5) ledger와 before의 값은 아직 같다. 이후 원장을 일부 바꾼 채 scope를 떠나더라도
        //     release되지 않았다면 저장한 lambda를 정확히 한 번 실행한다.
        // (6) closure 구성 가능성이 전제이며 구성 중 예외가 나면 원장 수정 전이다. 이 closure 저장은
        //     동적 할당을 요구하지 않는다. guard는 복사/이동할 수 없는 현재 thread의 자동 객체다.
        //     소멸자는 noexcept라 callback이 던지면 terminate되므로 아래 noexcept 이동 대입만 사용한다.
        ScopeExit rollback{[this, &before]() noexcept {
            // [첫 호출 계약: std::move와 std::vector<int>::operator=(vector&&)]
            // (1) 자유 함수 move에는 수신 객체가 없고 before는 살아 있는 vector<int> lvalue다.
            //     이동 대입 수신 ledger_.balances도 살아 있으며 부분 변경된 저장소를 소유할 수 있다.
            // (2) std::move<vector<int>&>(before)는 vector<int>&&를 반환하고, 이어서
            //     vector<int>::operator=(vector&&)가 선택된다.
            // (3) before 식은 move에 들어갈 때 lvalue이고 반환 식은 xvalue다. 저장 버퍼 소유권을
            //     ledger_.balances로 이전하겠다는 의미이며 const 객체에는 적용하지 않는다.
            // (4) move 결과는 이동 대입의 인자로 즉시 사용한다. 이동 대입 반환형 vector<int>&는
            //     같은 수신 객체를 가리키지만 여기서는 사용하지 않는다.
            // (5) 기본 std::allocator<int>에서는 기존 ledger 버퍼를 정리하고 before의 버퍼를 넘겨받아
            //     원래 잔액을 복구한다. before는 유효하지만 내용이 미지정인 이동 후 상태가 된다.
            // (6) std::move 자체는 constexpr/noexcept/상수 시간/무할당이고 수명을 늘리지 않는다.
            //     std::allocator는 is_always_equal이어서 이 이동 대입은 noexcept이며 보통 상수 시간이다.
            //     수신의 기존 원소 참조/반복자는 무효화되고, 옮긴 버퍼의 원소 참조는 새 소유자를
            //     가리킨다. 동시 접근은 없으며 같은 vector를 다른 thread가 건드리면 데이터 경쟁이다.
            ledger_.balances = std::move(before);
        }};

        // [RAII 수명 계약: ScopeExit 소멸자]
        // (1) 수신자는 위에서 완성된 active 또는 release된 scope_exit 객체 rollback이다.
        // (2) 인자 없는 ~ScopeExit() noexcept가 이 함수의 모든 return과 정상 scope 종료에서 호출된다.
        // (3) 명시 인자는 없고 내부에 소유한 closure만 사용한다.
        // (4) 반환값은 없다. active이면 closure의 void 결과도 사용하지 않는다.
        // (5) active이면 원장을 복구한 뒤 guard 수명이 끝나고, released이면 아무 동작 없이 끝난다.
        // (6) 소멸 자체는 상수 시간이고 할당/참조 무효화를 직접 만들지 않는다. closure가 캡처한
        //     service와 before가 살아 있어야 하며 이 선언 순서에서는 rollback이 before보다 먼저
        //     파괴된다. thread-safe wrapper가 아니므로 외부 동기화 없이 같은 원장을 공유하지 않는다.

        // source >= amount이므로 아래 뺄셈 결과는 0 이상이라 signed overflow가 없다.
        ledger_.balances[command.from] -= command.amount;

        // 이 예제 원장의 업무 상한은 계좌당 100이다. amount는 양수이므로 `100 - amount` 자체도 int
        // 범위 안이며, 덧셈 전에 목적지의 안전 상한을 비교하면 signed overflow라는 UB를 피할 수 있다.
        // 이 검사를 일부 변경 뒤에 둔 것은 실패 시 ScopeExit가 스냅샷을 실제로 복원하는 모습을 보이기
        // 위해서다. 값은 복원되지만 이동 대입 때문에 변경 전 vector 원소 관찰자는 무효화된다.
        constexpr int maximum_balance{100};
        if (ledger_.balances[command.to] > maximum_balance - command.amount) {
            return false;
        }

        // 위 조건 뒤의 합은 maximum_balance 이하라 안전하다. 기본 int 연산은 load/산술/store로 구현될
        // 수 있지만 실제 명령과 레지스터 사용은 CPU, 컴파일러, ABI, 최적화 옵션에 따라 달라진다.
        ledger_.balances[command.to] += command.amount;

        // [RAII commit 계약: ScopeExit::release]
        // (1) 수신 rollback의 정확한 타입은 위 closure를 소유한 active ScopeExit lvalue다.
        // (2) void release() noexcept 멤버가 선택된다.
        // (3) 명시 인자는 없고 closure나 원장의 소유권도 넘기지 않는다.
        // (4) 반환형은 void라 저장할 값이 없다.
        // (5) guard를 inactive로 바꾸어 이후 소멸자가 rollback closure를 실행하지 않게 한다.
        // (6) 전제조건 없이 상수 시간, 무할당, 예외 없음이며 vector 참조/반복자를 무효화하지 않는다.
        //     guard 자체는 공유하지 않으므로 별도 thread 보장은 필요하지 않다.
        rollback.release();
        return true;
    }

    // 뒤 const는 service의 관찰 함수임을 뜻한다. const Ledger& 반환은 복사 없이 원본을 빌린다.
    [[nodiscard]] const Ledger& view() const noexcept {
        return ledger_;
    }

private:
    // 참조 멤버는 원장을 소유하지 않는다. LedgerService를 원장보다 오래 살리면 dangling reference다.
    Ledger& ledger_;
};

int main() {
    // [첫 생성 계약: std::vector<int>(initializer_list<int>)]
    // (1) Ledger 생성자에 넘길 임시 std::vector<int> 결과 객체는 아직 없고 두 int 값은 유효하다.
    // (2) vector(initializer_list<int>, const allocator_type& = allocator_type())가 T=int로 선택된다.
    // (3) {100, 20}의 원소는 const int lvalue처럼 복사되고 기본 allocator 인자는 생략된다.
    //     초기화 목록 backing array의 소유권은 받지 않고 값으로 새 저장소를 만든다.
    // (4) 생성자는 반환값이 없으며 두 원소를 소유한 vector prvalue가 Ledger의 값 매개변수 initial을
    //     직접 초기화하고, 위 이동 생성 계약에 따라 balances로 전달된다.
    // (5) 최종 balances 크기는 2이고 원소는 순서대로 100, 20이다. backing array 수명과 독립이다.
    // (6) 시간/공간 O(2), 동적 할당과 bad_alloc 가능, 실패 시 Ledger 구성도 완료되지 않는다.
    //     성공 시 원소 참조는 vector 재할당/파괴까지 유효하고 구성 중 공유 접근은 없다.
    Ledger ledger{std::vector<int>{100, 20}};

    // ledger는 lvalue이므로 Ledger&에 결합한다. service가 먼저 파괴되도록 owner를 먼저 선언한다.
    LedgerService service{ledger};

    // TransferCommand{...}는 prvalue이고 const& 매개변수에 full-expression 끝까지 결합한다.
    // 값 반환 bool도 prvalue라 first를 직접 초기화하며 사용자 정의 복사/이동이나 RVO 대상이 없다.
    const bool first{service.apply(TransferCommand{0, 1, 30})};
    // 출금은 가능하지만 목적지가 상한 100을 넘으므로, 첫 차감 뒤 가드가 70,50 스냅샷을 복원한다.
    const bool second{service.apply(TransferCommand{0, 1, 60})};

    // const lvalue 참조는 원장의 수명을 늘리지 않는다. ledger가 main 끝까지 살아 있어 안전하다.
    const Ledger& final_state{service.view()};

    // [첫 호출 계약: std::cout의 operator<< 연쇄]
    // (1) 수신/왼쪽 객체는 정상 상태의 std::ostream인 std::cout이고 이 프로그램의 한 thread만 쓴다.
    // (2) first/second에는 basic_ostream<char>& basic_ostream<char>::operator<<(bool), 두 잔액에는
    //     basic_ostream<char>& basic_ostream<char>::operator<<(int)가 선택된다. 각 문자는
    //     template<class CharT,class Traits> basic_ostream<CharT,Traits>& operator<<(
    //     basic_ostream<CharT,Traits>&, CharT)에서 CharT=char, Traits=char_traits<char>로 선택된다.
    //     boolalpha를 설정하지 않았으므로 bool은 기본적으로 1/0 숫자 형식이다.
    // (3) first/second와 두 int 원소는 lvalue 값 입력, ' '와 '\n'은 char prvalue다. 어느 값이나
    //     스트림 버퍼의 소유권도 이동하지 않는다. operator[] 인자 0,1은 유효한 size_t로 변환된다.
    // (4) 각 삽입은 같은 ostream&를 반환해 다음 <<의 수신자로 사용하고 마지막 참조는 버린다.
    //     잔액 접근은 const_reference vector<int>::operator[](size_type position) const noexcept를
    //     선택하고 각각 const int&를 반환해 int 삽입 overload가 읽는다.
    // (5) 성공하면 "1 0 70 50\n"이 순서대로 버퍼에 기록되고 원장 값은 그대로다. 실패 시
    //     failbit/badbit가 설정될 수 있다.
    // (6) 출력 복잡도는 문자 수에 비례하고 버퍼 할당/로캘/시스템 호출은 구현별이다. exceptions mask가
    //     켜진 오류는 ios_base::failure를 던질 수 있다. 원소 참조 무효화는 없고 동시 출력 원자성에
    //     기대지 않는다. 구체 기계 명령과 flush 시점은 구현·OS·최적화에 따라 달라진다.
    std::cout << first << ' ' << second << ' ' << final_state.balances[0] << ' '
              << final_state.balances[1] << '\n';

    // 두 조건이 모두 참이어야 0을 반환한다. &&의 단락 평가는 조건 분기를 만들 수 있다.
    return (first && !second && final_state.balances[0] == 70 &&
            final_state.balances[1] == 50)
               ? 0
               : 1;
}
