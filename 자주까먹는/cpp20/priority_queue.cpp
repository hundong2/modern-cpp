#include <iostream>
#include <queue>
#include <vector>
#include <string>

int main() {
    auto cmp = [](const std::pair<int, std::string>& a, const std::pair<int, std::string>& b) {
        return a.first > b.first; // 우선순위가 높은 값이 먼저 오도록 비교, Min-Heap을 만들기 위해 a.first > b.first 사용
    };

    std::priority_queue<
        std::pair<int, std::string>, 
        std::vector<std::pair<int, std::string>>, 
        decltype(cmp)> minHeap(cmp);
    minHeap.push({3, "Task 3"});
    minHeap.push({1, "Task 1"});
    minHeap.push({2, "Task 2"});
    while(!minHeap.empty()) {
        auto task = minHeap.top();
        std::cout << "Priority: " << task.first << ", Task: " << task.second << std::endl;
        minHeap.pop();
    }
    return 0;

}


// Priority: 1, Task: Task 1
// Priority: 2, Task: Task 2
// Priority: 3, Task: Task 3