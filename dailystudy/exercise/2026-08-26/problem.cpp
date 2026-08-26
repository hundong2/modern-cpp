// <cstddef>는 다차원 크기와 인덱스에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 연습 결과를 출력하는 std::cout을 선언한다.
#include <iostream>
// <mdspan>은 쓰기 가능한 2차원 비소유 뷰 std::mdspan을 선언한다.
#include <mdspan>
// <vector>는 격자 원소를 연속 저장소에 소유하는 std::vector를 선언한다.
#include <vector>

// 동적 크기 두 축을 가진 int 뷰 별칭이다. mdspan은 int 원소를 소유하거나 수명을 연장하지 않는다.
using MatrixView = std::mdspan<int, std::dextents<std::size_t, 2>>;

// struct의 멤버는 기본 public이다. 수정 요청을 값으로 묶는 단순 DTO다.
struct CellUpdate {
    std::size_t row{};    // 대상 행 인덱스다.
    std::size_t column{}; // 대상 열 인덱스다.
    int value{};          // 새 셀 값을 소유한다.
};

// class는 기본 private이다. 연속 저장소와 차원을 한 객체가 함께 소유한다.
class IntGrid {
public:
    // 생성자는 반환형이 없다. explicit은 size_t 하나가 IntGrid로 뜻밖에 암시 변환되는 일을 막는다.
    explicit IntGrid(std::size_t side)
        // vector<int>(count,value)는 side*side개의 0을 복사해 연속 저장소에 만든다. 곱이 size_t에 표현되어야 한다.
        // O(side^2) 시간·공간이고 한 번 할당할 수 있으며 bad_alloc이면 객체 생성이 실패한다.
        : side_{side}, values_(side * side, 0) {}

    void apply(CellUpdate update) {
        // view()는 포인터와 두 extent를 가진 mdspan prvalue를 반환한다. local_view가 값으로 받아도 원소는 복사하지 않는다.
        MatrixView local_view{view()};
        // operator[](row,column)은 update의 두 size_t lvalue 값을 복사해 int&를 O(1)에 반환한다.
        // 반환 참조에 value를 복사 대입해 정확히 한 셀만 바뀌며 크기·할당·다른 셀은 유지된다.
        // row와 column이 side_ 미만이어야 하고, 범위 검사가 없으므로 위반 시 정의되지 않은 동작이 될 수 있다.
        local_view[update.row, update.column] = update.value;
    }

    [[nodiscard]] int diagonal_sum() {
        MatrixView local_view{view()}; // 이 함수 호출 동안 *this와 values_가 살아 있어 뷰 수명이 안전하다.
        int sum{};                     // int{}는 0으로 값 초기화된다.
        for (std::size_t index{}; index < side_; ++index) {
            // 같은 행·열 인덱스를 넘겨 대각 원소 int&를 얻고 값을 읽는다. 뷰와 원소는 수정하지 않는다.
            sum += local_view[index, index];
        }
        return sum; // int prvalue가 호출자 결과를 초기화하며 소유권 이전은 없다.
    }

private:
    [[nodiscard]] MatrixView view() {
        // vector<int>::data() noexcept는 인자 없이 첫 int*를 반환하고 vector 상태를 유지한다.
        // mdspan 생성자는 포인터, side_, side_ 세 값을 받아 행 우선 2차원 매핑을 만들며 반환형이 없는 생성자다.
        // 동적 할당은 없고 values_가 파괴되거나 재할당되면 반환 뷰가 댕글링하므로 외부에 장기 보관하지 않는다.
        return MatrixView{values_.data(), side_, side_};
    }

    std::size_t side_{};       // 정사각 격자의 한 변 길이다.
    std::vector<int> values_{}; // 실제 int 객체를 소유하는 연속 저장소다.
};

int main() {
    IntGrid grid{3U}; // 직접 목록 초기화가 explicit 생성자를 선택해 3x3 영 격자를 만든다.
    // apply는 CellUpdate prvalue를 값 매개변수로 직접 초기화하고 각 필드 값을 복사해 셀 하나만 바꾼다.
    grid.apply(CellUpdate{0U, 0U, 4});
    grid.apply(CellUpdate{1U, 1U, 5});
    grid.apply(CellUpdate{2U, 2U, 6});

    // diagonal_sum은 데이터 인자 없이 grid의 세 대각 원소를 읽어 int 15를 반환하고 total에 저장한다.
    const int total{grid.diagonal_sum()};
    // operator<<는 total과 개행을 cout에 쓰고 ostream&를 반환해 연쇄하며 최종 참조는 버린다.
    std::cout << total << '\n';
    return total == 15 ? 0 : 1; // 조건 연산자가 검증 결과에 따라 프로세스 종료 코드를 고른다.
}
