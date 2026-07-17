/*
Practice: Static Packet Segment Slicing

The briefing describes C++26 reflection-based member offset extraction. This
C++23 version keeps the exercise executable by using offsetof, std::span, and
std::expected to model the same zero-copy packet inspection pipeline.
*/

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <span>
#include <string_view>
#include <tuple>

struct GigabitPacket {
    int session_id;
    int payload_checksum;
};

constexpr std::size_t checksum_offset() {
    return offsetof(GigabitPacket, payload_checksum);
}

void inspect_segment(const GigabitPacket& packet) {
    std::cout << "[SLICING LOG]\n";
    std::cout << "  session_id       : " << packet.session_id << '\n';
    std::cout << "  payload_checksum : 0x" << std::hex
              << packet.payload_checksum << std::dec << '\n';
}

std::expected<const GigabitPacket*, std::string_view> parse_packet(std::span<const std::byte> buffer) {
    if (buffer.size() < sizeof(GigabitPacket)) {
        return std::unexpected("buffer is too small for GigabitPacket");
    }

    const auto address = reinterpret_cast<std::uintptr_t>(buffer.data());
    if (address % alignof(GigabitPacket) != 0) {
        return std::unexpected("buffer is not aligned for GigabitPacket");
    }

    return reinterpret_cast<const GigabitPacket*>(buffer.data());
}

template <typename... Args>
std::expected<int, std::string_view> dispatch_and_slice(Args&&... args) {
    static_assert(sizeof...(Args) >= 2, "marker and byte buffer are required");

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& marker = std::get<0>(packed);
    const auto buffer = std::get<1>(packed);

    std::cout << "[ENGINE] marker: " << marker << '\n';
    std::cout << "[HARDWARE] checksum offset: " << checksum_offset() << " bytes\n";

    const auto parsed = parse_packet(buffer);
    if (!parsed) {
        return std::unexpected(parsed.error());
    }

    inspect_segment(**parsed);
    return (*parsed)->payload_checksum;
}

void run_tests() {
    static_assert(checksum_offset() == sizeof(int));

    GigabitPacket packet{8888, 0xABCD};
    const auto* bytes = reinterpret_cast<const std::byte*>(&packet);
    const auto result = dispatch_and_slice("TEST", std::span<const std::byte>(bytes, sizeof(packet)));

    assert(result.has_value());
    assert(result.value() == 0xABCD);
}

int main() {
    run_tests();

    int line_card_isolation_flag{};
    (void)line_card_isolation_flag;

    GigabitPacket mock_packet{8888, 0xABCD};
    const auto* byte_ptr = reinterpret_cast<const std::byte*>(&mock_packet);
    std::span<const std::byte> incoming_buffer(byte_ptr, sizeof(mock_packet));

    const auto result = dispatch_and_slice("NIC_GIGABIT_CH_0", incoming_buffer, 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[RESULT] checksum: 0x" << std::hex << result.value() << std::dec << '\n';
    std::cout << "[TESTS] packet slicing tests passed\n";
    return 0;
}
