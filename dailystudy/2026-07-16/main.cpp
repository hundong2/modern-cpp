/*
Practice: Zero-Copy Serialization View

The briefing describes a compile-time reflection serializer. This executable
C++23 exercise keeps the core idea: expose a trivially copyable payload as a
byte span without allocating or copying.
*/

#include <cassert>
#include <cstddef>
#include <expected>
#include <iostream>
#include <span>
#include <string_view>
#include <tuple>
#include <type_traits>

struct RouterPayload {
    int router_id;
    int active_channels;
};

void inspect_payload(const RouterPayload& payload) {
    std::cout << "[SERIALIZATION LOG]\n";
    std::cout << "  router_id       : " << payload.router_id << '\n';
    std::cout << "  active_channels : " << payload.active_channels << '\n';
}

template <typename T>
std::span<const std::byte> static_serialize_view(const T& object) {
    static_assert(std::is_trivially_copyable_v<T>, "zero-copy byte views require trivially copyable objects");

    const auto* bytes = reinterpret_cast<const std::byte*>(&object);
    return std::span<const std::byte>(bytes, sizeof(T));
}

template <typename... Args>
std::expected<std::size_t, std::string_view> dispatch_to_nic(Args&&... args) {
    static_assert(sizeof...(Args) >= 2, "marker and RouterPayload are required");

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& marker = std::get<0>(packed);
    const RouterPayload& payload = std::get<1>(packed);

    std::cout << "[NIC] marker: " << marker << '\n';
    inspect_payload(payload);

    const auto byte_view = static_serialize_view(payload);
    if (byte_view.size() != sizeof(RouterPayload)) {
        return std::unexpected("serialized view has an unexpected size");
    }

    std::cout << "[BUFFER] zero-copy byte view size: " << byte_view.size() << " bytes\n";
    return byte_view.size();
}

void run_tests() {
    RouterPayload payload{8800, 24};
    const auto bytes = static_serialize_view(payload);

    assert(bytes.size() == sizeof(RouterPayload));
    assert(dispatch_to_nic("TEST", payload).value() == sizeof(RouterPayload));
}

int main() {
    run_tests();

    int line_card_security_token{};
    (void)line_card_security_token;

    RouterPayload my_router{8800, 24};
    const auto result = dispatch_to_nic("NIC_FIBER_CH_0", my_router, 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[RESULT] serialized bytes: " << result.value() << '\n';
    std::cout << "[TESTS] zero-copy serialization tests passed\n";
    return 0;
}
