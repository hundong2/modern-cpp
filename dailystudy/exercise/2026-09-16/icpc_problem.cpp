/*
문제 ID/제목: CSES 1724 - Graph Paths II
출처: https://cses.fi/problemset/task/1724/

문제 요약
- 1번부터 n번까지 번호가 붙은 정점과, 비용이 양수인 m개의 방향 간선이 주어진다.
- 1번 정점에서 n번 정점까지 정확히 k개의 간선을 사용하는 walk 중 총비용의 최솟값을 구한다.
  정점이나 간선을 다시 방문해도 되며, 간선 수가 k보다 적거나 많은 경로는 답 후보가 아니다.
- 조건을 만족하는 walk가 없으면 -1을 출력한다.

입력
- 첫 줄: 정점 수 n, 간선 수 m, 정확히 사용해야 하는 간선 수 k
- 다음 m줄: 방향 간선의 시작 정점 a, 도착 정점 b, 비용 c

출력
- 정확히 k개 간선으로 1번에서 n번까지 갈 수 있으면 최소 총비용, 그렇지 않으면 -1

공식 제약
- 1 <= n <= 100
- 1 <= m <= n(n-1)
- 1 <= k <= 10^9
- 1 <= a,b <= n
- 1 <= c <= 10^9
- 시간 제한 1초, 메모리 제한 512MB

공식 예제
- 입력:
  3 4 8
  1 2 5
  2 3 4
  3 1 1
  3 2 2
- 출력:
  27
*/

// <cstddef>는 연속 저장소의 크기와 인덱스에 쓰는 부호 없는 표준 타입 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 간선 수와 비용을 정확히 64비트 부호 있는 정수로 저장할 std::int64_t를 선언한다.
#include <cstdint>
// <iostream>은 표준 입력 std::cin, 표준 출력 std::cout, 스트림 설정 타입 std::ios를 선언한다.
#include <iostream>
// <vector>는 행렬 원소를 하나의 동적 연속 저장소로 소유할 std::vector를 선언한다.
#include <vector>

// using은 새 타입을 정의하는 것이 아니라 긴 기존 타입에 읽기 쉬운 별칭을 붙인다.
// Cost는 간선 비용, 누적 비용, k를 함께 담는다. 최댓값 10^9*10^9=10^18은 32비트 int를 넘는다.
using Cost = std::int64_t;

// 도달 불가능을 뜻하는 센티널이다. 가능한 답 10^18보다 크고 int64 최댓값보다 충분히 작다.
// 실제 덧셈 전에는 `left > kInfinity - right`를 검사하여 signed overflow(미정의 동작)를 막는다.
constexpr Cost kInfinity{4'000'000'000'000'000'000LL};

/*
값 범주·참조 바인딩·복사/이동·수명·소유권
- adjacency, result처럼 이름 있는 객체 표현식은 lvalue다. `Cost{0}`, `from - 1`, 생성자 인자 식은 prvalue다.
  이 코드에는 이동 변환 유틸리티 같은 명시적 xvalue 식이 없다.
- cell()의 비-const 반환식은 vector가 소유한 Cost 원소의 lvalue 참조다. 호출자는 그 참조로 값을 바꿀 수
  있지만, 행렬의 vector가 파괴되거나 저장소를 교환한 뒤에는 예전 참조를 보관하면 안 된다.
- multiply의 const 참조 매개변수는 두 큰 행렬을 복사하지 않고 읽기만 빌린다. output 참조는 호출자가
  소유한 scratch 행렬을 수정한다. 모든 참조 대상은 호출보다 오래 살아 있으므로 바인딩 수명이 안전하다.
- MinPlusMatrix는 vector를 값 멤버로 두어 저장소를 독점 소유하고, 객체 수명이 끝날 때 RAII로 자동 해제한다.
  swap_cells는 원소별 복사/이동 없이 같은 크기 행렬의 저장소 소유권만 교환한다.
- 행렬을 함수에서 값으로 반환하지 않고 출력 참조를 사용했으므로 이 구현의 핵심 식에는 RVO/NRVO가 필요 없다.
  값으로 지역 행렬을 반환하는 대안에서는 NRVO가 허용되지만 보장되지 않는 경우가 있고, C++17 이후 순수
  prvalue로 직접 초기화하는 경우에는 복사 생략이 보장된다. 여기서는 메모리 재사용을 더 명시적으로 드러낸다.
*/

// 구현 가까운 공용 알고리즘 문서: ../algorithm/min-plus-matrix-exponentiation.md
// class와 struct 모두 사용자 정의 타입이지만 class의 기본 접근은 private, struct의 기본 접근은 public이다.
// 차원과 평탄화 저장소의 일관성을 외부에서 깨지 못하게 감추고 제한된 연산만 공개하므로 class를 사용한다.
class MinPlusMatrix {
public:
    // 생성자는 반환형이 없고 새 객체의 수명을 시작한다. explicit은 int 하나가 행렬로 암시 변환되는 일을 막는다.
    // 두 매개변수는 값으로 복사한다. 기본 인자 덕분에 채움값을 생략하면 모든 칸이 kInfinity가 된다.
    // 멤버 초기화 목록은 본문보다 먼저 선언 순서대로 멤버를 직접 초기화한다. 중괄호 초기화는 축소 변환을 막는다.
    //
    // [생성 계약: std::vector<Cost> fill 생성자]
    // (1) 수신 객체는 아직 수명이 시작되지 않은 정확한 타입 std::vector<Cost>인 cells_이고 기존 원소,
    //     저장소, 반복자, 참조가 없다. dimension은 공식 범위 [1,100]인 int다.
    // (2) vector<Cost>::vector(size_type count, const Cost& value, const Allocator& = Allocator())를 선택하며
    //     value_type 템플릿 인자는 Cost=std::int64_t, Allocator는 기본 할당자 타입이다.
    // (3) 두 size_t prvalue를 곱한 count는 dimension^2이고 소유권 없는 개수 값이다. initial_value는 이름 있는
    //     Cost lvalue라 const Cost&에 바인딩되어 각 원소로 복사되며 호출자의 객체나 저장소 소유권은 옮기지 않는다.
    // (4) 생성자는 반환값이 없다. 성공하면 cells_가 정확히 dimension^2개의 Cost 원소를 독점 소유한다.
    // (5) 각 원소는 initial_value와 같고 size는 dimension^2다. 새 객체라 기존 관찰자를 무효화하지 않는다.
    // (6) 시간·공간 O(dimension^2)이고 동적 할당할 수 있다. count가 max_size를 넘으면 length_error,
    //     메모리 부족이면 bad_alloc이 가능하며 실패하면 MinPlusMatrix 생성도 완료되지 않는다. 원소 수명은
    //     cells_의 수명 또는 저장소 교환까지이고, 완성 전 객체를 다른 스레드에 공개하지 않는다.
    explicit MinPlusMatrix(const int dimension, const Cost initial_value = kInfinity)
        : dimension_{dimension},
          cells_(static_cast<std::size_t>(dimension)
                     * static_cast<std::size_t>(dimension),
                 initial_value) {}

    // const 멤버 함수는 *this의 논리 상태를 바꾸지 않는다. noexcept는 이 기본 타입 반환이 예외를 던지지 않음을 명시한다.
    [[nodiscard]] int dimension() const noexcept {
        return dimension_;
    }

    // 반환형 Cost&는 복사본이 아니라 행렬이 소유한 원소를 수정 가능한 lvalue로 빌려준다는 뜻이다.
    [[nodiscard]] Cost& cell(const int row, const int column) noexcept {
        // [호출 계약: std::vector<Cost>::operator[](size_type), 비-const overload]
        // (1) 수신 cells_는 정확한 타입 std::vector<Cost>의 비-const lvalue이고 size==dimension^2다.
        //     row와 column은 호출 전 각각 [0,dimension)의 유효한 int 인덱스여야 한다.
        // (2) constexpr reference vector<Cost>::operator[](size_type position) overload를 선택한다.
        // (3) row*dimension+column을 size_t로 계산한 prvalue 위치는 [0,size)다. 위치 값만 전달하고
        //     cells_나 원소의 소유권은 이동하지 않는다.
        // (4) 선택된 Cost 원소의 lvalue 참조를 반환하고 cell()의 반환값으로 그대로 전달해 읽기/쓰기에 사용한다.
        // (5) operator[] 자체는 vector의 값·size·capacity를 바꾸지 않는다. 이후 호출자가 반환 참조에 대입하면
        //     그 원소 값만 바뀌고 주소와 다른 원소는 유지된다.
        // (6) O(1), 무할당·비무효화다. 범위 검사를 하지 않아 인덱스 전제가 깨지면 UB다. 참조 수명은
        //     해당 저장소가 파괴되거나 swap될 때까지이며, 같은 원소를 여러 스레드가 동시 수정하지 않는다.
        return cells_[static_cast<std::size_t>(row)
                          * static_cast<std::size_t>(dimension_)
                      + static_cast<std::size_t>(column)];
    }

    // const 객체에서는 읽기 전용 const Cost&만 반환하여 알고리즘 입력 행렬을 실수로 바꾸지 못하게 한다.
    [[nodiscard]] const Cost& cell(const int row, const int column) const noexcept {
        // [호출 계약: const std::vector<Cost>::operator[](size_type)]
        // (1) 수신 cells_는 정확한 타입 const std::vector<Cost> lvalue로 보이며 size==dimension^2다.
        //     두 인덱스는 호출 전에 각각 [0,dimension)이다.
        // (2) constexpr const_reference vector<Cost>::operator[](size_type position) const overload를 선택한다.
        // (3) 평탄화 식이 만든 std::size_t prvalue 위치만 값 전달하며 원소나 저장소 소유권을 넘기지 않는다.
        // (4) 선택된 원소의 const Cost lvalue 참조를 반환해 비용을 복사해 읽는 데 사용한다.
        // (5) 관찰 호출이므로 cells_와 인자, 모든 주소·반복자·참조 상태는 그대로다.
        // (6) O(1), 무할당·비무효화다. 범위를 벗어나면 UB지만 세 중첩 for의 경계가 막는다. 반환 참조는
        //     저장소 교환·파괴 전까지만 유효하고 호출 동안 다른 스레드의 수정은 없다.
        return cells_[static_cast<std::size_t>(row)
                          * static_cast<std::size_t>(dimension_)
                      + static_cast<std::size_t>(column)];
    }

    // 두 행렬의 차원이 같다는 전제에서 저장소만 교환한다. scratch를 재사용해 O(n^2) 복사를 피하는 실무 관용구다.
    void swap_cells(MinPlusMatrix& other) noexcept {
        // [호출 계약: std::vector<Cost>::swap(vector&)]
        // (1) 수신 cells_와 인자 other.cells_는 정확한 타입 std::vector<Cost>의 서로 다른 비-const lvalue이며
        //     둘 다 같은 dimension^2개 Cost를 소유한다. 기본 allocator들이 같아 swap 전제가 충족된다.
        // (2) constexpr void vector<Cost>::swap(vector& other) noexcept(...) overload를 선택한다.
        // (3) other.cells_는 수정 가능한 vector lvalue라 vector&에 바인딩된다. 호출자는 버퍼 포인터 소유권을
        //     직접 넘기지 않지만 두 vector가 관리하는 저장소 소유권은 서로 교환된다.
        // (4) 반환형 void라 반환값은 없고, 이후 각 행렬이 상대가 계산한 원소들을 소유하게 하는 데 사용한다.
        // (5) 두 vector의 size/capacity/저장소가 서로 바뀐다. 원소 참조·포인터·반복자는 원소 자체를 계속
        //     가리키지만 이제 상대 컨테이너 소속이며, 과거 past-the-end 반복자는 보장되지 않는다. 코드는 보존하지 않는다.
        // (6) O(1), 원소별 할당·복사·이동이 없고 이 기본 allocator 조건에서는 예외를 던지지 않는다.
        //     서로 다른 allocator인데 전파도 동등성도 없으면 UB지만 여기서는 동일 기본 allocator다. 두 행렬을
        //     다른 스레드가 동시에 읽거나 쓰지 않으며 저장소 수명은 새 소유 vector를 따라간다.
        cells_.swap(other.cells_);
    }

private:
    // private 접근 지정자는 class 밖의 코드가 두 멤버를 따로 고쳐 평탄화 불변식을 깨는 것을 막는다.
    int dimension_;              // 기본 타입 int는 최대 100인 차원과 루프 인덱스를 충분히 담는다.
    std::vector<Cost> cells_;    // 템플릿 인자 Cost는 각 칸의 타입이며 n*n개 원소를 한 버퍼로 소유한다.
};

// 반환형 void는 결과 객체를 만들지 않고 output 참조가 가리키는 기존 행렬을 채운다는 뜻이다.
// 세 참조는 비소유 별칭이며 output은 left/right와 다른 객체여야 한다. 두 입력은 const라 수정할 수 없다.
void min_plus_multiply(
    const MinPlusMatrix& left,
    const MinPlusMatrix& right,
    MinPlusMatrix& output) {
    const int dimension{left.dimension()};

    // 이전 곱셈 결과가 남은 scratch의 모든 칸을 먼저 도달 불가능 상태로 되돌린다.
    for (int row{0}; row < dimension; ++row) {
        for (int column{0}; column < dimension; ++column) {
            output.cell(row, column) = kInfinity;
        }
    }

    // 불변식: pivot [0,pivot)까지 처리한 뒤 output[i][j]는 그 중 한 정점을 경계로 이어 붙인 최소 비용이다.
    // 루프 순서를 i-pivot-j로 두면 같은 left 값은 한 번 load하고 right의 한 행은 연속 메모리로 읽기 쉽다.
    for (int row{0}; row < dimension; ++row) {
        for (int pivot{0}; pivot < dimension; ++pivot) {
            const Cost left_cost{left.cell(row, pivot)};
            if (left_cost >= kInfinity) {
                continue;
            }

            for (int column{0}; column < dimension; ++column) {
                const Cost right_cost{right.cell(pivot, column)};
                if (right_cost >= kInfinity) {
                    continue;
                }

                // 양수 비용 문제지만, 센티널 설계 자체도 안전하도록 덧셈 전에 상한을 검사한다.
                // 이 조건 없이 매우 큰 두 signed 정수를 더하면 overflow가 발생해 C++ 미정의 동작이 된다.
                if (left_cost > kInfinity - right_cost) {
                    continue;
                }

                const Cost candidate{left_cost + right_cost};
                Cost& best{output.cell(row, column)};
                if (candidate < best) {
                    best = candidate;
                }
            }
        }
    }
}

// min-plus 항등행렬에서 시작해 이진 지수승을 수행한다. power는 계산용으로 바뀌고 result에 최종 A^k를 쓴다.
void min_plus_power(
    MinPlusMatrix& power,
    Cost exponent,
    MinPlusMatrix& result) {
    const int dimension{power.dimension()};

    // min-plus 항등원 I는 대각선 0, 나머지 INF다. 길이 0인 walk는 같은 정점에만 비용 0으로 존재한다.
    for (int row{0}; row < dimension; ++row) {
        for (int column{0}; column < dimension; ++column) {
            result.cell(row, column) = (row == column) ? Cost{0} : kInfinity;
        }
    }

    // 한 개 scratch만 재사용하므로 추가 행렬 수는 상수 개다. 생성 시 모든 칸은 INF다.
    MinPlusMatrix scratch{dimension, kInfinity};

    // 불변식: result (min-plus) power^exponent는 처음의 A^k와 같다.
    // 매 회 exponent를 절반으로 줄이므로 최대 floor(log2(k))+1회만 반복한다.
    while (exponent > Cost{0}) {
        // 최하위 비트가 1이면 현재 2^bit개 간선 블록을 선택한다.
        if ((exponent & Cost{1}) != Cost{0}) {
            min_plus_multiply(result, power, scratch);
            result.swap_cells(scratch);
        }

        exponent /= Cost{2};
        if (exponent > Cost{0}) {
            min_plus_multiply(power, power, scratch);
            power.swap_cells(scratch);
        }
    }
}

/*
정확성 핵심
1. 인접 행렬 A[a][b]는 a->b 평행 간선 중 최소 비용이다. 따라서 정확히 한 간선을 쓰는 최소 비용을 나타낸다.
2. min-plus 곱 (X⊗Y)[i][j]=min_p(X[i][p]+Y[p][j])는 앞 구간과 뒤 구간을 경계 p에서 이어
   정확히 두 구간 길이의 합만큼 간선을 쓰는 최소 비용을 만든다.
3. 이진 지수승 불변식 result⊗power^exponent=A^k가 매 비트 처리 뒤 유지된다. 종료 시 exponent=0이므로
   result=A^k이고 result[0][n-1]이 정확히 k개 간선을 사용한 최소 비용이다.

복잡도
- 한 번의 min-plus 곱은 O(n^3), 이진 지수승은 O(log k)번 곱하므로 총 O(n^3 log k) 시간이다.
- 평탄 행렬 세 개(adjacency/power, result, scratch)가 각각 n^2칸이므로 O(n^2) 추가 공간이다.
*/

/*
기계 실행 관점
- 평탄 인덱스 계산 뒤 vector 버퍼에서 Cost를 load하고, INF와 compare한 결과로 조건 branch한다.
- 도달 가능한 조합은 뺄셈 기반 overflow guard, 64비트 add, 현재 최솟값과 compare, 필요할 때 store를 수행한다.
- i-pivot-j 순서는 right의 한 행을 연속 접근하여 캐시 지역성에 유리할 수 있으나 실제 벡터화, 분기 예측,
  명령 선택과 성능은 CPU·ABI·컴파일러·표준 라이브러리 구현·최적화 옵션에 따라 달라진다.
- 가상 함수가 없으므로 가상 간접 호출은 의도하지 않았지만, 실제 인라이닝 여부도 컴파일러가 결정한다.
*/

// main은 프로그램의 시작 함수다. 반환형 int의 0은 운영체제에 정상 종료를 알린다.
int main() {
    // [호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 별도 수신 객체가 없는 static 함수이며, 표준 스트림에서 아직 어떤 입출력도 하지 않은 초기 상태다.
    // (2) static bool ios_base::sync_with_stdio(bool sync = true)를 인자 하나로 호출한다.
    // (3) false는 bool prvalue이며 C stdio와 C++ iostream의 동기화를 끄라는 허용값이다. 소유권 입력은 없다.
    // (4) 이전 동기화 상태 bool을 반환하지만 되돌리지 않으므로 반환값을 의도적으로 사용하지 않는다.
    // (5) 이후 C++ 표준 스트림이 독립 버퍼링될 수 있다. C stdio와 섞은 출력 순서에는 의존하지 않는다.
    // (6) 표준의 별도 복잡도 상한은 없다. 사용자 컨테이너 할당·무효화는 없지만 첫 I/O 뒤 호출 효과는
    //     구현 정의다. 반드시 지금 한 번만 설정하고 다른 스레드에서 동시에 표준 스트림을 조작하지 않는다.
    std::ios::sync_with_stdio(false);

    // [호출 계약: std::basic_ios<char>::tie(nullptr)]
    // (1) 수신 std::cin은 정확한 타입 std::istream의 살아 있는 lvalue이며 보통 std::cout에 묶여 있다.
    // (2) std::ostream* basic_ios<char>::tie(std::ostream* tiestr) setter overload를 선택한다.
    // (3) nullptr prvalue는 “연결할 출력 스트림 없음”인 null ostream*로 변환된다. 소유권을 이전하지 않는다.
    // (4) 이전 tie 대상의 비소유 std::ostream*를 반환하지만 복구하지 않으므로 사용하지 않는다.
    // (5) cin의 tie 포인터가 null이 되어 입력 전 cout 자동 flush가 사라지고 두 스트림 객체 수명은 유지된다.
    // (6) 표준의 별도 복잡도 상한이나 일률적 noexcept 보장은 없다. null이 아닌 대상은 tie 연결을 따라
    //     자신으로 돌아오지 않아야 하며, 여기서는 null이라 안전하다. 동시 접근 없이 초기 설정 때만 호출한다.
    std::cin.tie(nullptr);

    // int는 최대 10,000 이하인 정점/간선 수와 0-based 인덱스를 충분히 담는다.
    // Cost는 최대 10^9인 k를 담고 후속 산술 타입을 64비트로 통일한다. `{0}`은 확실한 0 초기화다.
    int vertex_count{0};
    int edge_count{0};
    Cost exact_edge_count{0};

    // [호출 계약: std::istream 정수 추출 operator>>]
    // (1) 왼쪽 수신 std::cin은 입력 가능한 std::istream lvalue이고 세 변수는 초기화된 수정 가능 lvalue다.
    // (2) 첫 두 호출은 basic_istream<char>& operator>>(int&), 셋째는 std::int64_t의 구현별 기반
    //     signed 정수 타입과 일치하는 정수 추출 overload를 선택한다.
    // (3) vertex_count/edge_count는 int&, exact_edge_count는 Cost&에 바인딩된다. 공식 범위의 십진 토큰을
    //     읽으며 입력 버퍼나 변수 소유권은 이전하지 않는다.
    // (4) 각 호출은 같은 istream&를 반환해 다음 >>의 수신자로 쓰고, 마지막 반환 참조는 사용하지 않는다.
    // (5) 성공하면 세 변수에 n,m,k가 저장되고 입력 위치가 세 토큰 뒤로 이동한다. 실패 시 상태 비트가
    //     설정될 수 있지만 공식 입력은 성공을 보장한다.
    // (6) 소비 문자 수에 선형이며 locale/streambuf 내부 작업이 가능하다. 예외 mask가 설정됐다면
    //     ios_base::failure가 가능하고, 숫자가 표현 범위 밖이면 failbit가 설정된다. 참조는 호출 동안만 쓰며
    //     같은 cin을 다른 스레드가 동시에 읽지 않는다.
    std::cin >> vertex_count >> edge_count >> exact_edge_count;

    // 직접 목록 초기화는 explicit 생성자를 호출한다. adjacency가 모든 n*n Cost 원소를 독점 소유한다.
    MinPlusMatrix adjacency{vertex_count, kInfinity};

    // for는 초기화-조건-증가를 한곳에 표현하며 정확히 m번 간선을 읽는다.
    for (int edge_index{0}; edge_index < edge_count; ++edge_index) {
        int from{0};
        int to{0};
        Cost weight{0};

        // 위와 같은 int&, Cost& 추출 overload다. 이름 있는 변수는 lvalue로 참조에 바인딩된다.
        std::cin >> from >> to >> weight;

        // 입력 경계에서 한 번만 1-based를 0-based로 바꾼다. 같은 방향의 평행 간선은 최소 비용만 남긴다.
        Cost& best_direct_cost{adjacency.cell(from - 1, to - 1)};
        if (weight < best_direct_cost) {
            best_direct_cost = weight;
        }
    }

    // result는 호출 전 INF 행렬이고, 함수가 min-plus 항등행렬에서 시작해 A^k로 바꾼다.
    MinPlusMatrix result{vertex_count, kInfinity};
    min_plus_power(adjacency, exact_edge_count, result);

    const Cost answer{result.cell(0, vertex_count - 1)};

    // [호출 계약: std::ostream의 정수/문자 삽입 operator<<]
    // (1) 수신 std::cout은 출력 가능한 std::ostream lvalue다. answer는 살아 있는 const Cost lvalue이며
    //     도달 불가능하면 kInfinity, 아니면 [1,10^18] 범위다.
    // (2) -1에는 ostream::operator<<(int), answer에는 std::int64_t의 구현별 기반 signed 정수 타입과
    //     일치하는 정수 삽입 overload, '\n'에는 비멤버 operator<<(basic_ostream<char>&, char)를 선택한다.
    // (3) -1은 int prvalue, answer는 lvalue-to-rvalue 변환된 Cost 값, '\n'은 char prvalue다. 출력 문자와
    //     스트림 버퍼의 소유권은 옮기지 않는다.
    // (4) 각 <<는 같은 std::ostream&를 반환해 다음 삽입의 수신자로 쓰고 마지막 참조는 사용하지 않는다.
    // (5) 성공하면 선택한 숫자와 줄바꿈이 cout 버퍼에 순서대로 추가된다. 입력 행렬과 answer는 변하지 않으며,
    //     실패하면 badbit/failbit가 설정될 수 있다.
    // (6) 출력 문자 수에 선형이고 streambuf 내부 할당/오류가 구현에 따라 가능하다. 예외 mask가 켜져 있으면
    //     ios_base::failure가 가능하다. 반환 참조 수명은 cout의 정적 수명 안이고 다른 스레드의 동시 출력은 없다.
    if (answer >= kInfinity) {
        std::cout << -1 << '\n';
    } else {
        std::cout << answer << '\n';
    }

    // main이 끝나면 result, adjacency가 생성 역순으로 파괴되어 vector 저장소도 자동 해제된다.
    return 0;
}
