#include <iostream>
#include <span>
#include <vector>

void print_span(std::span<const int> dataView) {
    for( int val: dataView ) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}
int main() {
    std::vector<int> numbers{1, 2, 3, 4, 5};
    print_span(numbers);
    std::span<const int> window = std::span(numbers).subspan(1, 3);
    std::cout << "Window: " << window.size() << " elements: " << window[0] << ", " << window[1] << ", " << window[2] << std::endl;
}

//Result 
// == execute: /workspace/modern-cpp/자주까먹는/build/cpp20/span ==

// 1 2 3 4 5 
// Window: 3 elements: 2, 3, 4