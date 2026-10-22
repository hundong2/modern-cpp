// 오늘의 실무 예제: 권한 비트들을 값 타입 안에 숨기고 std::popcount로 켜진 권한 수를 센다.
// 먼저 출력 `operator 2 1`을 예측한 뒤, audit 권한을 추가했을 때 어떤 값이 바뀌는지 확인한다.

// <bit>는 부호 없는 정수에서 1인 비트 개수를 세는 std::popcount를 선언한다.
#include <bit>
// <cstdint>는 정확히 64비트인 부호 없는 정수 별칭 std::uint64_t를 제공한다.
#include <cstdint>
// <iostream>은 표준 출력 객체 std::cout과 삽입 연산을 제공한다.
#include <iostream>
// <string>은 문자 저장소를 직접 소유하는 std::string을 제공한다.
#include <string>
// <utility>는 이름 있는 객체를 xvalue 식으로 바꾸는 std::move를 제공한다.
#include <utility>

// enum class는 열거자를 바깥 이름 공간에 흘리지 않고 정수와의 암시 변환도 막는다.
// 명시한 unsigned는 기저 타입이며 각 값은 64비트 마스크의 비트 위치다.
enum class Capability : unsigned {
    inspect = 0U,
    deploy = 1U,
    audit = 2U,
};

// using은 새 타입을 만들지 않고 긴 표준 타입에 문맥 이름을 붙인다.
using BitWord = std::uint64_t;

// class의 기본 접근은 private다. 외부가 bits_를 직접 조작하지 못하게 불변식을 감춘다.
class CapabilitySet {
public:
    // 기본 생성자는 bits_의 중괄호 초기값을 사용해 빈 집합을 만든다.
    constexpr CapabilitySet() noexcept = default;

    // explicit은 정수 하나가 권한 집합으로 뜻하지 않게 암시 변환되는 것을 막는다.
    // 멤버 초기화 목록은 생성자 본문 전에 bits_의 수명을 시작한다. 외부 저장값에 알 수
    // 없는 bit가 섞여도 알려진 세 bit만 남겨 class 불변식을 복구한다.
    explicit constexpr CapabilitySet(BitWord initial_bits) noexcept
        : bits_{initial_bits & kKnownBits} {}

    // void 반환형은 별도 결과값 없이 이 객체의 상태를 바꾼다는 뜻이다.
    constexpr void grant(Capability capability) noexcept {
        bits_ |= bit_for(capability);
    }

    // 함수 뒤 const는 *this의 관찰 가능한 상태를 바꾸지 않겠다는 약속이다.
    [[nodiscard]] constexpr bool contains(Capability capability) const noexcept {
        return (bits_ & bit_for(capability)) != BitWord{0};
    }

    [[nodiscard]] constexpr int count() const noexcept {
        // [첫 호출 계약: std::popcount(bits_)]
        // (1) 자유 함수이므로 수신 객체는 없다. 입력 bits_는 살아 있는 const CapabilitySet가
        //     소유하며 호출 직전 0개 이상의 유효 권한 비트를 가진 BitWord lvalue다.
        // (2) 선택 시그니처는 `template<class T> constexpr int popcount(T) noexcept`이고
        //     템플릿 인자 T는 BitWord, 즉 std::uint64_t로 추론된다.
        // (3) 유일한 인자는 bits_에서 값 복사되는 unsigned 정수 lvalue-to-rvalue 결과다.
        //     포인터나 참조를 보관하지 않고 집합·비트 저장소의 소유권도 옮기지 않는다.
        // (4) 반환형 int는 값이 1인 비트 수다. 이 호출부는 반환 prvalue를 그대로 돌려준다.
        // (5) 호출 뒤 bits_, CapabilitySet의 수명과 모든 다른 객체 상태는 그대로다.
        // (6) 64비트 고정 폭에서는 O(1) 상수 시간 작업이고 무할당·noexcept·무효화 없음이다.
        //     인자는 unsigned 정수여야 한다. 독립 값에는 데이터 경쟁이 없지만 같은 객체를
        //     병행 수정하면 외부 동기화가 필요하다. 실제 POPCNT 명령 사용 여부는 CPU,
        //     컴파일러, 표준 라이브러리와 최적화 옵션에 따라 달라진다.
        return std::popcount(bits_);
    }

private:
    // 알려진 비트의 mask는 raw 저장값을 역직렬화하는 경계에서 나머지 비트를 제거한다.
    static constexpr BitWord kKnownBits{BitWord{0b111}};

    // switch는 선언한 열거자마다 고정된 안전한 shift만 수행한다. 호출자가 임의 정수를
    // Capability로 강제 변환해 넘겨도 마지막 return으로 0을 돌려 shift 범위 UB를 막는다.
    [[nodiscard]] static constexpr BitWord bit_for(Capability capability) noexcept {
        switch (capability) {
        case Capability::inspect:
            return BitWord{1} << 0U;
        case Capability::deploy:
            return BitWord{1} << 1U;
        case Capability::audit:
            return BitWord{1} << 2U;
        }
        return BitWord{0};
    }

    // `{}`는 기본 정수 타입을 0으로 값 초기화한다. 이 객체가 유일한 비트 저장소 owner다.
    BitWord bits_{};
};

// struct의 기본 접근은 public이다. 계층 사이에 옮기는 단순 소유 DTO에 알맞다.
struct SessionPolicy {
    // string은 역할 문자를 독립 소유하므로 호출자의 임시 문자열 수명과 분리된다.
    std::string role;
    CapabilitySet capabilities;
};

// 반환형 SessionPolicy는 새 소유 값을 뜻한다. role은 값 매개변수라 함수가 자기 복사본을 소유한다.
[[nodiscard]] SessionPolicy make_operator_policy(std::string role) {
    CapabilitySet capabilities{};
    capabilities.grant(Capability::inspect);
    capabilities.grant(Capability::deploy);

    // [첫 생성 계약: std::move(role)와 std::string 이동 생성자]
    // (1) std::move는 수신 객체 없는 자유 함수이고 role은 이 함수가 소유하는 유효한
    //     std::string lvalue다. 목적 SessionPolicy::role subobject는 아직 생성 전이다.
    // (2) `remove_reference_t<T>&& move(T&&) noexcept`에서 T=std::string&가 추론되고,
    //     그 결과로 `basic_string(basic_string&&) noexcept` 이동 생성자를 선택한다.
    // (3) 인자는 이름 있는 role lvalue 하나다. move는 이를 xvalue로 cast할 뿐이고,
    //     이어지는 생성자가 문자 값과 재사용 가능한 자원을 새 role 멤버로 이전한다.
    // (4) move는 std::string&&를 반환해 즉시 생성 인자로 사용한다. 생성자는 반환값이 없고
    //     새 멤버 객체의 수명을 시작한다.
    // (5) 성공 뒤 결과 DTO가 원래 문자열 값을 소유하고 매개변수 role은 유효하지만 값이
    //     미지정인 moved-from 상태다. capabilities는 작은 값 타입이라 결과에 복사된다.
    // (6) move cast 자체는 O(1), 무할당, noexcept다. string 이동도 기본 allocator에서는
    //     상수 시간·noexcept이고 기존 role 관찰자는 새 owner를 가리킨다고 일반화하면 안 된다.
    //     독립 객체에는 데이터 경쟁이 없고, 반환 prvalue는 호출자의 목적 객체를 직접 초기화한다.
    return SessionPolicy{std::move(role), capabilities};
}

// const lvalue reference는 null이 될 수 없는 읽기 전용 별칭이며 policy의 소유권을 늘리지 않는다.
// 포인터는 null을 표현하고 다시 다른 대상을 가리킬 수 있다는 점이 reference와 다르다.
void print_policy(const SessionPolicy& policy) {
    // [첫 호출 계약: std::cout operator<< 연쇄]
    // (1) 수신 std::cout은 정상 상태인 std::ostream lvalue이고 policy와 그 string 멤버는
    //     이 함수가 끝날 때까지 살아 있다. 한 main thread만 이 레코드를 출력한다.
    // (2) 차례로 비멤버 `operator<<(ostream&, const string&)`, 문자 삽입 overload,
    //     멤버 `ostream::operator<<(int)`, 멤버 `operator<<(bool)`, 문자 overload를 선택한다.
    // (3) 피연산자는 const string lvalue, 세 char prvalue, count의 int prvalue와 contains의
    //     bool prvalue다. 모두 읽기 입력이며 객체나 버퍼 소유권을 cout에 넘기지 않는다.
    // (4) 각 호출은 같은 std::ostream&를 반환해 다음 `<<`의 왼쪽 피연산자로 사용하고,
    //     마지막 반환 참조는 버린다. `\n`은 줄바꿈이며 강제 flush를 요구하지 않는다.
    // (5) 성공하면 출력 버퍼와 출력 위치만 바뀌고 policy의 문자열·비트·수명은 유지된다.
    //     실패하면 failbit/badbit가 설정될 수 있다.
    // (6) 표준은 전체 형식 출력에 단일 O(...) 상한을 두지 않으며 문자 수, locale과
    //     streambuf에 따라 비용·내부 할당이 달라진다. 예외 mask가 켜졌다면 출력 오류가
    //     예외가 될 수 있다. policy 관찰자는 무효화되지 않으며 동시 레코드 원자성은 없다.
    std::cout << policy.role << ' ' << policy.capabilities.count() << ' '
              << policy.capabilities.contains(Capability::deploy) << '\n';
}

// main은 운영체제가 호출하는 진입점이며 int 0 반환은 정상 종료를 뜻한다.
int main() {
    // [첫 생성 계약: std::string 리터럴 생성과 make_operator_policy 값 전달]
    // (1) 아직 수명이 시작되지 않은 임시 std::string과 policy 목적 객체가 있다.
    // (2) `basic_string(const char*, const Allocator& = Allocator())`를 선택하고, 그 prvalue가
    //     make_operator_policy의 값 매개변수 std::string role을 직접 초기화한다.
    // (3) 인자는 null로 끝나는 문자열 리터럴 `"operator"`의 const char[9] lvalue가 pointer로
    //     변환된 값이다. 문자는 프로그램 끝까지 살며 생성 결과가 내용을 복사 소유한다.
    // (4) string 생성자는 반환값이 없고 임시 객체를 만든다. factory는 SessionPolicy prvalue를
    //     반환하며 policy의 직접 목록 초기화에 사용한다.
    // (5) policy가 역할 문자열과 권한 집합을 독립 소유한다. 반환 prvalue는 C++17 보장 복사
    //     생략으로 목적 객체를 직접 초기화하며 별도 SessionPolicy 이동을 요구하지 않는다.
    // (6) 문자열 길이에 O(n), 동적 할당 가능, std::bad_alloc 가능이다. 성공 뒤 policy의
    //     관찰자는 policy 수명까지 유효하고 같은 객체의 병행 변경은 동기화해야 한다.
    const SessionPolicy policy{make_operator_policy(std::string{"operator"})};

    // 이름 있는 policy 식은 lvalue이고 const reference 매개변수에 바인딩된다.
    // 호출은 복사/이동 없이 policy를 빌리며 print_policy가 끝나면 reference도 사라진다.
    print_policy(policy);

    // if는 bool 값을 비교해 조건 분기를 만든다. audit 권한은 주지 않았으므로 본문은 실행되지 않는다.
    if (policy.capabilities.contains(Capability::audit)) {
        return 1;
    }

    // 지역 객체는 블록을 나가며 선언의 역순으로 소멸하고 string 저장소도 RAII로 해제된다.
    return 0;
}
