// <atomic>은 여러 스레드가 데이터 경쟁 없이 합계를 갱신하는 std::atomic을 선언한다.
#include <atomic>
// <barrier>는 반복 가능한 단계 동기화 지점 std::barrier를 선언한다.
#include <barrier>
// <cstddef>는 barrier 참가자 수에 쓰는 std::ptrdiff_t를 선언한다.
#include <cstddef>
// <iostream>은 학습 결과를 쓰는 std::cout과 스트림 삽입 연산자를 선언한다.
#include <iostream>
// <thread>는 파괴 시 합류하는 C++20 std::jthread를 선언한다.
#include <thread>
// <utility>는 callable 소유권 이동에 쓰는 std::move를 선언한다.
#include <utility>
// <vector>는 입력 작업량과 작업 스레드 객체를 동적으로 소유하는 std::vector를 선언한다.
#include <vector>

// struct는 기본 접근이 public이다. 병렬 실행 뒤의 값 스냅샷을 단순 레코드로 전달한다.
struct ReleaseReport {
    int prepared{};          // 준비 단계 합계를 값으로 소유한다.
    int published{};         // 게시 단계 합계를 값으로 소유한다.
    int completed_phases{};  // barrier 완료 함수가 실행된 단계 수다.
};

// class는 기본 접근이 private이다. 단계 순서 정책을 public run 함수 뒤에 감춘다.
class ReleaseCoordinator {
public:
    // weights는 호출자가 소유한 vector<int>를 const 참조로 빌리며 원소를 복사하거나 수명을 연장하지 않는다.
    [[nodiscard]] ReleaseReport run(const std::vector<int>& weights) const {
        // vector::empty() const noexcept는 데이터 인자 없이 size==0 여부 bool을 O(1)에 반환하고 vector를 유지한다.
        // 참가자가 0인 barrier는 생성 후 멤버 호출을 할 수 없으므로 빈 입력은 값 초기화된 보고서를 즉시 반환한다.
        if (weights.empty()) {
            return ReleaseReport{};
        }

        std::atomic<int> prepared{};          // atomic<int>{}는 저장값을 0으로 값 초기화한다.
        std::atomic<int> published{};         // 여러 worker가 합계를 원자적으로 더한다.
        std::atomic<int> completed_phases{};  // barrier 완료 함수의 실행 횟수를 원자적으로 센다.

        // lambda는 completed_phases를 비소유 참조로 캡처한다. barrier보다 먼저 생성되어 더 나중에 파괴되므로 수명이 안전하다.
        auto on_phase_complete = [&completed_phases]() noexcept {
            // atomic<int>::fetch_add(1, relaxed)는 차이값 int 1과 메모리 순서 열거값을 값으로 받는다.
            // 증가 전 int를 반환하지만 여기서는 버린다. 저장값은 1 늘고 수정 순서 원자성만 보장하며 다른 데이터 동기화에는 기대지 않는다.
            // lock-free 여부는 구현에 따라 다르고 O(1)로 다루지만 실제 명령은 CPU·ABI·컴파일러에 따라 달라진다.
            completed_phases.fetch_add(1, std::memory_order_relaxed);
        };

        // 공용 표준 문서: ../standard-library/concurrency-time-filesystem.md
        // std::barrier<Completion>(expected, completion)는 반환형 없는 생성자다. expected는 ptrdiff_t 양수 참가자 수 값,
        // completion은 이름 있는 lambda lvalue를 std::move로 xvalue화해 barrier가 소유한다. weights가 비어 있지 않아야 한다.
        // 각 단계에서 expected번 도착하면 완료 callable을 한 번 실행하고 다음 단계 참가자 수를 expected로 다시 연다.
        std::barrier<decltype(on_phase_complete)> phase_gate{
            static_cast<std::ptrdiff_t>(weights.size()), std::move(on_phase_complete)};

        // jthread vector의 범위를 별도 블록으로 묶어 결과를 load하기 전에 모든 스레드가 파괴·join되게 한다.
        {
            std::vector<std::jthread> workers{}; // 각 원소가 OS 작업 스레드의 RAII 소유권을 가진다.
            // reserve(size_type)는 weights.size() 값을 받아 capacity만 확보하고 void를 반환한다.
            // 성공 뒤 size는 0이며 O(N) 할당 가능, 이후 N개 emplace에서 재할당과 jthread 이동을 피한다.
            workers.reserve(weights.size());

            // 범위 for는 weights의 begin/end를 내부에서 얻고 int 값으로 각 원소를 복사한다. vector 상태는 유지된다.
            for (const int weight : weights) {
                // emplace_back(args...)는 lambda prvalue를 전달해 jthread를 끝에서 직접 생성하고 jthread&를 반환한다.
                // 반환 참조는 사용하지 않는다. 성공 뒤 size가 1 늘고 새 스레드가 즉시 시작하며 생성 실패 시 system_error가 가능하다.
                // lambda는 weight를 값 복사하고 세 atomic과 barrier를 참조 캡처한다. workers가 먼저 join되어 참조 수명이 안전하다.
                workers.emplace_back([weight, &prepared, &published, &phase_gate]() noexcept {
                    // 첫 fetch_add는 이 worker의 weight를 relaxed 원자 덧셈하고 증가 전 값은 무시한다.
                    prepared.fetch_add(weight, std::memory_order_relaxed);

                    // barrier::arrive_and_wait()는 데이터 인자가 없고 반환형 void다. 이 단계 도착 수를 1 줄인 뒤 완료까지 막는다.
                    // 마지막 도착 스레드가 완료 함수를 실행하며, 완료 함수 종료는 풀린 호출들과 강하게 happens-before를 이룬다.
                    // 호출 뒤 barrier는 다음 단계를 시작한다. 같은 barrier 객체에 대한 동시 arrive 호출은 지원된다.
                    phase_gate.arrive_and_wait();

                    // 첫 barrier 뒤 모든 준비 덧셈이 끝났다. 게시 단계에서는 예시로 weight의 두 배를 원자 합산한다.
                    published.fetch_add(weight * 2, std::memory_order_relaxed);
                    // 두 번째 단계도 모든 worker가 게시를 끝낼 때까지 기다리고 완료 함수를 정확히 한 번 실행한다.
                    phase_gate.arrive_and_wait();
                });
            }
            // 블록 끝에서 vector가 역순으로 jthread를 파괴한다. 각 jthread 소멸자는 중지 요청 후 join해 worker 종료를 기다린다.
        }

        // atomic::load(relaxed)는 데이터 값 인자 없이 메모리 순서만 받고 관찰한 int를 반환하며 저장값은 바꾸지 않는다.
        // 모든 jthread join 뒤라 결과가 완료되어 있고 반환값은 ReleaseReport prvalue의 멤버를 직접 초기화한다.
        return ReleaseReport{prepared.load(std::memory_order_relaxed),
                             published.load(std::memory_order_relaxed),
                             completed_phases.load(std::memory_order_relaxed)};
    }
};

int main() {
    // initializer_list 생성자는 세 int를 새 연속 저장소에 복사해 weights가 소유한다. O(N), 할당 실패 가능성이 있다.
    const std::vector<int> weights{2, 3, 4};
    const ReleaseCoordinator coordinator{}; // 기본 생성된 무상태 서비스 객체다.
    // run은 weights lvalue를 const 참조로 빌리고 ReleaseReport prvalue를 반환해 report를 직접 초기화한다.
    const ReleaseReport report{coordinator.run(weights)};

    // operator<<는 cout 버퍼·상태를 갱신하고 ostream&를 연쇄 반환한다. 세 int와 구분 문자를 쓰며 최종 참조는 버린다.
    std::cout << report.prepared << ' ' << report.published << ' ' << report.completed_phases << '\n';

    // 실제 load·비교·조건 분기·호출·동기화 명령은 CPU·ABI·라이브러리·컴파일러·최적화 옵션에 따라 달라진다.
    return report.prepared == 9 && report.published == 18 && report.completed_phases == 2 ? 0 : 1;
}
