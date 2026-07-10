#include <cassert>
#include <cstdint>
#include <expected>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>

// 2026-07-05
// Topic: bit masks and packet flags.

struct PacketFlags {
    static constexpr std::uint8_t Encrypted = 0x01;   // 0000 0001
    static constexpr std::uint8_t Compressed = 0x02;  // 0000 0010
    static constexpr std::uint8_t Urgent = 0x04;      // 0000 0100
};

struct HardwarePacket {
    std::uint8_t flag_byte;
    std::string_view payload;
};

bool has_all_flags(std::uint8_t value, std::uint8_t flags) {
    // Bitwise AND extracts only the bits we care about.
    return (value & flags) == flags;
}

void inspect_packet(const HardwarePacket& packet) {
    std::cout << "[PACKET]\n";
    std::cout << "  flag_byte: 0x" << std::hex << std::uppercase
              << static_cast<int>(packet.flag_byte) << std::dec << '\n';
    std::cout << "  payload  : " << packet.payload << '\n';
}

template <typename... Args>
std::expected<void, std::string> dispatch_to_hardware(Args&&... args) {
    static_assert(sizeof...(Args) >= 2);

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& port = std::get<0>(packed);
    const HardwarePacket& packet = std::get<1>(packed);

    const auto secure_urgent = static_cast<std::uint8_t>(PacketFlags::Encrypted | PacketFlags::Urgent);
    if (has_all_flags(packet.flag_byte, secure_urgent)) {
        std::cout << "[SECURITY] encrypted urgent packet on " << port << '\n';
    }

    inspect_packet(packet);
    return {};
}

void run_tests() {
    const auto flags = static_cast<std::uint8_t>(PacketFlags::Encrypted | PacketFlags::Urgent);
    assert(has_all_flags(flags, PacketFlags::Encrypted));
    assert(has_all_flags(flags, PacketFlags::Urgent));
    assert(!has_all_flags(flags, PacketFlags::Compressed));
}

int main() {
    int line_card_hardware_status{};
    (void)line_card_hardware_status;

    run_tests();

    HardwarePacket packet{
        static_cast<std::uint8_t>(PacketFlags::Encrypted | PacketFlags::Urgent),
        "SECURE_STREAM_2026",
    };

    const auto result = dispatch_to_hardware("NIC_PORT_0", packet, 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[TESTS] bit mask demo passed\n";
    return 0;
}
