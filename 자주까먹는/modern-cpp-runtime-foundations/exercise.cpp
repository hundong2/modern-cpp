#include <atomic> // 실습 2에서 여러 스레드가 공유할 원자 카운터를 제공한다.
#include <cstddef> // 배열 크기를 표현하는 std::size_t를 제공한다.
#include <iostream> // 실습 결과를 표준 출력으로 확인한다.
#include <memory> // 실습 1의 RAII 배열 소유권에 unique_ptr을 사용한다.
#include <thread> // 실습 2에서 두 OS 스레드를 생성한다.
#include <vector> // 동적으로 정한 수의 스레드 핸들을 안전하게 소유한다.

class ExerciseBuffer final { // TODO 1: 복사 불가·이동 가능 규칙을 연습할 작은 자원 타입이다.
public: // 생성과 관찰에 필요한 최소 인터페이스를 공개한다.
    explicit ExerciseBuffer(const std::size_t size) // 정수에서 객체로의 뜻밖의 암시 변환을 차단한다.
        : values_{std::make_unique<int[]>(size)}, size_{size} { // 값 초기화된 int 배열을 한 번 할당하고 RAII로 소유한다.
    } // 멤버가 모두 완성된 상태로 생성자 실행을 끝낸다.
    ExerciseBuffer(const ExerciseBuffer&) = delete; // TODO 1 해설: 유일 소유 배열의 복사는 금지해야 한다.
    ExerciseBuffer& operator=(const ExerciseBuffer&) = delete; // TODO 1 해설: 복사 대입도 같은 이유로 금지한다.
    ExerciseBuffer(ExerciseBuffer&&) noexcept = default; // TODO: 이동 후 size_가 원본에 남는 것이 문제인지 README 질문에 답한다.
    ExerciseBuffer& operator=(ExerciseBuffer&&) noexcept = default; // unique_ptr의 안전한 이동 대입을 멤버별로 사용한다.
    [[nodiscard]] std::size_t size() const noexcept { return size_; } // 논리적 원소 개수를 값으로 관찰한다.

private: // 배열 주소와 크기가 외부에서 따로 변경되지 않도록 숨긴다.
    std::unique_ptr<int[]> values_; // 배열을 유일 소유하고 소멸 시 자동으로 delete[] 한다.
    std::size_t size_{0U}; // 이동된 원본에서 어떤 값을 보일지 고민할 상태 멤버다.
}; // ExerciseBuffer 타입 정의를 끝낸다.

int main() { // 세 가지 TODO를 안전하게 실험하는 프로그램 진입점이다.
    ExerciseBuffer source{16U}; // 16개 정수를 소유한 버퍼를 만든다.
    ExerciseBuffer destination{std::move(source)}; // unique_ptr 주소는 이동하지만 기본 이동은 source의 size_를 16으로 남긴다.
    std::cout << "moved destination size=" << destination.size() << '\n'; // 목적지의 크기가 16인지 확인한다.
    std::cout << "moved-from source size=" << source.size() << " (TODO: make this 0)\n"; // TODO 1: 사용자 정의 이동 생성자로 0을 만들라.

    std::atomic<int> counter{0}; // TODO 2: 원자성이 필요한 공유 카운터를 0으로 초기화한다.
    std::vector<std::thread> workers; // join 가능한 두 스레드를 소유할 컨테이너를 만든다.
    workers.reserve(2U); // 핸들 삽입 중 재할당을 피하도록 용량을 확보한다.
    for (int worker{0}; worker < 2; ++worker) { // 두 작업자를 생성한다.
        workers.emplace_back([&counter]() { // main의 counter를 비소유 참조로 잡되 join으로 수명을 보장한다.
            for (int iteration{0}; iteration < 1000; ++iteration) { // 각 스레드가 천 번 반복한다.
                counter.fetch_add(1, std::memory_order_seq_cst); // TODO 2: 단순 통계에 더 약한 어떤 memory_order가 충분한지 바꾸고 설명하라.
            } // 현재 스레드의 증가 작업을 끝낸다.
        }); // 새 OS 스레드를 시작한다.
    } // 두 스레드 생성을 끝낸다.
    for (auto& worker : workers) { // 생성된 모든 스레드 핸들을 순회한다.
        worker.join(); // 완료를 기다려 main의 후속 load와 동기화한다.
    } // 모든 join을 끝낸다.
    std::cout << "counter=" << counter.load() << " (expected 2000)\n"; // 원자적 증가가 손실되지 않았는지 확인한다.

    // TODO 3: 런타임에 교체할 필요 없는 처리 정책을 가상 함수와 CRTP 중 무엇으로 표현할지 README 기준으로 설계한다.
    return 0; // 성공을 반환하며 버퍼와 스레드 컨테이너를 역순으로 소멸시킨다.
} // 실습 프로그램 실행을 끝낸다.
