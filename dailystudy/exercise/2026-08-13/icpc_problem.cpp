/*
문제: BOJ 1717 - 집합의 표현
출처: https://www.acmicpc.net/problem/1717

요약: 0부터 n까지의 원소가 처음에는 서로 다른 집합에 있다. 합집합 연산과 두 원소가 같은
집합인지 묻는 연산을 차례로 처리한다.
입력: 첫 줄에 n과 연산 수 m이 주어진다. 이어지는 m줄은 `0 a b`(합치기) 또는
`1 a b`(같은 집합인지 질문)이다.
출력: 각 질문마다 같은 집합이면 YES, 아니면 NO를 한 줄에 출력한다.
제약: 1 <= n <= 1,000,000, 1 <= m <= 100,000, 0 <= a,b <= n.
예: 1과 3을 합친 뒤 1과 7을 물으면 NO이고, 이후 연결하여 다시 물으면 YES가 될 수 있다.
*/

// <iostream>은 빠른 표준 입력 std::cin과 출력 std::cout을 제공한다.
#include <iostream>
// <numeric>은 연속 번호를 채우는 std::iota를 제공한다.
#include <numeric>
// <utility>는 두 값을 교환하는 std::swap을 제공한다.
#include <utility>
// <vector>는 원소 수에 맞춰 동적으로 연속 저장하는 std::vector를 제공한다.
#include <vector>

// 구현 근처 참고 문서: ../algorithm/disjoint-set-union.md
// class의 기본 접근은 private이며, 내부 표현을 숨기고 합치기/질문 API만 public으로 공개한다.
class DisjointSet final {
public:
    // 생성자에는 반환형이 없다. explicit은 int가 DisjointSet으로 암시 변환되는 것을 막는다.
    explicit DisjointSet(int maximum)
        : parent_(static_cast<std::size_t>(maximum + 1)), rank_(parent_.size(), 0) {
        // iota는 각 원소의 부모를 자기 자신으로 둔다. 처음에는 모두 서로 다른 집합이라는 불변식이다.
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    // int 반환형은 대표 원소 번호를 돌려준다. 경로 압축으로 이후 탐색을 짧게 만든다.
    int find(int value) {
        // != 비교로 루트 여부를 확인한다. 루트의 부모는 자기 자신이다.
        if (parent_[value] != value) {
            // 재귀 반환값을 저장해 value에서 루트까지 경로를 직접 연결한다.
            parent_[value] = find(parent_[value]);
        }
        return parent_[value];
    }

    // bool 반환형은 실제로 서로 다른 두 집합을 합쳤는지 나타낸다.
    bool unite(int left, int right) {
        int left_root{find(left)};   // 중괄호 초기화한 기본 타입 int 변수다.
        int right_root{find(right)};
        if (left_root == right_root) { // == 비교가 참이면 이미 같은 집합이다.
            return false;
        }
        // 랭크가 큰 트리 아래에 작은 트리를 붙여 높이 증가를 억제한다.
        if (rank_[left_root] < rank_[right_root]) {
            std::swap(left_root, right_root);
        }
        parent_[right_root] = left_root; // 합친 뒤 두 루트의 대표가 같아진다는 불변식을 만든다.
        if (rank_[left_root] == rank_[right_root]) {
            ++rank_[left_root]; // 전위 증가 연산자로 같은 높이를 합친 새 루트의 랭크를 1 늘린다.
        }
        return true;
    }

private:
    std::vector<int> parent_{}; // 인덱스별 부모를 저장해 O(n) 공간을 사용한다.
    std::vector<int> rank_{};   // 트리 높이의 상한을 저장한다.
};

int main() {
    // 동기화를 끄고 묶음을 해제해 많은 입출력을 빠르게 처리한다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr); // nullptr은 아무 객체도 가리키지 않는 포인터 값이다.

    int n{}; // {}는 기본 타입을 0으로 초기화한다.
    int m{};
    std::cin >> n >> m; // >> 연산자는 입력을 왼쪽 변수에 저장한다.
    DisjointSet sets{n}; // explicit 생성자는 직접 중괄호 초기화로 올바르게 호출한다.

    // m번 반복하며, 경로 압축+랭크 합치기의 연산당 상각 시간은 O(alpha(n))이다.
    for (int index{}; index < m; ++index) {
        int command{};
        int a{};
        int b{};
        std::cin >> command >> a >> b;
        if (command == 0) { // 조건 분기로 합집합 명령을 구분한다.
            sets.unite(a, b); // 반환값이 필요 없으므로 호출만 수행한다.
        } else {
            // 두 대표가 같다는 것은 같은 집합이라는 필요충분조건이다.
            std::cout << (sets.find(a) == sets.find(b) ? "YES\n" : "NO\n");
        }
    }
    // 실제 기계 실행은 로드·저장·비교·분기·함수 호출을 포함하나 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return 0;
}

