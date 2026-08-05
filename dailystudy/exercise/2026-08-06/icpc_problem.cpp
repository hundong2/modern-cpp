/*
문제 ID/제목: BOJ 2252 - 줄 세우기
출처: https://www.acmicpc.net/problem/2252

문제 요약:
N명의 학생이 있고, M개의 키 비교가 "학생 A가 학생 B보다 앞에 서야 한다"는 방향 관계로 주어진다.
모든 비교를 만족하도록 학생 번호를 한 줄로 출력한다. 답이 여러 개라면 그중 하나면 충분하다.

입력:
첫 줄에 학생 수 N과 비교 수 M이 주어진다. 다음 M줄에는 A B가 주어지며 A가 B 앞에 서야 한다.

출력:
주어진 모든 선후 관계를 만족하는 학생 번호 N개를 공백으로 구분해 출력한다.

제약:
1 <= N <= 32,000, 1 <= M <= 100,000이며 입력은 순서를 만들 수 있도록 주어진다.

예제:
입력
3 2
1 3
2 3
출력 예
1 2 3
(1과 2의 상대 순서는 정해지지 않았으므로 2 1 3도 올바르다.)
*/

// <iostream>은 빠른 표준 입력 std::cin과 출력 std::cout을 제공한다.
#include <iostream>
// <queue>는 먼저 들어온 정점을 먼저 꺼내는 std::queue를 제공한다.
#include <queue>
// <vector>는 인접 리스트, 진입 차수, 결과를 저장하는 동적 배열을 제공한다.
#include <vector>

int main() { // 제출 프로그램 진입점은 종료 상태를 int로 반환한다.
    std::ios::sync_with_stdio(false); // C와 C++ 스트림 동기화를 끊어 많은 입력의 비용을 줄인다.
    std::cin.tie(nullptr); // nullptr는 아무 객체도 가리키지 않으며 입력 전 자동 출력 flush 연결을 해제한다.

    int student_count{}; // 학생 수를 중괄호로 0 초기화한다.
    int comparison_count{}; // 비교 관계 수를 0 초기화한다.
    std::cin >> student_count >> comparison_count; // >> 연산자가 공백으로 구분된 정수를 읽어 lvalue에 저장한다.

    // 사용 알고리즘 문서: ../algorithm/topological-sort.md
    // 인접 리스트는 모든 정점 쌍을 저장하는 행렬보다 희소한 M개 간선만 저장해 O(N+M) 공간을 쓴다.
    std::vector<std::vector<int>> graph(static_cast<std::size_t>(student_count + 1));
    // indegree[v]는 아직 제거되지 않은 v의 선행 학생 수라는 불변식을 유지한다.
    std::vector<int> indegree(static_cast<std::size_t>(student_count + 1), 0);

    for (int index{}; index < comparison_count; ++index) { // M개 관계를 정확히 한 번 읽는다.
        int before{}; // 방향 간선의 시작 학생 번호다.
        int after{}; // 방향 간선의 도착 학생 번호다.
        std::cin >> before >> after;
        graph[static_cast<std::size_t>(before)].push_back(after); // before -> after 간선을 인접 리스트 끝에 추가한다.
        ++indegree[static_cast<std::size_t>(after)]; // 전위 ++가 after의 선행 관계 수를 1 늘린다.
    }

    std::queue<int> ready{}; // 현재 진입 차수가 0인 학생만 담는 FIFO 컨테이너다.
    for (int student{1}; student <= student_count; ++student) { // 모든 학생의 최초 준비 상태를 검사한다.
        if (indegree[static_cast<std::size_t>(student)] == 0) { // == 비교가 선행 학생이 없음을 확인한다.
            ready.push(student); // 지금 배치해도 제약을 깨지 않는 학생을 큐에 넣는다.
        }
    }

    std::vector<int> order{}; // 완성되는 위상 순서를 소유한다.
    order.reserve(static_cast<std::size_t>(student_count)); // N칸을 미리 확보해 재할당과 기존 원소 이동을 줄인다.
    while (!ready.empty()) { // ! 연산자는 큐가 비지 않았을 때 반복하게 한다.
        const int current{ready.front()}; // front가 반환한 lvalue를 읽어 const 값으로 복사한다.
        ready.pop(); // 맨 앞 원소를 제거하며 current 값은 독립 복사본이라 계속 유효하다.
        order.push_back(current); // 진입 차수 0 정점을 결과 뒤에 확정한다.

        for (const int next : graph[static_cast<std::size_t>(current)]) { // current에서 나가는 각 간선을 한 번 순회한다.
            --indegree[static_cast<std::size_t>(next)]; // current를 제거했으므로 next의 남은 선행 수를 1 줄인다.
            if (indegree[static_cast<std::size_t>(next)] == 0) { // 마지막 선행 정점까지 처리됐는지 조건 분기한다.
                ready.push(next); // 이제 안전하게 배치할 수 있는 정점을 준비 큐에 넣는다.
            }
        }
    }

    for (std::size_t index{}; index < order.size(); ++index) { // 확정된 N개 번호를 순서대로 출력한다.
        if (index != 0U) { // != 비교로 첫 원소 앞에는 공백을 쓰지 않는다.
            std::cout << ' ';
        }
        std::cout << order[index]; // []가 연속 저장소의 해당 학생 번호 lvalue를 읽는다.
    }
    std::cout << '\n';
    // 각 정점과 간선을 한 번씩 처리하므로 시간 O(N+M), 그래프·차수·큐·결과에 공간 O(N+M)이다.
    // 실제 로드·저장·비교·분기·함수 호출과 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return 0; // 정상 종료를 운영체제에 알린다.
}
