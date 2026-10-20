// 오늘의 실무 패턴: 비동기 작업 큐에 등록되는 콜백이 서비스의 수명을 불필요하게 늘리지 않게 한다.
// factory는 객체를 shared ownership 안에서 만들고, 콜백은 weak ownership만 보관한다. 실행 순간의
// lock()이 성공한 동안에만 owner를 잠시 고정하므로 파괴 뒤의 callback도 dangling pointer를 쓰지 않는다.

// <functional>은 호출 가능한 대상을 type-erasure로 값 보관하는 std::function을 선언한다.
#include <functional>
// <iostream>은 표준 출력 객체 std::cout과 형식화 삽입 연산자를 선언한다.
#include <iostream>
// <memory>는 std::enable_shared_from_this, shared_ptr, weak_ptr, make_shared를 선언한다.
#include <memory>
// <utility>는 이름 있는 lvalue를 이동 후보인 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>

// using은 새 타입을 만드는 문법이 아니라 `bool(int)` 호출 규약을 읽기 쉬운 이름으로 붙인다.
// std::function의 template 인자 bool(int)는 반환형 bool, 매개변수형 int인 함수 타입이다.
using EventCallback = std::function<bool(int)>;

// class는 기본 접근이 private이고 struct는 기본 접근이 public이다. 여기서는 factory key와 임계값을
// 감추고 생성·콜백 API만 공개하려고 class를 쓴다. final은 추가 상속을 막지만 가상 호출을 만들지는 않는다.
// public 상속한 enable_shared_from_this 기반 부분은 자신을 소유하는 shared_ptr의 control block을 관찰한다.
class EventGate final : public std::enable_shared_from_this<EventGate> {
private:
    // 중첩 class 이름과 기본 생성자를 모두 private으로 막는다. 타입 이름을 쓰지 않은 `EventGate{{}, 1}`도
    // private 기본 생성자 접근 검사에서 거절된다. 바깥 EventGate가 key를 만들 수 있도록 friend로 선언한다.
    class FactoryKey final {
        friend class EventGate;

        FactoryKey() noexcept = default;

    public:
        // make_shared 구현이 create 안에서 만든 key를 EventGate의 값 매개변수까지 전달해야 하므로
        // 복사·이동 생성은 public이다. 외부에는 복사할 원본 key를 얻는 공개 경로가 없다.
        FactoryKey(const FactoryKey&) noexcept = default;
        FactoryKey(FactoryKey&&) noexcept = default;
    };

public:
    // 생성자는 반환형이 없고 태그와 기본 타입 int를 값으로 받는다. explicit은 이 두 값의 copy-list가
    // 뜻하지 않게 EventGate 값으로 변환되는 문맥을 막는다. 중괄호 초기화는 narrowing을 엄격히 검사한다.
    explicit EventGate(FactoryKey, int minimum_code) noexcept
        // [첫 생성 계약: std::enable_shared_from_this<EventGate> 기본 생성]
        // (1) 목적 base subobject는 아직 수명이 시작되지 않았고 연결할 shared control block도 없다.
        // (2) protected constexpr enable_shared_from_this() noexcept를 T=EventGate로 선택한다.
        // (3) 명시 인자는 없으며 외부 객체·pointer·reference의 소유권이나 수명을 빌리지 않는다.
        // (4) 생성자는 반환값 없이 비어 있는 내부 weak 관찰 상태를 가진 base subobject를 만든다.
        // (5) base 수명이 시작되지만 아직 owner와 연결되지 않는다. 뒤에서 make_shared가 만든 최초
        //     shared_ptr이 접근 가능한 이 기반 부분을 같은 control block과 연결한다.
        // (6) 상수 시간, 동적 할당·무효화·예외 없음이다. 완성 전 weak_from_this를 쓰지 않으며,
        //     기반 부분의 수명은 EventGate와 같고 같은 객체의 병행 생성·파괴는 허용하지 않는다.
        // 멤버 초기화 목록은 본문보다 먼저 실행되며 실제 순서는 base, minimum_code_ 선언 순서다.
        : std::enable_shared_from_this<EventGate>{}, minimum_code_{minimum_code} {}

    // 이 객체를 값으로 복사하거나 이동하면 새 객체의 enable_shared_from_this 기반은 원래 control block에
    // 연결되지 않는다. 그러면 살아 있는 복사본에서도 weak_from_this()가 빈 handle을 돌려줄 수 있으므로,
    // factory가 만든 shared owner 안에서만 객체가 존재한다는 불변식을 네 특수 멤버를 삭제해 강제한다.
    EventGate(const EventGate&) = delete;
    EventGate& operator=(const EventGate&) = delete;
    EventGate(EventGate&&) = delete;
    EventGate& operator=(EventGate&&) = delete;

    // static factory에는 this 수신 객체가 없다. shared_ptr<EventGate>는 T=EventGate인 소유 smart pointer다.
    [[nodiscard]] static std::shared_ptr<EventGate> create(int minimum_code) {
        // [첫 호출 계약: std::make_shared<EventGate>(FactoryKey{}, minimum_code)]
        // (1) 자유 함수라 수신 객체는 없다. 목적 EventGate와 control block은 아직 존재하지 않는다.
        // (2) shared_ptr<EventGate> make_shared<EventGate, FactoryKey, int&>(FactoryKey&&, int&)를
        //     선택하며 shared_ptr의 element_type template 인자는 EventGate다.
        // (3) FactoryKey{}는 허가를 나타내는 비소유 prvalue이고 minimum_code는 이름 있는 int lvalue다.
        //     forwarding reference는 각각 rvalue/lvalue로 결합하며 모든 int 값이 허용된다.
        // (4) 새 EventGate를 공동 소유하는 shared_ptr<EventGate> prvalue를 반환하고 factory 결과로 쓴다.
        // (5) 성공하면 객체와 control block의 수명이 시작되고 enable_shared_from_this 기반이 그 control
        //     block을 관찰한다. 인자 int와 태그에는 호출 뒤 의미 있는 상태 변화가 없다.
        // (6) 한 객체 구성에 O(1)이며 동적 저장소가 필요하다. 객체와 control block을 함께 배치할 수
        //     있지만 내부 배치·할당 세부는 구현에 따라 다르다. 할당/생성 실패 예외 시 완성 owner가
        //     없고 시작된 하위 객체는 정리된다. 반환 owner 파괴 전 객체가 살며 같은 shared_ptr 객체의
        //     병행 변경은 금지지만 서로 다른 shared/weak handle의 control-block 연산은 thread-safe하다.
        // 반환 식은 shared_ptr prvalue다. 호출자의 결과 객체를 직접 초기화해 shared_ptr 복사/이동이
        // 필요 없는 C++17 보장 복사 생략이며, 이름 있는 local을 반환하는 선택적 NRVO와는 다르다.
        return std::make_shared<EventGate>(FactoryKey{}, minimum_code);
    }

    // 반환형 EventCallback은 콜백 값의 owner이고, 이 const가 없는 함수는 내부 weak handle을 새로 얻는다.
    // 반환된 콜백은 EventGate를 강하게 소유하지 않으므로 queue에 오래 남아도 서비스 수명을 늘리지 않는다.
    [[nodiscard]] EventCallback make_callback() {
        // [첫 호출 계약: EventGate::weak_from_this]
        // (1) 수신 *this의 정확한 타입은 factory가 shared_ptr로 소유 중인 유효한 EventGate lvalue다.
        // (2) std::weak_ptr<EventGate> weak_from_this() noexcept 비-const overload를 선택한다.
        // (3) 명시 인자는 없고 this는 함수 동안 빌리는 비소유 pointer다. 객체가 factory 생성 뒤
        //     shared ownership에 연결되었다는 class 불변식이 허용 상태다.
        // (4) 같은 control block을 관찰하는 weak_ptr<EventGate> prvalue를 반환해 observer를 직접 만든다.
        // (5) EventGate 값은 바뀌지 않고 control block의 weak 관찰 bookkeeping만 늘 수 있다. owner의
        //     strong count와 수명은 늘지 않으며, 연결 전 객체였다면 빈 weak_ptr을 돌려준다.
        // (6) 상수 시간·무할당·noexcept이고 iterator/참조 무효화가 없다. 결과는 객체 파괴 뒤에도
        //     빈 상태를 안전하게 관찰하지만 객체 접근 권한은 lock 성공 때만 생긴다. 서로 다른 handle의
        //     control-block 연산은 thread-safe하고 같은 handle 객체 자체의 병행 변경은 동기화해야 한다.
        std::weak_ptr<EventGate> observer{weak_from_this()};

        // [첫 호출 계약: std::move(observer)]
        // (1) 자유 함수라 수신 객체가 없다. observer는 non-empty인 유효한 std::weak_ptr<EventGate>
        //     lvalue이고 아래 init-capture 멤버는 아직 존재하지 않는다.
        // (2) remove_reference_t<T>&& move(T&&) noexcept에서 T=std::weak_ptr<EventGate>&로 추론되어
        //     반환형은 std::weak_ptr<EventGate>&&다.
        // (3) observer 식은 이름 있는 lvalue이며 모든 유효한 weak_ptr 상태가 허용된다. 반환 xvalue는
        //     관찰 handle 상태를 새 캡처 멤버로 옮겨도 된다는 의도를 나타내고 strong ownership은 없다.
        // (4) 같은 객체를 가리키는 xvalue reference를 반환해 바로 weak_ptr 이동 생성에 사용하며
        //     reference 자체를 closure에 저장하지 않는다.
        // (5) move 함수 자체는 observer를 바꾸지 않는다. 바로 뒤 실제 weak_ptr 이동 생성이 상태를
        //     이전한다. 일반적인 moved-from 객체는 유효하지만 값이 미지정일 수 있음을 전제로 써야 한다.
        // (6) O(1)의 형 변환, 무할당·noexcept·무효화 없음이고 수명을 연장하지 않는다. 같은 observer
        //     객체를 다른 thread가 동시에 읽거나 변경하지 않는다.
        // [같은 식의 첫 생성 계약: init-capture의 std::weak_ptr<EventGate> 이동 생성]
        // (1) source observer는 위 xvalue가 가리키는 유효한 weak_ptr이고 목적 캡처 멤버는 아직 없다.
        // (2) weak_ptr(weak_ptr&&) noexcept를 T=EventGate로 선택한다.
        // (3) std::move 결과인 weak_ptr xvalue에서 weak 관찰 handle 상태를 넘기며 strong count는 늘지 않는다.
        // (4) 생성자는 반환값 없이 closure의 observer 멤버 수명을 시작한다.
        // (5) 캡처 멤버가 같은 control block을 관찰하고 원본 observer는 유효한 empty 상태가 된다.
        //     즉 일반 이동 후의 미지정 상태보다 이 정확한 weak_ptr 계약이 더 강하다.
        // (6) O(1), 무할당·noexcept·무효화 없음이다. 원본은 안전하게 파괴할 수 있고 캡처 handle의
        //     병행 변경은 동기화해야 한다.
        // [이 식에서의 생성 계약: closure 이동에 따른 std::weak_ptr<EventGate> 이동 생성]
        // (1) function에 전달될 임시 closure는 유효한 weak handle 멤버를 가지며 저장 target은 아직 없다.
        // (2) closure의 암시적 move constructor가 weak_ptr(weak_ptr&&) noexcept를 다시 호출한다.
        // (3) 임시 closure는 xvalue로 전달되고 내부 weak_ptr의 관찰 handle 상태를 target으로 넘긴다.
        // (4) weak_ptr 생성자는 반환값 없이 function target의 weak 멤버 수명을 시작한다.
        // (5) target이 같은 control block을 관찰하고 임시 closure의 source weak_ptr은 empty가 된다.
        // (6) O(1), 무할당·noexcept·무효화 없음이며 strong count는 바뀌지 않는다. 임시 source는 식
        //     끝에 파괴되고 target handle의 병행 변경에는 외부 동기화가 필요하다.
        // [첫 생성 계약: std::function<bool(int)>의 callable 대상 생성]
        // (1) 생성자는 기존 수신 객체에 대한 멤버 호출이 아니다. 목적 EventCallback은 아직 없고
        //     아래 lambda closure prvalue는 bool(int)로 호출 가능하며 source observer는 non-empty다.
        // (2) template<class F> function(F&&)에서 F는 이름 없는 closure 타입으로 추론되며
        //     EventCallback의 signature template 인자는 bool(int)다.
        // (3) 유일한 명시 인자 식 `[observer = std::move(observer)](int event_code) noexcept { ... }`는
        //     closure prvalue로 F&&에 결합된다. int literal 입력은 생성 때 없고 invocation 때 전달한다.
        //     callback이 closure 값을 소유하고, init-capture는 weak 관찰권만 이동한다.
        // (4) 생성자 반환형/반환값은 없다. 완성된 non-empty EventCallback 반환 prvalue가 호출자의
        //     callback을 직접 초기화한다.
        // (5) 성공 뒤 function이 callable과 weak handle의 수명을 소유하고 임시 closure는 식 끝에 끝난다.
        //     strong owner 수는 바뀌지 않는다.
        // (6) target은 bool(int) 호출 가능하고 CopyConstructible이어야 한다. 처리 시간은 O(1)이며
        //     small-object 최적화 여부에 따라 할당/실패 예외가 가능하다. 예외 시 완성 function은 없고
        //     임시가 정리된다. 관찰자 무효화는 없으며 callback 병행 변경과 target 공유 상태를 동기화한다.
        return EventCallback{
            [observer = std::move(observer)](int event_code) noexcept {
                // [첫 호출 계약: std::weak_ptr<EventGate>::lock]
                // (1) 수신 observer는 closure가 소유한 유효한 const weak_ptr lvalue다. 대상은 살아 있거나
                //     마지막 owner 파괴 뒤 expired인 두 상태 중 하나다.
                // (2) std::shared_ptr<EventGate> lock() const noexcept를 선택한다.
                // (3) 명시 인자와 소유권 선이동은 없다. control block을 읽고 strong count 증가를 시도한다.
                // (4) 생존 시 공동 소유 shared_ptr, 만료 시 empty shared_ptr prvalue를 반환해 pinned를 만든다.
                // (5) 성공 때 호출 동안 strong count가 하나 늘고 observer는 그대로다. 실패 때 상태 변화 없이
                //     empty 결과가 생긴다. pinned 소멸 때 성공으로 더한 strong ownership이 해제된다.
                // (6) O(1), 무할당·noexcept이며 성공/만료 판정과 strong count 획득은 원자적으로 이뤄진다.
                //     iterator 무효화는 없다. 같은 weak_ptr 객체의 쓰기와 겹치지 않고, 별도 owner가 다른
                //     shared_ptr handle을 reset하는 것은 control block 규칙 안에서 안전하다.
                const std::shared_ptr<EventGate> pinned{observer.lock()};

                // [첫 호출 계약: std::shared_ptr<EventGate>::operator bool]
                // (1) 수신 pinned는 lock 결과인 유효한 const shared_ptr lvalue이며 empty일 수 있다.
                // (2) explicit operator bool() const noexcept를 선택하고 뒤의 !는 반환 bool의 built-in 부정이다.
                // (3) 명시 인자·소유권 이동은 없으며 pinned를 읽기만 한다.
                // (4) 저장 pointer가 null이 아니면 true인 bool prvalue를 반환해 if 조건에 사용한다.
                // (5) pointer, strong count, 대상 객체와 인자는 변하지 않는다.
                // (6) O(1), 무할당·noexcept·무효화 없음이다. bool은 대상 수명을 연장하지 않으며 같은
                //     shared_ptr 객체를 동시에 reset하지 않는다. 조건 분기가 empty 역참조를 막는다.
                if (!pinned) {
                    return false;
                }

                // [첫 호출 계약: std::shared_ptr<EventGate>::operator->]
                // (1) 수신 pinned는 위 분기로 non-empty가 보장된 const shared_ptr<EventGate> lvalue다.
                // (2) element_type* operator->() const noexcept를 선택하며 element_type은 EventGate다.
                // (3) 명시 인자는 없고 소유권 이동 없이 저장된 pointer를 관찰한다.
                // (4) 유효한 EventGate*를 반환해 사용자 함수 accepts의 this로 즉시 사용한다.
                // (5) shared_ptr, strong count와 EventGate 상태는 operator-> 자체로 바뀌지 않는다.
                // (6) O(1), 무할당·noexcept·무효화 없음이다. null 결과를 멤버 호출에 쓰면 UB지만 앞의
                //     검사가 배제한다. 반환 raw pointer는 pinned보다 오래 저장하지 않고 병행 쓰기도 없다.
                return pinned->accepts(event_code);
            }};
    }

private:
    // 뒤 const는 논리 상태를 바꾸지 않는다는 뜻이고 noexcept는 예외가 경계를 넘지 않음을 타입에 적는다.
    // int는 작은 기본 값 매개변수라 복사하며 reference/pointer 수명 조건을 호출자에게 만들지 않는다.
    [[nodiscard]] bool accepts(int event_code) const noexcept {
        // 정수 두 개를 load해 비교한 bool prvalue를 반환한다. >=는 기본 타입 연산이라 STL 호출이 아니다.
        return event_code >= minimum_code_;
    }

    // {}는 int를 0으로 값 초기화하고, 실제 생성 시 멤버 초기화 목록이 전달값으로 덮어 초기화한다.
    int minimum_code_{};
};

int main() {
    // create(10)의 10은 int prvalue다. 반환 shared_ptr prvalue가 owner를 직접 초기화해 별도 복사/이동은
    // 없다. 이름이 붙은 owner는 lvalue이고 EventGate의 강한 소유자다. T*는 null 가능 비소유 주소,
    // T&는 null일 수 없는 별칭이며 둘 다 소유권을 늘리지 않지만 shared_ptr<T>는 여기서 수명을 소유한다.
    std::shared_ptr<EventGate> owner{EventGate::create(10)};

    // owner->의 표준 operator-> 계약은 위 첫 계약과 같다. factory 불변식상 non-empty이고 사용자 함수가
    // 반환한 EventCallback prvalue가 callback을 직접 초기화한다. callback은 weak ownership만 가진다.
    EventCallback callback{owner->make_callback()};

    // [첫 호출 계약: std::function<bool(int)>::operator()]
    // (1) 수신 callback은 weak-capturing lambda를 보유한 non-empty EventCallback lvalue다.
    // (2) bool function<bool(int)>::operator()(int) const를 선택한다.
    // (3) 12는 허용되는 int prvalue이며 값으로 target에 전달된다. owner 소유권은 인자로 넘기지 않는다.
    // (4) target이 EventGate를 pin하고 판정한 bool을 반환하며 before_reset을 초기화하는 데 사용한다.
    // (5) 첫 호출에서는 owner가 살아 있어 임시 pinned가 호출 동안 strong count를 늘렸다가 반환 시 줄인다.
    //     callback과 owner의 장기 소유 상태는 그대로다.
    // (6) type-erased dispatch 자체는 O(1)이고 이 target 전체도 O(1), 호출 중 별도 할당은 없다.
    //     non-empty가 전제이며 empty라면 예외가 난다. target은 noexcept라 사용자 예외가 없고 관찰자
    //     무효화도 없다. 같은 callback을 병행 호출하려면 target의 공유 상태 안전성도 확인해야 한다.
    const bool before_reset{callback(12)};

    // [첫 호출 계약: std::shared_ptr<EventGate>::reset]
    // (1) 수신 owner는 EventGate의 마지막 strong owner인 non-empty shared_ptr lvalue다. 직전 callback의
    //     지역 pinned는 이미 파괴되었고 callback에는 weak_ptr만 남았다.
    // (2) void reset() noexcept 무인자 overload를 선택한다.
    // (3) 명시 인자나 새 소유권은 없다.
    // (4) 반환형 void라 결과 값은 없고 파괴 여부는 뒤 callback 결과로 검증한다.
    // (5) owner가 empty가 되고 strong count가 0이어서 EventGate 수명이 끝난다. control block은 callback의
    //     weak handle 때문에 남지만 객체는 파괴되며 기존 raw pointer/reference는 모두 dangling이다.
    // (6) control-block 갱신 자체는 O(1)이고 여기서는 상수 크기 소멸도 수행한다. 무할당·noexcept다.
    //     대상 소멸은 해당 관찰자를 무효화하지만 weak_ptr은 안전한 expired 상태가 된다. 같은 owner
    //     객체를 다른 thread가 동시에 읽거나 reset하면 data race이므로 그런 공유에는 별도 동기화가 필요하다.
    owner.reset();

    // 같은 std::function 호출 계약을 재사용한다. 이번 lock은 empty shared_ptr을 돌려 안전하게 false다.
    const bool after_reset{callback(12)};

    // [첫 호출 계약: std::cout의 operator<< 연쇄]
    // (1) 왼쪽 수신은 유효한 std::ostream lvalue std::cout이며 이 프로그램의 main thread만 쓴다.
    // (2) bool에는 basic_ostream<char>::operator<<(bool), 문자에는
    //     operator<<(basic_ostream<char>&, char) overload가 선택된다.
    // (3) 두 bool은 const lvalue에서 값으로 읽고 ' ', '\n'은 char prvalue다. 소유권 이동은 없다.
    // (4) 각 호출은 같은 ostream&를 반환해 다음 <<의 수신으로 쓰고 마지막 반환 참조는 버린다.
    // (5) 기본 bool 숫자 형식에서 정확히 "1 0\n"을 기록하고 callback 상태는 변하지 않는다.
    // (6) 표준은 장치 I/O 연쇄 전체의 이식 가능한 단일 복잡도나 내부 할당 횟수를 고정하지 않는다.
    //     실패는 상태 bit와 설정된 예외로 보고된다. 다른 객체 관찰자를 무효화하지 않지만 여러 thread의
    //     복합 레코드 원자성은 보장하지 않는다.
    std::cout << before_reset << ' ' << after_reset << '\n';

    // 값 범주·수명 정리: 이름 있는 owner/callback은 lvalue, factory와 lock의 결과는 prvalue다.
    // std::move(owner) 같은 식을 썼다면 owner lvalue를 xvalue로 바꿔 shared_ptr 이동 생성에 결합하지만,
    // 오늘 weak observer에는 실제로 써 init-capture 멤버를 만들었다. 그 뒤 closure도 function 안으로
    // 이동된다. shared owner 자체는 이동하지 않으며 반환 prvalue에는 보장 복사 생략이 적용되고 local
    // 반환의 NRVO와 구별한다. problem.cpp의 lvalue copy-capture와 비교하면 차이가 더 선명하다.
    //
    // 기계 실행 관점에서 weak lock은 control block의 strong count load, 원자적 갱신 시도, 성공/만료 비교와
    // 조건 분기를 만들 수 있다. reset은 count store/감소와 마지막-owner 분기를 만들 수 있고 std::function의
    // type erasure는 함수 pointer 같은 간접 호출이 될 수 있으나 C++ virtual 호출이라고 단정할 수는 없다.
    // compiler가 인라인·간접 호출 제거를 할 수도 있으며 정확한 명령과 비용은 CPU, ABI, 표준 라이브러리,
    // compiler와 최적화 옵션에 따라 달라진다.
    //
    // 초보자 검증: callback이 shared_ptr을 캡처했다면 reset 뒤 EventGate가 살아 `after_reset==1`이 된다.
    // 직접 해보기: weak capture를 유지한 채 첫 입력을 9로 바꾸어 출력이 "0 0"인지 확인해 보자.
    return 0;
}
