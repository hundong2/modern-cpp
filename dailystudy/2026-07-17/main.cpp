/*
Practice: Thread Affinity Mapping Simulation

The briefing talks about C++26 reflection-driven hardware thread affinity. This
portable C++23 version does not pin OS threads. Instead, it models the mapping
decision, checks the requested core against hardware_concurrency(), and runs a
small async-style stage with std::jthread.
*/

#include <algorithm>
#include <cassert>
#include <expected>
#include <iostream>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

struct alignas(64) ExchangeChannel {
    int core_target_id;
    std::string_view channel_name;
};

struct CoreAssignment {
    int requested_core;
    unsigned hardware_threads;
    std::string_view channel_name;
    bool within_range;
};

void inspect_topology(const ExchangeChannel& channel) {
    std::cout << "[AFFINITY TOPOLOGY LOG]\n";

    const std::vector<std::pair<std::string_view, std::string_view>> fields{
        {"core_target_id", "integer CPU core request"},
        {"channel_name", "network channel label"}
    };

    for (const auto& [name, meaning] : fields) {
        std::cout << "  " << name << " -> " << meaning << '\n';
    }

    std::cout << "  requested core : " << channel.core_target_id << '\n';
    std::cout << "  channel        : " << channel.channel_name << '\n';
}

CoreAssignment map_to_core(const ExchangeChannel& channel) {
    const unsigned detected = std::max(1u, std::thread::hardware_concurrency());
    const bool valid = channel.core_target_id >= 0
        && static_cast<unsigned>(channel.core_target_id) < detected;

    return CoreAssignment{
        channel.core_target_id,
        detected,
        channel.channel_name,
        valid
    };
}

template <typename Handler>
void run_async_stage(ExchangeChannel channel, Handler&& handler) {
    std::jthread worker([channel, callback = std::forward<Handler>(handler)]() mutable {
        callback(channel);
    });
}

template <typename... Args>
std::expected<CoreAssignment, std::string_view> dispatch_to_core(Args&&... args) {
    static_assert(sizeof...(Args) >= 2, "marker and exchange channel are required");

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& marker = std::get<0>(packed);
    const ExchangeChannel& target_channel = std::get<1>(packed);

    std::cout << "[ENGINE] marker: " << marker << '\n';
    inspect_topology(target_channel);

    const CoreAssignment assignment = map_to_core(target_channel);
    if (!assignment.within_range) {
        return std::unexpected("requested core is outside detected hardware range");
    }

    run_async_stage(target_channel, [](const ExchangeChannel& channel) {
        std::cout << "[THREAD] simulated lock-free pin to core "
                  << channel.core_target_id << " for " << channel.channel_name << '\n';
    });

    return assignment;
}

void run_tests() {
    static_assert(alignof(ExchangeChannel) == 64);

    ExchangeChannel primary{0, "TEST_CH"};
    const auto assignment = dispatch_to_core("TEST", primary);
    assert(assignment.has_value());
    assert(assignment->requested_core == 0);
}

int main() {
    run_tests();

    int line_card_affinity_status{};
    (void)line_card_affinity_status;

    ExchangeChannel primary_ch{0, "NIC_FIBER_CH_0"};
    const auto result = dispatch_to_core("CORE_AFFINITY_EXEC", primary_ch, 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[RESULT] hardware threads detected: "
              << result->hardware_threads << '\n';
    std::cout << "[TESTS] affinity mapping tests passed\n";
    return 0;
}
