/*
Practice: Error Recovery Pipeline

The briefing talks about compile-time reflection-driven recovery. This C++23
version keeps the same architecture in executable form: parse a raw byte span,
inspect a packet, and recover from a flagged error without throwing exceptions.
*/

#include <cassert>
#include <cstddef>
#include <expected>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <tuple>

struct FaultyPacket {
    int packet_id;
    int error_flags; // 0 means normal. Non-zero bits describe recoverable errors.
};

void inspect_packet(const FaultyPacket& packet) {
    std::cout << "[RECOVERY LOG]\n";
    std::cout << "  packet_id  : " << packet.packet_id << '\n';
    std::cout << "  error_flags: 0x" << std::hex << packet.error_flags << std::dec << '\n';
}

std::expected<const FaultyPacket*, std::string> parse_faulty_packet(std::span<const std::byte> buffer) {
    if (buffer.size() < sizeof(FaultyPacket)) {
        return std::unexpected("buffer is too small for FaultyPacket");
    }

    // reinterpret_cast is intentionally low-level. The size check above is the
    // minimum guard before treating raw bytes as a packet layout.
    return reinterpret_cast<const FaultyPacket*>(buffer.data());
}

template <typename... Args>
std::expected<int, std::string> dispatch_and_recover(Args&&... args) {
    static_assert(sizeof...(Args) >= 2, "marker and byte span are required");

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& marker = std::get<0>(packed);
    const auto buffer = std::get<1>(packed);

    std::cout << "[ENGINE] marker: " << marker << '\n';

    const auto parsed = parse_faulty_packet(buffer);
    if (!parsed) {
        return std::unexpected(parsed.error());
    }

    int recovery_count = 0;
    for (const FaultyPacket* packet : std::views::single(*parsed)) {
        if (packet->error_flags != 0) {
            ++recovery_count;
            std::cout << "[WARNING] recoverable packet error: 0x"
                      << std::hex << packet->error_flags << std::dec << '\n';
        }
    }

    inspect_packet(**parsed);
    return recovery_count;
}

std::expected<int, std::string> run_recovery_gateway() {
    int line_card_isolation_flag{};
    (void)line_card_isolation_flag;

    FaultyPacket mock_packet{9999, 0x01};
    const auto* byte_ptr = reinterpret_cast<const std::byte*>(&mock_packet);
    std::span<const std::byte> incoming_buffer(byte_ptr, sizeof(mock_packet));

    return dispatch_and_recover("NIC_FAULT_CH_0", incoming_buffer, 2026);
}

void run_tests() {
    FaultyPacket packet{1, 0x02};
    const auto* bytes = reinterpret_cast<const std::byte*>(&packet);
    const auto result = dispatch_and_recover("TEST", std::span<const std::byte>(bytes, sizeof(packet)));
    assert(result.has_value());
    assert(result.value() == 1);
}

int main() {
    run_tests();

    const auto result = run_recovery_gateway();
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[RESULT] recovery count: " << result.value() << '\n';
    std::cout << "[TESTS] recovery tests passed\n";
    return 0;
}

/*
Execution result:
[ENGINE] marker: TEST
[WARNING] recoverable packet error: 0x2
[RECOVERY LOG]
  packet_id  : 1
  error_flags: 0x2
[ENGINE] marker: NIC_FAULT_CH_0
[WARNING] recoverable packet error: 0x1
[RECOVERY LOG]
  packet_id  : 9999
  error_flags: 0x1
[RESULT] recovery count: 1
[TESTS] recovery tests passed
*/
