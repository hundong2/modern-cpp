// <cstddef>는 컨테이너 크기와 mdspan 인덱스에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 학습 결과를 쓰는 std::cout과 스트림 삽입 연산자를 선언한다.
#include <iostream>
// <mdspan>은 연속 메모리를 다차원 인덱스로 바라보는 std::mdspan과 std::dextents를 선언한다.
#include <mdspan>
// <utility>는 이름 있는 소유 객체를 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>
// <vector>는 측정값을 연속 메모리에 소유하는 std::vector를 선언한다.
#include <vector>

// using은 긴 클래스 템플릿 인스턴스에 별칭을 붙일 뿐 새 타입이나 객체를 만들지 않는다.
// const int는 이 뷰를 통해 원소를 고칠 수 없다는 뜻이고, dextents<size_t, 2>는 두 축 크기를 실행 시간에 저장한다.
using ConstMetricView = std::mdspan<const int, std::dextents<std::size_t, 2>>;

// struct는 기본 접근이 public이다. 행별 분석 결과를 단순한 값 레코드로 전달한다.
struct RowSummary {
    int total{};   // int{}는 0으로 값 초기화되며 행 합계를 값으로 소유한다.
    int maximum{}; // 행 최댓값을 값으로 소유하므로 원본 저장소의 수명과 독립적이다.
};

// class는 기본 접근이 private이다. 저장 형태와 행·열 불변식을 public 관찰 함수 뒤에 감춘다.
class MetricBatch {
public:
    // 생성자에는 반환형이 없다. 세 값 매개변수는 차원 값을 복사하고 samples 소유권을 함수 경계에서 인수한다.
    // explicit은 세 인자가 필요한 현재 생성자에는 필수는 아니지만, 변환 생성자가 아니라는 설계 의도를 분명히 한다.
    explicit MetricBatch(std::size_t rows, std::size_t columns, std::vector<int> samples)
        : rows_{rows}, columns_{columns}, samples_{std::move(samples)} {}

    // const 멤버 함수는 Batch 상태를 바꾸지 않는다. 반환 mdspan 값은 포인터와 두 extent만 복사하며 원소를 소유하지 않는다.
    [[nodiscard]] ConstMetricView view() const {
        // vector<int>::data() const noexcept는 데이터 인자 없이 첫 원소 const int*를 반환하고 samples_를 유지한다.
        // dextents mdspan 생성자는 그 포인터와 rows_, columns_ lvalue의 값을 복사해 2차원 매핑을 만든다.
        // 생성자는 반환값이 없고 동적 할당도 없다. rows_*columns_가 실제 원소 수와 같아야 모든 인덱싱이 유효하다.
        // 반환 prvalue가 호출자의 뷰를 직접 초기화할 수 있지만 Batch 수명이나 vector 재할당 뒤에는 댕글링한다.
        return ConstMetricView{samples_.data(), rows_, columns_};
    }

private:
    std::size_t rows_{};       // 행 수를 음수 없는 크기 타입으로 소유한다.
    std::size_t columns_{};    // 열 수를 소유하며 rows_*columns_는 samples_.size()와 같아야 한다.
    std::vector<int> samples_{}; // 실제 int 객체와 연속 버퍼 수명은 Batch가 독점 관리한다.
};

// 분석 서비스는 저장소를 소유하지 않고 읽기 전용 2차원 뷰를 값으로 보관한다.
class BatchAnalyzer {
public:
    // 생성자에는 반환형이 없다. explicit은 mdspan 하나가 Analyzer로 뜻밖에 암시 변환되는 일을 막는다.
    // view는 작은 비소유 값으로 복사되어 view_를 직접 초기화하며 원본 MetricBatch 수명을 연장하지 않는다.
    explicit BatchAnalyzer(ConstMetricView view) : view_{view} {}

    [[nodiscard]] RowSummary summarize(std::size_t row) const {
        RowSummary result{}; // 집합체 값 초기화가 두 int를 0으로 만든다.
        // mdspan::extent(1)는 축 번호 size_t 값 1을 받고 열 수 size_t를 O(1)에 반환한다.
        // 수신 view_와 원본 원소는 바뀌지 않고 반환값은 반복 상한에 즉시 사용된다. 축 번호는 rank 2보다 작아야 한다.
        const std::size_t column_count{view_.extent(1)};
        // 행은 호출자가 extent(0) 미만으로 보장한다. 반복마다 column이 1 증가하고 열 구간 [0,column_count)을 돈다.
        for (std::size_t column{}; column < column_count; ++column) {
            // mdspan::operator[](row,column)은 두 size_t 값 인자를 복사해 layout_right 오프셋을 계산하고 const int&를 반환한다.
            // O(1), 할당·상태 변화·범위 검사가 없다. 두 인덱스가 각 extent 미만이라는 전제조건을 만족해야 한다.
            // 참조가 빌리는 원소는 Batch가 소유하며 Batch 파괴·vector 재할당 전까지만 유효하다. 여기서는 즉시 int로 복사한다.
            const int value{view_[row, column]};
            result.total += value; // +=는 기존 합계를 읽고 value를 더해 같은 int 멤버에 저장한다.
            // 첫 열이거나 더 큰 값을 찾으면 비교 결과 bool prvalue에 따라 maximum을 갱신한다.
            if (column == 0U || value > result.maximum) {
                result.maximum = value;
            }
        }
        // 이름 있는 result는 lvalue지만 값 반환 시 NRVO가 가능하고, 적용되지 않아도 작은 값 복사가 가능하다.
        return result;
    }

private:
    ConstMetricView view_; // 포인터·extent만 소유하고 int 원소는 소유하지 않는 private 멤버다.
};

int main() {
    // initializer_list 생성자는 여섯 int를 새 연속 저장소에 복사해 samples가 소유하게 한다.
    // O(N) 시간·공간이고 할당 실패 시 std::bad_alloc 가능성이 있으며 성공 뒤 size와 capacity는 최소 6이다.
    std::vector<int> samples{3, 4, 5, 1, 7, 2};

    // std::move(samples)는 이름 있는 vector lvalue를 xvalue로 바꾼다. 값 매개변수와 samples_ 이동 생성자가 버퍼 소유권을 옮긴다.
    // batch는 rows=2, columns=3, 원소 6개를 소유한다. 이동 뒤 samples는 유효하지만 내용은 지정되지 않아 사용하지 않는다.
    MetricBatch batch{2U, 3U, std::move(samples)};

    // batch.view()는 mdspan prvalue를 반환한다. Analyzer 생성자가 이를 값으로 복사해도 원소 복사는 없다.
    // analyzer가 빌리는 batch가 더 먼저 생성되어 더 나중에 파괴되므로 현재 수명 순서는 안전하다.
    BatchAnalyzer analyzer{batch.view()};

    // summarize는 row 값 0을 복사 입력으로 받고 RowSummary prvalue를 반환해 first를 직접 초기화한다.
    const RowSummary first{analyzer.summarize(0U)};
    // 두 번째 호출은 같은 view를 유지한 채 row 1만 읽고 second에 독립적인 합계·최댓값을 저장한다.
    const RowSummary second{analyzer.summarize(1U)};

    // operator<<는 cout 버퍼·상태를 갱신하고 ostream&를 연쇄 반환한다. 네 int와 구분 문자를 값으로 쓰고 최종 참조는 버린다.
    // 출력 호출은 소유 Batch와 비소유 mdspan의 상태를 바꾸지 않으며 I/O 실패는 cout 상태 비트로 남을 수 있다.
    std::cout << first.total << ' ' << first.maximum << ' ' << second.total << ' '
              << second.maximum << '\n';

    // ==와 &&가 만든 bool prvalue로 성공 코드를 선택한다. 실제 load·비교·분기 형태는 CPU·ABI·최적화에 따라 달라진다.
    return first.total == 12 && first.maximum == 5 && second.total == 10 &&
                   second.maximum == 7
               ? 0
               : 1;
}
