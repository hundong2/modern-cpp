// 초보자 실습: unsigned 64비트 값 하나를 권한 집합처럼 감싸고 켜진 비트 수를 센다.
// `Permission::remove` 추가 줄을 지운 뒤 출력이 `2 1`로 바뀌는지 직접 확인한다.

// <bit>는 std::popcount 함수 템플릿을 선언한다.
#include <bit>
// <cstdint>는 std::uint64_t 고정 폭 부호 없는 정수 별칭을 제공한다.
#include <cstdint>
// <iostream>은 std::cout과 정수·문자 출력 연산을 제공한다.
#include <iostream>

// 강한 열거형은 read를 평범한 정수나 다른 enum 값과 실수로 섞지 못하게 한다.
enum class Permission : unsigned {
    read = 0U,
    write = 1U,
    remove = 2U,
};

// class는 기본 private이므로 raw 비트는 공개하지 않고 의미 있는 함수만 노출한다.
class PermissionMask {
public:
    // explicit은 `PermissionMask mask = 7;` 같은 뜻하지 않은 암시 변환을 금지한다.
    // 멤버 초기화 목록은 본문 전에 bits_ 수명을 시작하고 알려진 세 bit만 보존한다.
    explicit constexpr PermissionMask(std::uint64_t initial_bits) noexcept
        : bits_{initial_bits & kKnownBits} {}

    // 매개변수 permission은 작은 enum 값이므로 값으로 받는다. 반환형 void는 결과값이 없다.
    constexpr void add(Permission permission) noexcept {
        bits_ |= mask_for(permission);
    }

    // const 멤버 함수는 bits_를 바꾸지 않는다. bool은 참/거짓 기본 타입이다.
    [[nodiscard]] constexpr bool contains(Permission permission) const noexcept {
        return (bits_ & mask_for(permission)) != std::uint64_t{0};
    }

    [[nodiscard]] constexpr int count() const noexcept {
        // [첫 호출 계약: std::popcount(bits_)]
        // (1) 수신 객체 없는 자유 함수다. bits_는 살아 있는 const PermissionMask가 소유한
        //     유효한 std::uint64_t lvalue이고 최대 세 개의 알려진 비트만 켜져 있다.
        // (2) 선택 시그니처는 `template<class T> constexpr int popcount(T) noexcept`이며
        //     템플릿 인자 T는 std::uint64_t로 추론된다.
        // (3) 유일한 입력은 bits_의 값 복사 결과인 unsigned 정수 prvalue다. 함수가 pointer,
        //     reference 또는 소유권을 저장하지 않으며 음수 signed 타입은 허용 대상이 아니다.
        // (4) 반환형 int는 1인 비트 개수이고 이 함수의 반환값으로 사용한다.
        // (5) 호출 뒤 bits_, 인자 값, PermissionMask의 크기와 수명은 모두 그대로다.
        // (6) 고정 64비트에서는 O(1) 상수 시간·공간, 무할당·noexcept·무효화 없음이다. 올바른 unsigned
        //     값 호출에는 별도 오류/UB가 없다. 같은 mask의 병행 쓰기는 동기화해야 하며
        //     실제 명령 수는 CPU·컴파일러·최적화 옵션에 따라 달라진다.
        return std::popcount(bits_);
    }

private:
    static constexpr std::uint64_t kKnownBits{std::uint64_t{0b111}};

    // 선언한 열거자마다 안전한 상수 shift만 쓴다. 강제 변환된 미지의 enum 값은 0 mask가
    // 되어 상태를 바꾸지 않으므로 shift count가 64 이상일 때의 미정의 동작도 없다.
    [[nodiscard]] static constexpr std::uint64_t mask_for(Permission permission) noexcept {
        switch (permission) {
        case Permission::read:
            return std::uint64_t{1} << 0U;
        case Permission::write:
            return std::uint64_t{1} << 1U;
        case Permission::remove:
            return std::uint64_t{1} << 2U;
        }
        return std::uint64_t{0};
    }

    // 이 기본 타입 멤버가 실제 저장소이며 PermissionMask 객체가 값을 직접 소유한다.
    std::uint64_t bits_{};
};

// 함수의 반환형과 매개변수 목록을 분리해서 읽는다. 여기에는 입력 매개변수가 없다.
[[nodiscard]] constexpr PermissionMask make_example_mask() noexcept {
    PermissionMask mask{std::uint64_t{0}};
    mask.add(Permission::read);
    mask.add(Permission::write);
    mask.add(Permission::remove);

    // 이름 있는 mask는 lvalue지만 return 시 NRVO가 허용된다. NRVO가 되지 않아도 이 값 타입의
    // 암시적 이동/복사는 64비트 값 하나를 복사한다. 아래처럼 prvalue를 직접 반환하는 경우와 달리
    // NRVO 자체는 보장된 규칙이 아니라 허용된 최적화다.
    return mask;
}

int main() {
    // factory 호출 결과는 PermissionMask prvalue다. 같은 타입 목적 mask를 초기화할 때 중간
    // 임시를 따로 복사할 필요가 없다. 이름을 얻은 뒤 식 `mask`는 lvalue가 된다.
    const PermissionMask mask{make_example_mask()};

    // [첫 호출 계약: std::cout 정수·bool·문자 operator<< 연쇄]
    // (1) 수신 std::cout은 초기화된 std::ostream lvalue이고 mask는 살아 있는 const 값이다.
    // (2) count의 int와 contains의 bool에는 각각 멤버 산술 삽입 overload가, 공백과 개행에는
    //     비멤버 `operator<<(std::ostream&, char)` overload가 선택된다.
    // (3) 입력 피연산자는 int/bool/char prvalue이며 모두 값만 읽고 소유권을 옮기지 않는다.
    // (4) 각 호출은 같은 std::ostream&를 반환해 다음 삽입에 쓰고 마지막 반환 참조는 버린다.
    // (5) 성공 뒤 출력 위치·버퍼만 바뀌며 mask 값과 수명은 그대로다. 실패 시 상태 비트가 바뀐다.
    // (6) 전체 O(...) 상한은 표준에 따로 없고 생성 문자·locale·streambuf에 달린다. 내부 할당과
    //     설정된 예외 mask에 따른 출력 오류 예외가 가능하다. mask 관찰자는 무효화되지 않으며
    //     같은 stream의 동시 레코드 순서를 자동 보장하지 않는다.
    std::cout << mask.count() << ' ' << mask.contains(Permission::write) << '\n';

    // 직접 해보기: Permission::remove 줄을 지우고 count가 2인지, write가 여전히 true인지 설명한다.
    return 0;
}
