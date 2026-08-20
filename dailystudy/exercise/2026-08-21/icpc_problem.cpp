/*
문제 ID·제목: BOJ 5052 - 전화번호 목록
출처: https://www.acmicpc.net/problem/5052 (Baekjoon Online Judge, ICPC 계열 문제)

문제 요약:
각 테스트 케이스에 여러 전화번호가 주어진다. 어떤 번호가 다른 번호의 접두사라면, 긴 번호를 누르기 전에
짧은 번호로 전화가 연결될 수 있으므로 목록은 일관성이 없다. 모든 서로 다른 두 번호 사이에 접두사 관계가
없으면 YES, 하나라도 있으면 NO를 출력한다. 아래 설명은 원문의 요지를 다시 쓴 것이며 문제 원문 전체를 복제하지 않는다.

입력:
첫 줄에 테스트 케이스 수 T가 주어진다. 각 케이스마다 전화번호 수 N과 이어지는 N개의 전화번호가 한 줄씩 주어진다.

출력:
각 테스트 케이스마다 목록이 일관되면 YES, 아니면 NO를 한 줄에 출력한다.

제약:
T는 최대 50, N은 최대 10,000이며 전화번호는 길이 최대 10의 숫자 문자열이다.

예제:
입력
2
3
911
97625999
91125426
5
113
12340
123440
12345
98346

출력
NO
YES
*/

// <array>는 각 트라이 노드의 숫자 0..9 자식 인덱스 열 개를 고정 저장하는 std::array를 선언한다.
#include <array>
// <cstddef>는 컨테이너 크기와 인덱스에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 입력 std::cin, 출력 std::cout과 고속 입출력 설정을 선언한다.
#include <iostream>
// <string>은 전화번호 문자를 독점 소유하는 std::string을 선언한다.
#include <string>
// <vector>는 트라이 노드를 연속 메모리에 소유하는 std::vector를 선언한다.
#include <vector>

// 공용 알고리즘 문서: ../algorithm/prefix-trie.md
// struct는 기본 접근이 public이다. 대회용 노드는 상태만 묶으며 children의 0은 간선 없음이라는 sentinel이다.
struct TrieNode {
    // array<int, 10>{}은 모든 int를 0으로 값 초기화하며 동적 할당이 없다.
    std::array<int, 10> children{};
    bool terminal{}; // bool{}은 false이며 여기서 끝나는 기존 전화번호가 없다는 뜻이다.
};

// nodes는 트라이 전체를 소유하고 number는 호출 동안 읽기만 하는 비소유 const 참조다.
// 반환 bool은 이번 삽입까지 접두사 충돌이 없는지 나타낸다.
[[nodiscard]] bool insert_consistently(std::vector<TrieNode>& nodes, const std::string& number) {
    std::size_t current{}; // 루트 노드는 nodes[0]이고 중괄호 값 초기화로 0이 된다.

    // 범위 for는 number의 begin/end 반복자를 내부적으로 얻어 각 char 값을 복사한다. number는 바뀌지 않는다.
    for (const char digit : number) {
        // 현재 위치가 이미 번호 끝이면 그 기존 짧은 번호가 이번 번호의 진접두사다.
        if (nodes[current].terminal) {
            return false;
        }

        // 입력 계약상 digit은 '0'..'9'이므로 뺄셈 결과 0..9가 array의 유효 인덱스다.
        const std::size_t slot{static_cast<std::size_t>(digit - '0')};
        int child{nodes[current].children[slot]};

        // 0 sentinel이면 아직 이 숫자 간선이 없으므로 새 노드를 node pool 끝에 만든다.
        if (child == 0) {
            // vector<TrieNode>::push_back(TrieNode&&)는 TrieNode prvalue를 끝 원소로 이동한다.
            // 반환형은 void라 무시한다. 성공하면 size가 1 증가하며 용량 부족 시 O(N) 재할당과 bad_alloc이 가능하다.
            // 재할당은 기존 포인터·참조·반복자를 무효화하지만 우리는 인덱스만 저장해 안전하다. 상각 O(1)이다.
            nodes.push_back(TrieNode{});

            // vector::size()는 인자 없이 원소 수를 size_type 값으로 반환하며 nodes 상태를 바꾸지 않는 O(1) 관찰이다.
            // 방금 추가한 마지막 인덱스는 size()-1이고 문제 최대 총 노드 수가 int 범위보다 작아 명시 변환이 안전하다.
            child = static_cast<int>(nodes.size() - 1U);
            nodes[current].children[slot] = child; // 부모에서 새 자식 인덱스로 간선을 연결한다.
        }

        current = static_cast<std::size_t>(child); // 다음 반복 불변식의 현재 노드로 이동한다.
    }

    // 같은 번호가 중복되어도 서로가 접두사이므로 일관되지 않다.
    if (nodes[current].terminal) {
        return false;
    }

    // 새 번호 끝에 기존 자식이 하나라도 있으면 새 번호가 기존 긴 번호의 진접두사다.
    for (const int child : nodes[current].children) {
        if (child != 0) {
            return false;
        }
    }

    nodes[current].terminal = true; // 충돌이 없을 때만 이 경로의 끝을 번호로 표시한다.
    return true;
}

int main() {
    // sync_with_stdio(bool)의 인자 false는 C/C++ 표준 스트림 동기화를 끄라는 값이다.
    // 이전 bool 상태 반환은 버린다. 호출 뒤 전역 스트림 설정이 바뀌며 C stdio와 임의 혼용 순서는 보장되지 않는다.
    std::ios::sync_with_stdio(false);
    // cin.tie(nullptr)는 ostream* 널 포인터를 받아 입력 전 자동 flush 연결을 해제한다.
    // 이전 ostream* 반환은 버리며 스트림 소유권은 바뀌지 않는다. 대화형 문제가 아니므로 수동 flush가 필요 없다.
    std::cin.tie(nullptr);

    int test_cases{};
    // operator>>(int&)는 test_cases lvalue를 갱신하고 입력 위치를 전진시킨다. 반환 istream&는 bool 문맥에서 실패 검사에 쓴다.
    if (!(std::cin >> test_cases)) {
        return 0;
    }

    // for는 정확히 T개 테스트 케이스를 처리하며 ++case_index가 int 저장값을 1 증가시킨다.
    for (int case_index{}; case_index < test_cases; ++case_index) {
        int count{};
        // 추출 성공을 입력 계약이 보장한다. 반환 istream&는 다음 연쇄가 없어 버린다.
        std::cin >> count;

        // initializer_list 생성자가 루트 TrieNode 하나를 복사해 소유하는 vector를 만든다. 공간은 O(1)에서 시작한다.
        std::vector<TrieNode> nodes{TrieNode{}};
        bool consistent{true};

        for (int index{}; index < count; ++index) {
            std::string number{}; // 빈 소유 문자열을 만들며 보통 할당하지 않는다.
            // operator>>(string&)은 공백 전 숫자를 number가 소유하도록 바꾸고 입력을 전진시킨다.
            // 반환 istream&는 버린다. 길이에 선형이고 문자열 재할당/bad_alloc 또는 입력 실패 상태가 가능하다.
            std::cin >> number;

            // &&는 단락 평가한다. 이미 false면 더 삽입하지 않지만 남은 입력은 위에서 계속 소비한다.
            // 함수 반환 bool을 즉시 검사하고 false이면 이후에도 false를 유지한다.
            if (consistent && !insert_consistently(nodes, number)) {
                consistent = false;
            }
        }

        // operator<<는 문자열 리터럴과 개행을 출력 버퍼에 쓰고 ostream&를 연쇄 반환한다.
        // 최종 반환 참조는 버리며 consistent, nodes의 값과 수명은 바뀌지 않는다.
        std::cout << (consistent ? "YES\n" : "NO\n");
    }

    return 0;
}
