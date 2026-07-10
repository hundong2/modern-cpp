#include <cassert>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>

// 2026-07-07
// Topic: vtable-free static polymorphism with std::variant.

struct HttpPacket {
    std::string_view url;

    void process() const {
        std::cout << "[HTTP] route url: " << url << '\n';
    }
};

struct DnsPacket {
    std::string_view domain;

    void process() const {
        std::cout << "[DNS] resolve domain: " << domain << '\n';
    }
};

using PacketVariant = std::variant<HttpPacket, DnsPacket>;

template <typename Packet>
void inspect_protocol(const Packet& packet) {
    // C++26 reflection could report the type name automatically. In C++23,
    // overload behavior is selected statically by the template instantiation.
    packet.process();
}

void route_packet_statically(const PacketVariant& packet) {
    // std::visit calls the lambda with the currently active alternative.
    // This avoids a base class and virtual function table.
    std::visit([](const auto& actual_packet) {
        inspect_protocol(actual_packet);
    }, packet);
}

template <typename... Args>
std::expected<void, std::string> dispatch_to_switch(Args&&... args) {
    static_assert(sizeof...(Args) >= 2);

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& marker = std::get<0>(packed);
    const PacketVariant& packet = std::get<1>(packed);

    std::cout << "[SWITCH] marker: " << marker << '\n';
    route_packet_statically(packet);
    return {};
}

void run_tests() {
    PacketVariant packet = DnsPacket{"example.com"};
    assert(std::holds_alternative<DnsPacket>(packet));
}

int main() {
    int line_card_register_buffer{};
    (void)line_card_register_buffer;

    run_tests();

    PacketVariant incoming = HttpPacket{"/api/v2/exchange/2026"};
    const auto result = dispatch_to_switch("MARKER_URGENT", incoming, 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[TESTS] variant dispatch demo passed\n";
    return 0;
}
