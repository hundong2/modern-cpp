/*
Practice: Static Buffer Validation

Network code often receives raw bytes first and only later interprets those
bytes as a packet struct. This example validates the byte count before reading
the packet fields.
*/

#include <cassert>
#include <cstddef>
#include <expected>
#include <iostream>
#include <span>
#include <string>

struct SecurePacket {
    int packet_id;
    int security_flags;
};

void inspect_packet(const SecurePacket& packet) {
    std::cout << "[VALIDATOR LOG]\n";
    std::cout << "  packet_id     : " << packet.packet_id << '\n';
    std::cout << "  security_flags: 0x" << std::hex << packet.security_flags << std::dec << '\n';
}

template <typename Packet>
std::expected<const Packet*, std::string> parse_packet(std::span<const std::byte> buffer) {
    // sizeof(Packet) is known at compile time. The runtime buffer must be at
    // least that large before we can safely read fields from it.
    if (buffer.size() < sizeof(Packet)) {
        return std::unexpected("buffer is smaller than packet layout");
    }

    // reinterpret_cast is a low-level cast. It does not copy bytes; it asks the
    // compiler to view the address as a different pointer type.
    const auto* packet = reinterpret_cast<const Packet*>(buffer.data());
    return packet;
}

std::expected<SecurePacket, std::string> run_validator_gateway() {
    int line_card_isolation_flag{};
    (void)line_card_isolation_flag;

    SecurePacket mock_packet{5555, 0x7F};
    const auto* byte_ptr = reinterpret_cast<const std::byte*>(&mock_packet);
    std::span<const std::byte> incoming_buffer(byte_ptr, sizeof(mock_packet));

    const auto parsed = parse_packet<SecurePacket>(incoming_buffer);
    if (!parsed) {
        return std::unexpected(parsed.error());
    }

    inspect_packet(**parsed);
    return **parsed;
}

void run_tests() {
    SecurePacket packet{1, 2};
    const auto* bytes = reinterpret_cast<const std::byte*>(&packet);
    assert(parse_packet<SecurePacket>(std::span<const std::byte>(bytes, sizeof(packet))).has_value());
    assert(!parse_packet<SecurePacket>(std::span<const std::byte>(bytes, 1)).has_value());
}

int main() {
    run_tests();

    const auto result = run_validator_gateway();
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[RESULT] accepted packet id: " << result->packet_id << '\n';
    std::cout << "[TESTS] validator tests passed\n";
    return 0;
}

/*
Execution result:
[VALIDATOR LOG]
  packet_id     : 5555
  security_flags: 0x7f
[RESULT] accepted packet id: 5555
[TESTS] validator tests passed
*/
