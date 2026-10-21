// 초보자 실습: optional 값이 있을 때만 계산하는 분기를 C++23 transform으로 표현한다.
// 먼저 결과를 예측한 뒤 `score * 2`를 `score + 10`으로 바꾸고 출력도 직접 검증해 본다.

// <iostream>은 std::cout과 출력 삽입 연산을 제공한다.
#include <iostream>
// <optional>은 선택적 값 std::optional과 빈 상태 표식 std::nullopt를 제공한다.
#include <optional>

// int는 작은 기본 타입이라 값으로 받고 돌려주면 별도 pointer/reference 수명 조건이 생기지 않는다.
[[nodiscard]] std::optional<int> find_score(int learner_id) noexcept {
    // if는 비교 결과가 true일 때만 중괄호 block을 실행하는 조건 제어문이다.
    if (learner_id == 7) {
        // [첫 생성 계약: int에서 std::optional<int> 값 생성]
        // (1) 목적 optional은 아직 없고 21은 int prvalue다.
        // (2) template<class U=T> constexpr optional(U&&)에서 T=int, U=int를 선택한다.
        // (3) 21은 허용되는 모든 int 중 하나이고 값만 contained storage로 전달하며 외부 소유권이 없다.
        // (4) 생성자는 별도 반환값 없이 값 21을 가진 optional<int> prvalue를 함수 결과로 만든다.
        // (5) optional 안 int의 수명이 시작되고 외부 상태는 변하지 않는다.
        // (6) O(1), 무할당이며 int 생성은 예외가 없다. 참조 무효화·공유 상태·data race가 없다.
        return 21;
    }

    // [첫 생성 계약: std::optional<int>(std::nullopt_t)]
    // (1) 목적 optional은 아직 없고 std::nullopt는 표준의 불변 nullopt_t 상수 객체다.
    // (2) constexpr optional(nullopt_t) noexcept를 T=int로 선택한다.
    // (3) 빈 상태 태그만 값으로 전달하며 int나 외부 자원을 소유하지 않는다.
    // (4) 생성자는 별도 반환값 없이 disengaged optional<int> prvalue를 반환값으로 만든다.
    // (5) contained int는 생성되지 않고 인자 상태도 바뀌지 않는다.
    // (6) O(1), 무할당, noexcept이며 참조 무효화나 thread 공유 상태가 없다.
    return std::nullopt;
}

// 함수 반환형은 optional<int>다. "학생 없음"과 "점수 0"을 서로 다른 상태로 유지한다.
[[nodiscard]] std::optional<int> doubled_score(int learner_id) {
    // [첫 호출 계약: std::optional::transform]
    // (1) 수신 객체는 find_score가 반환한 유효한 optional<int> prvalue다. engaged 또는 empty일 수 있다.
    // (2) template<class F> constexpr auto transform(F&&) && overload를 선택한다. F는 아래 lambda의
    //     closure 타입이고 U=int이므로 반환형은 optional<int>다.
    // (3) 상태 없는 lambda prvalue가 F&&에 결합된다. 값이 있을 때만 contained int를 int&&로 받아
    //     값 복사로 score를 초기화한다. 모든 int 값이 허용되고 외부 소유권은 없다.
    // (4) 값이 있으면 lambda 결과를 가진 optional<int>, 없으면 빈 optional<int>를 반환해 함수 결과로 쓴다.
    // (5) 임시 입력 optional은 full-expression 끝에 파괴된다. 외부 상태는 없고 lambda 호출 횟수는 0 또는 1이다.
    // (6) O(1), 무할당이다. signed int 곱셈이 표현 범위를 넘으면 UB지만 여기 가능한 값은 21이라 안전하다.
    //     일반 transform은 callable/U 생성 예외를 잡지 않지만 member 시그니처 자체도 noexcept는 아니다.
    //     이 정확한 lambda는 noexcept이고 안전한 int 연산만 하므로 정상 실행의 throw 경로는 없다. 만약
    //     noexcept callable 밖으로 예외가 빠져나오면 전파되지 않고 terminate한다. iterator/참조 무효화와
    //     공유 thread 상태는 없다.
    return find_score(learner_id).transform([](int score) noexcept {
        // *는 built-in 정수 연산이다. 21*2는 표현 가능한 int prvalue 42다.
        return score * 2;
    });
}

int main() {
    // 함수 반환 optional prvalue가 두 const 객체를 직접 초기화하므로 불필요한 optional 복사는 없다.
    const std::optional<int> known{doubled_score(7)};
    const std::optional<int> unknown{doubled_score(99)};

    // [첫 호출 계약: std::optional<int>::value_or]
    // (1) 수신 known은 값 42를 가진 유효한 const optional<int> lvalue다.
    // (2) template<class U> constexpr T value_or(U&&) const &, T=int, U=int overload를 선택한다.
    // (3) -1은 int prvalue fallback이며 int로 변환 가능하다. 값이 있어도 인자 식 자체는 이미 평가되지만
    //     선택 결과에는 쓰이지 않는다. 외부 소유권은 없다.
    // (4) engaged면 contained int를 복사한 42, 비었으면 fallback으로 만든 int를 값으로 반환한다.
    //     여기 반환값은 ostream 삽입에 사용한다.
    // (5) known과 fallback은 바뀌지 않는다.
    // (6) int에서는 O(1), 무할당, 예외 없음이다. 일반 T에서는 선택된 복사/변환 비용과 예외를 따른다.
    //     참조 무효화가 없고 같은 optional의 병행 변경은 없어야 한다.
    // [첫 호출 계약: std::cout operator<< 연쇄]
    // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 main thread만 쓴다.
    // (2) int에는 ostream::operator<<(int), 문자에는 operator<<(ostream&, char)가 선택된다.
    // (3) value_or 결과 int prvalue 두 개와 ' ', '\n' char prvalue를 값으로 읽고 소유권을 옮기지 않는다.
    // (4) 각 삽입은 같은 ostream&를 돌려 다음 <<에 사용하고 마지막 반환 참조는 버린다.
    // (5) "42 -1\n" 기록을 시도하며 optional들은 바뀌지 않는다. 장치 실패 때 stream 상태 bit가 바뀐다.
    // (6) 표준은 장치 I/O의 단일 복잡도/할당 상한을 고정하지 않는다. 실패는 상태 bit 또는 설정된 예외로
    //     보고된다. 다른 참조는 무효화하지 않으며 복합 출력의 thread 원자성은 보장하지 않는다.
    std::cout << known.value_or(-1) << ' ' << unknown.value_or(-1) << '\n';

    // 기계 실행 관점에서는 optional 존재 bit load, 조건 분기, 정수 load/곱셈, ostream 호출을 떠올릴 수
    // 있다. 실제 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 상수 전파로 사라지거나 합쳐질 수 있다.

    // 직접 해보기:
    // 1) find_score(99)의 lambda 안에 임시 출력문을 넣고 호출되지 않는지 확인한다.
    // 2) value_or(0)과 빈 optional의 operator*를 비교해, 전자는 정의되지만 후자는 전제조건 위반임을 설명한다.
    // 3) transform lambda가 optional<int>를 반환하면 optional<optional<int>>가 된다는 점을 확인하고,
    //    평탄화가 필요할 때 왜 and_then을 선택해야 하는지 말해 본다.
    return 0;
}
