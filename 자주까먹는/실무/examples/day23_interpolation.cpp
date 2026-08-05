#include <algorithm> // 보정 계수를 0~1 범위로 제한한다.
#include <cmath>     // frame-rate 독립 지수 보정의 std::exp를 제공한다.
#include <iostream>  // 예측/표시 위치를 출력한다.

class ClientCharacter {                             // 서버 권위 상태를 부드럽게 표시하는 모형이다.
public:                                             // packet 수신과 render update API를 공개한다.
    void on_server_state(double position, double velocity) { // 검증된 snapshot을 받는다.
        target_position_ = position;                // 권위 위치로 예측 기준을 교정한다.
        velocity_ = velocity;                       // 다음 snapshot 전 추측항법에 쓸 속도를 저장한다.
    }                                               // timestamp/sequence 처리는 단순 모형에서 생략했다.

    void update(double dt_seconds) {                // 한 render frame의 경과 초를 받는다.
        target_position_ += velocity_ * dt_seconds; // 마지막 속도로 목표 위치를 예측한다.
        constexpr double response = 10.0;           // 초당 오차 감소 속도를 조절한다.
        const double alpha = 1.0 - std::exp(-response * dt_seconds); // frame-rate 독립 지수 보정 계수다.
        display_position_ += (target_position_ - display_position_) * alpha; // 화면 위치를 부드럽게 접근시킨다.
    }                                               // 큰 오차 snap threshold는 실무 정책으로 추가한다.

    double display_position() const noexcept { return display_position_; } // 화면에 그릴 값을 읽는다.
    double target_position() const noexcept { return target_position_; }   // 디버깅용 예측 목표를 읽는다.

private:                                            // 권위/표시 상태를 외부 직접 수정에서 보호한다.
    double display_position_ = 0.0;                 // 실제 렌더링에 사용하는 부드러운 위치다.
    double target_position_ = 0.0;                  // 서버 snapshot+예측으로 얻은 목표 위치다.
    double velocity_ = 0.0;                         // 마지막으로 알려진 속도다.
};                                                  // client presentation 상태 정의를 끝낸다.

int main() {                                        // packet 사이의 예측과 보정을 시험한다.
    ClientCharacter player;                         // 위치 0인 client 표현을 만든다.
    player.on_server_state(0.0, 10.0);              // 서버가 초당 10 이동을 알린다.
    for (int frame = 0; frame < 3; ++frame) player.update(0.1); // 0.3초 동안 snapshot 없이 예측한다.
    player.on_server_state(20.0, 10.0);             // 지연된 권위 위치로 목표를 교정한다.
    for (int frame = 0; frame < 5; ++frame) player.update(0.1); // 순간 이동 없이 목표를 추적한다.
    std::cout << player.display_position() << ' ' << player.target_position() << '\n'; // 표시/목표 차이를 관찰한다.
    return player.display_position() > 0.0 ? 0 : 1; // 화면 위치가 진행했는지 검증한다.
}                                                   // player를 파괴한다.
