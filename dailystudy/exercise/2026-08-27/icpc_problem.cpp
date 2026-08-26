/*
문제 ID·제목: BOJ 11375 - 열혈강호
출처: Baekjoon Online Judge
출처 URL: https://www.acmicpc.net/problem/11375

문제 요약:
N명의 직원과 M개의 일이 있다. 직원마다 처리할 수 있는 일의 목록이 다르며, 직원 한 명은 최대 한 개의
일만 맡고 일 하나도 최대 한 명에게만 배정할 수 있다. 가능한 직원-일 쌍 가운데 일부를 골라 동시에
배정할 수 있는 일의 최대 개수를 구한다. 저작권이 있는 문제 원문 전체 대신 핵심 조건을 한글로 다시 썼다.

입력:
첫 줄에 직원 수 N과 일 수 M이 주어진다. 이어지는 N개 줄에서 i번째 직원이 할 수 있는 일의 수 K_i와
그 일 번호들이 주어진다. 입력의 일 번호는 1부터 M까지이며 구현에서는 0 기반으로 바꾼다.

출력:
조건을 어기지 않고 배정할 수 있는 일의 최대 개수를 한 줄에 출력한다.

제약:
1 <= N, M <= 1,000이다. 모든 가능한 직원-일 관계 수를 E라 하면 아래 Kuhn 증강 경로 구현은 최악
O(N*E) 시간, O(N+M+E) 공간을 사용한다. N=1,000에서는 재귀 깊이도 직원 수 이하이다.

예제:
입력
5 5
2 1 2
1 1
2 2 3
3 3 4 5
1 1

출력
4
*/

// <iostream>은 표준 입력·출력과 고속 입출력 설정을 선언한다.
#include <iostream>
// <vector>는 인접 목록, 일의 현재 담당자, 방문 회차를 동적으로 소유하는 std::vector를 선언한다.
#include <vector>

// 공용 알고리즘 문서: ../algorithm/bipartite-matching-augmenting-path.md
// worker가 아직 방문하지 않은 일을 따라가며 현재 매칭을 뒤집을 수 있는 증강 경로를 찾는다.
[[nodiscard]] bool try_assign(
    int worker,
    const std::vector<std::vector<int>>& jobs_by_worker,
    std::vector<int>& owner_by_job,
    std::vector<int>& seen_stamp,
    int stamp) {
    // vector::operator[](size_type)은 worker를 부호 없는 인덱스로 변환해 해당 인접 vector의 const&를 O(1)에 반환한다.
    // 문제 입력으로 worker가 [0,N)에 있음을 보장하며 두 vector의 크기·용량·소유권은 바뀌지 않는다.
    // 범위 for는 반환 컨테이너의 begin/end 반복자를 내부에서 얻고 각 int를 값 복사해 가능한 일을 순회한다.
    for (const int job : jobs_by_worker[static_cast<std::size_t>(worker)]) {
        // seen_stamp[job]은 현재 증강 시도에서 이 일을 이미 검사했는지 나타낸다. 같은 일 재방문은 사이클 탐색을 막는다.
        if (seen_stamp[static_cast<std::size_t>(job)] == stamp) {
            continue;
        }
        seen_stamp[static_cast<std::size_t>(job)] = stamp; // 현재 회차를 저장해 이 시도 동안만 방문 상태를 유지한다.

        // owner_by_job[job]==-1이면 빈 일이다. 아니면 현재 담당자를 다른 일로 재귀 이동할 수 있는지 먼저 확인한다.
        // ||는 왼쪽이 true면 오른쪽 재귀 호출을 하지 않는 단락 평가로 빈 일을 즉시 선택한다.
        if (owner_by_job[static_cast<std::size_t>(job)] == -1 ||
            try_assign(owner_by_job[static_cast<std::size_t>(job)], jobs_by_worker,
                       owner_by_job, seen_stamp, stamp)) {
            // 재귀 성공 뒤 기존 담당자는 다른 일로 옮겨졌거나 원래 비어 있다. 현재 worker를 저장해 경로 간선을 뒤집는다.
            owner_by_job[static_cast<std::size_t>(job)] = worker;
            return true; // 매칭 크기가 정확히 1 늘어난 증강 경로를 찾았음을 값으로 반환한다.
        }
    }

    return false; // 도달 가능한 모든 일을 검사했지만 자유 일로 끝나는 교대 경로가 없었다.
}

int main() {
    // sync_with_stdio(false)는 bool false로 C/C++ 스트림 동기화를 끄고 이전 bool 반환값은 버린다.
    // 전역 스트림 설정이 바뀌며 이후 C stdio와 임의 순서로 섞지 않는다. 멀티스레드 동기화를 뜻하지 않는다.
    std::ios::sync_with_stdio(false);
    // cin.tie(nullptr)는 ostream* 널 값을 받아 입력 전 cout 자동 flush 연결을 끊고 이전 포인터 반환은 버린다.
    std::cin.tie(nullptr);

    int worker_count{}; // int{}는 0이며 최대 1000인 직원 수를 저장한다.
    int job_count{};    // 최대 1000인 일 수를 저장한다.
    // operator>>(int&) 두 호출은 각 lvalue에 입력을 저장하고 같은 istream&를 반환해 연쇄한다.
    // 입력 위치와 상태가 바뀌며 반환 참조는 마지막에 버린다. 문제 계약상 두 정수가 유효하다고 가정한다.
    std::cin >> worker_count >> job_count;

    // vector<vector<int>>(count)는 count개의 빈 내부 vector를 값 초기화해 외부 연속 저장소가 소유한다.
    // O(N) 초기화와 할당이 가능하고 실패 시 bad_alloc 예외가 발생한다. 이후 외부 크기는 바꾸지 않는다.
    std::vector<std::vector<int>> jobs_by_worker(static_cast<std::size_t>(worker_count));
    for (int worker{}; worker < worker_count; ++worker) {
        int possible_count{}; // 이 직원과 연결된 간선 수다.
        std::cin >> possible_count; // operator>>가 possible_count와 스트림 상태를 갱신하고 반환 istream&는 버린다.
        for (int index{}; index < possible_count; ++index) {
            int one_based_job{}; // 입력의 1기반 일 번호를 먼저 보관한다.
            std::cin >> one_based_job; // 유효한 일 번호를 읽고 스트림 위치를 다음 토큰으로 옮긴다.
            // push_back(const int&)은 계산된 int prvalue를 새 끝 원소로 구성하고 void를 반환한다.
            // 성공 뒤 내부 vector size가 1 늘며 상각 O(1), 재할당 시 그 내부 vector의 기존 관찰자만 무효화된다.
            jobs_by_worker[static_cast<std::size_t>(worker)].push_back(one_based_job - 1);
        }
    }

    // vector<int>(job_count,-1)는 모든 일을 미배정 센티널 -1로 채운다. O(M) 시간·공간이며 버퍼를 소유한다.
    std::vector<int> owner_by_job(static_cast<std::size_t>(job_count), -1);
    // 방문 배열을 매 직원마다 O(M) 초기화하지 않고 stamp 정수로 구분한다. 0은 아직 어느 시도에서도 보지 않았다는 뜻이다.
    std::vector<int> seen_stamp(static_cast<std::size_t>(job_count), 0);

    int matching_size{}; // 지금까지 찾은 서로 충돌하지 않는 배정 수이며 매 성공 때만 1 증가한다.
    for (int worker{}; worker < worker_count; ++worker) {
        const int stamp{worker + 1}; // 1..N의 서로 다른 회차 값이라 이전 방문 표식을 지울 필요가 없다.
        // try_assign은 그래프를 const 참조로 빌리고 두 상태 vector는 비const 참조로 갱신한다.
        // bool 반환값은 if가 즉시 검사하며 true면 증강 경로 하나가 생겼으므로 매칭 크기를 1 늘린다.
        if (try_assign(worker, jobs_by_worker, owner_by_job, seen_stamp, stamp)) {
            ++matching_size;
        }
    }

    // operator<<는 matching_size와 개행을 cout 버퍼에 쓰고 ostream&를 연쇄 반환하며 최종 참조는 버린다.
    std::cout << matching_size << '\n';
    // 각 직원마다 간선을 재탐색할 수 있어 최악 O(N*E), 저장은 인접 목록과 세 배열을 합쳐 O(N+M+E)다.
    return 0;
}
