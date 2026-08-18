// <array>는 길이가 타입에 포함되는 고정 배열 std::array를 선언한다.
#include <array>
// <bit>는 std::bit_cast, std::byteswap, std::endian을 선언한다.
#include <bit>
// <cstddef>는 원시 저장 바이트를 나타내는 std::byte와 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 프로토콜 폭을 고정하는 std::uint32_t를 선언한다.
#include <cstdint>
// <iostream>은 학습 결과를 확인할 std::cout을 선언한다.
#include <iostream>

// Sequence는 서로 바꾸어 쓰면 안 되는 순번을 별도 타입으로 만드는 실무형 강한 타입이다.
class Sequence {
public:
    // explicit 단일 인자 생성자는 Sequence sequence = 42U 같은 암시 변환을 막고 직접 초기화만 허용한다.
    explicit constexpr Sequence(std::uint32_t value) noexcept : value_{value} {}
    // 반환형 uint32_t와 빈 매개변수 목록을 가진 const 함수이며 저장 값을 바꾸지 않고 prvalue로 복사한다.
    [[nodiscard]] constexpr std::uint32_t raw() const noexcept { return value_; }

private:
    std::uint32_t value_{}; // private 멤버는 생성 시 받은 32비트 값을 객체 수명 동안 소유한다.
};

// struct는 기본 public이라 단순 데이터 전달 객체에 적합하다.
struct AuditRecord {
    Sequence sequence;
    std::uint32_t event_code{};
};

// using은 새 타입을 정의하지 않고 표준 array 특수화에 도메인 이름을 붙인다.
using WireRecord = std::array<std::byte, 8>;
using WireWord = std::array<std::byte, 4>;

// 값 매개변수 value를 복사해 받고, 예외 없이 네 바이트를 소유하는 WireWord 값으로 반환한다.
[[nodiscard]] constexpr WireWord encode_word(std::uint32_t value) noexcept {
    // byteswap(value)는 little-endian 호스트에서만 선택되며 value를 바꾸지 않고 뒤집은 prvalue를 반환한다.
    // 템플릿 인자는 uint32_t로 추론되고 반환을 local에 저장한다. 정수 외 타입은 금지되며 O(1), 무할당이다.
    const std::uint32_t ordered{
        std::endian::native == std::endian::little ? std::byteswap(value) : value};
    // bit_cast<WireWord>(ordered)는 같은 4바이트 객체 표현을 array 값으로 복사한다.
    // 입력 lvalue의 소유권·수명·값은 유지되고 반환 prvalue가 독립된 바이트를 소유한다.
    return std::bit_cast<WireWord>(ordered);
}

// const 참조는 호출자의 네 바이트를 함수 실행 동안 빌리고 복원한 32비트 값을 prvalue로 반환한다.
[[nodiscard]] constexpr std::uint32_t decode_word(const WireWord& bytes) noexcept {
    // bit_cast<uint32_t>(bytes)는 const array 참조를 읽어 새 정수 prvalue를 반환하며 원본은 바뀌지 않는다.
    const std::uint32_t ordered{std::bit_cast<std::uint32_t>(bytes)};
    return std::endian::native == std::endian::little ? std::byteswap(ordered) : ordered;
}

// record는 비소유 const lvalue 참조이고 반환 WireRecord는 원본과 독립된 바이트 저장소를 소유한다.
[[nodiscard]] constexpr WireRecord serialize(const AuditRecord& record) noexcept {
    const WireWord sequence_bytes{encode_word(record.sequence.raw())};
    const WireWord event_bytes{encode_word(record.event_code)};
    WireRecord result{};

    // array::size()는 인자 없이 수신 고정 배열의 원소 수 4를 size_type 값으로 O(1)에 반환하고
    // 배열을 바꾸지 않는다. 반환값을 반복 조건과 두 번째 필드의 오프셋에 사용한다.
    for (std::size_t index{}; index < sequence_bytes.size(); ++index) {
        // operator[]는 index가 범위 안이라는 전제에서 O(1) 참조를 반환한다. 대입 뒤 result 두 원소만
        // 바뀌고 고정 배열은 할당·재할당하지 않아 포인터나 참조를 무효화하지 않는다.
        result[index] = sequence_bytes[index];
        result[index + sequence_bytes.size()] = event_bytes[index];
    }
    return result;
}

// wire를 수정하지 않고 읽어 새 AuditRecord 값으로 반환하므로 반환 뒤 wire의 수명과 무관하다.
[[nodiscard]] constexpr AuditRecord deserialize(const WireRecord& wire) noexcept {
    WireWord sequence_bytes{};
    WireWord event_bytes{};
    for (std::size_t index{}; index < sequence_bytes.size(); ++index) {
        sequence_bytes[index] = wire[index];
        event_bytes[index] = wire[index + sequence_bytes.size()];
    }
    return AuditRecord{Sequence{decode_word(sequence_bytes)}, decode_word(event_bytes)};
}

// endian의 세 열거자는 컴파일 시간 값을 제공하며, 지원하지 않는 혼합 엔디언에서는 컴파일을 거부한다.
static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big);

int main() {
    const AuditRecord source{Sequence{99U}, 503U}; // explicit 생성자를 직접 호출해 두 public 필드를 초기화한다.
    // serialize의 const 참조 매개변수는 source lvalue에 바인딩되고 반환 prvalue가 bytes를 직접 초기화한다.
    const WireRecord bytes{serialize(source)};
    // deserialize는 bytes lvalue를 빌리고 반환 AuditRecord prvalue로 restored를 직접 초기화한다.
    const AuditRecord restored{deserialize(bytes)};

    // ostream operator<<는 순번, 공백, 이벤트 코드, 개행을 차례로 쓰고 매번 같은 ostream&를 반환한다.
    // 최종 반환 참조는 저장하지 않으며 source/bytes/restored는 유지된다. 비용은 출력 길이에 선형이다.
    std::cout << restored.sequence.raw() << ' ' << restored.event_code << '\n';

    // 함수 반환 객체는 prvalue로 목적 객체를 직접 초기화할 수 있다. 이름 있는 source와 bytes는 lvalue이며
    // const 참조에 바인딩되어 복사되지 않는다. 이 예제에는 소유 자원을 넘길 xvalue나 이동 연산이 필요 없다.
    return restored.sequence.raw() == 99U && restored.event_code == 503U ? 0 : 1;
}
