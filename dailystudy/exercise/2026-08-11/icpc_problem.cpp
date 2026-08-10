/*
문제 ID/제목: BOJ 11286 - 절댓값 힙
출처: https://www.acmicpc.net/problem/11286
문제 요약: 정수를 저장하는 자료구조에 양수와 음수를 삽입한다. 0 명령을 받으면 절댓값이 가장 작은 값을 제거해 출력하며, 절댓값이 같으면 실제 정수가 더 작은 값을 먼저 선택한다. 비어 있으면 0을 출력한다.
입력: 첫 줄에 연산 수 N, 이어지는 N개 줄에 정수 x가 하나씩 주어진다. x가 0이 아니면 삽입하고, 0이면 우선순위가 가장 높은 값을 제거·출력한다.
출력: 각 0 명령마다 제거한 값 또는 빈 자료구조의 0을 한 줄에 하나씩 출력한다.
제약: 1 <= N <= 100,000, 입력 정수의 절댓값은 2^31보다 작다. 0은 삭제 명령으로만 쓰인다.
예제: 1, -1을 넣고 0을 두 번 실행하면 절댓값은 같으므로 -1, 1 순서로 출력한다.
*/

// <cstdlib>는 정수 절댓값 함수 std::abs의 오버로드를 제공한다.
#include <cstdlib>
// <iostream>은 표준 입력 std::cin과 출력 std::cout을 제공한다.
#include <iostream>
// <queue>는 힙 기반 어댑터 std::priority_queue를 제공한다.
#include <queue>
// <vector>는 priority_queue가 내부 저장소로 사용할 연속 컨테이너를 제공한다.
#include <vector>

// 알고리즘 문서: ../algorithm/priority-queue-custom-comparator.md
// struct의 기본 접근은 public이며 비교 규칙만 담는 함수 객체에 알맞다.
struct AbsoluteGreater {
    // const 참조는 int 복사를 피하려는 표현이며, 뒤 const는 비교자 상태를 바꾸지 않음을 뜻한다.
    [[nodiscard]] bool operator()(const int& left, const int& right) const {
        const int left_abs{std::abs(left)};   // 함수 호출로 left의 절댓값을 계산해 const int에 저장한다.
        const int right_abs{std::abs(right)}; // 제약상 INT_MIN이 없어 int 범위에서 안전하다.
        if (left_abs != right_abs) { // != 비교가 참이면 절댓값이 다른 분기로 들어간다.
            return left_abs > right_abs; // priority_queue에서 큰 우선순위 값이 top에 오도록 반대 순서를 반환한다.
        }
        return left > right; // 절댓값 동률 불변식: 실제 정수가 작은 쪽을 먼저 꺼낸다.
    }
};

int main() {
    // 입출력 동기화를 끄고 묶음을 풀어 많은 명령을 빠르게 처리한다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr); // nullptr은 아무 출력 스트림도 가리키지 않는 포인터 값이다.

    int operation_count{}; // 기본 정수 타입 int를 중괄호로 0 초기화한다.
    std::cin >> operation_count; // >> 연산자가 입력 값을 변수에 저장한다.

    // 템플릿 인자는 값 타입, 내부 컨테이너, 비교자이다. heap 불변식에 따라 top()이 원하는 최소값이다.
    std::priority_queue<int, std::vector<int>, AbsoluteGreater> heap{};
    // N번 반복하며 각 push/pop은 O(log N), top/empty는 O(1), 전체 시간은 O(N log N), 공간은 O(N)이다.
    for (int index{0}; index < operation_count; ++index) {
        int command{}; // 매 반복마다 명령 저장용 int 객체가 생성되고 반복 끝에서 수명이 끝난다.
        std::cin >> command;
        if (command != 0) { // 0이 아닌 명령은 삽입이다.
            heap.push(command); // 표준 라이브러리 호출이 원소를 넣고 비교자로 힙 불변식을 복구한다.
            continue; // 현재 반복의 나머지를 건너뛰고 다음 입력으로 간다.
        }
        if (heap.empty()) { // bool 반환 함수로 삭제할 원소가 없는지 검사한다.
            std::cout << 0 << '\n';
            continue;
        }
        std::cout << heap.top() << '\n'; // top()의 const 참조가 가리키는 최우선 원소를 pop 전에 출력한다.
        heap.pop(); // 반환값 없이 최우선 원소를 제거하고 O(log N)에 힙 불변식을 복구한다.
    }
    // 비교·로드·저장·조건 분기·함수 호출의 실제 명령은 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.
    return 0; // 운영체제에 정상 종료 코드 0을 반환한다.
}
