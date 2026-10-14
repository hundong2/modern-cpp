// <cstdint>는 저장 형식의 폭을 고정하는 std::uint16_t를 선언한다.
#include <cstdint>
// <iostream>은 연습 결과를 출력할 std::cout을 선언한다.
#include <iostream>
// <utility>는 강한 열거형의 기저 값을 얻는 std::to_underlying을 선언한다.
#include <utility>

// 비트 하나마다 권한 하나를 배정한다. enum class라서 무관한 정수와 자동으로 섞이지 않는다.
enum class Permission : std::uint16_t {
    read = 1U,
    write = 2U,
    deploy = 4U,
};

// 두 enum 값을 결합하는 도메인 연산이다. 반환형과 두 값 매개변수를 명시하면 호출 계약이 보인다.
[[nodiscard]] constexpr Permission combine(
    const Permission left,
    const Permission right) noexcept {
    // [첫 호출 계약: std::to_underlying(left/right)]
    // (1) 멤버 수신 객체가 없는 자유 함수이고 두 인자는 유효한 Permission 값인 const lvalue 식이다.
    // (2) 두 호출 모두 to_underlying<Permission>(Permission)을 선택하며 반환형은 uint16_t다.
    // (3) 각 enum 인자는 값으로 복사되고 참조·포인터·소유권을 저장하지 않는다. 모든 열거값을 허용한다.
    // (4) 각 호출은 uint16_t prvalue를 반환해 built-in 비트 OR의 피연산자로 사용한다.
    // (5) left/right와 외부 객체는 그대로이며 OR 결과만 새 Permission 값으로 명시 변환된다.
    // (6) 각 호출 O(1), 무할당, noexcept, 무효화 없음이다. enum 이외 타입은 컴파일 오류이고
    //     런타임 오류·UB가 없다. 공유 상태를 쓰지 않아 독립 호출 사이 스레드 제약도 없다.
    return static_cast<Permission>(
        std::to_underlying(left) | std::to_underlying(right));
}

// class는 표현 정수(bits_)를 감추고 의미 있는 질의만 공개한다. 기본 접근 지정자는 private다.
class PermissionSet {
public:
    // explicit은 Permission 하나가 PermissionSet으로 뜻밖에 암시 변환되지 않게 한다.
    // 멤버 초기화 목록은 생성자 본문 전에 bits_를 정확한 값으로 직접 초기화한다.
    explicit constexpr PermissionSet(const Permission permissions) noexcept
        : bits_{std::to_underlying(permissions)} {}

    // const 멤버 함수는 bits_를 바꾸지 않는다. 값 매개변수는 작은 enum을 복사한다.
    [[nodiscard]] constexpr bool contains(const Permission required) const noexcept {
        const std::uint16_t mask{std::to_underlying(required)};
        return (bits_ & mask) == mask;
    }

    [[nodiscard]] constexpr std::uint16_t wire_code() const noexcept {
        return bits_;
    }

private:
    // 기본 타입은 소유권이나 동적 수명이 없는 값이다. {}가 0으로 초기화해 쓰레기 비트를 막는다.
    std::uint16_t bits_{};
};

// 반환값은 독립된 작은 객체다. 이름 있는 local은 NRVO 후보이고, 미적용 시 암시적 이동/복사가 가능하다.
[[nodiscard]] constexpr PermissionSet make_policy() noexcept {
    PermissionSet policy{combine(Permission::read, Permission::deploy)};
    return policy;
}

// const reference는 null이 될 수 없고 다른 객체로 재바인딩되지 않는 비소유 별칭이다.
// 포인터와 마찬가지로 PermissionSet의 수명을 연장하지 않으므로 호출 동안 원본이 살아 있어야 한다.
void print_policy(const PermissionSet& policy) {
    // if는 bool 결과에 따라 한 블록만 실행하는 조건 분기다.
    if (policy.contains(Permission::read)) {
        // [첫 호출 계약: std::cout << const char*]
        // (1) 수신 cout은 프로그램 수명의 유효한 ostream lvalue다.
        // (2) 선택 overload는 문자 포인터를 받는 비멤버 삽입이며 왼쪽은 ostream&다.
        // (3) 오른쪽은 정적 수명의 null 종료 문자열을 가리키는 const char* prvalue다. 소유권은 유지된다.
        // (4) 같은 ostream&를 반환하지만 이 문장에서는 반환 참조를 버린다.
        // (5) 성공하면 문자가 버퍼에 기록되고 실패하면 stream 상태 비트가 바뀔 수 있다.
        // (6) 복잡도는 출력 문자/장치에 좌우되고 공통 점근 상한은 없다. 예외 mask에 따라 I/O 예외가
        //     가능하며 부분 출력은 롤백되지 않는다. 여러 스레드 레코드 원자성은 보장하지 않는다.
        std::cout << "read=yes\n";
    }
    if (policy.contains(Permission::deploy)) {
        std::cout << "deploy=yes\n";
    }

    // [첫 호출 계약: std::cout << uint16_t << char]
    // (1) 최초 수신 cout은 앞선 출력 뒤에도 유효한 ostream lvalue이며 오류 상태일 수 있다.
    // (2) const char* 비멤버 삽입 뒤 이 구현에서 uint16_t인 unsigned short 멤버 삽입과
    //     char 비멤버 삽입 overload가 차례로 선택된다.
    // (3) "wire="은 정적 수명의 null 종료 배열에서 변환된 비소유 포인터이고, wire_code() 결과는
    //     uint16_t prvalue, '\n'은 char prvalue다. 어느 인자도 소유권을 넘기지 않는다.
    // (4) 각 연산은 같은 ostream&를 반환해 다음 <<의 수신으로 사용하며 마지막 반환 참조는 버린다.
    // (5) 성공하면 접두어·십진 정수·개행이 순서대로 기록되고 실패하면 stream 상태 비트가 바뀔 수
    //     있으나 policy는 그대로다.
    // (6) 비용은 형식화·문자 수·장치에 좌우되고 공통 점근 상한은 없다. 예외 mask에 따라 I/O 예외가
    //     가능하며 부분 출력은 롤백되지 않는다. 같은 stream의 레코드 단위 병행 출력은 보장하지 않는다.
    std::cout << "wire=" << policy.wire_code() << '\n';
}

int main() {
    // make_policy()의 prvalue가 policy를 직접 초기화한다. 불필요한 임시 복사를 요구하지 않는다.
    const PermissionSet policy{make_policy()};

    // 컴파일러가 만든 복사 생성자는 uint16_t 값 하나를 복사한다. copied와 policy는 독립 객체다.
    const PermissionSet copied{policy};
    print_policy(copied);

    // 기계 관점에서는 정수 load, bitwise AND/OR, 비교, 조건 분기, 출력 store로 낮아질 수 있다.
    // 구체 명령과 분기 제거 여부는 CPU·컴파일러·최적화 옵션에 따라 달라져 특정 어셈블리를 단정하지 않는다.
    return 0;
}
