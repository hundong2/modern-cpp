#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <tuple>

// 2026-07-08
// Topic: MMIO-style register access with volatile.
//
// Real MMIO writes to physical addresses are not safe in a user-mode practice
// program. This example uses a local mock register block with volatile fields so
// the concept can be practiced without touching real hardware.

struct NetworkRegisterFabric {
    volatile std::uint32_t transmit_trigger;
    volatile std::uint32_t buffer_status;
};

struct DevicePacket {
    int device_id;
    std::string_view stream_data;
};

void inspect_packet(const DevicePacket& packet) {
    std::cout << "[DEVICE PACKET]\n";
    std::cout << "  device_id  : " << packet.device_id << '\n';
    std::cout << "  stream_data: " << packet.stream_data << '\n';
}

std::expected<void, std::string> write_to_registers(NetworkRegisterFabric& registers,
                                                    const DevicePacket& packet) {
    if (packet.stream_data.empty()) {
        return std::unexpected("stream data must not be empty");
    }

    // volatile prevents the compiler from removing these writes as "unused".
    // That matters for hardware registers because writing itself has side effects.
    registers.transmit_trigger = 0x1;
    registers.buffer_status = 0x00FF00FF;
    return {};
}

template <typename... Args>
std::expected<void, std::string> dispatch_to_hardware_register(NetworkRegisterFabric& registers,
                                                               Args&&... args) {
    static_assert(sizeof...(Args) >= 2);

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& channel = std::get<0>(packed);
    const DevicePacket& packet = std::get<1>(packed);

    std::cout << "[REG_ENGINE] channel: " << channel << '\n';
    auto result = write_to_registers(registers, packet);
    if (!result) {
        return result;
    }

    inspect_packet(packet);
    std::cout << "[MMIO MOCK] trigger=" << registers.transmit_trigger
              << ", status=0x" << std::hex << registers.buffer_status << std::dec << '\n';
    return {};
}

void run_tests() {
    NetworkRegisterFabric registers{0, 0};
    DevicePacket packet{777, "NIC_REG_BURST_STREAM"};
    auto result = write_to_registers(registers, packet);
    assert(result.has_value());
    assert(registers.transmit_trigger == 1);
}

int main() {
    int line_card_interrupt_vector{};
    (void)line_card_interrupt_vector;

    run_tests();

    NetworkRegisterFabric registers{0, 0};
    DevicePacket packet{777, "NIC_REG_BURST_STREAM"};
    const auto result = dispatch_to_hardware_register(registers, "NIC_TX_CHANNEL", packet, 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[TESTS] MMIO mock demo passed\n";
    return 0;
}
