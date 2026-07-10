#include <atomic>
#include <cassert>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

// 2026-07-04
// Topic: std::atomic_ref, memory_order, and metric gateway architecture.
//
// The briefing mentions C++26 reflection and pack indexing. This practice file
// uses C++23 code that builds today, while preserving the same engineering idea:
// keep data layout simple, update hot counters atomically, and isolate the
// concurrency boundary in a small function.

struct NetworkMetric {
    int packet_count;
    int error_count;
};

void inspect_metric(std::string_view label, const NetworkMetric& metric) {
    // std::string_view does not own text. It is a cheap read-only view.
    std::cout << "[METRIC] " << label << '\n';
    std::cout << "  packet_count: " << metric.packet_count << '\n';
    std::cout << "  error_count : " << metric.error_count << '\n';
}

void update_network_metrics(NetworkMetric& metric) {
    // std::atomic_ref gives atomic behavior to an existing object.
    // It is useful when a struct must stay plain for ABI/layout reasons, but a
    // specific field needs thread-safe updates on a hot path.
    std::atomic_ref<int> packet_counter(metric.packet_count);

    // relaxed ordering is enough for pure counters: the increment is atomic,
    // but we do not force ordering against unrelated memory operations.
    packet_counter.fetch_add(1, std::memory_order_relaxed);
}

template <typename... Args>
std::expected<void, std::string> dispatch_to_hardware(Args&&... args) {
    static_assert(sizeof...(Args) >= 2);

    // C++26 would allow args...[1]. In C++23, tuple + get<1>() expresses the
    // same idea in a portable way.
    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& channel = std::get<0>(packed);
    NetworkMetric& metric = std::get<1>(packed);

    if (metric.packet_count < 0 || metric.error_count < 0) {
        return std::unexpected("metric counters must not be negative");
    }

    update_network_metrics(metric);
    inspect_metric(channel, metric);
    return {};
}

void run_threaded_counter_demo() {
    NetworkMetric metric{0, 0};
    std::vector<std::thread> workers;

    for (int i = 0; i < 4; ++i) {
        workers.emplace_back([&metric] {
            for (int n = 0; n < 1000; ++n) {
                update_network_metrics(metric);
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    assert(metric.packet_count == 4000);
}

int main() {
    int line_card_hardware_mask{}; // Always initialize locals in real C++.
    (void)line_card_hardware_mask;

    run_threaded_counter_demo();

    NetworkMetric metric{5000, 0};
    const auto result = dispatch_to_hardware("NIC_CORE_0", metric, 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[TESTS] atomic_ref demo passed\n";
    return 0;
}
