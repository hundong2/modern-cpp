#include <atomic>
#include <cassert>
#include <expected>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

// 2026-07-06
// Topic: lock-free pointer replacement with compare_exchange_strong.

struct alignas(64) RoutingNode {
    int node_id;
    std::string_view target_route;
};

void inspect_node(const RoutingNode& node) {
    std::cout << "[ROUTING NODE]\n";
    std::cout << "  node_id     : " << node.node_id << '\n';
    std::cout << "  target_route: " << node.target_route << '\n';
}

class RoutingTable {
public:
    explicit RoutingTable(RoutingNode* initial) : current_(initial) {}

    bool try_replace(RoutingNode* expected, RoutingNode* next) {
        // compare_exchange_strong means:
        // "If current_ still equals expected, replace it with next atomically."
        return current_.compare_exchange_strong(
            expected,
            next,
            std::memory_order_release,
            std::memory_order_relaxed);
    }

    RoutingNode* load() const {
        return current_.load(std::memory_order_acquire);
    }

private:
    std::atomic<RoutingNode*> current_;
};

template <typename... Args>
std::expected<void, std::string> dispatch_and_swap_node(RoutingTable& table, Args&&... args) {
    static_assert(sizeof...(Args) >= 2);

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& marker = std::get<0>(packed);
    RoutingNode* new_node = std::get<1>(packed);

    RoutingNode* expected = table.load();
    if (!table.try_replace(expected, new_node)) {
        return std::unexpected("routing node was changed by another writer");
    }

    std::cout << "[ENGINE] marker: " << marker << '\n';
    std::cout << "[CAS] routing node replaced without mutex\n";
    inspect_node(*new_node);
    return {};
}

void run_tests() {
    auto a = std::make_unique<RoutingNode>(RoutingNode{1, "A"});
    auto b = std::make_unique<RoutingNode>(RoutingNode{2, "B"});
    RoutingTable table(a.get());

    assert(table.try_replace(a.get(), b.get()));
    assert(table.load() == b.get());
}

int main() {
    int line_card_isolation_mask{};
    (void)line_card_isolation_mask;

    run_tests();

    auto initial_node = std::make_unique<RoutingNode>(RoutingNode{101, "PRIMARY_FIBER_PATH"});
    auto next_node = std::make_unique<RoutingNode>(RoutingNode{102, "BACKUP_SATELLITE_PATH"});

    RoutingTable table(initial_node.get());
    const auto result = dispatch_and_swap_node(table, "SWITCH_FABRIC_0", next_node.get(), 2026);
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[TESTS] CAS demo passed\n";
    return 0;
}
