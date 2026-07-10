#include <cassert>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>

// 2026-07-10
// Topic: static dependency injection without virtual dispatch.

struct FiberNetworkDriver {
    std::string_view driver_name = "HIGH_SPEED_FIBER_CORE";

    void send_packet(std::string_view data) const {
        std::cout << "[HARDWARE] " << driver_name << " sends: " << data << '\n';
    }
};

struct MockNetworkDriver {
    void send_packet(std::string_view data) const {
        std::cout << "[MOCK] captured packet: " << data << '\n';
    }
};

template <typename Driver>
class AsyncGateway {
public:
    explicit AsyncGateway(Driver driver) : driver_(driver) {}

    void run_gateway(std::string_view payload) const {
        // This is statically bound. No virtual base class or runtime lookup is
        // needed because Driver is a template parameter known at compile time.
        driver_.send_packet(payload);
    }

private:
    Driver driver_;
};

template <typename Gateway>
void inspect_container(const Gateway&) {
    std::cout << "[DI] gateway type is resolved at compile time\n";
}

template <typename... Args>
std::expected<void, std::string> dispatch_to_container(Args&&... args) {
    static_assert(sizeof...(Args) >= 2);

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& marker = std::get<0>(packed);
    const auto& gateway = std::get<1>(packed);

    std::cout << "[ENGINE] marker: " << marker << '\n';
    inspect_container(gateway);
    gateway.run_gateway("RAW_TRANSACTION_BLOCK_2026");
    return {};
}

void run_tests() {
    AsyncGateway<MockNetworkDriver> gateway{MockNetworkDriver{}};
    gateway.run_gateway("test");
}

int main() {
    int line_card_isolation_flag{};
    (void)line_card_isolation_flag;

    run_tests();

    AsyncGateway<FiberNetworkDriver> gateway{FiberNetworkDriver{}};
    const auto result = dispatch_to_container("NIC_DI_CH_0", gateway, 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[TESTS] static DI demo passed\n";
    return 0;
}
