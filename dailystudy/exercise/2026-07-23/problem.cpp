#include <cassert>     // assert로 실제 결과와 기대 결과를 비교 검증한다.
#include <chrono>      // std::chrono::milliseconds 시간 타입을 사용한다.
#include <iostream>    // std::cout으로 통과 메시지를 출력한다.
#include <stop_token>  // std::stop_token을 함수 매개변수 타입으로 사용한다.
#include <thread>      // std::jthread와 sleep_for를 사용한다.

// F는 호출 가능한 타입을 받는 템플릿 인자다. 함수 반환형은 int다.
template <typename F>
[[nodiscard]] int run_until_stopped(std::stop_token token, F action) {
    int count{0};  // 기본 정수 변수 count를 중괄호로 0 초기화한다.
    while (!token.stop_requested()) {  // ! 연산으로 중지 여부를 뒤집어 반복 조건을 만든다.
        action(count);                  // 함수 객체를 호출하고 count lvalue를 매개변수에 전달한다.
        ++count;                        // 전위 ++ 연산자로 저장된 값을 1 증가시킨다.
        if (count >= 3) {               // >= 비교 결과에 따라 조건 분기한다.
            break;                      // 연습 실행이 무한히 계속되지 않도록 반복문을 끝낸다.
        }
    }
    return count;  // 지역 값의 복사본을 반환하며 count의 수명은 함수 종료 때 끝난다.
}

int main() {
    int observed{0};  // 작업 스레드가 갱신하고 join 뒤 주 스레드가 읽는 기본 타입 변수다.

    // 람다의 token은 jthread가 전달하는 값 매개변수이고, &observed는 비소유 lvalue 참조 캡처다.
    std::jthread worker{[&observed](std::stop_token token) {
        observed = run_until_stopped(token, [](int value) {
            // sleep_for 표준 라이브러리 함수 호출로 각 작업을 짧게 지연한다.
            std::this_thread::sleep_for(std::chrono::milliseconds{1});
            (void)value;  // void로 명시 변환해 이 연습에서는 값을 사용하지 않음을 표시한다.
        });
    }};

    worker.join();               // 합류 후에 읽으므로 observed에 동시 접근하지 않는다.
    assert(observed == 3);       // == 비교 연산자가 기대값과 실제값을 비교한다.
    std::cout << "연습 통과: " << observed << "회 실행\n";
    return 0;
}
