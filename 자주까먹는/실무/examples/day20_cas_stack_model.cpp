#include <atomic>   // CAS로 head index를 원자 갱신한다.
#include <iostream> // pop 결과를 출력한다.
#include <optional> // 빈 stack 결과를 표현한다.

class FixedCasStack {                               // 메모리 회수를 제거한 고정 배열 CAS 학습 모형이다.
public:                                             // 단일 producer 초기화와 pop API를 공개한다.
    FixedCasStack() {                               // 세 노드를 미리 연결해 allocation race를 피한다.
        nodes_[0] = Node{10, -1};                   // 마지막 노드는 다음이 없음을 -1로 표시한다.
        nodes_[1] = Node{20, 0};                    // 두 번째 노드는 index 0을 가리킨다.
        nodes_[2] = Node{30, 1};                    // head 노드는 index 1을 가리킨다.
        head_.store(2);                             // 완성된 목록을 한 번에 공개한다.
    }                                               // 이후 nodes_는 읽기 전용이라 data race가 없다.

    std::optional<int> pop() {                      // 여러 consumer가 동시에 호출할 수 있다.
        int observed = head_.load();                // 현재 head index를 관찰한다.
        while (observed != -1) {                    // 빈 stack이 될 때까지 CAS를 시도한다.
            const int next = nodes_[observed].next; // 관찰한 immutable 노드의 다음 index를 읽는다.
            if (head_.compare_exchange_weak(observed, next)) { // head가 그대로면 next로 바꾼다.
                return nodes_[observed].value;      // 성공한 thread만 해당 값을 가져간다.
            }                                       // 실패 시 observed가 최신 head로 갱신될 수 있다.
        }                                           // -1을 관찰하면 stack이 비었다.
        return std::nullopt;                        // 삭제할 동적 메모리가 없어 reclamation 문제를 피한다.
    }                                               // 실제 가변 stack에는 ABA/reclamation 해법이 필요하다.

private:                                            // 노드 표현과 head를 감춘다.
    struct Node { int value = 0; int next = -1; }; // 고정 index로 연결한 immutable 노드다.
    Node nodes_[3]{};                               // 프로그램 수명 동안 주소가 바뀌지 않는 저장소다.
    std::atomic<int> head_{-1};                     // CAS 대상이 되는 현재 head index다.
};                                                  // 학습 모형 정의를 끝낸다.

int main() {                                        // 세 값을 순서대로 pop한다.
    FixedCasStack stack;                            // 30→20→10 stack을 만든다.
    const auto a = stack.pop();                     // head 30을 제거한다.
    const auto b = stack.pop();                     // 다음 20을 제거한다.
    const auto c = stack.pop();                     // 마지막 10을 제거한다.
    const auto empty = stack.pop();                 // 빈 결과를 확인한다.
    if (!a || !b || !c || empty) return 1;          // 예상한 optional 상태가 아니면 실패한다.
    std::cout << *a << ' ' << *b << ' ' << *c << '\n'; // 30 20 10을 출력한다.
    return (*a == 30 && *b == 20 && *c == 10) ? 0 : 1; // 순서까지 검증한다.
}                                                   // stack을 파괴한다.
