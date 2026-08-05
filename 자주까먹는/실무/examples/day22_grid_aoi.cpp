#include <algorithm>     // std::max와 std::min으로 cell 범위를 자른다.
#include <iostream>      // 주변 entity ID를 출력한다.
#include <unordered_set> // 각 cell의 비소유 entity pointer 집합을 제공한다.
#include <vector>        // 2차원 grid와 결과 목록을 소유한다.

struct Entity {                                      // world가 별도로 소유하는 게임 객체다.
    int id;                                          // stable하다고 가정한 식별자다.
    float x;                                         // world 좌표 x다.
    float y;                                         // world 좌표 y다.
};                                                   // grid pointer는 이 객체를 소유하지 않는다.

class Grid {                                         // 균일 2D 공간 분할 인덱스다.
public:                                              // 생성·등록·조회 API를 공개한다.
    Grid(int cells_per_axis, float cell_size)        // 정사각 grid 크기와 cell 폭을 받는다.
        : size_(cells_per_axis), cell_size_(cell_size), // 멤버 선언 순서대로 크기와 폭을 먼저 저장한다.
          cells_(size_, std::vector<Cell>(size_)) {} // size×size 빈 cell을 소유한다.

    void add(Entity& entity) {                       // Entity 수명이 Grid보다 길다는 계약의 비소유 참조다.
        const auto [cx, cy] = cell_of(entity);       // 좌표를 유효한 cell index로 바꾼다.
        cells_[cx][cy].insert(&entity);              // 주소를 관찰자로 저장하며 삭제 책임은 없다.
    }                                                // entity 이동/파괴 시 별도 갱신이 필요하다.

    std::vector<const Entity*> nearby(const Entity& entity) const { // 주변 3×3 cell을 조회한다.
        const auto [cx, cy] = cell_of(entity);       // 중심 entity의 cell을 계산한다.
        std::vector<const Entity*> result;           // 발견된 비소유 관찰 포인터들을 반환할 값이다.
        for (int x = std::max(0, cx - 1); x <= std::min(size_ - 1, cx + 1); ++x) { // x 경계를 자른다.
            for (int y = std::max(0, cy - 1); y <= std::min(size_ - 1, cy + 1); ++y) { // y 경계를 자른다.
                for (const Entity* other : cells_[x][y]) { // 해당 cell의 후보만 순회한다.
                    if (other->id != entity.id) result.push_back(other); // 자기 자신을 제외한다.
                }                                    // 한 cell 후보 순회를 끝낸다.
            }                                        // y 인접 cell 순회를 끝낸다.
        }                                            // x 인접 cell 순회를 끝낸다.
        return result;                               // 값 반환으로 결과 컨테이너 소유권을 넘긴다.
    }                                                // exact radius가 필요하면 후보에 거리 검사를 추가한다.

private:                                             // cell 표현과 좌표 변환을 감춘다.
    using Cell = std::unordered_set<Entity*>;        // 비소유 pointer 집합임을 별칭 문맥으로 표시한다.
    std::pair<int, int> cell_of(const Entity& entity) const { // world 좌표를 grid index로 변환한다.
        const int cx = std::clamp(static_cast<int>(entity.x / cell_size_), 0, size_ - 1); // x를 경계 안에 둔다.
        const int cy = std::clamp(static_cast<int>(entity.y / cell_size_), 0, size_ - 1); // y를 경계 안에 둔다.
        return {cx, cy};                             // 두 index를 값으로 반환한다.
    }                                                // 음수 좌표 정책은 clamp로 명시했다.
    int size_;                                       // 한 축의 cell 개수다.
    float cell_size_;                                // world 단위의 cell 폭이다.
    std::vector<std::vector<Cell>> cells_;           // grid가 cell 컨테이너들을 소유한다.
};                                                   // Grid 정의를 끝낸다.

int main() {                                         // 가까운 두 객체와 먼 객체를 등록한다.
    Grid grid{10, 100.0F};                           // 1000×1000 영역에 해당하는 10×10 grid다.
    Entity a{1, 150.0F, 150.0F};                    // cell (1,1)의 query entity다.
    Entity b{2, 180.0F, 120.0F};                    // 같은 cell의 가까운 entity다.
    Entity c{3, 850.0F, 850.0F};                    // 먼 cell의 entity다.
    grid.add(a); grid.add(b); grid.add(c);           // Entity가 모두 살아 있는 동안 주소를 등록한다.
    const auto result = grid.nearby(a);              // a 인접 cell 후보를 구한다.
    for (const Entity* entity : result) std::cout << entity->id << '\n'; // 2만 출력한다.
    return result.size() == 1 && result[0]->id == 2 ? 0 : 1; // 결과를 검증한다.
}                                                    // result→grid→entities 역순 파괴에 주의한다.
