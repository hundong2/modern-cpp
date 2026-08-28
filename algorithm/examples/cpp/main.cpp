#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using Graph = std::vector<std::vector<int>>;
using WeightedGraph = std::vector<std::vector<std::pair<int, int>>>;

void check(bool condition, const std::string& name) {
    if (!condition) {
        throw std::runtime_error("check failed: " + name);
    }
    std::cout << "[PASS] " << name << '\n';
}

int linear_search(const std::vector<int>& values, int target) {
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (values[i] == target) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// [left, right) 안에 target 이상인 첫 위치를 유지하는 lower_bound 구현이다.
std::size_t lower_bound_index(const std::vector<int>& sorted, int target) {
    std::size_t left = 0;
    std::size_t right = sorted.size();
    while (left < right) {
        const std::size_t middle = left + (right - left) / 2;
        if (sorted[middle] < target) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }
    return left;
}

void merge_sort_range(std::vector<int>& values, std::vector<int>& buffer,
                      std::size_t begin, std::size_t end) {
    if (end - begin <= 1) {
        return;
    }
    const std::size_t middle = begin + (end - begin) / 2;
    merge_sort_range(values, buffer, begin, middle);
    merge_sort_range(values, buffer, middle, end);

    std::size_t left = begin;
    std::size_t right = middle;
    std::size_t out = begin;
    while (left < middle && right < end) {
        buffer[out++] = values[left] <= values[right] ? values[left++] : values[right++];
    }
    while (left < middle) {
        buffer[out++] = values[left++];
    }
    while (right < end) {
        buffer[out++] = values[right++];
    }
    std::copy(buffer.begin() + static_cast<std::ptrdiff_t>(begin),
              buffer.begin() + static_cast<std::ptrdiff_t>(end),
              values.begin() + static_cast<std::ptrdiff_t>(begin));
}

void merge_sort(std::vector<int>& values) {
    std::vector<int> buffer(values.size());
    merge_sort_range(values, buffer, 0, values.size());
}

struct TreeNode {
    explicit TreeNode(int value) : value(value) {}
    int value;
    std::unique_ptr<TreeNode> left;
    std::unique_ptr<TreeNode> right;
};

void preorder(const TreeNode* node, std::vector<int>& result) {
    if (node == nullptr) {
        return;
    }
    result.push_back(node->value);
    preorder(node->left.get(), result);
    preorder(node->right.get(), result);
}

void inorder(const TreeNode* node, std::vector<int>& result) {
    if (node == nullptr) {
        return;
    }
    inorder(node->left.get(), result);
    result.push_back(node->value);
    inorder(node->right.get(), result);
}

void postorder(const TreeNode* node, std::vector<int>& result) {
    if (node == nullptr) {
        return;
    }
    postorder(node->left.get(), result);
    postorder(node->right.get(), result);
    result.push_back(node->value);
}

std::vector<int> level_order(const TreeNode* root) {
    if (root == nullptr) {
        return {};
    }
    std::vector<int> result;
    std::queue<const TreeNode*> pending;
    pending.push(root);
    while (!pending.empty()) {
        const TreeNode* current = pending.front();
        pending.pop();
        result.push_back(current->value);
        if (current->left) {
            pending.push(current->left.get());
        }
        if (current->right) {
            pending.push(current->right.get());
        }
    }
    return result;
}

std::vector<int> bfs(const Graph& graph, int start) {
    std::vector<int> order;
    std::vector<bool> visited(graph.size(), false);
    std::queue<int> pending;
    visited[static_cast<std::size_t>(start)] = true;
    pending.push(start);
    while (!pending.empty()) {
        const int current = pending.front();
        pending.pop();
        order.push_back(current);
        for (const int next : graph[static_cast<std::size_t>(current)]) {
            if (!visited[static_cast<std::size_t>(next)]) {
                visited[static_cast<std::size_t>(next)] = true;
                pending.push(next);
            }
        }
    }
    return order;
}

void dfs_visit(const Graph& graph, int current, std::vector<bool>& visited,
               std::vector<int>& order) {
    visited[static_cast<std::size_t>(current)] = true;
    order.push_back(current);
    for (const int next : graph[static_cast<std::size_t>(current)]) {
        if (!visited[static_cast<std::size_t>(next)]) {
            dfs_visit(graph, next, visited, order);
        }
    }
}

std::vector<int> dfs(const Graph& graph, int start) {
    std::vector<bool> visited(graph.size(), false);
    std::vector<int> order;
    dfs_visit(graph, start, visited, order);
    return order;
}

std::vector<long long> dijkstra(const WeightedGraph& graph, int start) {
    constexpr long long infinity = std::numeric_limits<long long>::max() / 4;
    using State = std::pair<long long, int>;
    std::vector<long long> distance(graph.size(), infinity);
    std::priority_queue<State, std::vector<State>, std::greater<State>> heap;
    distance[static_cast<std::size_t>(start)] = 0;
    heap.emplace(0, start);

    while (!heap.empty()) {
        const auto [current_distance, current] = heap.top();
        heap.pop();
        if (current_distance != distance[static_cast<std::size_t>(current)]) {
            continue; // 더 좋은 경로가 이미 기록된 오래된 힙 항목이다.
        }
        for (const auto& [next, weight] : graph[static_cast<std::size_t>(current)]) {
            const long long candidate = current_distance + weight;
            if (candidate < distance[static_cast<std::size_t>(next)]) {
                distance[static_cast<std::size_t>(next)] = candidate;
                heap.emplace(candidate, next);
            }
        }
    }
    return distance;
}

std::vector<int> topological_sort(const Graph& graph) {
    std::vector<int> indegree(graph.size(), 0);
    for (const auto& edges : graph) {
        for (const int next : edges) {
            ++indegree[static_cast<std::size_t>(next)];
        }
    }
    std::queue<int> ready;
    for (std::size_t vertex = 0; vertex < indegree.size(); ++vertex) {
        if (indegree[vertex] == 0) {
            ready.push(static_cast<int>(vertex));
        }
    }
    std::vector<int> order;
    while (!ready.empty()) {
        const int current = ready.front();
        ready.pop();
        order.push_back(current);
        for (const int next : graph[static_cast<std::size_t>(current)]) {
            if (--indegree[static_cast<std::size_t>(next)] == 0) {
                ready.push(next);
            }
        }
    }
    return order; // 크기가 V보다 작으면 사이클이 있다.
}

class DisjointSet {
public:
    explicit DisjointSet(int size) : parent_(static_cast<std::size_t>(size)),
                                     sizes_(static_cast<std::size_t>(size), 1) {
        for (int i = 0; i < size; ++i) {
            parent_[static_cast<std::size_t>(i)] = i;
        }
    }

    int find(int value) {
        int& parent = parent_[static_cast<std::size_t>(value)];
        if (parent != value) {
            parent = find(parent); // 경로 압축
        }
        return parent;
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) {
            return false;
        }
        if (sizes_[static_cast<std::size_t>(a)] < sizes_[static_cast<std::size_t>(b)]) {
            std::swap(a, b);
        }
        parent_[static_cast<std::size_t>(b)] = a;
        sizes_[static_cast<std::size_t>(a)] += sizes_[static_cast<std::size_t>(b)];
        return true;
    }

private:
    std::vector<int> parent_;
    std::vector<int> sizes_;
};

std::vector<long long> prefix_sums(const std::vector<int>& values) {
    std::vector<long long> prefix(values.size() + 1, 0);
    for (std::size_t i = 0; i < values.size(); ++i) {
        prefix[i + 1] = prefix[i] + values[i];
    }
    return prefix;
}

long long maximum_window_sum(const std::vector<int>& values, std::size_t width) {
    if (width == 0 || width > values.size()) {
        throw std::invalid_argument("window width must be in [1, size]");
    }
    long long window = 0;
    for (std::size_t i = 0; i < width; ++i) {
        window += values[i];
    }
    long long best = window;
    for (std::size_t right = width; right < values.size(); ++right) {
        window += values[right] - values[right - width];
        best = std::max(best, window);
    }
    return best;
}

long long climb_stairs(int steps) {
    if (steps < 0) {
        return 0;
    }
    long long previous = 1; // dp[0]
    long long current = 1;  // dp[1]
    for (int i = 2; i <= steps; ++i) {
        const long long next = previous + current;
        previous = current;
        current = next;
    }
    return current;
}

int main() {
    try {
        const std::vector<int> values{7, 2, 9, 2, 5};
        check(linear_search(values, 9) == 2, "linear search");

        std::vector<int> sorted = values;
        merge_sort(sorted);
        check(sorted == std::vector<int>({2, 2, 5, 7, 9}), "merge sort");
        check(lower_bound_index(sorted, 2) == 0 && lower_bound_index(sorted, 6) == 3,
              "binary lower bound");

        auto root = std::make_unique<TreeNode>(1);
        root->left = std::make_unique<TreeNode>(2);
        root->right = std::make_unique<TreeNode>(3);
        root->left->left = std::make_unique<TreeNode>(4);
        root->left->right = std::make_unique<TreeNode>(5);
        std::vector<int> pre;
        std::vector<int> in;
        std::vector<int> post;
        preorder(root.get(), pre);
        inorder(root.get(), in);
        postorder(root.get(), post);
        check(pre == std::vector<int>({1, 2, 4, 5, 3}), "tree preorder DFS");
        check(in == std::vector<int>({4, 2, 5, 1, 3}), "tree inorder DFS");
        check(post == std::vector<int>({4, 5, 2, 3, 1}), "tree postorder DFS");
        check(level_order(root.get()) == std::vector<int>({1, 2, 3, 4, 5}),
              "tree level-order BFS");

        const Graph graph{{1, 2}, {0, 3, 4}, {0, 4}, {1}, {1, 2}};
        check(bfs(graph, 0) == std::vector<int>({0, 1, 2, 3, 4}), "graph BFS");
        check(dfs(graph, 0) == std::vector<int>({0, 1, 3, 4, 2}), "graph DFS");

        const WeightedGraph weighted{{{1, 4}, {2, 1}}, {{3, 1}}, {{1, 2}, {3, 5}}, {}};
        check(dijkstra(weighted, 0) == std::vector<long long>({0, 3, 1, 4}),
              "Dijkstra shortest path");

        const Graph dag{{1, 2}, {3}, {3}, {}};
        const auto topo = topological_sort(dag);
        check(topo.size() == dag.size() && topo.front() == 0 && topo.back() == 3,
              "topological sort");

        DisjointSet sets(5);
        sets.unite(0, 1);
        sets.unite(1, 2);
        check(sets.find(0) == sets.find(2) && sets.find(0) != sets.find(3),
              "disjoint set union");

        const auto prefix = prefix_sums(values);
        check(prefix[4] - prefix[1] == 13, "prefix sum [1, 4)");
        check(maximum_window_sum(values, 3) == 18, "sliding window maximum sum");
        check(climb_stairs(5) == 8, "dynamic programming");

        std::cout << "ALL CHECKS PASSED\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
