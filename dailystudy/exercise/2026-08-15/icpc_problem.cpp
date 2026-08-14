/*
문제: BOJ 2042 - 구간 합 구하기
출처: https://www.acmicpc.net/problem/2042
요약: N개의 수가 있는 배열에서 한 위치의 값을 새 값으로 바꾸는 명령과 두 위치 사이의 합을 묻는 명령을 처리한다.
입력: 첫 줄에 수의 개수 N, 변경 횟수 M, 합 질의 횟수 K가 주어진다. 다음 N줄은 초기값이다.
      이어지는 M+K줄은 `1 b c`이면 b번째 값을 c로 바꾸고, `2 b c`이면 b번째부터 c번째까지의 합을 묻는다.
출력: 종류 2인 각 명령의 구간 합을 입력 순서대로 한 줄에 하나씩 출력한다.
제약: 1<=N<=1,000,000, 1<=M,K<=10,000이며 입력 값과 합을 위해 64비트 부호 있는 정수를 사용한다.
예제: [1,2,3,4,5]에서 3번째를 6으로 바꾸면 2~5의 합은 17이고, 5번째를 2로 바꾸면 3~5의 합은 12이다.
*/

// <cstddef>는 컨테이너 크기와 인덱스 타입 std::size_t를 제공한다.
#include <cstddef>
// <iostream>은 빠른 표준 입력 std::cin과 출력 std::cout을 제공한다.
#include <iostream>
// <vector>는 동적 연속 배열 std::vector를 제공한다.
#include <vector>

// 구현 참고 문서: ../algorithm/fenwick-tree.md
// class는 기본 private이며 펜윅 트리 배열과 그 불변식을 외부에서 직접 바꾸지 못하게 한다.
class FenwickTree final {
public:
    // 생성자는 반환형이 없다. explicit은 크기 정수가 트리 객체로 암시 변환되는 것을 막는다.
    explicit FenwickTree(std::size_t size)
        // 1 기반 인덱스를 위해 size+1개의 long long을 값 0으로 초기화한다.
        : tree_(size + 1, 0) {}

    // 한 위치에 delta를 더하고 그 위치를 포함하는 상위 구간 합들을 갱신한다.
    void add(std::size_t index, long long delta) {
        while (index < tree_.size()) { // index가 저장 범위 안인 동안 최대 O(log N)번 반복한다.
            tree_[index] += delta; // += 연산자로 현재 노드가 나타내는 구간 합을 갱신한다.
            index += lowbit(index); // 다음으로 이 위치를 포함하는 더 큰 구간 노드로 이동한다.
        }
    }

    // 1번부터 index번까지의 누적 합을 읽기만 하므로 const 멤버 함수다.
    [[nodiscard]] long long prefix_sum(std::size_t index) const {
        long long result{}; // 기본 타입을 중괄호로 0 초기화한다.
        while (index > 0) { // 0은 1 기반 펜윅 트리 순회의 종료 표식이다.
            result += tree_[index]; // 서로 겹치지 않는 담당 구간의 합을 누적한다.
            index -= lowbit(index); // 최하위 1비트를 지워 아직 더하지 않은 왼쪽 구간으로 이동한다.
        }
        return result; // 계산된 64비트 정수 값을 복사해 반환한다.
    }

    // 닫힌 구간 [left, right]는 두 누적 합의 차이로 구한다.
    [[nodiscard]] long long range_sum(std::size_t left, std::size_t right) const {
        return prefix_sum(right) - prefix_sum(left - 1); // - 연산자가 앞부분 [1,left-1]을 제거한다.
    }

private:
    // lowbit는 index의 이진 표현에서 가장 낮은 1비트 값이며 담당 구간 길이다.
    [[nodiscard]] static std::size_t lowbit(std::size_t index) {
        // 부호 없는 정수의 -index는 모듈러 산술로 정의되며 &가 최하위 1비트만 남긴다.
        return index & -index;
    }

    // vector의 템플릿 인자 long long은 각 노드 구간 합의 기본 타입이다.
    std::vector<long long> tree_{};
};

// int 반환형 main은 온라인 저지가 호출하는 프로그램 진입점이다.
int main() {
    std::ios::sync_with_stdio(false); // C와 C++ 스트림 동기화를 끊어 많은 입력을 빠르게 처리한다.
    std::cin.tie(nullptr); // nullptr 포인터 값으로 입력 스트림과 출력 스트림의 자동 묶음을 해제한다.

    std::size_t number_count{}; // 배열 원소 수를 중괄호로 0 초기화한다.
    int update_count{};         // 값 변경 명령 수는 기본 타입 int다.
    int query_count{};          // 합 질의 명령 수는 기본 타입 int다.
    std::cin >> number_count >> update_count >> query_count; // >>가 입력 토큰을 각 lvalue 변수에 저장한다.

    FenwickTree tree{number_count}; // 직접 초기화로 1 기반 펜윅 트리를 만든다.
    std::vector<long long> values(number_count + 1, 0); // 대입 갱신의 차이를 구할 현재 값 배열이다.
    for (std::size_t index{1}; index <= number_count; ++index) { // 1부터 N까지 정확히 N번 반복한다.
        std::cin >> values[index]; // 초기 64비트 값을 해당 lvalue 원소에 저장한다.
        tree.add(index, values[index]); // 점 추가를 호출해 펜윅 트리 불변식을 만든다.
    }

    const int command_count{update_count + query_count}; // + 연산자로 전체 명령 수를 계산한다.
    for (int command_index{}; command_index < command_count; ++command_index) { // M+K개 명령을 순서대로 처리한다.
        int kind{};             // 1은 대입 갱신, 2는 구간 합 질의다.
        std::size_t second{};   // 두 번째 토큰은 두 명령 모두 1 기반 위치다.
        long long third{};      // 세 번째 토큰은 새 값 또는 오른쪽 위치이므로 64비트로 읽는다.
        std::cin >> kind >> second >> third; // 세 토큰을 변수에 저장한다.

        if (kind == 1) { // == 비교가 대입 갱신 명령인지 판별한다.
            const long long delta{third - values[second]}; // 새 값과 이전 값의 차이를 한 번 계산한다.
            values[second] = third; // 현재 값 배열을 새 값으로 저장해 다음 갱신의 기준을 유지한다.
            tree.add(second, delta); // 합 자료구조에는 차이만 더해 O(log N)에 대입을 반영한다.
        } else { // 문제에서 나머지 유효한 명령 종류는 2인 구간 합 질의다.
            const auto right{static_cast<std::size_t>(third)}; // 명시적 형 변환으로 오른쪽 위치 타입을 맞춘다.
            std::cout << tree.range_sum(second, right) << '\n'; // 함수 호출 결과와 줄바꿈을 출력한다.
        }
    }

    // 반복은 메모리 로드·저장·비교·분기를 만들 수 있으나 실제 명령은 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return 0; // 모든 명령을 정상 처리했음을 운영체제에 알린다.
}
