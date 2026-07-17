/*
Bonus Practice: Static DI + Security Validation Pipeline

The user provided two 2026-07-13 briefings. main.cpp keeps the error recovery
briefing; this bonus executable covers the static dependency injection and
zero-copy security validation pipeline from the other briefing.
*/

#include <cassert>
#include <cstddef>
#include <expected>
#include <iostream>
#include <span>
#include <string_view>
#include <tuple>

struct AdvancedSecurityValidator {
    std::string_view engine_name = "STATIC_SANDBOX_V4";

    bool verify_integrity(std::span<const std::byte> buffer) const {
        return buffer.size() >= 8;
    }
};

template <typename ValidatorType>
struct IntegratedGateway {
    ValidatorType validator;

    std::expected<std::size_t, std::string_view> process_ingress(std::span<const std::byte> buffer) const {
        if (!validator.verify_integrity(buffer)) {
            return std::unexpected("packet is smaller than the required header");
        }

        std::cout << "[GATEWAY] " << validator.engine_name
                  << " verified " << buffer.size() << " bytes\n";
        return buffer.size();
    }
};

template <typename Gateway>
void inspect_pipeline(const Gateway&) {
    std::cout << "[INTEGRATED SYSTEM DUMP]\n";
    std::cout << "  component slot: validator\n";
    std::cout << "  binding       : compile-time template dependency\n";
}

template <typename... Args>
std::expected<std::size_t, std::string_view> dispatch_to_pipeline(Args&&... args) {
    static_assert(sizeof...(Args) >= 3, "marker, gateway, and byte span are required");

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& marker = std::get<0>(packed);
    const auto& gateway = std::get<1>(packed);
    const auto buffer = std::get<2>(packed);

    std::cout << "[ENGINE] marker: " << marker << '\n';
    inspect_pipeline(gateway);
    return gateway.process_ingress(buffer);
}

void run_tests() {
    std::byte raw_packet[8]{std::byte{0x7E}};
    std::span<const std::byte> buffer(raw_packet);

    IntegratedGateway<AdvancedSecurityValidator> gateway{AdvancedSecurityValidator{}};
    const auto result = dispatch_to_pipeline("TEST", gateway, buffer);

    assert(result.has_value());
    assert(result.value() == 8);
}

int main() {
    run_tests();

    int line_card_security_token{};
    (void)line_card_security_token;

    std::byte raw_packet[8]{
        std::byte{0x7E}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
        std::byte{0x01}, std::byte{0x02}, std::byte{0x03}, std::byte{0x04}
    };
    std::span<const std::byte> buffer_view(raw_packet);

    IntegratedGateway<AdvancedSecurityValidator> gateway{AdvancedSecurityValidator{}};
    const auto result = dispatch_to_pipeline("NIC_INTEGRATED_CH_0", gateway, buffer_view, 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[RESULT] validated bytes: " << result.value() << '\n';
    std::cout << "[TESTS] integrated gateway tests passed\n";
    return 0;
}
