/*
문제 요약 — CSES 1144 "Salary Queries"
출처: https://cses.fi/problemset/task/1144/

n명 직원의 현재 급여가 주어진다. 이후 두 종류의 명령을 순서대로 처리한다.
- ! k x: k번째 직원의 급여를 x로 바꾼다.
- ? a b: 현재 급여가 닫힌 구간 [a, b] 안에 있는 직원 수를 출력한다.

입력: 첫 줄에 직원 수 n과 명령 수 q, 둘째 줄에 n개의 초기 급여가 온다.
      이어지는 q개 줄에는 위 두 형식 가운데 하나가 주어진다.
출력: 각 '?' 명령마다 해당 범위의 직원 수를 한 줄에 하나씩 출력한다.
제약: 1 <= n, q <= 200,000, 최초 급여와 새 급여는 1 이상 10^9 이하이다.
      갱신의 직원 번호는 1 <= k <= n, 범위 질의는 1 <= a <= b <= 10^9를 만족한다.
예제:
  입력                         출력
  5 3                          3
  3 7 2 2 5                    2
  ? 2 3
  ! 3 6
  ? 2 3

풀이 개요: 초기 급여와 앞으로 갱신될 모든 급여를 오프라인 좌표 압축한다. 압축 순위별
직원 수를 Fenwick tree에 저장하면 갱신과 접두 빈도 질의가 각각 O(log(n+q))다.
전체 시간은 O((n+q) log(n+q)), 추가 공간은 O(n+q)이다.
*/

#include <algorithm> // std::sort/unique/lower_bound/upper_bound: 좌표 정렬·중복 제거·순위 검색에 쓴다.
#include <cstddef>   // std::size_t: vector 인덱스와 압축 좌표 개수를 표현한다.
#include <iostream>  // std::cin/std::cout: 명령을 읽고 범위 질의 답을 출력한다.
#include <vector>    // std::vector<T>: 급여, 명령, 압축 좌표, Fenwick 배열을 연속 저장한다.

// using 별칭은 새 타입을 만들지 않는다. 긴 표준 타입에 알고리즘상의 뜻만 부여한다.
using Index = std::size_t;
using Count = int;

// struct의 멤버는 기본적으로 public이다. 반대로 아래 class의 멤버는 기본적으로 private이다.
// char는 명령 종류 한 글자, int는 문제의 최대 급여 10^9와 직원 수 200,000을 담기에 충분하다.
struct Query final {
    char kind{}; // 중괄호 초기화 {}는 char를 널 문자 값 0으로 값 초기화한다.
    int first{}; // '!'이면 1-based 직원 번호, '?'이면 구간 왼쪽 끝이다.
    int second{}; // '!'이면 새 급여, '?'이면 구간 오른쪽 끝이다.
};

// 구현 가까운 공용 알고리즘 문서: ../algorithm/fenwick-tree.md
// class는 내부 배열을 private으로 숨겨 "각 칸은 담당 구간 빈도의 합"이라는 불변식을 보호한다.
class FenwickTree final {
public:
    // [첫 생성 계약: std::vector<Count>(count, value)]
    // (1) 수신 객체 tree_는 아직 수명이 시작되지 않은 std::vector<int> 멤버다.
    // (2) vector(size_type count, const int& value, const Allocator& = Allocator()) 생성자에서 T=int가 선택된다.
    // (3) size+1U는 Index prvalue인 칸 수, Count{}는 값 0인 int prvalue이며 소유권을 넘길 외부 자원은 없다.
    // (4) 생성자는 반환값이 없고, 성공하면 count개의 독립적인 0을 소유하는 tree_를 완성한다.
    // (5) 인자는 변하지 않으며 생성된 각 원소의 수명은 tree_의 수명/erase·재할당까지 이어진다.
    // (6) O(size) 시간·공간과 한 번 수준의 할당이 들며 bad_alloc/length_error 시 객체 생성이 실패한다.
    //     성공 뒤 별도 동기화 없는 같은 vector의 동시 쓰기는 허용되지 않고, 아직 외부 반복자는 없다.
    // explicit는 Index 하나가 FenwickTree로 암시 변환되는 것을 막는다. 콜론 뒤는 멤버 초기화 목록이다.
    explicit FenwickTree(const Index size)
        : tree_(size + 1U, Count{}) {}

    // 반환형 void는 결과 객체 대신 tree_의 상태를 바꾼다는 뜻이다. delta는 +1 또는 -1이다.
    void add(Index index, const Count delta) {
        // [첫 호출 계약: std::vector<Count>::size]
        // (1) 수신 tree_는 size+1개의 int를 소유한 유효한 std::vector<int> lvalue다.
        // (2) constexpr size_type size() const noexcept 오버로드가 선택되며 명시 인자는 없다.
        // (3) 수신 객체를 const 관찰로 빌릴 뿐 원소나 소유권을 입력으로 옮기지 않는다.
        // (4) 현재 원소 수를 std::vector<int>::size_type 값으로 반환해 while 조건에서 사용한다.
        // (5) tree_의 크기·capacity·원소·반복자 상태는 바뀌지 않는다.
        // (6) O(1), 무할당·noexcept·무효화 없음이다. 다른 스레드가 tree_를 동시에 수정하면 안 된다.
        while (index < tree_.size()) {
            // [첫 호출 계약: std::vector<Count>::operator[]]
            // (1) 수신 tree_는 유효한 std::vector<int>이고 불변식상 1 <= index < tree_.size()다.
            // (2) reference operator[](size_type position)의 비const 오버로드가 선택된다. 표준 선언에는
            //     noexcept 예외 명세가 붙지 않는다.
            // (3) index는 Index lvalue를 값 복사한 위치 인자이며 원소/컨테이너 소유권은 이동하지 않는다.
            // (4) 해당 int 원소의 int&를 반환하고 +=의 왼쪽 피연산자로 즉시 사용해 참조를 저장하지 않는다.
            // (5) 참조가 가리키는 담당 구간 빈도만 delta만큼 바뀌고 크기/capacity/반복자는 유지된다.
            // (6) O(1), 무할당·무효화 없음이다. 표준 시그니처상 무예외를 약속하지 않으며, 범위를 검사하지
            //     않아 position>=size()면 UB다. 같은 원소에 대한 동시 읽기/쓰기는 데이터 경쟁이다.
            tree_[index] += delta;
            // lowbit를 더하면 현재 점을 포함하는 바로 다음 상위 담당 구간으로 이동한다.
            index += lowbit(index);
        }
    }

    // 뒤의 const는 이 멤버 함수가 tree_를 바꾸지 않으며 const 객체에도 호출 가능하다는 약속이다.
    [[nodiscard]] Count prefix_sum(Index index) const noexcept {
        Count result{}; // int의 값 초기화이므로 0에서 누적을 시작한다.
        while (index > 0U) {
            // [첫 호출 계약: const std::vector<Count>::operator[]]
            // (1) 수신 tree_는 const this를 통해 보는, size+1개 int를 소유한 const std::vector<int> lvalue다.
            // (2) const_reference operator[](size_type position) const 오버로드가 선택되며 표준 선언에는
            //     noexcept 예외 명세가 붙지 않는다.
            // (3) index는 1<=index<tree_.size()인 Index lvalue이고 위치 값만 전달해 소유권을 옮기지 않는다.
            // (4) const int&를 반환하고 += 오른쪽에서 lvalue-to-rvalue 변환으로 읽으며 참조는 저장하지 않는다.
            // (5) tree_·원소·크기·capacity·index는 이 접근으로 바뀌지 않는다.
            // (6) O(1), 무할당·무효화 없음이고 표준 시그니처상 무예외를 약속하지 않는다. 범위 밖이면
            //     UB이며, 다른 스레드가 같은 원소를 수정하는 동안 읽으면 데이터 경쟁이다.
            result += tree_[index];
            // 불변식: 지금까지 더한 담당 구간들은 겹치지 않고 원래 접두 구간의 오른쪽을 덮는다.
            index -= lowbit(index);
        }
        return result;
    }

private:
    // static 함수는 특정 객체의 this 포인터가 필요 없다. 부호 없는 Index의 모듈러 산술을 이용한다.
    [[nodiscard]] static constexpr Index lowbit(const Index index) noexcept {
        return index & (~index + 1U);
    }

    // std::vector의 템플릿 인자 Count=int는 각 압축 구간에 저장할 빈도 타입이다.
    std::vector<Count> tree_;
};

// const 참조는 vector를 복사하거나 소유하지 않고, 함수 실행 동안 읽기 전용 lvalue로 빌린다.
// 반환값은 value보다 작은 압축 값의 개수이며, 정확한 value의 1-based Fenwick 순위는 여기에 1을 더한다.
[[nodiscard]] Index lower_position(const std::vector<int>& coordinates, const int value) {
    // [첫 호출 계약: const std::vector<int>::begin]
    // (1) 수신 coordinates는 오름차순·중복 제거된, 수명 중인 const std::vector<int> lvalue다.
    // (2) const_iterator begin() const noexcept 오버로드가 선택되고 명시 인자는 없다.
    // (3) coordinates를 읽기 전용으로 빌리며 원소나 버퍼 소유권을 넘기지 않는다.
    // (4) 첫 원소 또는 빈 경우 end와 같은 const_iterator를 반환해 first가 소유 없이 관찰한다.
    // (5) 컨테이너·원소·capacity는 변하지 않고 반복자 하나의 값만 만들어진다.
    // (6) O(1), 무할당·noexcept다. 이후 비const 재할당/erase가 반복자를 무효화하며 동시 수정은 금지된다.
    const auto first{coordinates.begin()};

    // [첫 호출 계약: const std::vector<int>::end]
    // (1) 수신 coordinates는 위 first와 같은 살아 있는 const std::vector<int>이다.
    // (2) const_iterator end() const noexcept 오버로드가 선택되고 명시 인자는 없다.
    // (3) 읽기 전용 수신만 사용하며 데이터 인자와 소유권 이전은 없다.
    // (4) 마지막 원소 다음 위치의 const_iterator를 반환해 반열린 범위 끝으로 사용한다.
    // (5) coordinates는 그대로이고 반환 반복자는 역참조하지 않는 센티널 역할을 한다.
    // (6) O(1), 무할당·noexcept·무효화 없음이다. 뒤의 구조 변경은 반복자를 무효화할 수 있고 동시 수정은 금지된다.
    const auto last{coordinates.end()};

    // [첫 호출 계약: std::lower_bound]
    // (1) 수신 객체 없는 알고리즘이며 [first,last)는 정렬된 vector<int>의 유효한 const 반복자 범위다.
    // (2) ForwardIt lower_bound(ForwardIt, ForwardIt, const T&)에서 ForwardIt=vector<int>::const_iterator,
    //     T=int인 기본 std::less 의미의 오버로드가 선택된다.
    // (3) first/last는 const_iterator lvalue를 값 복사하고 value는 const int lvalue로 빌리며 소유권 이전이 없다.
    // (4) *it >= value인 첫 위치(없으면 last)의 const_iterator를 반환해 found가 보관한다.
    // (5) 범위와 value는 바뀌지 않고 새 반복자 값만 생긴다.
    // (6) O(log M) 비교(임의 접근 반복자라 이동도 O(log M)), 무할당이다. 범위가 value 기준으로
    //     partition되지 않으면 전제조건 위반이며 비교/반복자 오류 외 예외는 없고 동시 수정은 금지된다.
    const auto found{std::lower_bound(first, last, value)};

    // [첫 호출 계약: vector const_iterator의 거리 뺄셈 식]
    // (1) 두 피연산자 found와 first는 같은 vector 범위의 유효한 random-access const_iterator lvalue다.
    // (2) random_access_iterator가 요구하는 `found - first` 식을 사용한다. 구체 iterator 타입과
    //     operator- 함수의 매개변수 전달 형태는 구현 세부사항이라 표준 시그니처로 단정하지 않는다.
    // (3) 두 반복자 const lvalue를 관찰하며 구현이 값/const 참조로 받을 수 있고 소유권을 옮기지 않는다.
    // (4) found-first 원소 거리를 signed difference_type으로 반환해 Index로 명시 변환한다.
    // (5) 반복자와 coordinates는 바뀌지 않고 반환 prvalue만 만들어진다.
    // (6) vector 반복자에서는 O(1), 무할당이며 표준화된 식에 noexcept를 가정하지 않는다. 서로 다른
    //     컨테이너 또는 무효 반복자의 뺄셈은 UB이나 현재 같은 반열린 범위가 전제를 보장한다.
    return static_cast<Index>(found - first);
}

// 반환값은 value 이하인 압축 값의 개수다. 이 수를 그대로 Fenwick 접두 위치로 쓸 수 있다.
[[nodiscard]] Index upper_position(const std::vector<int>& coordinates, const int value) {
    const auto first{coordinates.begin()};
    const auto last{coordinates.end()};

    // [첫 호출 계약: std::upper_bound]
    // (1) 수신 객체 없는 알고리즘이고 [first,last)는 정렬·중복 제거된 vector<int> const 범위다.
    // (2) ForwardIt upper_bound(ForwardIt, ForwardIt, const T&)에서 반복자는 vector<int>::const_iterator,
    //     T=int인 기본 비교 오버로드가 선택된다.
    // (3) 두 반복자는 값 복사, value는 const int lvalue 참조 입력이며 모두 비소유 관찰이다.
    // (4) value보다 큰 첫 원소 위치(없으면 last)의 const_iterator를 반환해 found가 보관한다.
    // (5) 입력 범위와 value는 변하지 않고 반복자 값만 새로 생긴다.
    // (6) O(log M) 비교와 임의 접근 이동, 무할당이다. 정렬/partition 전제 위반 시 결과 보장이 없으며
    //     현재 int 비교는 예외가 없고 호출 중 같은 vector를 다른 스레드가 수정하면 안 된다.
    const auto found{std::upper_bound(first, last, value)};
    // 같은 vector의 유효 반복자 뺄셈 계약은 lower_position에서 처음 설명했다.
    return static_cast<Index>(found - first);
}

int main() {
    // [첫 호출 계약: std::ios::sync_with_stdio]
    // (1) 특정 수신 객체가 없는 std::ios_base의 정적 전역 스트림 설정이며 아직 표준 입출력을 하지 않았다.
    // (2) static bool std::ios_base::sync_with_stdio(bool sync=true)가 std::ios 별칭을 통해 선택된다.
    // (3) false는 bool prvalue이고 소유권 의미가 없으며 C/C++ 표준 스트림 동기화를 끄라는 값이다.
    // (4) 이전 동기화 상태 bool을 반환하지만 이 풀이에서는 사용하지 않는다.
    // (5) 표준 스트림은 유효하고 빨라질 수 있으나 이후 C stdio와 섞은 출력 순서는 별도 보장이 없다.
    // (6) 표준이 점근 복잡도나 내부 buffer 할당 여부를 정하지 않으며 컨테이너 반복자 무효화는 해당하지
    //     않는다. 첫 I/O 뒤 호출 효과는 구현 정의이고 전역 설정을 여러 스레드가 동시에 바꾸지 않아야 한다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie]
    // (1) 수신 std::cin은 살아 있고 아직 입력하지 않은 std::istream 객체다.
    // (2) std::ostream* basic_ios::tie(std::ostream* tied_stream) 설정 오버로드가 선택된다.
    // (3) nullptr는 null pointer prvalue로 비소유 값이며 새 출력 스트림 소유권을 만들지 않는다.
    // (4) 이전 tie 대상 std::ostream*을 반환하지만 사용하지 않는다.
    // (5) cin은 입력 전 cout 자동 flush 연결 없이 유지되고 어느 스트림의 수명도 바뀌지 않는다.
    // (6) 표준이 복잡도를 명시하지 않고 무할당·무효화 없음이다. 수명 끝난 포인터를 주면 위험하지만 nullptr는
    //     안전하며 동일 스트림 설정의 동시 변경은 외부 동기화가 필요하다.
    std::cin.tie(nullptr);

    int employee_count{};
    int query_count{};

    // [첫 호출 계약: basic_istream::operator>>(int&)]
    // (1) 수신 std::cin은 유효한 std::istream이고 employee_count/query_count는 살아 있는 int lvalue다.
    // (2) basic_istream<char>& operator>>(int&) 멤버 오버로드가 두 번 연쇄 선택된다.
    // (3) 각 int& 인자는 수정 가능한 lvalue 참조이며 추출 값을 저장하지만 객체 소유권은 이동하지 않는다.
    // (4) 매 호출은 같은 std::istream&를 반환해 두 번째 추출에 쓰고 마지막 참조는 버린다.
    // (5) 성공 시 두 정수와 입력 위치가 갱신되고, 실패 시 대상 값은 보장된 규칙에 따라 유지/0 설정될 수 있으며
    //     failbit 또는 badbit가 설정된다.
    // (6) 소비 문자/locale/장치에 비례하고 자체 컨테이너 할당은 없다. 형식·범위 오류는 failbit, 장치 오류는
    //     badbit이며 exception mask에 따라 예외가 가능하고 같은 stream의 동시 추출은 금지된다.
    std::cin >> employee_count >> query_count;

    // [첫 호출 계약: basic_ios::operator!]
    // (1) 수신 std::cin은 바로 앞 두 정수 추출 뒤의 상태 비트를 가진 살아 있는 std::istream lvalue다.
    // (2) bool basic_ios::operator!() const가 unary ! 표현으로 선택되고 명시 인자는 없다.
    // (3) 수신을 const로 관찰할 뿐 버퍼/소유권 입력은 없다.
    // (4) fail()과 같은 bool을 반환해 if 분기에 사용하며 failbit 또는 badbit이면 true다.
    // (5) 스트림 상태·위치와 두 정수는 이 관찰로 더 바뀌지 않는다.
    // (6) 상수 시간 성격이고 할당·무효화가 없다. 표준 선언에는 noexcept가 없으므로 타입 수준 무예외를
    //     약속하지 않는다. 공유 스트림의 동시 상태 변경은 외부 동기화가 필요하다.
    if (!std::cin) {
        return 0;
    }

    // [첫 생성 계약: std::vector<int>(count)]
    // (1) salaries는 생성 전의 지역 std::vector<int> 객체다.
    // (2) explicit vector(size_type count, const Allocator& = Allocator()) 생성자에서 T=int가 선택된다.
    // (3) employee_count를 Index prvalue로 변환한 비음수 개수이며 외부 버퍼 소유권을 받지 않는다.
    // (4) 생성자는 반환값 없이 count개의 값 초기화된 int(모두 0)를 소유하는 salaries를 만든다.
    // (5) 인자는 불변이고 원소 수명은 salaries가 소유하며 이후 대입으로 각 값만 바뀐다.
    // (6) O(n) 시간·공간, 할당 가능, bad_alloc/length_error 시 생성 실패다. 성공 전 외부 반복자는 없다.
    std::vector<int> salaries(static_cast<Index>(employee_count));

    for (int employee{}; employee < employee_count; ++employee) {
        // add에서 처음 설명한 vector<int>::operator[] 계약이 적용된다. 변환한 인덱스는 n 미만이다.
        std::cin >> salaries[static_cast<Index>(employee)];
    }

    // [첫 생성 계약: std::vector<int> 복사 생성자]
    // (1) coordinates는 생성 전이고 salaries는 n개 초기 급여를 소유한 유효한 const로 읽을 수 있는 lvalue다.
    // (2) vector(const vector& other) 복사 생성자에서 T=int가 선택된다.
    // (3) salaries를 const 참조로 빌려 모든 int를 복사하고 salaries의 버퍼 소유권은 넘기지 않는다.
    // (4) 반환값 없이 같은 값을 독립 소유하는 coordinates를 완성한다.
    // (5) salaries는 그대로이고 두 vector의 버퍼/원소 수명은 독립적이다.
    // (6) O(n) 시간·공간과 할당이 들며 bad_alloc 시 coordinates 생성만 실패한다. 성공 뒤 반복자는 서로 호환되지 않는다.
    std::vector<int> coordinates(salaries);

    // [첫 호출 계약: std::vector<int>::reserve]
    // (1) 수신 coordinates는 n개 값을 소유한 수정 가능한 std::vector<int>이다.
    // (2) void reserve(size_type new_capacity) 멤버가 선택된다.
    // (3) n+q를 Index prvalue로 전달하며 이는 앞으로 저장할 초기/갱신 급여 최대 개수이고 소유권 입력이 아니다.
    // (4) 반환형은 void라 값은 없고 capacity 확보라는 부수 효과만 사용한다.
    // (5) 성공 시 size/원소는 같고 capacity>=요청값이다. 재할당했다면 기존 포인터·참조·반복자는 모두 무효다.
    // (6) 현재 capacity보다 크면 O(n) 이동/복사와 할당, 아니면 O(1)이다. bad_alloc/length_error 가능하며
    //     int에서는 실패 시 원래 vector가 유지되는 강한 보장을 기대할 수 있고 같은 객체 동시 접근은 금지된다.
    coordinates.reserve(static_cast<Index>(employee_count + query_count));

    // [첫 생성 계약: std::vector<Query>(count)]
    // (1) queries는 아직 생성되지 않은 지역 std::vector<Query> 객체다.
    // (2) explicit vector(size_type count, const Allocator& = Allocator())에서 T=Query가 선택된다.
    // (3) query_count를 Index prvalue로 전달하고 외부 객체의 소유권은 받지 않는다.
    // (4) 반환값 없이 count개의 값 초기화된 Query를 소유하는 queries를 만든다.
    // (5) 각 Query 멤버는 0으로 시작하고 수명은 vector가 관리하며 인자 값은 변하지 않는다.
    // (6) O(q) 시간·공간과 할당, bad_alloc/length_error 가능이다. 재할당 전이라 외부 반복자는 없다.
    std::vector<Query> queries(static_cast<Index>(query_count));

    for (int query_index{}; query_index < query_count; ++query_index) {
        Query query{}; // 사용자 정의 aggregate의 값 초기화이며 세 멤버가 모두 0으로 시작한다.

        // [첫 호출 계약: operator>>(std::istream&, char&)]
        // (1) 수신 std::cin은 앞 입력 뒤 유효한 std::istream이고 query.kind는 수정 가능한 char lvalue다.
        // (2) operator>>(basic_istream<char>&, char&) 비멤버 문자 추출 오버로드가 선택된다.
        // (3) cin lvalue와 kind lvalue를 비소유 참조로 받아 공백을 건너뛴 다음 문자 하나를 kind에 저장한다.
        // (4) 같은 std::istream&를 반환하지만 이 문장에서는 사용하지 않는다.
        // (5) 성공 시 kind와 입력 위치가 바뀌고, 실패 시 상태 비트가 설정되며 kind는 갱신되지 않는다.
        // (6) 입력 장치/locale 비용, 무할당이다. EOF·형식 실패는 failbit/eofbit, 장치 오류는 badbit이며
        //     exception mask에 따라 예외가 가능하고 같은 stream의 동시 입력은 금지된다.
        std::cin >> query.kind;
        std::cin >> query.first >> query.second;

        // [첫 호출 계약: std::vector<Query>::operator[]]
        // (1) 수신 queries는 q개 Query를 소유하고 0<=query_index<q인 수정 가능한 std::vector<Query>다.
        // (2) Query& operator[](size_type position)의 비const 오버로드가 선택되며 표준 선언에는
        //     noexcept 예외 명세가 붙지 않는다.
        // (3) query_index를 Index prvalue로 변환한 위치이며 query/버퍼 소유권을 넘기지 않는다.
        // (4) 해당 Query의 lvalue 참조를 반환해 오른쪽 query의 세 값을 복사 대입하고 참조는 저장하지 않는다.
        // (5) 그 원소만 입력 명령으로 바뀌며 vector 크기/capacity와 모든 반복자는 유지된다.
        // (6) O(1), 무할당·무효화 없음이며 표준 시그니처상 무예외를 약속하지 않는다. 범위 밖은 UB지만
        //     루프가 막고 같은 원소 동시 접근은 금지된다.
        queries[static_cast<Index>(query_index)] = query;

        if (query.kind == '!') {
            // [첫 호출 계약: std::vector<int>::push_back]
            // (1) 수신 coordinates는 초기 급여를 소유하고 reserve로 최대 n+q capacity를 확보한 vector<int>다.
            // (2) void push_back(const int& value) lvalue 복사 오버로드가 선택된다.
            // (3) query.second는 살아 있는 int lvalue로 읽기만 하며 Query의 소유권을 넘기지 않는다.
            // (4) 반환형은 void이고, 새 압축 후보를 뒤에 추가하는 부수 효과를 사용한다.
            // (5) size가 1 증가하고 끝에 같은 int가 생긴다. capacity 안이면 기존 참조/반복자는 유지되되 end는 무효다.
            // (6) 상각 O(1), 여기서는 예약 범위라 재할당이 없다. 일반적으로 bad_alloc 가능·재할당 시 전부 무효이며,
            //     int 복사 실패는 없고 같은 vector에 대한 동시 접근은 금지된다.
            coordinates.push_back(query.second);
        }
    }

    // [첫 호출 계약: mutable std::vector<int>::begin]
    // (1) 수신 coordinates는 모든 초기/갱신 급여 후보를 소유한 수정 가능한 std::vector<int> lvalue다.
    // (2) iterator begin() noexcept의 비const 오버로드가 선택되고 명시 인자는 없다.
    // (3) 수신 버퍼를 비소유로 빌리며 값/소유권 인자는 없다.
    // (4) 첫 원소를 가리키는 mutable iterator를 반환해 sort_first가 보관한다.
    // (5) vector는 변하지 않지만 반복자를 통한 원소 수정 권한이 생긴다.
    // (6) O(1), 무할당·noexcept다. 구조 변경 시 무효화될 수 있고 호출/사용 중 동시 구조 수정은 금지된다.
    const auto sort_first{coordinates.begin()};

    // [첫 호출 계약: mutable std::vector<int>::end]
    // (1) 수신 coordinates는 sort_first와 같은 수정 가능한 vector<int>다.
    // (2) iterator end() noexcept의 비const 오버로드가 선택되고 명시 인자는 없다.
    // (3) 수신만 비소유로 사용하며 별도 데이터 인자는 없다.
    // (4) 마지막 다음 위치의 mutable iterator를 반환해 sort_last가 반열린 범위 끝으로 보관한다.
    // (5) 원소/크기/capacity는 변하지 않고 end 반복자는 역참조하지 않는다.
    // (6) O(1), 무할당·noexcept다. 이후 erase가 이 반복자를 무효화하며 그 전 동시 수정은 금지된다.
    const auto sort_last{coordinates.end()};

    // [첫 호출 계약: std::sort]
    // (1) 수신 객체 없는 알고리즘이고 [sort_first,sort_last)는 같은 vector<int>의 유효한 mutable 범위다.
    // (2) void sort(RandomIt first, RandomIt last)에서 RandomIt=vector<int>::iterator가 선택되고,
    //     C++20 이후 생략된 비교는 std::less{}의 엄격 약순서 의미를 쓴다.
    // (3) 반복자 두 개를 값 복사해 범위를 빌리며 int 원소를 이동/교환하지만 vector 버퍼 소유권은 옮기지 않는다.
    // (4) 반환형은 void이고 오름차순 재배치 결과만 사용한다.
    // (5) size/capacity/반복자 유효성은 유지되지만 각 위치의 int 값 순서는 바뀐다.
    // (6) O(M log M) 비교 상한과 구현별 스택 공간이 들고 별도 원소 할당은 요구되지 않는다. int 비교·이동은
    //     무예외이며 유효 random-access 범위가 전제이고 호출 중 동시 접근은 금지된다.
    std::sort(sort_first, sort_last);

    // [첫 호출 계약: std::unique]
    // (1) 수신 객체 없는 알고리즘이고 [sort_first,sort_last)는 정렬된 mutable vector<int> 범위다.
    // (2) ForwardIt unique(ForwardIt first, ForwardIt last)에서 ForwardIt=vector<int>::iterator가 선택된다.
    // (3) 반복자들을 값 복사하고 C++20 이후 기본 std::equal_to{}로 인접 int를 비교해 중복 아닌 값을
    //     앞쪽으로 이동한다. vector buffer의 소유권 이전은 없다.
    // (4) 중복 제거 후 논리 범위의 끝 iterator를 반환해 unique_end가 보관한다.
    // (5) [first,unique_end)는 유일한 정렬 값이고 뒤 꼬리 값은 유효하지만 미지정 상태이며 실제 size는 아직 같다.
    // (6) M>0이면 정확히 M-1회 비교하는 O(M), 무할당이다. int 이동은 무예외이고 유효 범위가 전제이며,
    //     반복자는 구조적으로 유지되지만 호출 중 같은 원소에 동시 접근하면 데이터 경쟁이다.
    const auto unique_end{std::unique(sort_first, sort_last)};

    // [첫 호출 계약: std::vector<int>::erase(iterator, iterator)]
    // (1) 수신 coordinates는 물리 크기 M이고 [unique_end,sort_last)가 중복 꼬리인 vector<int>이다.
    // (2) iterator erase(const_iterator first, const_iterator last) 범위 오버로드가 iterator 변환으로 선택된다.
    // (3) 두 iterator lvalue가 같은 컨테이너의 유효한 순서 범위를 나타내며 버퍼 소유권은 이동하지 않는다.
    // (4) 지운 범위 뒤 새 위치(여기서는 새 end)의 iterator를 반환하지만 사용하지 않는다.
    // (5) 꼬리가 파괴되어 size가 유일 값 개수로 줄고 unique_end 및 그 이후와 기존 end 반복자는 무효다.
    // (6) 뒤에 남은 원소 수만큼 이동하고 제거 원소만큼 소멸하는 선형 시간, 무할당이다. int에서는 무예외이며
    //     범위가 뒤집히거나 타 컨테이너 반복자면 UB이고 같은 vector의 동시 접근은 금지된다.
    coordinates.erase(unique_end, sort_last);

    FenwickTree frequency{coordinates.size()};
    for (int employee{}; employee < employee_count; ++employee) {
        const int salary{salaries[static_cast<Index>(employee)]};
        frequency.add(lower_position(coordinates, salary) + 1U, 1);
    }

    // 대회 핵심 불변식:
    // 1) salaries[i]는 재생 시점 i번째 직원의 정확한 현재 급여다.
    // 2) Fenwick의 각 압축 순위 빈도는 그 급여를 받는 현재 직원 수와 같다.
    // 3) 그러므로 prefix_sum(p)는 압축 순위 p 이하 급여의 직원 수다.
    for (int query_index{}; query_index < query_count; ++query_index) {
        const Query query{queries[static_cast<Index>(query_index)]};

        if (query.kind == '!') {
            const Index employee_index{static_cast<Index>(query.first - 1)};
            const int previous_salary{salaries[employee_index]};

            // 이전 급여 빈도를 먼저 빼고 새 급여 빈도를 더하면 불변식 2가 보존된다.
            // 같은 값으로 반복 갱신해도 -1과 +1이 상쇄되므로 별도 분기가 필요 없다.
            frequency.add(lower_position(coordinates, previous_salary) + 1U, -1);
            salaries[employee_index] = query.second;
            frequency.add(lower_position(coordinates, query.second) + 1U, 1);
        } else {
            // #([a,b]) = #(salary<=b) - #(salary<a). query 경계가 압축 목록에 없어도
            // upper_bound/lower_bound의 삽입 위치를 쓰므로 빈 틈과 양 끝 경계가 정확하다.
            const Index right_prefix{upper_position(coordinates, query.second)};
            const Index before_left{lower_position(coordinates, query.first)};
            const Count answer{frequency.prefix_sum(right_prefix) - frequency.prefix_sum(before_left)};

            // [첫 호출 계약: basic_ostream::operator<<(int)]
            // (1) 수신 std::cout은 살아 있는 std::ostream이고 answer는 [0,n]의 const int lvalue다.
            // (2) basic_ostream<char>& operator<<(int value) 멤버 오버로드가 선택된다.
            // (3) answer 값을 복사해 형식화하며 값/스트림 버퍼의 소유권을 넘기지 않는다.
            // (4) 같은 std::ostream&를 반환하지만 이 문장에서는 사용하지 않는다.
            // (5) answer는 그대로이고 cout 버퍼/상태와 외부 출력 위치가 진행된다.
            // (6) 표준은 공통 점근 복잡도나 내부 buffer 할당 여부를 정하지 않으며 컨테이너 무효화는 해당하지
            //     않는다. 장치 실패는 상태 비트를 세우고 exception mask에 따라 예외가 가능하다. 위에서 C/C++
            //     stream 동기화를 껐으므로 같은 stream에 동시 접근하려면 외부 동기화가 필요하며, 없으면 데이터
            //     경쟁이 가능하다.
            std::cout << answer;

            // [첫 호출 계약: operator<<(std::ostream&, char)]
            // (1) 수신 std::cout은 바로 앞 정수 출력 뒤의 유효한 std::ostream lvalue다.
            // (2) basic_ostream<char>& operator<<(basic_ostream<char>&, char) 비멤버 오버로드가 선택된다.
            // (3) 첫 인자는 cout lvalue, 둘째 '\n'은 char prvalue이며 둘 다 비소유 입력이다.
            // (4) 같은 std::ostream&를 반환하지만 사용하지 않는다.
            // (5) 줄바꿈 문자 하나와 출력 상태가 갱신되며 endl과 달리 강제 flush하지 않는다.
            // (6) 표준은 공통 점근 복잡도나 내부 buffer 할당 여부를 정하지 않으며 컨테이너 무효화는 해당하지
            //     않는다. 장치 오류와 설정된 예외가 가능하다. 위에서 동기화를 껐으므로 같은 stream의 동시 접근은
            //     외부 동기화가 필요하고, 그렇지 않으면 데이터 경쟁이 가능하다.
            std::cout << '\n';
        }
    }

    // 실행 관점: vector의 연속 메모리에서 빈도를 load/store하고, lowbit 루프와 이진 탐색은 비교·조건 분기를
    // 수행한다. 정확한 명령어·분기 제거·캐시 효과는 CPU, 컴파일러, 표준 라이브러리와 최적화 옵션에 따라 다르다.
    return 0;
}
