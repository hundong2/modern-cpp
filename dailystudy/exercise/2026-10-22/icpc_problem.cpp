/*
문제: CSES 1735 - Range Updates and Sums
공식 출처: https://cses.fi/problemset/task/1735/

[문제 요약]
길이 n인 정수 배열을 유지하면서 다음 세 명령을 온라인으로 처리한다.
1) 닫힌 구간 [a,b]의 모든 원소에 x를 더한다.
2) 닫힌 구간 [a,b]의 모든 원소를 x로 바꾼다.
3) 닫힌 구간 [a,b]의 원소 합을 출력한다.
대입과 덧셈은 순서를 바꾸면 결과가 달라지므로, 아직 자식에게 내리지 않은
연산을 정확한 시간 순서로 합성하는 것이 핵심이다.

[입력]
- 첫 줄: 배열 길이 n, 명령 수 q
- 둘째 줄: 초기값 t_1, ..., t_n
- 이어지는 q줄:
  - `1 a b x`: [a,b]에 x를 더한다.
  - `2 a b x`: [a,b]를 x로 대입한다.
  - `3 a b`: [a,b]의 합을 묻는다.

[출력]
3번 명령마다 현재 구간 합을 한 줄에 하나씩 출력한다.

[제약]
- 1 <= n,q <= 200,000
- 1 <= t_i,x <= 1,000,000
- 1 <= a <= b <= n

[예제 - 공식 수치를 학습용으로 다시 설명]
입력
6 5
2 3 1 1 5 3
3 3 5
1 2 4 2
3 3 5
2 2 4 5
3 3 5

처음 3..5의 합은 7이다. 2..4에 2를 더한 뒤 같은 구간의 합은 11이고,
2..4를 모두 5로 바꾼 뒤에는 15다. 따라서 출력은 차례로 7, 11, 15다.
*/

// <cstddef>는 컨테이너 크기와 인덱스에 쓰는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력 std::cin, 표준 출력 std::cout과 스트림 설정을 제공한다.
#include <iostream>
// <vector>는 초기 배열, 세그먼트 트리 합, 지연 태그를 연속 저장하고 소유한다.
#include <vector>

// using은 긴 타입 이름에 문제의 의미를 붙이는 별칭 선언이며 새 타입을 만들지는 않는다.
// long long은 표준상 최소 64비트이므로 위에서 계산한 최대 합을 안전하게 담는다.
using Value = long long;
// 배열 위치와 입력 개수는 최대 200,000이어서 기본 부호 있는 int 범위에 충분하다.
using Index = int;
// 트리 노드 번호는 vector의 size_type과 같은 계열인 std::size_t를 사용한다.
using NodeIndex = std::size_t;

// 자세한 원리와 변형 문서: ../algorithm/lazy-segment-tree.md
// 대회 불변식: 각 tree_[node]는 자식 tag가 아직 내려가지 않았어도 담당 구간의 현재 논리 합이다.
// LazyTag는 f(v)=(has_assignment ? assignment : v)+addition을 나타낸다. 새 대입은 과거
// 대입/덧셈을 덮고, 새 덧셈은 addition에 쌓인다. 그러므로 set→add와 add→set은 다르다.
// 일부 구간을 방문하기 직전에만 push하고 대입 뒤 덧셈 순으로 내려 시간 순서를 보존한다.
// build는 O(n), 각 명령은 O(log n), 전체는 O(n+q log n), 저장소는 O(n), stack은 O(log n)이다.
//
// 64비트 근거: 한 원소는 약 2.00001e11, 전체 합은 약 4.00002e16까지 커질 수 있지만
// signed long long의 최소 보장 최대치 약 9.22e18보다 작다. 따라서 원소, 합, 태그와
// 길이 곱을 모두 Value로 계산한다. 인덱스는 최대 200,000이므로 int 범위 안이다.
//
// 기계 실행 관점: 재귀 탐색은 합/tag load·store와 구간 비교·조건 분기를 만들 수 있다.
// 연속 vector는 지역성에 유리할 수 있지만 실제 load/store 수, 분기 예측, register 배치와
// inline 여부는 CPU·ABI·컴파일러·최적화 옵션에 따라 달라 특정 어셈블리로 단정하지 않는다.
// class는 기본 접근이 private이다. public API만 외부에 열어 태그 불변식을 보호한다.
class RangeAssignAddSumTree {
private:
    // struct는 기본 접근이 public인 단순 값 묶음이다. 세 필드는 하나의 지연 함수를 표현한다.
    struct LazyTag {
        // 중괄호 초기화 `{}`는 bool을 false, 정수들을 0으로 값 초기화한다.
        bool has_assignment{};
        Value assignment{};
        Value addition{};
    };

public:
    // explicit은 vector 하나가 트리 객체로 뜻하지 않게 암시 변환되는 것을 막는다.
    // const lvalue 참조는 호출자가 소유한 초기 배열을 복사하지 않고 읽기만 하며,
    // 생성자가 끝난 뒤 참조를 보관하지 않으므로 호출자 배열의 이후 수명에 의존하지 않는다.
    explicit RangeAssignAddSumTree(const std::vector<Value>& values)
        /*
        [std::vector<Value>::size 첫 호출 계약]
        (1) 수신 객체는 `const std::vector<Value>`인 values이며, 생성이 끝난 유효한
            연속 컨테이너이고 공식 제약상 원소가 1개 이상이다.
        (2) 선택 시그니처는 `size_type size() const noexcept`이다.
        (3) 명시적 인자는 없고 소유권 이동도 없다.
        (4) 반환형은 `std::vector<Value>::size_type`이며 현재 원소 수를 돌려준다.
            여기서는 제약상 int에 들어감을 근거로 명시 변환해 size_ 초기화에 쓴다.
        (5) 호출 뒤 values의 크기, 용량, 원소와 관찰자 유효성은 바뀌지 않는다.
        (6) 전제는 유효한 객체라는 것뿐이며 O(1), 무할당, 무효화 없음, 예외 없음이다.
            const 읽기끼리는 가능하지만 다른 스레드의 동시 변경과는 동기화되지 않는다.
        */
        // [첫 호출 계약: std::vector<Value>::size]
        // (1) 수신 values는 유효한 const vector<Value> lvalue다.
        // (2) 선택 시그니처는 size_type size() const noexcept다.
        // (3) 인자는 없고 소유권 이동도 없다.
        // (4) 현재 원소 수를 반환해 제약상 안전한 Index 변환에 사용한다.
        // (5) 수신 객체와 관찰자는 바뀌지 않는다.
        // (6) O(1), 무할당·무효화·예외 없음이며 동시 변경은 허용하지 않는다.
        : size_{static_cast<Index>(values.size())},
          /*
          [std::vector<Value> 채움 생성자 첫 호출 계약]
          (1) tree_는 아직 수명이 시작되기 전인 `std::vector<Value>` 멤버다.
          (2) 선택 오버로드는 `vector(size_type count, const Value& value)`이며 기본
              allocator를 사용한다.
          (3) count는 `values.size()*4`의 size_type prvalue, value는 `Value{0}`
              prvalue가 const 참조에 바인딩된다. 0이 각 노드로 복사되며 소유권 이전은 없다.
          (4) 생성자는 반환값이 없고, 완성된 vector를 tree_ 자체로 만든다.
          (5) 성공하면 size가 count이고 capacity가 count 이상인 유효한 vector가
              count개의 0을 독점 소유한다.
              values는 변하지 않는다.
          (6) count는 사용 가능한 최대 크기 이하여야 한다. O(count) 시간/공간이고 보통
              한 번 동적 할당한다. 실패 시 std::bad_alloc 등이 전파되며 미완성 멤버는
              정리된다. 성공 뒤 참조 수명은 tree_ 또는 재할당까지이며 동기화는 제공하지 않는다.
          */
          tree_(values.size() * 4U, Value{0}),
          /*
          [std::vector<LazyTag> 채움 생성자 첫 호출 계약]
          (1) lazy_는 아직 수명이 시작되기 전인 `std::vector<LazyTag>` 멤버다.
          (2) 선택 오버로드/템플릿 인자는 `std::vector<LazyTag>::vector(
              size_type, const LazyTag&)`와 기본 allocator다.
          (3) count는 `values.size()*4`의 size_type prvalue, 두 번째 인자는
              `LazyTag{false,0,0}`과 같은 값 초기화 prvalue이며 복사될 뿐 소유권은 없다.
          (4) 반환값은 없고 lazy_ 객체 자체가 생성 결과다.
          (5) 성공하면 모든 원소가 항등 태그인 count 크기 vector를 lazy_가 소유한다.
          (6) O(count) 시간/공간, 동적 할당 가능, 실패 시 bad_alloc/원소 생성 예외가
              전파되고 부분 생성 원소는 정리된다. 재할당 전까지 원소 주소가 유효하며
              컨테이너 자체는 내부 동기화를 제공하지 않는다.
          */
          lazy_(values.size() * 4U, LazyTag{}) {
        // 공식 입력은 비어 있지 않지만, 방어적으로 빈 vector의 [0,-1] 빌드를 피한다.
        if (size_ > 0) {
            build(1U, 0, size_ - 1, values);
        }
    }

    // 반환형 void는 결과값 없이 상태를 바꾼다는 뜻이다. 매개변수는 모두 값 복사다.
    void range_add(Index query_left, Index query_right, Value delta) {
        add_impl(1U, 0, size_ - 1, query_left, query_right, delta);
    }

    // 새 대입은 이전 지연 함수를 덮는 비가환 연산이다.
    void range_assign(Index query_left, Index query_right, Value value) {
        assign_impl(1U, 0, size_ - 1, query_left, query_right, value);
    }

    // [[nodiscard]]는 계산한 합을 실수로 버리는 호출에 컴파일러가 경고할 수 있게 한다.
    // 논리 배열은 읽기만 하지만 push가 내부 표현을 갱신하므로 이 함수는 const가 아니다.
    [[nodiscard]] Value range_sum(Index query_left, Index query_right) {
        return sum_impl(1U, 0, size_ - 1, query_left, query_right);
    }

private:
    // 닫힌 구간 길이를 Value로 바꿔 곱셈 전 int 범위를 벗어날 가능성을 제거한다.
    [[nodiscard]] static Value segment_length(Index left, Index right) {
        return static_cast<Value>(right - left + 1);
    }

    // 원본 배열을 잎에 저장한 뒤 두 자식 합으로 내부 노드를 만드는 O(n) 함수다.
    void build(NodeIndex node, Index left, Index right,
               const std::vector<Value>& values) {
        // 길이 1이면 이 노드는 원소 하나를 담당하는 잎이다.
        if (left == right) {
            /*
            [std::vector<Value>::operator[] 첫 호출 계약]
            (1) 왼쪽 수신 객체 tree_는 size 4*n의 변경 가능한 `std::vector<Value>`,
                오른쪽 values는 size n의 `const std::vector<Value>`이며 둘 다 유효하다.
            (2) 각각 `Value& operator[](size_type)`와
                `const Value& operator[](size_type) const`가 선택된다.
            (3) node는 [1,4*n) 범위의 size_type lvalue이고, 변환한 left는 [0,n)의
                size_type prvalue다. 인덱스나 원소 소유권은 이전되지 않는다.
            (4) 각 호출은 해당 원소의 lvalue 참조를 반환한다. 오른쪽 값은 읽고,
                왼쪽 참조는 대입 대상으로 사용한다.
            (5) 대입으로 tree_[node] 값만 바뀌며 두 vector의 크기/용량은 그대로다.
            (6) operator[]는 범위를 검사하지 않아 인덱스가 벗어나면 UB다. 여기서는
                재귀 구간/4*n 할당 불변식이 범위를 보장한다. O(1), 무할당, 참조
                무효화 없음이다. 서로 다른 원소라도 동시 접근 규칙은 호출자가 지켜야 한다.
            */
            // [첫 호출 계약: std::vector<Value>::operator[] const/non-const]
            // (1) 수신 tree_와 values는 각각 유효한 non-const/const vector<Value> lvalue다.
            // (2) 선택 오버로드는 Value& 및 const Value& operator[](size_type)다.
            // (3) 인자는 불변식이 범위 안으로 보장한 size_type 인덱스이며 소유권 이동은 없다.
            // (4) 원소 lvalue 참조를 반환해 왼쪽은 쓰고 오른쪽은 읽는다.
            // (5) 왼쪽 원소 값만 바뀌고 크기·용량·관찰자는 그대로다.
            // (6) O(1), 무할당·무효화·예외 없음이지만 범위 밖 인자는 UB이고 동시 쓰기는 동기화하지 않는다.
            tree_[node] = values[static_cast<std::size_t>(left)];
            return;
        }

        // 이 식은 left+right를 먼저 더하지 않아 일반적인 인덱스 덧셈 오버플로를 피한다.
        const Index middle{left + (right - left) / 2};
        build(node * 2U, left, middle, values);
        build(node * 2U + 1U, middle + 1, right, values);
        pull(node);
    }

    // 두 자식의 최신 합을 부모에 저장해 tree 불변식을 복구한다.
    void pull(NodeIndex node) {
        tree_[node] = tree_[node * 2U] + tree_[node * 2U + 1U];
    }

    // 노드 전체에 대입을 즉시 반영한다. 새 대입은 과거 태그를 완전히 덮는다.
    void apply_assignment(NodeIndex node, Index left, Index right, Value value) {
        tree_[node] = value * segment_length(left, right);
        /*
        [std::vector<LazyTag>::operator[] 첫 호출 계약]
        (1) 수신 객체 lazy_는 size 4*n의 변경 가능한 `std::vector<LazyTag>`이며,
            모든 원소 수명이 시작된 상태다.
        (2) 선택 오버로드는 `LazyTag& operator[](size_type)`이다.
        (3) node는 재귀 불변식이 [1,4*n)로 보장하는 size_type lvalue이며 소유권 이동은 없다.
        (4) 반환형은 LazyTag lvalue 참조이고 지역 참조 tag에 바인딩해 세 필드를 수정한다.
        (5) vector 크기/용량은 그대로이고 해당 태그 값만 바뀐다. tag는 함수가 끝날 때
            소멸하지만 참조 대상은 lazy_와 함께 계속 산다.
        (6) 범위 밖이면 UB, 유효 범위에서는 O(1), 무할당, 무효화/예외 없음이다.
            같은 태그에 대한 동시 읽기/쓰기는 데이터 경쟁이며 이 단일 스레드 풀이에는 없다.
        */
        // [첫 호출 계약: std::vector<LazyTag>::operator[]]
        // (1) 수신 lazy_는 유효한 non-const vector<LazyTag> lvalue다.
        // (2) 선택 오버로드는 LazyTag& operator[](size_type)다.
        // (3) node는 범위 안 size_type 인자이고 소유권 이동이 없다.
        // (4) 원소 lvalue 참조를 반환해 tag에 바인딩한다.
        // (5) 태그 필드만 바뀌며 vector 크기·용량과 다른 관찰자는 유지된다.
        // (6) O(1), 무할당·무효화·예외 없음이나 범위 밖은 UB이고 동시 쓰기는 동기화하지 않는다.
        LazyTag& tag{lazy_[node]};
        tag.has_assignment = true;
        tag.assignment = value;
        tag.addition = Value{0};
    }

    // 기존 태그가 어떤 형태든 새 덧셈은 그 뒤에 실행되므로 addition에 누적한다.
    void apply_addition(NodeIndex node, Index left, Index right, Value delta) {
        tree_[node] += delta * segment_length(left, right);
        lazy_[node].addition += delta;
    }

    // 부모 태그를 두 자식에 같은 시간 순서로 전달하고 부모를 항등 태그로 되돌린다.
    void push(NodeIndex node, Index left, Index right) {
        // 잎은 자식이 없으므로 태그를 아래로 보낼 필요가 없다.
        if (left == right) {
            return;
        }

        // 값 복사는 push 도중 부모 태그를 초기화해도 적용할 정보를 안전하게 보존한다.
        const LazyTag pending{lazy_[node]};
        // 항등 태그면 메모리 쓰기와 재귀 호출을 피한다.
        if (!pending.has_assignment && pending.addition == Value{0}) {
            return;
        }

        const Index middle{left + (right - left) / 2};
        // 대입을 먼저 적용해야 그보다 오래된 자식 태그가 정확히 덮인다.
        if (pending.has_assignment) {
            apply_assignment(node * 2U, left, middle, pending.assignment);
            apply_assignment(node * 2U + 1U, middle + 1, right, pending.assignment);
        }
        // 같은 부모 태그에서 덧셈은 대입 뒤에 실행된 연산이므로 나중에 내린다.
        if (pending.addition != Value{0}) {
            apply_addition(node * 2U, left, middle, pending.addition);
            apply_addition(node * 2U + 1U, middle + 1, right, pending.addition);
        }
        lazy_[node] = LazyTag{};
    }

    // 목표 구간에 덧셈을 적용하는 재귀 구현이다.
    void add_impl(NodeIndex node, Index left, Index right,
                  Index query_left, Index query_right, Value delta) {
        // 전혀 겹치지 않으면 현재 노드와 태그를 바꾸지 않는다.
        if (query_right < left || right < query_left) {
            return;
        }
        // 완전 포함이면 자식까지 내려가지 않고 합과 태그만 O(1)에 바꾼다.
        if (query_left <= left && right <= query_right) {
            apply_addition(node, left, right, delta);
            return;
        }

        push(node, left, right);
        const Index middle{left + (right - left) / 2};
        add_impl(node * 2U, left, middle, query_left, query_right, delta);
        add_impl(node * 2U + 1U, middle + 1, right,
                 query_left, query_right, delta);
        pull(node);
    }

    // 목표 구간에 대입을 적용한다. 완전 포함 노드에서는 이전 태그를 모두 버린다.
    void assign_impl(NodeIndex node, Index left, Index right,
                     Index query_left, Index query_right, Value value) {
        if (query_right < left || right < query_left) {
            return;
        }
        if (query_left <= left && right <= query_right) {
            apply_assignment(node, left, right, value);
            return;
        }

        push(node, left, right);
        const Index middle{left + (right - left) / 2};
        assign_impl(node * 2U, left, middle, query_left, query_right, value);
        assign_impl(node * 2U + 1U, middle + 1, right,
                    query_left, query_right, value);
        pull(node);
    }

    // 겹치는 표준 구간들의 합을 반환한다. 합의 항등원 0을 비겹침 결과로 쓴다.
    [[nodiscard]] Value sum_impl(NodeIndex node, Index left, Index right,
                                 Index query_left, Index query_right) {
        if (query_right < left || right < query_left) {
            return Value{0};
        }
        if (query_left <= left && right <= query_right) {
            return tree_[node];
        }

        push(node, left, right);
        const Index middle{left + (right - left) / 2};
        return sum_impl(node * 2U, left, middle, query_left, query_right)
             + sum_impl(node * 2U + 1U, middle + 1, right,
                        query_left, query_right);
    }

    // 멤버 선언 순서가 생성자 초기화 순서다. size_가 먼저, 소유 vector들이 뒤따른다.
    Index size_{};
    std::vector<Value> tree_;
    std::vector<LazyTag> lazy_;
};

// main은 운영체제가 호출하는 진입점이며 int 반환값 0은 정상 종료를 뜻한다.
int main() {
    /*
    [std::ios_base::sync_with_stdio 첫 호출 계약]
    (1) 별도 수신 객체가 없는 정적 함수이며, 표준 스트림은 프로그램 시작 뒤 유효하고
        아직 이 프로그램의 입출력 연산을 수행하지 않은 상태다.
    (2) 시그니처는 `static bool sync_with_stdio(bool sync = true)`이다.
    (3) false는 bool prvalue이며 C stdio와 C++ iostream 사이 동기화를 끄라는 설정값이다.
        소유권 의미는 없다.
    (4) bool 반환값은 호출 전 동기화 상태를 뜻하지만 여기서는 사용하지 않는다.
    (5) 이후 C와 C++ 표준 스트림을 섞을 때 자동 순서 동기화를 기대할 수 없고,
        C++ 스트림 버퍼링 자유도가 커진다.
    (6) 표준 입출력 전에 호출해야 동작이 구현 정의로 흐르지 않는다. 표준은 별도
        복잡도나 noexcept 보장을 명시하지 않으므로 상수 시간·예외 없음으로 단정하지 않는다.
        사용자 컨테이너 관찰자를 무효화하지 않으며, 전역 스트림 설정이므로 실행 중
        다른 스레드와 동시에 바꾸지 않는다.
    */
    // [첫 호출 계약: std::ios::sync_with_stdio]
    // (1) 정적 함수라 수신 객체가 없고 표준 스트림은 아직 이 프로그램에서 사용되지 않았다.
    // (2) 선택 시그니처는 static bool ios_base::sync_with_stdio(bool)이다.
    // (3) 입력 false는 동기화를 끄는 bool prvalue이며 소유권 의미가 없다.
    // (4) 이전 동기화 상태 bool을 반환하지만 사용하지 않는다.
    // (5) C/C++ 표준 스트림 사이 자동 순서 동기화가 이후 해제된다.
    // (6) 표준의 별도 복잡도·noexcept 보장은 없고 관찰자 수명/무효화는 바뀌지 않으며 동시 설정하지 않는다.
    std::ios::sync_with_stdio(false);
    /*
    [std::basic_ios<char>::tie 첫 호출 계약]
    (1) 수신 객체는 유효한 `std::istream` 객체 std::cin이고 기본적으로 출력 스트림에
        tie되어 있을 수 있다.
    (2) 선택 오버로드는 `std::ostream* tie(std::ostream* tied_stream)`이다.
    (3) nullptr은 null pointer literal인 prvalue이며 새 연결 대상이 없음을 뜻한다. 소유권을
        주고받지 않고 포인터 관찰 관계만 바꾼다.
    (4) 반환형은 이전 tie 대상 `std::ostream*`이고 여기서는 사용하지 않는다.
    (5) 이후 cin 입력 전에 cout이 자동 flush되지 않는다. cout의 수명은 변하지 않는다.
    (6) 순환 tie를 만들면 안 되며 여기서는 nullptr라 안전하다. 표준은 별도 복잡도와
        noexcept 보장을 명시하지 않는다. 사용자 참조를 무효화하지 않으며 전역 스트림
        설정의 동시 변경은 동기화하지 않는다.
    */
    // [첫 호출 계약: std::cin.tie]
    // (1) 수신 std::cin은 초기화된 정확한 타입 std::istream의 유효한 lvalue다.
    // (2) 선택 오버로드는 std::ostream* basic_ios<char>::tie(std::ostream*)다.
    // (3) 입력 nullptr는 새 tie 대상이 없다는 prvalue이고 스트림 소유권을 옮기지 않는다.
    // (4) 이전 tie 대상 포인터를 반환하지만 사용하지 않는다.
    // (5) 이후 입력 전 자동 flush 연결만 제거되고 두 스트림 수명은 유지된다.
    // (6) 표준의 별도 복잡도·noexcept 보장은 없고 무효화/할당은 없으며 동시에 설정하지 않는다.
    std::cin.tie(nullptr);

    // `{}`는 자동 저장 기간 int 두 개를 0으로 값 초기화한다.
    Index size{};
    Index query_count{};
    /*
    [std::istream 정수 추출 operator>> 첫 호출 계약]
    (1) 수신 객체는 오류 비트가 없는 유효한 `std::istream` std::cin이다.
    (2) 두 연쇄 호출 모두 `std::istream& operator>>(int&)` 오버로드가 선택된다.
    (3) size와 query_count는 수정 가능한 Index(int) lvalue이며 소유권 이전은 없다.
        공백을 건너뛴 뒤 십진 정수를 읽는다.
    (4) 첫 호출이 반환한 같은 std::istream lvalue 참조는 두 번째 추출 연결에 쓰고,
        마지막 호출이 반환한 참조는 버린다.
    (5) 성공하면 두 변수에 입력값이 저장되고 입력 위치가 전진한다. 실패하면
        failbit/eofbit/badbit가 설정될 수 있으며 해당 값 사용은 이 문제의 유효 입력 전제에 따른다.
    (6) 값이 int 표현 범위 안이어야 하며 공식 제약이 보장한다. 표준은 이 형식 입력에
        별도 점근 복잡도를 명시하지 않고 로케일·streambuf 구현에 따라 작업량이 달라진다.
        exceptions mask가 켜졌다면 failure를 던질 수 있으나 기본 설정에서는 상태 비트로
        보고한다. 동시 입력 동기화는 제공하지 않는다.
    */
    // [첫 호출 계약: std::istream 정수 추출 operator>>]
    // (1) 수신 std::cin은 유효한 std::istream lvalue이고 두 목적지는 수정 가능한 int lvalue다.
    // (2) 선택 오버로드는 basic_istream<char>& operator>>(int&)다.
    // (3) 입력 인자는 공식 범위의 비소유 lvalue 참조이며 소유권 이동이 없다.
    // (4) 첫 반환 stream&는 연쇄 호출에 쓰고 마지막 반환 참조는 버린다.
    // (5) 성공하면 값/입력 위치가 바뀌고 실패하면 상태 비트가 설정된다.
    // (6) 표준의 별도 점근 복잡도는 없으며 범위 오류·exceptions mask 예외가 가능하고 동시 입력은 동기화하지 않는다.
    std::cin >> size >> query_count;

    /*
    [초기값용 std::vector<Value> 채움 생성자 호출 계약]
    (1) values는 아직 수명이 시작되기 전인 `std::vector<Value>` 지역 객체다.
    (2) 앞서 설명한 `vector(size_type, const Value&)` 오버로드를 Value로 인스턴스화한다.
    (3) size를 명시 변환한 count는 [1,200000], Value{0}은 복사할 초기값이며 소유권은 없다.
    (4) 반환값 없이 values 자체를 생성한다.
    (5) 성공하면 size개의 0을 소유하고 그 수명은 main 블록 끝까지다.
    (6) O(n) 시간/공간, 할당 실패 예외 가능, 재할당 전 원소 참조 유효, 자체 동기화 없음이다.
    */
    // [첫 생성 계약: std::vector<Value> 채움 생성자]
    // (1) values는 아직 수명이 시작되지 않은 정확한 타입 std::vector<Value> 지역 객체다.
    // (2) 선택 오버로드는 vector(size_type, const Value&)와 기본 allocator다.
    // (3) 입력은 범위 안 count와 Value{0} const 참조이며 소유권 이동은 없다.
    // (4) 반환값 없이 values 자체를 생성한다.
    // (5) 성공하면 count개의 0을 소유하며 관찰자는 아직 없다.
    // (6) O(n) 시간/공간, 할당 가능, length_error/bad_alloc 가능, 실패 시 부분 원소를 정리하고 내부 동기화는 없다.
    std::vector<Value> values(static_cast<std::size_t>(size), Value{0});
    // for는 초기화-조건 비교-증가를 반복하며 모든 초기 원소를 정확히 한 번 읽는다.
    for (Index index{}; index < size; ++index) {
        /*
        [std::istream Value 추출 operator>> 첫 호출 계약]
        (1) 수신 객체는 앞선 입력이 성공한 유효한 std::istream std::cin이다.
        (2) Value가 long long이므로 `std::istream& operator>>(long long&)`가 선택되고,
            vector 접근은 앞서 계약한 비검사 `Value& operator[](size_type)`다.
        (3) 인자는 values[index]라는 수정 가능한 Value lvalue이며 소유권 이전은 없다.
            index는 [0,size)라 접근이 유효하고 입력은 [1,1000000]이다.
        (4) 추출은 같은 stream lvalue 참조를 반환하지만 이 문장에서는 버린다.
        (5) 성공하면 해당 원소와 입력 위치가 바뀌고 vector 크기/용량은 그대로다.
        (6) 숫자가 Value 범위여야 하며 제약이 보장한다. 표준은 별도 점근 복잡도를
            명시하지 않으며 vector 재할당/참조 무효화는 없다. 실패 상태, exceptions mask,
            로케일·streambuf 및 스레드 조건은 int 추출과 같다.
        */
        // [첫 호출 계약: std::istream Value 추출 operator>>]
        // (1) 수신 std::cin과 values는 유효하고 목적지는 범위 안 Value lvalue다.
        // (2) 선택 오버로드는 basic_istream<char>& operator>>(long long&)와 vector<Value>::operator[]이다.
        // (3) 입력은 비소유 목적지 참조와 범위 안 size_type 인자이며 소유권 이동이 없다.
        // (4) 추출은 stream&를 반환하지만 버리고 인덱싱은 Value&를 반환해 목적지로 쓴다.
        // (5) 성공하면 원소/입력 위치만 바뀌며 vector 크기·용량은 유지된다.
        // (6) 별도 점근 복잡도 보장은 없고 실패 상태/예외가 가능하며 인덱스 범위 밖은 UB, 할당·무효화는 없다.
        std::cin >> values[static_cast<std::size_t>(index)];
    }

    // 직접 리스트 초기화는 explicit 생성자를 허용한다. values는 const 참조로 빌드 중 읽힌다.
    // 생성 결과는 이름 있는 lvalue tree가 되며 복사/이동이나 RVO에 의존하지 않는다.
    RangeAssignAddSumTree tree{values};

    for (Index query_number{}; query_number < query_count; ++query_number) {
        Index type{};
        Index left{};
        Index right{};
        // 세 int lvalue에 명령 종류와 1-based 양끝을 읽는다. 추출 계약은 위와 같다.
        std::cin >> type >> left >> right;
        // 내부 구현은 0-based 닫힌 구간을 사용하므로 양끝을 한 번만 변환한다.
        --left;
        --right;

        // type 비교는 값 로드와 조건 분기로 구현될 수 있으며 세 명령 중 하나만 실행한다.
        if (type == 1) {
            Value delta{};
            std::cin >> delta;
            tree.range_add(left, right, delta);
        } else if (type == 2) {
            Value value{};
            std::cin >> value;
            tree.range_assign(left, right, value);
        } else {
            /*
            [std::ostream 삽입 operator<< 첫 호출 계약]
            (1) 수신 객체는 유효한 `std::ostream` std::cout이고 이전 출력 실패가 없다고 전제한다.
            (2) 첫 호출은 멤버 `basic_ostream<char>& basic_ostream<char>::operator<<(long long)`,
                이어진 호출은 Traits가 char_traits<char>인 비멤버 문자 삽입 템플릿
                `basic_ostream<char, Traits>& operator<<(basic_ostream<char, Traits>&, char)`다.
            (3) 첫 인자는 range_sum이 돌려준 Value prvalue로 소유권이 없고, 두 번째는
                줄바꿈 문자 '\n' char prvalue다. '\n'은 강제 flush를 요구하지 않는다.
            (4) 각 호출은 같은 ostream lvalue 참조를 반환하며 두 번째 삽입 연결에 사용된다.
            (5) 성공하면 문자가 출력 버퍼에 기록되고 tree와 인자 값은 변하지 않는다.
                실패하면 failbit/badbit가 설정될 수 있다.
            (6) 값은 Value 범위다. 표준은 별도 점근 복잡도를 명시하지 않고 로케일과
                streambuf 구현에 따라 작업량·내부 할당이 달라질 수 있다. 출력 실패와
                exceptions mask 설정에 따라 예외가 날 수 있다. 사용자 참조 무효화는 없으며
                같은 stream에 대한 동시 비동기 출력 순서는 보장하지 않는다.
            */
            // [첫 호출 계약: std::ostream 정수·문자 삽입 operator<<]
            // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 합과 '\n'은 유효한 값이다.
            // (2) 선택 오버로드는 멤버 operator<<(long long)와 비멤버 operator<<(ostream&, char)다.
            // (3) 두 피연산자는 Value/char prvalue이며 소유권 이동이 없다.
            // (4) 첫 반환 stream&는 연쇄 호출에 쓰고 마지막 반환 참조는 버린다.
            // (5) 출력 버퍼만 바뀌며 tree와 인자 수명은 유지되고 실패 시 상태 비트가 설정된다.
            // (6) 별도 점근 복잡도 보장은 없고 streambuf 할당/오류·설정된 예외가 가능하며 관찰자 무효화는 없다.
            std::cout << tree.range_sum(left, right) << '\n';
        }
    }

    // int prvalue 0을 반환해 모든 자동 객체를 역순으로 소멸시킨 뒤 정상 종료한다.
    return 0;
}
