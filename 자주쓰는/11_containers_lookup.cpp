#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

struct Product {
    int id{};
    std::string name;
};

int main() {
    std::vector<Product> products{
        {3, "keyboard"},
        {1, "monitor"},
        {2, "mouse"},
    };

    const auto found = std::find_if(products.begin(), products.end(),
        [](const Product& product) {
            return product.id == 2;
        });

    if (found != products.end()) {
        std::cout << "vector find: " << found->name << '\n';
    }

    std::map<int, std::string> ordered{
        {3, "keyboard"},
        {1, "monitor"},
        {2, "mouse"},
    };

    std::unordered_map<int, std::string> hashed{
        {3, "keyboard"},
        {1, "monitor"},
        {2, "mouse"},
    };

    std::cout << "map find: " << ordered.find(2)->second << '\n';
    std::cout << "unordered_map find: " << hashed.find(2)->second << '\n';

    // vector:
    // - 연속 메모리라 캐시 지역성이 좋습니다.
    // - 작은 데이터나 순회가 많은 데이터에 강합니다.
    // - 중간 삽입/삭제는 원소 이동이 필요합니다.
    //
    // map:
    // - 보통 트리 기반입니다.
    // - 키 정렬 순서가 필요할 때 사용합니다.
    // - 노드마다 힙 할당이 있어 캐시 지역성은 vector보다 불리합니다.
    //
    // unordered_map:
    // - 해시 기반 평균 O(1) 조회를 기대합니다.
    // - 정렬 순서는 없습니다.
    // - rehash 시 버킷 배열이 재구성되므로 iterator 무효화 조건을 알아야 합니다.
}

