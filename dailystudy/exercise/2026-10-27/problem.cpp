#include <iostream>    // std::cout으로 결정적인 연습 결과를 출력한다.
#include <stop_token>  // std::stop_source, std::stop_token, std::stop_callback을 제공한다.
#include <utility>     // std::move로 callback closure를 xvalue로 전달한다.

// using 별칭은 새 타입을 만들지 않는다. 여기서는 기본 타입 int의 역할을 더 분명히 읽게 한다.
using Score = int;

// struct는 기본 public이며 검증 결과처럼 단순한 데이터 묶음에 알맞다. class는 반대로 기본 private다.
// public/private 접근 지정자는 compile-time 인터페이스 경계이지 runtime 보안 장치가 아니다.
// 이 aggregate에는 사용자 정의 생성자가 없어서 멤버 초기화 목록도 없다. 생성자가 하나의 값을 받는 class라면
// explicit으로 뜻하지 않은 암시 변환을 막고 `: member_{argument}` 목록으로 본문보다 먼저 멤버를 초기화한다.
struct ExerciseResult {
    bool first_request{};   // {}는 bool을 false로 값 초기화한다.
    bool second_request{};
    Score score{};          // {}는 int를 0으로 값 초기화한다.
};

// 반환형 ExerciseResult인 함수다. 매개변수가 없으므로 실행 상태는 모두 지역 객체가 소유한다.
[[nodiscard]] ExerciseResult run_exercise() {
    // [첫 호출 계약: std::stop_source 기본 생성]
    // (1) 수신 source는 아직 수명이 시작되지 않은 std::stop_source 객체다.
    // (2) stop_source() 기본 생성자를 선택하며 nostopstate_t overload와 템플릿 인자는 없다.
    // (3) 위치 인자는 없고 빈 중괄호는 기본 생성 문법이다.
    // (4) 생성자는 반환값이 없으며 source라는 이름 있는 lvalue 객체의 수명을 시작한다.
    // (5) source는 새 공유 stop-state와 요청 권한을 소유하고 초기 requested 상태는 false다.
    // (6) 전제조건은 없다. 표준은 복잡도 상한을 별도 규정하지 않으며 공유 상태 할당 실패 시
    //     bad_alloc이 가능하고, 상태 수명은 모든 관련 handle이
    //     사라질 때 끝난다. 생성 시점에는 다른 실행 흐름에 공개되지 않아 데이터 경쟁이 없다.
    std::stop_source source{};

    // [첫 호출 계약: std::stop_source::get_token + std::stop_token 직접 초기화]
    // (1) 수신 source는 미요청 공유 상태를 가진 std::stop_source lvalue이고 token은 아직 없다.
    // (2) stop_token get_token() const noexcept가 선택되고 같은 타입 prvalue로 token을 직접 초기화한다.
    // (3) 위치 인자는 없다. source는 const this로 빌리며 요청 권한은 source에 남는다.
    // (4) get_token은 std::stop_token prvalue를 반환해 지역 token 자체를 만든다. 반환값을 보존한다.
    // (5) source는 변하지 않고 token은 같은 상태를 관찰·공유해 내부 상태 수명을 유지한다.
    // (6) 이 호출은 noexcept지만 표준은 복잡도 상한과 내부 추가 할당 여부를 따로 규정하지 않는다.
    //     기존 handle을 무효화하지 않으며 source/token wrapper 자체의 비동기 접근은 동기화해야 한다.
    const std::stop_token token{source.get_token()};

    Score score{};
    auto add_one = [&score]() noexcept { ++score; };
    auto add_ten = [&score]() noexcept { score += 10; };

    // decltype은 평가 없이 closure 타입을 얻는다. stop_callback의 템플릿 인자는 저장할 정확한 callable 타입이다.
    using OneCallback = std::stop_callback<decltype(add_one)>;
    using TenCallback = std::stop_callback<decltype(add_ten)>;

    // [첫 호출 계약: std::move + std::stop_callback<Callback> 생성]
    // (1) one/tens는 아직 없는 callback 객체다. token은 미요청 상태를 관찰하고 두 closure는 유효한 lvalue다.
    // (2) std::move<T>(T&&) noexcept에서 각 T는 closure&로 추론되어 xvalue를 만든다. 이어
    //     template<class C> stop_callback(const stop_token&, C&&)가 각 저장 Callback 타입으로 선택된다.
    // (3) token은 const stop_token lvalue로 상태를 공유한다. move 결과들은 closure xvalue이며 score의
    //     reference를 담은 callable 소유권만 callback으로 넘긴다. reference는 pointer와 달리 null을 표현하지
    //     않지만 둘 다 대상 수명을 늘리지 않는다. score 자체는 계속 이 함수가 소유한다.
    // (4) move는 rvalue reference를 반환해 생성 인자로 쓰고, 두 생성자는 반환값 없이 객체 수명을 시작한다.
    // (5) one/tens가 같은 상태에 각각 등록된다. 원본 closure는 moved-from이지만 사용하지 않는다.
    //     아직 요청 전이므로 score는 0이고 callback 실행 순서는 등록 순서로 보장되지 않는다.
    // (6) token 객체는 두 생성자 호출 동안만 유효하면 되고, 등록된 callback들이 stop-state 소유권을
    //     공유한다. 빌린 score는 callback 실행/소멸까지 살아야 한다. closure 이동/호출이 noexcept라
    //     생성도 noexcept다. 표준은 등록 복잡도·할당 상한을 별도 규정하지 않는다. callback 객체는
    //     copy/move 불가이며, 동시 실행이면 score 접근을 동기화해야 하지만 여기서는 한 스레드다.
    OneCallback one{token, std::move(add_one)};
    TenCallback ten{token, std::move(add_ten)};

    // [첫 호출 계약: std::stop_source::request_stop]
    // (1) 수신 source는 두 callback이 등록된 미요청 std::stop_source lvalue다.
    // (2) bool request_stop() noexcept가 선택되며 인자/템플릿 인자는 없다.
    // (3) 숨은 this 외 위치 인자는 없다. source는 요청 권한을 계속 소유한다.
    // (4) 첫 false->true 전이는 true, 반복 요청은 false를 반환하고 두 결과 모두 저장한다.
    // (5) 첫 호출은 one/ten을 반환 전에 각각 정확히 한 번 실행해 순서와 무관하게 score=11을 만든다.
    //     두 번째 호출은 상태나 score를 바꾸지 않는다.
    // (6) 성공 호출 시간은 callback 수/실행 시간에 의존하고 표준 복잡도 보장은 없다. 함수는 noexcept이며
    //     callback 예외 이탈은 terminate다. 상태 전이는 thread-safe이고 동시 요청 중 하나만 true다.
    const bool first{source.request_stop()};
    const bool second{source.request_stop()};

    auto add_hundred = [&score]() noexcept { score += 100; };
    using LateCallback = std::stop_callback<decltype(add_hundred)>;

    // [호출 계약: 이미 요청된 token에 늦은 callback 등록]
    // (1) late는 아직 없고 token은 requested=true인 상태를 관찰하며 score는 11이다.
    // (2) stop_callback(const stop_token&, C&&)에서 Callback/C는 add_hundred closure 타입이다.
    // (3) token은 const lvalue이고 std::move(add_hundred)는 closure xvalue다. score는 reference로만 빌린다.
    // (4) 생성자는 결과를 반환하지 않는다. 생성 완료 전 현재 실행 흐름에서 callable을 한 번 실행한다.
    // (5) score는 즉시 111이 되며 late는 미래 요청에서 다시 실행될 등록 상태로 남지 않는다.
    // (6) score 수명이 충분하고 callback은 noexcept다. 다른 스레드가 score를 동시에 만지면 데이터 경쟁이나,
    //     여기서는 단일 실행 흐름이다. 소멸 시 실행 중인 다른 스레드가 있다면 완료를 기다릴 수 있다.
    LateCallback late{token, std::move(add_hundred)};

    // ExerciseResult{...}는 prvalue다. 반환 객체를 직접 초기화할 수 있어 같은 타입 임시의 복사/이동은
    // C++17 보장 복사 생략 대상이다. first/second/score 이름은 모두 lvalue이며 그 값을 복사해 넣는다.
    return ExerciseResult{first, second, score};
}

int main() {
    // run_exercise()의 prvalue 결과가 result를 직접 초기화한다. const reference는 쓰지 않고 값으로 소유한다.
    const ExerciseResult result{run_exercise()};

    // 이해를 증명하는 초보자 검증 단계: 첫 요청만 true이고, 두 초기 callback과 늦은 callback이 한 번씩
    // 실행되어 1+10+100이 되었는지 실제 식으로 검사한다. ||는 왼쪽이 true면 오른쪽을 생략한다.
    if (!result.first_request || result.second_request || result.score != 111) {
        return 1;
    }

    // [첫 호출 계약: std::basic_ostream<char> operator<< 연쇄]
    // (1) 수신 std::cout은 초기화된 std::ostream lvalue이고 출력 가능한 상태라고 기대한다.
    // (2) const char* 비멤버 overload, bool/int 멤버 산술 overload, char overload가 순서대로 선택된다.
    // (3) 문자열 literal은 정적 수명의 null 종료 char 배열에서 비소유 포인터로 변환된다. bool/int는 값으로
    //     읽고 '\n'은 char prvalue다. 포인터는 null이면 안 되고 유효한 null 종료 문자열이어야 한다.
    // (4) 각 overload는 같은 ostream&를 반환해 다음 호출에 쓰며 최종 reference는 버린다.
    // (5) cout buffer/상태가 갱신되고 result는 변하지 않는다. 기본 bool 서식은 1/0이고 '\n'은 강제 flush가 아니다.
    // (6) 출력 문자 수에 선형이며 buffer 할당/장치 I/O가 가능하다. 실패는 state bit와 exceptions mask에 따라
    //     ios_base::failure가 될 수 있다. 여러 스레드의 compound 출력 순서는 별도 동기화가 필요하다.
    std::cout << "first=" << result.first_request
              << " second=" << result.second_request
              << " score=" << result.score << '\n';

    // 기계 실행 관점에서 bool/score 검사는 load·compare·조건 분기가 될 수 있고, request_stop은 보통 원자적
    // 상태 전이와 callback 간접 호출을 포함한다. 정확한 명령, inline, 분기 제거 여부는 CPU·표준 라이브러리·
    // compiler·최적화 옵션에 따라 달라 특정 assembly를 단정할 수 없다.
    return 0;
}
