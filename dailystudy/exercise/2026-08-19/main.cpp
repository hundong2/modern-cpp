// <array>는 크기가 컴파일 시간에 정해진 연속 바이트 컨테이너 std::array를 선언한다.
#include <array>
// <bit>는 객체 표현을 안전하게 복사하는 std::bit_cast와 바이트 순서 도구를 선언한다.
#include <bit>
// <cstddef>는 정수 산술과 구분되는 바이트 타입 std::byte와 크기 타입 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 정확히 32비트인 부호 없는 정수 별칭 std::uint32_t를 가능한 구현에서 선언한다.
#include <cstdint>
// <iostream>은 검증 결과를 쓰는 표준 출력 객체 std::cout과 삽입 연산자를 선언한다.
#include <iostream>

// MessageKind는 원시 정수에 도메인 의미를 부여해 다른 정수와 실수로 섞이는 일을 막는 값 객체다.
class MessageKind {
public:
    // 생성자는 반환형이 없다. explicit은 uint32_t 하나가 MessageKind로 암시 변환되는 일을 막지만
    // MessageKind{7U}처럼 의도가 드러나는 직접 초기화는 허용한다.
    explicit constexpr MessageKind(std::uint32_t value) noexcept : value_{value} {}

    // const 멤버 함수는 객체를 바꾸지 않고 저장된 기본 타입 값을 prvalue로 반환한다.
    [[nodiscard]] constexpr std::uint32_t raw() const noexcept { return value_; }

private:
    // class의 기본 접근은 private이며 이 멤버는 public 함수만 통해 읽을 수 있다.
    std::uint32_t value_{};
};

// struct의 기본 접근은 public이다. 두 필드는 응용 계층의 호스트 바이트 순서 값을 소유한다.
struct PacketHeader {
    MessageKind kind;
    std::uint32_t payload_size{};
};

// using은 새 타입을 만들지 않고 정확히 8바이트인 표준 array 타입에 읽기 쉬운 별칭을 붙인다.
using EncodedHeader = std::array<std::byte, 8>;
using EncodedWord = std::array<std::byte, 4>;

// PacketCodec은 도메인 객체와 네트워크 바이트 표현의 변환 책임을 경계 하나에 모은다.
class PacketCodec {
public:
    // const lvalue 참조는 header를 빌릴 뿐 복사하거나 소유하지 않는다. 반환 array prvalue가 결과를 소유한다.
    [[nodiscard]] static constexpr EncodedHeader encode(const PacketHeader& header) noexcept {
        const EncodedWord kind_bytes{encode_word(header.kind.raw())};
        const EncodedWord size_bytes{encode_word(header.payload_size)};
        EncodedHeader result{}; // array의 모든 std::byte 원소를 0으로 값 초기화한다.

        // size_t는 컨테이너 인덱스용 부호 없는 기본 타입이다. kind_bytes의 정확한 타입은
        // array<byte,4>이고 size()는 인자 없이 고정 원소 수 size_type{4}를 O(1)에 반환한다.
        // 수신 배열의 값·수명은 바뀌지 않고 반환값을 반복 조건과 오프셋에 사용한다.
        for (std::size_t index{}; index < kind_bytes.size(); ++index) {
            // array::operator[](size_type)는 범위 검사 없이 std::byte&를 O(1)에 반환한다.
            // index<4가 전제조건을 보장한다. 고정 배열은 재할당하지 않아 다른 참조는 무효화되지 않는다.
            result[index] = kind_bytes[index];
            result[index + kind_bytes.size()] = size_bytes[index];
        }
        return result; // 이름 있는 지역 값은 이동 또는 NRVO로 반환되며 소유권이 호출자 결과로 간다.
    }

    // wire의 수명 동안만 const 참조로 읽고 두 32비트 값을 복원한 PacketHeader prvalue를 반환한다.
    [[nodiscard]] static constexpr PacketHeader decode(const EncodedHeader& wire) noexcept {
        EncodedWord kind_bytes{};
        EncodedWord size_bytes{};
        for (std::size_t index{}; index < kind_bytes.size(); ++index) {
            kind_bytes[index] = wire[index];
            size_bytes[index] = wire[index + kind_bytes.size()];
        }

        // PacketHeader{...}는 prvalue다. 반환 목적 객체를 직접 초기화하므로 불필요한 복사가 생기지 않는다.
        return PacketHeader{MessageKind{decode_word(kind_bytes)}, decode_word(size_bytes)};
    }

private:
    [[nodiscard]] static constexpr EncodedWord encode_word(std::uint32_t host_value) noexcept {
        // std::byteswap<uint32_t>(value)는 값 매개변수 하나의 바이트 순서를 뒤집은 uint32_t prvalue를
        // O(1) 시간·공간에 반환하고 host_value는 바꾸지 않는다. 정수 타입만 허용하며 예외·할당은 없다.
        const std::uint32_t network_value{
            std::endian::native == std::endian::little ? std::byteswap(host_value) : host_value};

        // std::bit_cast<EncodedWord>(network_value)는 목적 타입을 템플릿 인자로, 같은 크기의 trivially copyable
        // uint32_t lvalue를 const 참조 의미로 읽는다. 비트를 새 array 값에 복사해 반환하고 원본·수명은 유지된다.
        // 동적 할당과 예외는 없으며 포인터 별칭이나 정렬 위반을 만들지 않는다.
        return std::bit_cast<EncodedWord>(network_value);
    }

    [[nodiscard]] static constexpr std::uint32_t decode_word(const EncodedWord& network_bytes) noexcept {
        // bit_cast<uint32_t>는 네 바이트 객체 표현을 새 정수 값으로 복사한다. 반환값을 network_value에 저장하고
        // network_bytes와 그 원소는 바뀌지 않는다. 두 타입 크기가 같고 trivially copyable이어야 한다.
        const std::uint32_t network_value{std::bit_cast<std::uint32_t>(network_bytes)};
        return std::endian::native == std::endian::little ? std::byteswap(network_value) : network_value;
    }
};

// 오늘 구현은 혼합 엔디언 시스템을 지원하지 않는다. 컴파일 시간 조건으로 경계를 명시한다.
static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big);

int main() {
    // MessageKind의 explicit 생성자를 직접 호출하고 PacketHeader의 public 필드를 순서대로 초기화한다.
    const PacketHeader original{MessageKind{7U}, 1024U};
    const EncodedHeader wire{PacketCodec::encode(original)}; // 반환 prvalue로 고정 배열을 직접 초기화한다.
    const PacketHeader decoded{PacketCodec::decode(wire)};

    // std::cout의 정확한 타입은 std::ostream이다. 각 operator<<는 값 하나를 버퍼에 기록하고 같은
    // ostream&를 반환해 다음 삽입을 연쇄한다. 최종 참조는 버리고 객체들은 유지한다. I/O 실패는 상태 비트에
    // 기록되고 예외 마스크를 별도 설정하지 않았으므로 보통 던지지 않는다. 시간은 출력 문자 수에 선형이다.
    std::cout << decoded.kind.raw() << ' ' << decoded.payload_size << '\n';

    // &&는 왼쪽이 거짓이면 오른쪽 비교를 생략한다. bool 결과로 종료 코드를 선택한다.
    const bool round_trip_ok{decoded.kind.raw() == 7U && decoded.payload_size == 1024U};

    // 바이트 변환은 메모리 load/store나 바이트 재배열로 구현될 수 있지만 정확한 명령은 CPU, ABI,
    // 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.
    return round_trip_ok ? 0 : 1;
}
