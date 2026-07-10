#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <expected>
#include <iomanip>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <tuple>

// 2026-07-09
// Topic: alignas, layout, and byte-view serialization.

struct alignas(8) VariableTelemetry {
    int device_id;
    int status_mask;
};

void inspect_payload(const VariableTelemetry& payload) {
    std::cout << "[LAYOUT]\n";
    std::cout << "  alignof: " << alignof(VariableTelemetry) << '\n';
    std::cout << "  sizeof : " << sizeof(VariableTelemetry) << '\n';
    std::cout << "  device_id  : " << payload.device_id << '\n';
    std::cout << "  status_mask: 0x" << std::hex << payload.status_mask << std::dec << '\n';
}

std::span<const std::byte> as_bytes(const VariableTelemetry& payload) {
    // This is a non-owning view over the object's bytes. It is valid only while
    // payload is alive and unchanged.
    return {reinterpret_cast<const std::byte*>(&payload), sizeof(VariableTelemetry)};
}

template <typename... Args>
std::expected<void, std::string> dispatch_to_transmitter(Args&&... args) {
    static_assert(sizeof...(Args) >= 2);

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& channel = std::get<0>(packed);
    const VariableTelemetry& payload = std::get<1>(packed);

    std::cout << "[TRANSMITTER] channel: " << channel << '\n';
    inspect_payload(payload);

    const auto bytes = as_bytes(payload);
    std::cout << "[BUFFER] zero-copy byte view size: " << bytes.size() << " bytes\n";
    return {};
}

void run_tests() {
    static_assert(alignof(VariableTelemetry) == 8);
    VariableTelemetry payload{9901, 0x00FF00FF};
    assert(as_bytes(payload).size() == sizeof(VariableTelemetry));
}

int main() {
    int pipeline_security_flag{};
    (void)pipeline_security_flag;

    run_tests();

    VariableTelemetry payload{9901, 0x00FF00FF};
    const auto result = dispatch_to_transmitter("NIC_ETH_0", payload, 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[TESTS] layout serialization demo passed\n";
    return 0;
}
