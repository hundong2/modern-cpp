#include <algorithm> // 과도한 frame time을 clamp한다.
#include <chrono>    // 시간 단위를 타입으로 안전하게 표현한다.
#include <iostream>  // simulation 결과를 출력한다.

struct World {                                      // 고정 step으로 갱신할 최소 world다.
    double position = 0.0;                          // 권위 있는 1차원 위치 상태다.
    double velocity = 2.0;                          // 초당 이동량을 뜻한다.
    void tick(double seconds) { position += velocity * seconds; } // 단위가 초인 delta로 적분한다.
};                                                  // world 상태 정의를 끝낸다.

int main() {                                        // 실제 clock 대신 결정적 frame 입력을 사용한다.
    constexpr double fixed_dt = 1.0 / 20.0;         // 서버를 초당 20 tick으로 설정한다.
    constexpr int max_steps = 5;                    // 한 frame의 따라잡기를 제한한다.
    const double frame_times[] = {0.02, 0.08, 0.20}; // 0.30초의 가상 경과 시간을 제공한다.
    double accumulator = 0.0;                       // 아직 simulation에 적용하지 않은 시간을 보관한다.
    World world;                                    // 초기 위치 0, 속도 2인 권위 상태다.
    for (double frame : frame_times) {              // network/event loop iteration을 흉내 낸다.
        accumulator += std::min(frame, 0.25);       // 매우 긴 stall 한 번이 폭주하지 않게 clamp한다.
        int steps = 0;                              // 이번 frame에서 실행한 고정 step 수다.
        while (accumulator >= fixed_dt && steps < max_steps) { // 충분한 시간과 예산이 있을 때만 갱신한다.
            world.tick(fixed_dt);                   // 항상 같은 dt로 결정적 simulation을 진행한다.
            accumulator -= fixed_dt;                // 적용한 시간만 누적기에서 뺀다.
            ++steps;                                // spiral 방지 예산을 소비한다.
        }                                           // 남은 소량 시간은 다음 frame으로 넘긴다.
    }                                               // 모든 가상 frame을 처리했다.
    std::cout << world.position << '\n';            // 부동소수 오차를 포함한 위치를 관찰한다.
    return world.position > 0.0 ? 0 : 1;            // world가 실제 진행했는지만 검증한다.
}                                                   // world와 누적 상태를 파괴한다.
