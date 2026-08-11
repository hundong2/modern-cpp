// <chrono>는 단조 시계와 시간 간격 타입을 제공한다.
#include <chrono>
// <iostream>은 표준 출력 객체 std::cout을 제공한다.
#include <iostream>
// <string_view>는 문자열을 소유하지 않고 바라보는 가벼운 뷰를 제공한다.
#include <string_view>
// <utility>는 lvalue를 xvalue로 바꾸는 std::move를 제공한다.
#include <utility>

// using은 긴 템플릿 타입에 읽기 쉬운 별칭을 붙인다.
using Clock = std::chrono::steady_clock;

// class의 멤버는 기본적으로 private이며, 자원 관리 세부 구현을 감춘다.
class ScopeTimer final {
public:
    // 생성자에는 반환형이 없고, explicit은 string_view 하나가 타이머로 암시 변환되는 일을 막는다.
    // name은 비소유 뷰이므로 가리킨 문자열이 타이머보다 오래 살아야 한다.
    explicit ScopeTimer(std::string_view name)
        : name_{name}, start_{Clock::now()} {} // 멤버 초기화 목록이 두 멤버를 직접 초기화한다.

    // 복사를 막아 하나의 측정 구간이 두 객체에 중복 소유되는 의미를 피한다.
    ScopeTimer(const ScopeTimer&) = delete;
    ScopeTimer& operator=(const ScopeTimer&) = delete;

    // 소멸자는 스코프 종료 시 자동 호출되어 RAII 방식으로 경과 시간을 보고한다.
    ~ScopeTimer() {
        // now()의 prvalue와 start_ lvalue의 차이는 duration prvalue가 되며 결과 객체로 직접 초기화된다.
        const auto elapsed{Clock::now() - start_};
        // duration_cast 함수 템플릿의 명시적 인자는 원하는 단위가 microseconds임을 정한다.
        const auto micros{std::chrono::duration_cast<std::chrono::microseconds>(elapsed)};
        std::cout << name_ << ": " << micros.count() << " us\n"; // <<는 값을 출력 스트림에 순서대로 삽입한다.
    }

private:
    std::string_view name_{};       // 비소유 멤버: 문자열 데이터의 수명을 연장하지 않는다.
    Clock::time_point start_{};     // 값 멤버: 시작 시각을 타이머가 직접 소유한다.
};

int main() {
    // 문자열 리터럴은 프로그램 끝까지 살아 있으므로 비소유 string_view에 안전하게 바인딩된다.
    const ScopeTimer timer{"daily-main"}; // 중괄호 직접 초기화는 explicit 생성자를 올바르게 호출한다.
    // 실제 기계 코드는 시계 함수 호출·로드·저장·출력 호출을 포함할 수 있으나 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return 0; // int 반환값 0은 정상 종료를 뜻한다.
}
