#include <iostream>
#include <string>
#include <vector>

struct Item {
    std::string name;
    std::vector<int> scores;
};

int main() {
    std::vector<Item> items{
        {"alpha", {10, 20, 30}},
        {"bravo", {40, 50, 60}},
        {"charlie", {70, 80, 90}},
    };

    std::cout << "1) 값 복사: for (auto item : items)\n";
    for (auto item : items) {
        // item은 원본 원소가 아니라 복사본입니다.
        // Item 안에는 std::string과 std::vector가 있으므로 복사 비용이 작지 않습니다.
        // vector는 내부 원소를 힙에 보관하므로 복사 시 힙 메모리 할당과 원소 복사가
        // 발생할 수 있습니다. 읽기만 할 때는 실무에서 거의 피하는 형태입니다.
        item.name += "_copied";
        std::cout << item.name << '\n';
    }

    std::cout << "\n2) const 참조: for (const auto& item : items)\n";
    for (const auto& item : items) {
        // item은 원본 원소를 가리키는 별명(reference)입니다.
        // const가 있으므로 item.name을 바꿀 수 없습니다.
        // 복사하지 않으므로 큰 객체를 읽을 때 가장 자주 쓰는 형태입니다.
        std::cout << item.name << " score_count=" << item.scores.size() << '\n';
    }

    std::cout << "\n3) 수정 참조: for (auto& item : items)\n";
    for (auto& item : items) {
        // 원본을 직접 수정합니다.
        // 메모리 관점에서 복사는 없지만, 원본 컨테이너의 상태가 바뀝니다.
        // 읽기 전용이면 const auto&를 쓰고, 수정 의도가 있을 때만 auto&를 씁니다.
        item.scores.push_back(100);
    }

    std::cout << "\n4) 인덱스 루프가 필요한 경우\n";
    for (std::size_t index = 0; index < items.size(); ++index) {
        // index가 필요한 로직이면 range-for보다 인덱스 루프가 명확합니다.
        // std::size_t는 컨테이너 크기를 표현하는 부호 없는 정수 타입입니다.
        std::cout << index << ": " << items[index].name
                  << " score_count=" << items[index].scores.size() << '\n';
    }
}

