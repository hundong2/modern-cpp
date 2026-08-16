/*
문제 ID/제목: UVa 11402 - Ahoy, Pirates!
출처: https://onlinejudge.org/index.php?Itemid=8&option=com_onlinejudge&page=show_problem&problem=2397
공식 문제 PDF: https://onlinejudge.org/contests/193-a6da26e8/11402.pdf

문제 요약:
0과 1로 표현된 해적 배열이 있다. 입력의 여러 문자열 블록을 지정 횟수만큼 반복해 전체 배열을 만든다.
이후 닫힌 구간 [a, b]에 대해 전부 1로 바꾸기(F), 전부 0으로 바꾸기(E), 0과 1 뒤집기(I),
구간 안의 1 개수 묻기(S)를 온라인으로 처리한다. 원문 이야기를 복제하지 않고 연산 의미만 요약했다.

입력:
첫 줄은 테스트 케이스 수다. 각 케이스에서 블록 수 M 뒤에 M개의 (반복 횟수, 0/1 문자열) 쌍이 온다.
이어 명령 수 Q와 Q개의 `명령 a b`가 주어진다. a와 b는 0부터 시작하고 양끝을 모두 포함한다.

출력:
테스트 케이스마다 `Case k:`를 출력한다. S 명령마다 케이스 안의 질의 번호를 1부터 매겨
`Q번호: 1의 개수` 형식으로 출력한다.

제약:
완성 배열 길이 N은 1 이상 1,024,000 이하, M은 100 이하, 각 반복 횟수는 200 이하,
각 블록 문자열 길이는 50 이하, Q는 1 이상 1,000 이하이다. 따라서 매 명령마다 구간을 직접 훑는
O(NQ) 방식은 위험하고, 지연 전파 세그먼트 트리로 빌드 O(N), 각 명령 O(log N), 공간 O(N)에 처리한다.

설명용 예제:
입력
1
1
1
00101
7
S 0 4
F 1 3
S 0 4
I 0 2
S 0 4
E 3 4
S 0 4

출력
Case 1:
Q1: 2
Q2: 4
Q3: 3
Q4: 1
*/

// <cstddef>는 컨테이너 크기와 인덱스 변환에 쓰는 std::size_t를 제공한다.
#include <cstddef>
// <iostream>은 온라인 저지의 표준 입력 std::cin과 표준 출력 std::cout을 제공한다.
#include <iostream>
// <string>은 입력 블록과 완성된 0/1 배열을 소유하는 std::string을 제공한다.
#include <string>
// <vector>는 세그먼트 트리 값과 지연 연산을 연속 메모리에 저장하는 std::vector를 제공한다.
#include <vector>

// 알고리즘 문서: ../algorithm/lazy-segment-tree.md
// enum class는 지연 연산을 서로 섞이지 않는 이름 있는 값으로 표현하고 암시적 정수 변환을 막는다.
enum class LazyOp : unsigned char {
    none,     // 자식에게 전달할 미처리 연산이 없는 항등 상태다.
    set_zero, // 담당 구간의 모든 값을 0으로 대입한다.
    set_one,  // 담당 구간의 모든 값을 1로 대입한다.
    invert,   // 담당 구간의 0과 1을 서로 뒤집는다.
};

// 지연 전파 세그먼트 트리는 구간의 1 개수와 아직 자식에 전달하지 않은 연산을 함께 보관한다.
class LazySegmentTree {
// public 영역은 생성, 구간 갱신, 구간 질의라는 자료구조 계약만 노출한다.
public:
    // 생성자는 반환형이 없고 explicit은 string 하나가 트리로 뜻밖에 암시 변환되는 것을 막는다.
    explicit LazySegmentTree(const std::string& bits)
        // size_는 int 인덱스 계산에 사용할 배열 길이를 값으로 복사한다.
        : size_{static_cast<int>(bits.size())},
          // 노드 배열 4N은 재귀 세그먼트 트리가 필요한 공간의 안전한 상한이다.
          ones_(bits.size() * 4U, 0),
          // 모든 노드는 처음에 미전파 연산이 없다는 none 상태다.
          lazy_(bits.size() * 4U, LazyOp::none) {
        // 문제 제약은 N>=1이지만 방어적으로 빈 입력에서는 재귀 빌드를 시작하지 않는다.
        if (size_ > 0) {
            // 루트 노드 1은 전체 닫힌 구간 [0, N-1]을 나타낸다.
            build(1, 0, size_ - 1, bits);
        }
    }

    // 구간 변경 함수는 호출자가 정한 닫힌 구간과 연산 값을 받는다.
    void update(int query_left, int query_right, LazyOp operation) {
        // 재귀 함수가 루트부터 필요한 O(log N)개의 표준 구간만 방문한다.
        range_update(1, 0, size_ - 1, query_left, query_right, operation);
    }

    // [[nodiscard]]는 질의 결과를 버리는 실수를 컴파일러가 경고할 수 있게 한다.
    [[nodiscard]] int query(int query_left, int query_right) {
        // 1의 개수는 최대 N이므로 문제 제한에서 int 범위에 안전하다.
        return range_query(1, 0, size_ - 1, query_left, query_right);
    }

// private 영역은 노드 인덱스와 지연 합성 불변식을 외부가 직접 깨뜨리지 못하게 한다.
private:
    // 알고리즘 문서: ../algorithm/lazy-segment-tree.md
    // existing 뒤에 incoming을 적용한 하나의 대표 연산을 계산한다.
    [[nodiscard]] static LazyOp compose(LazyOp existing, LazyOp incoming) {
        // 새 대입은 이전 대입이나 반전을 모두 덮어쓰므로 그대로 반환한다.
        if (incoming == LazyOp::set_zero || incoming == LazyOp::set_one) {
            return incoming; // 함수의 LazyOp 반환값이 새 대표 연산이다.
        }
        // none은 상태를 바꾸지 않는 항등 연산이다.
        if (incoming == LazyOp::none) {
            return existing; // 항등 연산 뒤에도 기존 합성 상태가 유지된다.
        }

        // 여기서 incoming은 invert이며 switch가 기존 네 상태의 합성 결과를 완전하게 나눈다.
        switch (existing) {
        case LazyOp::none:
            return LazyOp::invert; // 아무 작업 뒤 반전은 반전 하나다.
        case LazyOp::set_zero:
            return LazyOp::set_one; // 0 대입 뒤 반전하면 모두 1이다.
        case LazyOp::set_one:
            return LazyOp::set_zero; // 1 대입 뒤 반전하면 모두 0이다.
        case LazyOp::invert:
            // 두 번 반전은 원래 값이므로 지연 연산이 사라진다.
            return LazyOp::none;
        }

        // 모든 enum 값을 위에서 반환하지만 컴파일러의 제어 흐름 요구를 만족하는 방어적 반환이다.
        return LazyOp::none;
    }

    // 초기 문자열의 1 개수를 재귀적으로 합쳐 트리를 O(N)에 만든다.
    void build(int node, int left, int right, const std::string& bits) {
        // 구간 길이가 1이면 실제 배열 원소 하나에 대응하는 잎 노드다.
        if (left == right) {
            // == 비교 결과 bool을 조건 연산자가 1 또는 0의 int로 바꾼다.
            ones_[node] = bits[static_cast<std::size_t>(left)] == '1' ? 1 : 0;
            return; // 잎은 더 나눌 자식이 없으므로 빌드를 끝낸다.
        }

        // 중간점은 정수 나눗셈으로 내림되며 이 식은 left+right 오버플로를 피한다.
        // 부모 구간을 겹치지 않는 두 자식 구간으로 나누는 중간 인덱스다.
        const int middle{left + (right - left) / 2};
        // node*2와 node*2+1은 각각 왼쪽과 오른쪽 자식 인덱스다.
        build(node * 2, left, middle, bits);
        build(node * 2 + 1, middle + 1, right, bits); // 오른쪽 자식 구간을 만든다.
        // 부모의 불변식은 담당 구간의 1 개수가 두 자식 합과 같다는 것이다.
        ones_[node] = ones_[node * 2] + ones_[node * 2 + 1];
    }

    // 한 노드 전체에 연산을 즉시 반영하고 자식용 지연 연산을 합성한다.
    void apply(int node, int left, int right, LazyOp operation) {
        // none은 항등 연산이라 값과 지연 상태를 바꿀 필요가 없다.
        if (operation == LazyOp::none) {
            return; // void 함수이므로 반환값 없이 조기 종료한다.
        }

        // 닫힌 구간 [left,right]의 원소 수는 right-left+1이다.
        const int length{right - left + 1};
        // 대입은 구간 합을 즉시 결정하고 반전은 1의 수를 전체 길이에서 뺀다.
        if (operation == LazyOp::set_zero) {
            ones_[node] = 0; // 모든 값이 0이면 1의 개수도 0이다.
        } else if (operation == LazyOp::set_one) {
            ones_[node] = length; // 모든 값이 1이면 1의 개수는 구간 길이다.
        } else {
            ones_[node] = length - ones_[node]; // 기존 0의 개수가 반전 뒤 1의 개수다.
        }

        // 기존 지연 연산 다음에 새 연산을 적용하는 순서로 합성해야 교환 불가능한 대입이 정확하다.
        lazy_[node] = compose(lazy_[node], operation);
    }

    // 부모에 미뤄 둔 연산을 두 자식에 전달한 뒤 부모의 지연 표시를 비운다.
    void push(int node, int left, int right) {
        // 잎에는 자식이 없고 none이면 전달할 일이 없으므로 조기 반환한다.
        if (lazy_[node] == LazyOp::none || left == right) {
            return; // 자식에 적용할 작업이 없는 경로다.
        }

        // 부모 구간을 두 자식에 적용할 때 쓸 안전한 중간 인덱스를 계산한다.
        const int middle{left + (right - left) / 2};
        // 같은 연산을 자식 구간 길이에 맞춰 적용하며 자식의 기존 지연 연산과도 합성한다.
        apply(node * 2, left, middle, lazy_[node]);
        apply(node * 2 + 1, middle + 1, right, lazy_[node]); // 오른쪽 자식에도 전달한다.
        // 자식이 이제 의미를 이어받았으므로 부모는 미전파 연산이 없다.
        lazy_[node] = LazyOp::none;
    }

    // 알고리즘 문서: ../algorithm/lazy-segment-tree.md
    // 목표 구간을 완전히 덮는 표준 구간에만 연산을 저장해 구간 갱신을 O(log N)에 처리한다.
    void range_update(int node, int left, int right,
                      int query_left, int query_right, LazyOp operation) {
        // 두 닫힌 구간이 겹치지 않으면 현재 노드는 갱신에 기여하지 않는다.
        if (query_right < left || right < query_left) {
            return; // 현재 하위 트리 전체를 건너뛴다.
        }
        // 현재 구간이 목표 안에 완전히 들어가면 자식까지 내려가지 않고 지연 적용한다.
        if (query_left <= left && right <= query_right) {
            apply(node, left, right, operation);
            return; // 완전 포함이므로 더 깊은 재귀는 지연한다.
        }

        // 일부만 겹칠 때는 자식 값이 최신이어야 하므로 먼저 부모의 미룬 연산을 전달한다.
        push(node, left, right);
        // 현재 구간을 왼쪽 [left,middle]과 오른쪽 [middle+1,right]로 나눈다.
        const int middle{left + (right - left) / 2};
        // 왼쪽과 오른쪽 표준 구간을 재귀 갱신한다. 겹치지 않는 쪽은 즉시 반환한다.
        range_update(node * 2, left, middle, query_left, query_right, operation);
        range_update(node * 2 + 1, middle + 1, right, query_left, query_right, operation); // 오른쪽이다.
        // 자식이 바뀐 뒤 부모 합을 다시 계산해 구간 합 불변식을 복구한다.
        ones_[node] = ones_[node * 2] + ones_[node * 2 + 1];
    }

    // 목표 구간의 1 개수를 표준 구간 합으로 분해해 O(log N)에 반환한다.
    [[nodiscard]] int range_query(int node, int left, int right,
                                  int query_left, int query_right) {
        // 겹치지 않는 구간의 합 항등원은 0이다.
        if (query_right < left || right < query_left) {
            return 0; // 다른 구간 합에 더해도 값을 바꾸지 않는다.
        }
        // 완전히 포함된 노드의 ones_는 지연 상태가 있어도 현재 구간 합 자체는 항상 최신이다.
        if (query_left <= left && right <= query_right) {
            return ones_[node]; // 최신인 현재 노드 요약을 즉시 사용한다.
        }

        // 자식 일부를 읽기 전에 부모 연산을 내려야 자식 합도 실제 배열 상태와 일치한다.
        push(node, left, right);
        // 양쪽 재귀 질의의 경계를 정하는 중간 인덱스다.
        const int middle{left + (right - left) / 2};
        // 두 자식에서 얻은 1 개수를 + 연산자로 합친다.
        return range_query(node * 2, left, middle, query_left, query_right)
             + range_query(node * 2 + 1, middle + 1, right, query_left, query_right); // 오른쪽 합을 더한다.
    }

    // int는 N과 재귀 인덱스 범위에 충분하고 size_는 생성 뒤 바뀌지 않는다.
    int size_{};
    // vector<int>는 각 노드 담당 구간의 1 개수를 소유한다.
    std::vector<int> ones_{};
    // vector<LazyOp>는 각 노드에 아직 자식으로 내리지 않은 합성 연산을 소유한다.
    std::vector<LazyOp> lazy_{};
};

// 입력 문자 명령을 강한 enum 값으로 변환해 파싱과 자료구조 구현의 책임을 나눈다.
[[nodiscard]] LazyOp operation_from(char command) {
    // F는 전체를 1, E는 전체를 0, 그 밖의 변경 명령 I는 반전을 뜻한다.
    if (command == 'F') {
        return LazyOp::set_one; // F 문자에 대응하는 강한 enum 값이다.
    }
    // 두 번째 조건 분기는 E 명령인지 비교한다.
    if (command == 'E') {
        return LazyOp::set_zero; // E 문자에 대응하는 강한 enum 값이다.
    }
    return LazyOp::invert; // 호출 경로의 나머지 변경 문자는 I다.
}

// main은 입력 조립, 명령 분기, 세그먼트 트리 호출, 출력 형식을 담당한다.
int main() {
    // C와 C++ 스트림 동기화를 끄면 많은 문자 입력을 더 빠르게 처리할 수 있다.
    std::ios::sync_with_stdio(false);
    // nullptr는 아무 객체도 가리키지 않는 포인터 값이며 자동 출력 flush 연결을 끊는다.
    std::cin.tie(nullptr);

    // 기본 정수 변수는 {}로 0 값 초기화한다.
    int test_count{};
    // 입력이 없다면 실행할 케이스가 없으므로 정상 종료한다.
    if (!(std::cin >> test_count)) {
        return 0;
    }

    // for 초기화식, <= 조건, 전위 ++ 갱신으로 케이스 번호 1..T를 정확히 순회한다.
    for (int case_number{1}; case_number <= test_count; ++case_number) {
        // block_count는 압축 입력을 이루는 (반복 횟수, 패턴) 쌍의 개수다.
        int block_count{};
        // >> 연산자는 표준 입력의 정수를 block_count lvalue에 저장한다.
        std::cin >> block_count;
        // string은 append된 0/1 문자를 소유하고 빈 문자열에서 시작한다.
        std::string pirates{};

        // 각 블록은 반복 횟수와 패턴 문자열 한 쌍이다.
        for (int block{}; block < block_count; ++block) {
            // repeat_count는 이번 pattern을 이어 붙일 횟수다.
            int repeat_count{};
            // pattern은 이번 블록의 0/1 문자를 직접 소유한다.
            std::string pattern{};
            // 두 번의 >> 추출이 반복 횟수와 공백 없는 문자열을 차례로 읽는다.
            std::cin >> repeat_count >> pattern;
            // 같은 패턴을 repeat_count번 append해 문제의 압축 입력을 실제 배열로 푼다.
            for (int repeat{}; repeat < repeat_count; ++repeat) {
                pirates.append(pattern);
            }
        }

        // pirates는 이름 있는 lvalue라 const string& 생성자 매개변수에 복사 없이 바인딩된다.
        LazySegmentTree tree{pirates};
        // command_count는 이 케이스에서 온라인으로 처리할 명령 수다.
        int command_count{};
        // 표준 입력에서 명령 수를 읽는다.
        std::cin >> command_count;
        // 질의 번호는 S 명령을 만날 때만 전위 증가한다.
        int query_number{};
        // << 연산자 연쇄가 케이스 제목과 개행을 표준 출력에 기록한다.
        std::cout << "Case " << case_number << ":\n";

        // 모든 온라인 명령을 입력 순서대로 처리해야 이후 질의가 앞선 변경을 관찰한다.
        for (int index{}; index < command_count; ++index) {
            // char는 F, E, I, S 중 문자 하나를 저장하는 기본 문자 타입이다.
            char command{};
            // left와 right는 양끝을 포함하는 0 기반 구간 경계다.
            int left{};
            int right{}; // 같은 int 타입으로 오른쪽 경계를 0 초기화한다.
            // 한 명령의 문자와 두 정수 경계를 각 lvalue 변수에 저장한다.
            std::cin >> command >> left >> right;

            // S는 읽기 질의이고 나머지 세 문자는 구간 변경이다.
            if (command == 'S') {
                // ++query_number는 출력 전에 번호를 1 증가시키는 전위 증가 연산자다.
                std::cout << 'Q' << ++query_number << ": " << tree.query(left, right) << '\n';
            } else {
                // 함수 호출로 문자 명령을 enum으로 바꾸고 지연 구간 갱신을 수행한다.
                tree.update(left, right, operation_from(command));
            }
        }
    }

    // 재귀 호출은 비교·조건 분기·메모리 로드/저장을 수행하지만 구체 명령은 CPU, ABI,
    // 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령 하나로 단정하지 않는다.
    return 0;
}
