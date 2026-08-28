"""알고리즘 기초를 한 파일에서 실행하고 검증하는 Python 3 예제."""

from __future__ import annotations

from collections import deque
from dataclasses import dataclass
import heapq
from typing import Optional


def check(condition: bool, name: str) -> None:
    if not condition:
        raise AssertionError(f"check failed: {name}")
    print(f"[PASS] {name}")


def linear_search(values: list[int], target: int) -> int:
    for index, value in enumerate(values):
        if value == target:
            return index
    return -1


def lower_bound(sorted_values: list[int], target: int) -> int:
    """반열린 구간 [left, right)에서 target 이상인 첫 위치를 반환한다."""
    left, right = 0, len(sorted_values)
    while left < right:
        middle = left + (right - left) // 2
        if sorted_values[middle] < target:
            left = middle + 1
        else:
            right = middle
    return left


def merge_sort(values: list[int]) -> list[int]:
    if len(values) <= 1:
        return values.copy()
    middle = len(values) // 2
    left = merge_sort(values[:middle])
    right = merge_sort(values[middle:])
    merged: list[int] = []
    left_index = right_index = 0
    while left_index < len(left) and right_index < len(right):
        if left[left_index] <= right[right_index]:
            merged.append(left[left_index])
            left_index += 1
        else:
            merged.append(right[right_index])
            right_index += 1
    merged.extend(left[left_index:])
    merged.extend(right[right_index:])
    return merged


@dataclass
class TreeNode:
    value: int
    left: Optional[TreeNode] = None
    right: Optional[TreeNode] = None


def preorder(node: Optional[TreeNode], result: list[int]) -> None:
    if node is None:
        return
    result.append(node.value)
    preorder(node.left, result)
    preorder(node.right, result)


def inorder(node: Optional[TreeNode], result: list[int]) -> None:
    if node is None:
        return
    inorder(node.left, result)
    result.append(node.value)
    inorder(node.right, result)


def postorder(node: Optional[TreeNode], result: list[int]) -> None:
    if node is None:
        return
    postorder(node.left, result)
    postorder(node.right, result)
    result.append(node.value)


def level_order(root: Optional[TreeNode]) -> list[int]:
    if root is None:
        return []
    result: list[int] = []
    pending = deque([root])
    while pending:
        current = pending.popleft()
        result.append(current.value)
        if current.left is not None:
            pending.append(current.left)
        if current.right is not None:
            pending.append(current.right)
    return result


def bfs(graph: list[list[int]], start: int) -> list[int]:
    visited = [False] * len(graph)
    visited[start] = True  # 큐에 넣을 때 표시해야 중복 삽입을 막는다.
    pending = deque([start])
    order: list[int] = []
    while pending:
        current = pending.popleft()
        order.append(current)
        for neighbor in graph[current]:
            if not visited[neighbor]:
                visited[neighbor] = True
                pending.append(neighbor)
    return order


def dfs(graph: list[list[int]], start: int) -> list[int]:
    visited = [False] * len(graph)
    order: list[int] = []

    def visit(current: int) -> None:
        visited[current] = True
        order.append(current)
        for neighbor in graph[current]:
            if not visited[neighbor]:
                visit(neighbor)

    visit(start)
    return order


def dijkstra(graph: list[list[tuple[int, int]]], start: int) -> list[float | int]:
    distance: list[float | int] = [float("inf")] * len(graph)
    distance[start] = 0
    heap: list[tuple[int, int]] = [(0, start)]
    while heap:
        current_distance, current = heapq.heappop(heap)
        if current_distance != distance[current]:
            continue
        for neighbor, weight in graph[current]:
            candidate = current_distance + weight
            if candidate < distance[neighbor]:
                distance[neighbor] = candidate
                heapq.heappush(heap, (candidate, neighbor))
    return distance


def topological_sort(graph: list[list[int]]) -> list[int]:
    indegree = [0] * len(graph)
    for edges in graph:
        for neighbor in edges:
            indegree[neighbor] += 1
    ready = deque(vertex for vertex, degree in enumerate(indegree) if degree == 0)
    order: list[int] = []
    while ready:
        current = ready.popleft()
        order.append(current)
        for neighbor in graph[current]:
            indegree[neighbor] -= 1
            if indegree[neighbor] == 0:
                ready.append(neighbor)
    return order


class DisjointSet:
    def __init__(self, size: int) -> None:
        self.parent = list(range(size))
        self.sizes = [1] * size

    def find(self, value: int) -> int:
        if self.parent[value] != value:
            self.parent[value] = self.find(self.parent[value])  # 경로 압축
        return self.parent[value]

    def unite(self, first: int, second: int) -> bool:
        first, second = self.find(first), self.find(second)
        if first == second:
            return False
        if self.sizes[first] < self.sizes[second]:
            first, second = second, first
        self.parent[second] = first
        self.sizes[first] += self.sizes[second]
        return True


def prefix_sums(values: list[int]) -> list[int]:
    prefix = [0]
    for value in values:
        prefix.append(prefix[-1] + value)
    return prefix


def maximum_window_sum(values: list[int], width: int) -> int:
    if not 1 <= width <= len(values):
        raise ValueError("window width must be in [1, len(values)]")
    window = sum(values[:width])
    best = window
    for right in range(width, len(values)):
        window += values[right] - values[right - width]
        best = max(best, window)
    return best


def climb_stairs(steps: int) -> int:
    if steps < 0:
        return 0
    previous, current = 1, 1
    for _ in range(2, steps + 1):
        previous, current = current, previous + current
    return current


def main() -> None:
    values = [7, 2, 9, 2, 5]
    check(linear_search(values, 9) == 2, "linear search")
    sorted_values = merge_sort(values)
    check(sorted_values == [2, 2, 5, 7, 9], "merge sort")
    check(lower_bound(sorted_values, 2) == 0 and lower_bound(sorted_values, 6) == 3,
          "binary lower bound")

    root = TreeNode(1, TreeNode(2, TreeNode(4), TreeNode(5)), TreeNode(3))
    pre: list[int] = []
    in_order: list[int] = []
    post: list[int] = []
    preorder(root, pre)
    inorder(root, in_order)
    postorder(root, post)
    check(pre == [1, 2, 4, 5, 3], "tree preorder DFS")
    check(in_order == [4, 2, 5, 1, 3], "tree inorder DFS")
    check(post == [4, 5, 2, 3, 1], "tree postorder DFS")
    check(level_order(root) == [1, 2, 3, 4, 5], "tree level-order BFS")

    graph = [[1, 2], [0, 3, 4], [0, 4], [1], [1, 2]]
    check(bfs(graph, 0) == [0, 1, 2, 3, 4], "graph BFS")
    check(dfs(graph, 0) == [0, 1, 3, 4, 2], "graph DFS")

    weighted = [[(1, 4), (2, 1)], [(3, 1)], [(1, 2), (3, 5)], []]
    check(dijkstra(weighted, 0) == [0, 3, 1, 4], "Dijkstra shortest path")

    dag = [[1, 2], [3], [3], []]
    topo = topological_sort(dag)
    check(len(topo) == len(dag) and topo[0] == 0 and topo[-1] == 3, "topological sort")

    sets = DisjointSet(5)
    sets.unite(0, 1)
    sets.unite(1, 2)
    check(sets.find(0) == sets.find(2) != sets.find(3), "disjoint set union")

    prefix = prefix_sums(values)
    check(prefix[4] - prefix[1] == 13, "prefix sum [1, 4)")
    check(maximum_window_sum(values, 3) == 18, "sliding window maximum sum")
    check(climb_stairs(5) == 8, "dynamic programming")
    print("ALL CHECKS PASSED")


if __name__ == "__main__":
    main()
