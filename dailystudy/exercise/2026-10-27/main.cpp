#include <iostream>    // std::cout 표준 출력 객체와 문자열/정수 출력 연산자를 선언한다.
#include <stop_token>  // C++20 std::stop_source, std::stop_token, std::stop_callback을 선언한다.
#include <utility>     // std::move로 이름 있는 객체를 xvalue 식으로 바꾼다.

// 이 예제의 아키텍처 역할은 세 가지로 나뉜다.
// 1) CancellationCoordinator는 중지 상태의 소유자이자 "중지를 요청하는 포트"다.
// 2) std::stop_token은 그 상태를 소유하지 않는다고 생각하기 쉽지만, 실제로는 공유 중지 상태의
//    수명을 함께 유지하는 값 타입 관찰자다. 다만 요청 권한은 없다.
// 3) std::stop_callback은 등록한 callable과 등록 수명을 소유한다. 객체가 살아 있는 동안만
//    요청과 연결되며, 성공한 첫 요청은 등록된 callable을 request_stop() 안에서 동기 실행한다.

// using은 새 정수 타입을 만들지 않고 기존 기본 타입 int에 도메인 이름을 붙인다.
using CleanupCount = int;

// struct의 기본 접근 지정자는 public이다. 작은 실행 결과/상태 묶음에 적합하다.
struct CleanupLedger {
    CleanupCount callback_runs{};  // 중괄호 초기화는 기본 타입 int를 0으로 값 초기화한다.
    bool resource_open{true};      // bool은 참/거짓을 저장하며 자원이 처음에는 열려 있음을 뜻한다.

    // 반환형 void는 결과 값을 만들지 않는다는 뜻이고, noexcept는 예외가 빠져나오지 않음을 약속한다.
    void close_once() noexcept {
        // if는 비교 결과가 true일 때만 중괄호 블록을 실행한다. 블록은 지역 수명 범위이기도 하다.
        if (resource_open) {
            resource_open = false;
            ++callback_runs;  // 전위 ++는 저장된 int를 1 증가시키고 증가한 lvalue를 만든다.
        }
    }
};

// class의 기본 접근 지정자는 private이다. 공유 중지 상태를 외부가 마음대로 교체하지 못하게 한다.
class CancellationCoordinator {
public:
    // 생성자는 반환형이 없다. explicit은 stop_source 하나가 coordinator로 암시 변환되는 일을 막는다.
    explicit CancellationCoordinator(std::stop_source source) noexcept
        // [첫 호출 계약: std::move(source) + std::stop_source 이동 생성]
        // (1) 수신 객체는 아직 수명이 시작되지 않은 private 멤버 source_이며 정확한 타입은
        //     std::stop_source다. 매개변수 source는 유효한 이름 있는 std::stop_source lvalue다.
        // (2) std::move는 <utility>의 template<class T> remove_reference_t<T>&& move(T&&) noexcept에서
        //     T=std::stop_source&로 선택된다. 이어 stop_source(stop_source&&) noexcept가 선택된다.
        // (3) 인자 source는 std::move 전에는 lvalue, 결과 식은 std::stop_source&& xvalue다. 공유 상태
        //     소유권을 멤버로 옮기려는 의미이며 null 포인터나 범위 값 같은 추가 허용값 제약은 없다.
        // (4) move는 xvalue reference를 반환해 이동 생성에 사용되고, 이동 생성자는 별도 반환값이 없다.
        // (5) source_는 source가 가리키던 공유 중지 상태를 갖는다. 매개변수 source는 연관 상태가 없는
        //     유효한 moved-from 객체가 되고 함수 끝에서 안전하게 파괴된다.
        // (6) 전제조건은 source가 유효한 객체라는 것뿐이다. move cast는 O(1)·noexcept이고,
        //     stop_source 이동은 noexcept지만 표준이 별도 복잡도·할당 상한을 명시하지 않는다.
        //     기존 token/callback은 공유 상태 자체를 계속 가리키며 무효화되지 않는다.
        //     아직 다른 스레드에 공개되지 않은 생성 중 객체이므로 이 식 자체의 데이터 경쟁은 없다.
        : source_{std::move(source)} {}

    // const 멤버 함수는 coordinator의 관찰 가능한 상태를 바꾸지 않는다.
    [[nodiscard]] std::stop_token token() const noexcept {
        // [첫 호출 계약: std::stop_source::get_token]
        // (1) 수신자는 const std::stop_source lvalue인 source_이며, 유효한 공유 중지 상태를 가진다.
        // (2) 선택된 시그니처는 std::stop_token get_token() const noexcept다. 템플릿 인자는 없다.
        // (3) 위치 인자는 없다. 숨은 this는 const 객체를 비소유로 가리키며 source_를 변경하지 않는다.
        // (4) std::stop_token prvalue를 반환하고 호출자가 자신의 token 값으로 직접 초기화한다.
        // (5) source_는 그대로이고 반환 token은 같은 공유 중지 상태를 관찰하며 그 상태의 수명을 유지한다.
        // (6) 이 호출은 noexcept지만 표준은 복잡도 상한이나 내부 추가 할당 여부를 따로 규정하지 않는다.
        //     source_나 기존 관찰자를 무효화하지 않는다. 서로 다른 stop_source/token 객체의 동시 관찰은 허용되지만 같은 wrapper 객체의 비동기
        //     파괴와 접근 수명은 호출자가 동기화해야 한다.
        return source_.get_token();
    }

    [[nodiscard]] bool request_stop() noexcept {
        // [첫 호출 계약: std::stop_source::request_stop]
        // (1) 수신자는 유효한 공유 중지 상태를 가진 std::stop_source lvalue source_다.
        // (2) 선택된 시그니처는 bool request_stop() noexcept다. 인자와 템플릿 인자는 없다.
        // (3) 위치 인자는 없고, 숨은 this는 요청 권한을 가진 source_를 가리킨다.
        // (4) 이 호출이 false->true 전이를 처음 수행했으면 true, 이미 요청됐으면 false를 반환하며
        //     반환값은 호출자가 중복 요청 여부를 기록하는 데 사용한다.
        // (5) 첫 성공 호출은 요청 상태를 영구히 true로 만들고 당시 등록 callback을 호출이 끝나기 전에
        //     동기 실행한다. 반복 호출은 상태와 callback 실행 횟수를 바꾸지 않는다.
        // (6) 연관 stop-state가 있어야 true가 가능하다. 성공 호출 비용은 등록 callback 수/실행 시간에
        //     의존하고 표준은 별도 복잡도·할당 상한을 주지 않는다. 함수 자체는 noexcept이며 callback이
        //     예외를 내보내면 std::terminate다. 요청 상태 갱신은 thread-safe이고 동시 요청 중 하나만 true다.
        return source_.request_stop();
    }

private:
    // 이 멤버 하나가 "요청 권한"을 소유한다. token/callback은 같은 내부 stop-state를 공유한다.
    std::stop_source source_;
};

int main() {
    CleanupLedger ledger{};  // 이름 있는 ledger는 lvalue이며 main 끝까지 실제 정리 상태를 소유한다.

    // [첫 호출 계약: std::stop_source 기본 생성]
    // (1) 수신 객체는 아직 존재하지 않는 std::stop_source 임시 객체다.
    // (2) 선택된 시그니처는 stop_source()이며 nostopstate_t overload가 아닌 기본 생성자다.
    // (3) 위치 인자와 템플릿 인자는 없다. 빈 중괄호는 기본 생성과 값 초기화 문법을 요청한다.
    // (4) 생성자는 반환값이 없고 식 std::stop_source{}는 새 임시 객체인 prvalue다.
    // (5) 임시는 새 공유 중지 상태와 요청 권한을 갖고 coordinator 값 매개변수를 직접 초기화한 뒤,
    //     그 매개변수에서 coordinator.source_로 이동된다.
    // (6) 전제조건은 없다. 표준은 복잡도 상한을 별도 규정하지 않으며 공유 상태 할당 실패 시
    //     std::bad_alloc이 가능하다. 생성한 상태는
    //     마지막 source/token/callback이 파괴될 때 정리된다. 아직 공유되지 않아 데이터 경쟁은 없다.
    CancellationCoordinator coordinator{std::stop_source{}};

    // [첫 호출 계약: std::stop_token을 coordinator.token()의 prvalue로 직접 초기화]
    // (1) 수신 객체 token은 아직 수명이 시작되지 않은 const std::stop_token이다.
    // (2) token()이 반환한 같은 타입 prvalue로 직접 초기화한다. C++17 보장 복사 생략이 적용되는
    //     초기화이고 별도 copy/move constructor 호출이 필수적이지 않다.
    // (3) coordinator는 이름 있는 CancellationCoordinator lvalue이고, 호출 중 const reference인 this로
    //     빌려진다. 반환 token은 stop-state를 공유하지만 요청 권한은 없다.
    // (4) token()의 반환형은 std::stop_token이며 그 값을 지역 객체 자체로 사용한다.
    // (5) coordinator는 변하지 않고 token 수명이 시작된다. 둘은 같은 stop-state 수명을 함께 유지한다.
    // (6) token()은 noexcept지만 표준 복잡도 상한과 내부 추가 할당 여부는 따로 보장되지 않는다.
    //     지역 token은 coordinator보다 나중에 만들어져 먼저 파괴된다.
    //     같은 token wrapper를 다른 스레드에서 파괴하면서 읽지 않는다는 일반 객체 수명 전제가 있다.
    const std::stop_token token{coordinator.token()};

    // 람다의 &ledger 캡처는 소유권 없는 reference를 closure 안에 저장한다. pointer와 달리 null 상태를
    // 표현하지 않지만 수명을 연장하지도 않는다. callback보다 ledger가 오래 살아야 한다.
    auto cleanup_action = [&ledger]() noexcept {
        ledger.close_once();
    };

    // decltype(cleanup_action)은 컴파일러가 만든 고유 closure 타입이다. using은 그 타입을 인자로 넣은
    // stop_callback 특수화에 읽을 수 있는 별칭을 붙인다. stop_callback은 copy/move가 모두 금지된다.
    using CleanupCallback = std::stop_callback<decltype(cleanup_action)>;

    bool first_request{};
    bool second_request{};
    {
        // [첫 호출 계약: std::stop_callback<closure> 생성]
        // (1) 수신 객체 cleanup은 아직 없는 CleanupCallback이다. token은 stop 미요청 상태를 관찰하고,
        //     cleanup_action은 &ledger를 가진 유효한 이름 있는 closure lvalue다.
        // (2) 선택 형식은 template<class C> explicit stop_callback(const stop_token&, C&&)이며 저장
        //     Callback=decltype(cleanup_action), C=decltype(cleanup_action)이다.
        // (3) 첫 인자 token은 const std::stop_token lvalue로 공유 상태를 빌린다. 두 번째 인자
        //     std::move(cleanup_action)은 closure xvalue이고 저장 callable로 이동한다. 이동은 캡처한
        //     ledger reference의 대상을 옮기지 않으며 ledger 소유권도 넘기지 않는다.
        // (4) 생성자는 반환값이 없고 cleanup 객체 수명을 시작한다. std::move는 xvalue reference를
        //     반환해 이 생성자 인자로 사용한다.
        // (5) cleanup은 callable을 직접 소유하고 token의 stop-state에 등록된다. cleanup_action은 유효한
        //     moved-from closure지만 다시 쓰지 않는다. 아직 요청 전이므로 callback은 실행되지 않는다.
        // (6) token 객체는 생성자 호출 동안만 유효하면 되고, 등록된 cleanup이 stop-state 소유권을
        //     공유한다. 반면 빌린 ledger는 callback 실행/소멸이 끝날 때까지 살아야 한다. callable 이동이
        //     noexcept라 이 생성도 noexcept이며, 등록/해제 복잡도·할당 방식은 표준이 별도 보장하지 않는다.
        //     소멸자는 등록을 해제하고 다른 스레드 실행 중이면 완료를 기다릴 수 있다. 예외 이탈은 terminate다.
        CleanupCallback cleanup{token, std::move(cleanup_action)};

        // 첫 호출은 callback을 이 함수가 반환하기 전에 정확히 한 번 실행한다. 반환 bool을 버리지 않는다.
        first_request = coordinator.request_stop();
        // 요청 상태는 단조롭다. 두 번째 호출은 false이며 같은 cleanup을 다시 실행하지 않는다.
        second_request = coordinator.request_stop();
    }  // cleanup 소멸자는 이미 실행·등록 해제된 callback을 안전하게 정리한다.

    CleanupCount late_runs{};
    auto late_action = [&late_runs]() noexcept {
        ++late_runs;
    };
    using LateCallback = std::stop_callback<decltype(late_action)>;

    // [호출 계약: 이미 중지된 token에 std::stop_callback을 늦게 등록]
    // (1) 수신 late는 아직 없는 LateCallback이고 token은 stop_requested()==true인 상태를 관찰한다.
    // (2) 같은 stop_callback(const stop_token&, C&&) 템플릿에서 Callback/C는 late_action closure 타입이다.
    // (3) token은 const lvalue이며 요청 권한 없이 stop-state 수명을 공유하는 관찰 handle이다.
    //     std::move(late_action)은 closure xvalue다. 캡처한
    //     late_runs lvalue reference는 빌릴 뿐 소유하거나 수명을 늘리지 않는다.
    // (4) 생성자 반환값은 없다. 생성이 끝나기 전에 callback이 현재 main 실행 흐름에서 한 번 실행된다.
    // (5) late_runs는 즉시 1이 된다. 요청은 이미 완료됐으므로 late는 실행 대기 등록으로 남지 않는다.
    // (6) late_runs가 생성/소멸·즉시 실행 동안 유효해야 한다. callback은 noexcept다. 다른 스레드와 같은
    //     int를 동기화 없이 읽거나 쓰면 데이터 경쟁이지만 이 예제는 단일 스레드라 안전하다.
    LateCallback late{token, std::move(late_action)};

    // [첫 호출 계약: std::basic_ostream<char>::operator<< overload 연쇄]
    // (1) 수신 std::cout은 초기화된 std::ostream lvalue이며 badbit가 설정되지 않았다고 기대한다.
    // (2) const char*에는 비멤버 character-string 출력 overload, bool/int에는 ostream 멤버 산술
    //     overload, '\n'에는 char overload가 차례로 선택된다. 템플릿 char 타입은 char다.
    // (3) 문자열 literal은 const char 배열 lvalue에서 포인터로 변환되며 정적 수명·비소유다. bool과 int는
    //     값으로 읽고, '\n'은 char prvalue다. C 문자열은 null 종료돼 있어야 한다.
    // (4) 각 호출은 같은 std::ostream&를 반환해 다음 <<의 수신자로 사용한다. 최종 반환 reference는 버린다.
    // (5) cout의 서식/오류 상태와 외부 출력 장치가 갱신되고 인자 값은 변하지 않는다. '\n'은 flush를
    //     강제하지 않는다. 기본 boolalpha=false라 bool은 1/0으로 출력된다.
    // (6) 출력량에 선형이고 stream 내부 buffering이 가능하다. 실패는 상태 bit를 세우며 exceptions mask에
    //     따라 ios_base::failure를 던질 수 있다. 같은 stream에 대한 동시 compound 출력 순서는 별도
    //     동기화가 필요하다. literal/지역 값의 reference를 저장하지 않아 수명 무효화는 없다.
    std::cout << "first=" << first_request
              << " second=" << second_request
              << " cleanup=" << ledger.callback_runs
              << " open=" << ledger.resource_open << '\n';
    std::cout << "late=" << late_runs << '\n';

    // 값 범주 요약: coordinator/token/ledger처럼 이름 있는 식은 lvalue, std::stop_source{}는 prvalue,
    // std::move(cleanup_action)는 xvalue다. 이동은 cast가 아니라 뒤따르는 stop_callback 생성자가 한다.
    // token 반환 prvalue는 지역 token에 직접 구성될 수 있어 불필요한 복사/이동이 없다(RVO/보장 복사 생략).
    // 기계 실행 관점에서 request_stop은 보통 공유 상태의 원자적 비교/저장과 조건 분기, 등록 callback으로의
    // 간접 호출을 포함할 수 있다. close_once는 load·비교·조건 분기·store가 될 수 있다. 실제 명령과
    // inline 여부는 CPU, 표준 라이브러리 구현, compiler, 최적화 옵션에 따라 달라 특정 assembly를 단정하지 않는다.
    return 0;
}
