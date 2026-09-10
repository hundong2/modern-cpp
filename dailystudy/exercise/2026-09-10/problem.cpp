// <iostream>은 연습 결과를 출력하는 std::cout/std::ostream과 삽입 연산을 선언한다.
#include <iostream>
// <memory>는 custom deleter를 가진 std::unique_ptr와 C++23 std::out_ptr 어댑터를 선언한다.
#include <memory>
// <utility>는 명시적으로 소유권 이동 후보를 표시하는 std::move를 선언한다.
#include <utility>

// struct는 기본 public이라 C API와 주고받는 단순 레코드에 알맞다. {}는 code의 기본값을 0으로 만든다.
struct LegacyTicket {
    int code{};
};

// LegacyTicket**는 함수가 새 포인터 값을 기록할 출력 슬롯 주소다. output 자체와 *output을 구별한다.
extern "C" int legacy_issue_ticket(int code, LegacyTicket** output) noexcept {
    if (output == nullptr || code <= 0) {
        return 1;
    }
    try {
        // C++ new로 만든 객체이므로 짝이 되는 해제 함수는 delete를 사용해야 한다.
        *output = new LegacyTicket{code};
    } catch (...) {
        return 2;
    }
    return 0;
}

extern "C" void legacy_release_ticket(LegacyTicket* ticket) noexcept {
    delete ticket;
}

struct TicketCloser {
    void operator()(LegacyTicket* ticket) const noexcept {
        legacy_release_ticket(ticket);
    }
};

// 템플릿 인자 LegacyTicket은 관리 대상 타입, TicketCloser는 파괴 정책이다.
using TicketPtr = std::unique_ptr<LegacyTicket, TicketCloser>;

// class의 기본 private 접근으로 raw handle을 숨기고, public 관찰 함수만 제공한다.
class Ticket {
public:
    // explicit은 TicketPtr에서 Ticket으로의 우연한 암시 변환을 막는다. 값 매개변수는 소유권 sink다.
    // [호출 계약: std::move(owner)와 TicketPtr 이동 생성]
    // (1) move에는 수신자가 없고 owner는 유효한 TicketPtr 값 매개변수 lvalue, owner_는 구성 전이다.
    // (2) move<T>(T&&)에서 T=TicketPtr&가 추론되어 TicketPtr&&를 반환하고 unique_ptr 이동 생성이 선택된다.
    // (3) 인자 owner는 빈 상태나 한 Ticket 소유 상태를 모두 허용한다. move는 빌려 xvalue로 바꾸고 후속
    //     생성자가 원시 포인터와 deleter의 단독 소유권을 이전한다.
    // (4) 반환 TicketPtr&&는 owner_ 구성에 즉시 사용하고 생성자 자체 반환값은 없다.
    // (5) owner_가 자원을 소유하며 owner는 빈 유효 상태다. 관리 객체 주소와 code는 그대로다.
    // (6) O(1), 무할당·noexcept이고 기존 대상 포인터를 무효화하지 않는다. 동일 owner의 동시 변경은 금지하며
    //     owner_의 수명 종료가 deleter 호출과 자원 해제를 보장한다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
    explicit Ticket(TicketPtr owner) noexcept
        : owner_{std::move(owner)} {}

    Ticket(const Ticket&) = delete;
    Ticket& operator=(const Ticket&) = delete;
    Ticket(Ticket&&) noexcept = default;
    Ticket& operator=(Ticket&&) noexcept = default;
    ~Ticket() = default;

    [[nodiscard]] static Ticket issue(int code) {
        // [생성 계약: TicketPtr 기본 생성]
        // (1) 목적 owner는 아직 없고 TicketCloser는 기본 생성 가능한 상태 없는 타입이다.
        // (2) T=LegacyTicket, D=TicketCloser인 unique_ptr() noexcept 기본 생성자가 선택되며 데이터 인자는 없다.
        // (3) 외부 포인터나 소유권 입력이 없고 deleter 기본 생성 가능성이 전제다.
        // (4) 반환값 없는 생성자가 빈 TicketPtr owner의 수명을 시작한다.
        // (5) 저장 포인터는 null이고 관리 객체가 없다.
        // (6) O(1), 무할당·noexcept·비무효화다. owner가 나중에 non-null이면 소멸자가 deleter를 한 번 부르며,
        //     같은 owner를 여러 실행 흐름에서 동시에 수정하지 않는다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        TicketPtr owner{};

        // [호출 계약: std::out_ptr(owner) 출력 슬롯 어댑터]
        // (1) 자유 함수의 인자 owner는 살아 있는 빈 TicketPtr lvalue이고 C 함수는 호출 중 쓸 Ticket**를 받는다.
        // (2) out_ptr<Pointer=void, Smart=TicketPtr, Args={}>(Smart&)가 포인터 타입 LegacyTicket*을 추론해
        //     std::out_ptr_t<TicketPtr, LegacyTicket*> prvalue를 만들고 LegacyTicket** 변환을 선택한다.
        // (3) owner는 non-const lvalue reference로 빌린다. 변환된 슬롯 주소는 adapter 수명 밖에 저장하면 안 된다.
        // (4) adapter 반환값은 둘째 C 인자로 쓰고, legacy_issue_ticket의 int 반환값은 status에 저장한다.
        // (5) adapter 생성 때 owner를 reset한다. 성공한 C 호출이 내부 슬롯에 포인터를 쓰고, 전체 표현식 끝
        //     adapter 소멸이 owner에 그 포인터를 채택시킨다. 실패 계약에서는 슬롯이 null인 채 owner도 빈 상태다.
        // (6) adapter/reset은 이 unique_ptr 특수화에서 O(1)·무할당이다. C 함수 할당 실패는 status 2다.
        //     out_ptr 생성은 일반적으로 noexcept 명세가 아니며 슬롯 수명 위반은 미정의 동작이다. 동기화는 없다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        const int status{legacy_issue_ticket(code, std::out_ptr(owner))};

        if (status != 0) {
            return Ticket{TicketPtr{}}; // 위와 같은 빈 기본 생성; 같은 타입 prvalue 반환은 직접 구성된다.
        }
        return Ticket{std::move(owner)}; // 이름 있는 lvalue 소유자를 xvalue로 바꿔 넘긴다.
    }

    [[nodiscard]] bool valid() const noexcept {
        // [호출 계약: TicketPtr::operator bool]
        // (1) 수신자는 살아 있는 const TicketPtr lvalue owner_이며 빈 상태 또는 단독 소유 상태다.
        // (2) explicit operator bool() const noexcept가 선택되고 데이터 인자는 없다.
        // (3) const this만 빌리며 원시 포인터와 deleter를 옮기지 않는다.
        // (4) 저장 포인터가 non-null인지 나타내는 bool prvalue를 반환해 호출자 조건/출력에 쓴다.
        // (5) owner_와 대상의 상태·수명은 변하지 않는다.
        // (6) O(1), 무할당·비무효화·noexcept다. 같은 owner_의 동시 변경이 없어야 하며 자체 동기화는 없다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        return static_cast<bool>(owner_);
    }

    [[nodiscard]] int code() const noexcept {
        if (!owner_) { // 위 operator bool 계약을 재사용해 역참조 전에 빈 상태를 거른다.
            return -1;
        }

        // [호출 계약: TicketPtr::operator->]
        // (1) 수신자는 LegacyTicket을 소유하는 non-null const TicketPtr lvalue owner_다.
        // (2) pointer operator->() const noexcept가 선택되고 pointer는 LegacyTicket*이다.
        // (3) 데이터 인자는 없고 앞선 검사로 non-null 전제조건을 만족한다. 소유권은 빌리기만 한다.
        // (4) 비소유 원시 포인터 prvalue를 반환하고 내장 ->가 code lvalue를 찾아 int 값을 복사한다.
        // (5) owner_, 대상, deleter는 바뀌지 않는다.
        // (6) O(1), 무할당·비무효화·noexcept다. owner_의 reset/이동/소멸 뒤 포인터는 사용할 수 없고 null
        //     역참조는 미정의 동작이다. 대상 동시 변경에는 별도 동기화가 필요하다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        return owner_->code;
    }

private:
    TicketPtr owner_; // private 멤버가 C 자원의 단독 소유권과 수명을 맡는다.
};

int main() {
    // 함수가 반환한 Ticket prvalue는 각 const 객체에 직접 구성된다. 이름이 붙은 issued/rejected는 lvalue다.
    const Ticket issued{Ticket::issue(42)};
    const Ticket rejected{Ticket::issue(0)};

    // [호출 계약: std::ostream 삽입 연쇄]
    // (1) 수신자는 정상 상태인 std::cout의 std::ostream lvalue이고 Ticket 두 객체는 살아 있다.
    // (2) int/bool에는 ostream 멤버 삽입, 두 char에는 operator<<(std::ostream&, char)가 선택된다.
    // (3) code() int prvalue, 공백/개행 char prvalue, valid() bool prvalue를 값으로 읽고 소유권은 이동하지 않는다.
    // (4) 각 삽입은 같은 std::ostream&를 반환해 다음 호출에 쓰며 마지막 참조는 버린다.
    // (5) 성공하면 `42 0\n`이 버퍼에 추가되고 두 Ticket은 그대로다.
    // (6) 복잡도는 locale·변환·버퍼·장치에 의존한다. 버퍼 할당, 상태 비트, 설정된 예외 마스크 예외가 가능하고
    //     참조 무효화는 없다. 단일 실행 흐름이라 레코드 혼합 문제도 없다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cout << issued.code() << ' ' << rejected.valid() << '\n';

    // 기계 관점에서 adapter 슬롯 store, status/null 비교, 조건 분기, 포인터 load와 deleter 호출이 생길 수 있다.
    // 정확한 인라인·복사 생략·할당·명령 선택은 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라
    // 달라지므로 특정 어셈블리나 실행 비용 하나로 단정하지 않는다.
    return 0;
}
