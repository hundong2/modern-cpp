// <array>는 작업 두 개를 고정 크기로 소유하는 std::array를 선언한다.
#include <array>
// <atomic>은 스레드 간 합계를 데이터 경쟁 없이 저장하는 std::atomic을 선언한다.
#include <atomic>
// <barrier>는 두 단계 사이를 동기화하는 std::barrier를 선언한다.
#include <barrier>
// <iostream>은 결과 출력용 std::cout과 operator<<를 선언한다.
#include <iostream>
// <thread>는 범위 끝에서 자동 합류하는 std::jthread를 선언한다.
#include <thread>

int main() {
    std::atomic<int> staged{};   // 첫 단계가 만든 합계이며 0으로 원자 값 초기화된다.
    std::atomic<int> verified{}; // 둘째 단계가 읽고 확정할 합계다.

    // std::barrier<> 생성자는 expected=2를 값으로 받아 참가자 두 명의 반복 가능한 단계 경계를 만든다.
    // 기본 완료 함수는 추가 동작이 없는 함수 객체다. 생성자는 반환형이 없고 gate가 동기화 상태를 소유한다.
    std::barrier phase_gate{2};

    // array<jthread,2>는 두 jthread를 직접 목록 초기화한다. 각 생성자는 lambda prvalue를 소유하고 즉시 실행을 시작한다.
    // lambda는 staged, verified, gate를 비소유 참조 캡처하며 array가 먼저 파괴·join되어 세 객체의 수명이 더 길다.
    std::array<std::jthread, 2> workers{
        std::jthread{[&staged, &verified, &phase_gate]() noexcept {
            // fetch_add(3, relaxed)는 증가 전 int를 반환하지만 버리고 staged 저장값만 원자적으로 3 늘린다.
            staged.fetch_add(3, std::memory_order_relaxed);
            // arrive_and_wait()는 인자·반환값 없이 첫 단계 도착을 기록하고 다른 참가자가 올 때까지 대기한다.
            phase_gate.arrive_and_wait();
            // barrier 단계 동기화 뒤 staged의 두 덧셈이 완료되었다. load는 값을 바꾸지 않고 int를 반환한다.
            verified.fetch_add(staged.load(std::memory_order_relaxed), std::memory_order_relaxed);
            phase_gate.arrive_and_wait(); // 검증 단계도 두 worker가 모두 끝날 때까지 맞춘다.
        }},
        std::jthread{[&staged, &verified, &phase_gate]() noexcept {
            staged.fetch_add(4, std::memory_order_relaxed); // 두 번째 기여 4를 원자적으로 더하고 이전 값은 버린다.
            phase_gate.arrive_and_wait(); // 첫 worker와 같은 첫 단계에 정확히 한 번 도착한다.
            verified.fetch_add(staged.load(std::memory_order_relaxed), std::memory_order_relaxed);
            phase_gate.arrive_and_wait(); // 두 번째 단계 참가자 수도 2로 자동 재설정되어 재사용된다.
        }}};

    // array::operator[](size_type)은 인덱스 0의 jthread&를 O(1)에 반환한다. joinable()은 bool로 소유 상태만 관찰한다.
    // join()은 데이터 인자와 반환값 없이 첫 스레드 종료를 기다리고 성공 뒤 joinable=false로 만든다.
    if (workers[0].joinable()) {
        workers[0].join();
    }
    // 두 번째 jthread도 명시적으로 join해 출력 전에 verified의 최종값이 확정되도록 한다.
    if (workers[1].joinable()) {
        workers[1].join();
    }

    // atomic::load(relaxed)는 저장값을 바꾸지 않고 int 14를 반환한다. join이 완료 가시성 경계를 제공한다.
    const int result{verified.load(std::memory_order_relaxed)};
    // operator<<는 result와 개행을 cout에 쓰고 ostream&를 반환하며 최종 참조는 버린다.
    std::cout << result << '\n';
    return result == 14 ? 0 : 1;
}
