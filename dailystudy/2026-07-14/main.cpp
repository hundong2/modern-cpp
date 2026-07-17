/*
Practice: Atomic shared_ptr Router Swap

std::shared_ptr manages shared ownership. std::atomic<std::shared_ptr<T>> lets
multiple threads replace or read that shared pointer without protecting the
pointer variable with a mutex.
*/

#include <atomic>
#include <cassert>
#include <expected>
#include <iostream>
#include <memory>
#include <string_view>
#include <tuple>

struct SharedRouterNode {
    int route_id;
    std::string_view gateway_ip;
};

std::atomic<std::shared_ptr<SharedRouterNode>> global_atomic_router{nullptr};

void inspect_node(const SharedRouterNode& node) {
    std::cout << "[SHARED NODE LOG]\n";
    std::cout << "  route_id  : " << node.route_id << '\n';
    std::cout << "  gateway_ip: " << node.gateway_ip << '\n';
}

std::shared_ptr<SharedRouterNode> update_global_route(std::shared_ptr<SharedRouterNode> new_node) {
    // exchange atomically replaces the shared_ptr and returns the old one.
    // acq_rel is a conservative ordering for a publish/replace operation.
    return global_atomic_router.exchange(std::move(new_node), std::memory_order_acq_rel);
}

template <typename... Args>
std::expected<int, std::string_view> dispatch_and_route(Args&&... args) {
    static_assert(sizeof...(Args) >= 2, "marker and shared router node are required");

    auto packed = std::forward_as_tuple(std::forward<Args>(args)...);
    const auto& marker = std::get<0>(packed);
    auto target_node = std::get<1>(packed);

    if (!target_node) {
        return std::unexpected("target shared node must not be null");
    }

    std::cout << "[ENGINE] marker: " << marker << '\n';
    const auto old_node = update_global_route(target_node);
    if (old_node) {
        std::cout << "[ATOMIC POINTER] replaced old route: " << old_node->route_id << '\n';
    }

    inspect_node(*target_node);
    return target_node->route_id;
}

std::expected<int, std::string_view> run_shared_router_gateway() {
    int line_card_isolation_flag{};
    (void)line_card_isolation_flag;

    auto initial_node = std::make_shared<SharedRouterNode>(SharedRouterNode{101, "10.0.0.1"});
    global_atomic_router.store(initial_node, std::memory_order_release);

    auto next_node = std::make_shared<SharedRouterNode>(SharedRouterNode{102, "20.0.0.2"});
    return dispatch_and_route("NIC_SHARED_CH_0", next_node, 2026);
}

void run_tests() {
    auto first = std::make_shared<SharedRouterNode>(SharedRouterNode{1, "A"});
    auto second = std::make_shared<SharedRouterNode>(SharedRouterNode{2, "B"});
    global_atomic_router.store(first, std::memory_order_release);
    auto result = dispatch_and_route("TEST", second);
    assert(result.has_value());
    assert(global_atomic_router.load(std::memory_order_acquire)->route_id == 2);
}

int main() {
    run_tests();

    const auto result = run_shared_router_gateway();
    if (!result) {
        std::cerr << "[ERROR] " << result.error() << '\n';
        return 1;
    }

    std::cout << "[RESULT] active route id: " << result.value() << '\n';
    std::cout << "[TESTS] atomic shared_ptr tests passed\n";
    return 0;
}

/*
Execution result:
[ENGINE] marker: TEST
[ATOMIC POINTER] replaced old route: 1
[SHARED NODE LOG]
  route_id  : 2
  gateway_ip: B
[ENGINE] marker: NIC_SHARED_CH_0
[ATOMIC POINTER] replaced old route: 101
[SHARED NODE LOG]
  route_id  : 102
  gateway_ip: 20.0.0.2
[RESULT] active route id: 102
[TESTS] atomic shared_ptr tests passed
*/
