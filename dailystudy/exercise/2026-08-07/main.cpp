// <iostream>은 표준 출력 객체 std::cout을 제공한다.
#include <iostream>
// <string>은 소유권을 가진 문자열 std::string을 제공한다.
#include <string>
// <utility>는 전달된 여러 호출 대상을 묶는 std::forward를 제공한다.
#include <utility>
// <variant>는 여러 타입 중 정확히 하나를 값으로 보관하는 std::variant와 std::visit을 제공한다.
#include <variant>

// struct의 기본 접근은 public이다. 데이터만 묶는 단순 상태에 알맞다.
struct Idle { std::string reason{}; }; // 중괄호 초기화는 빈 문자열로 안전하게 초기화한다.
struct Running { int completed{}; }; // int 멤버는 {}로 0이 된다.
struct Failed { int error_code{}; };

// using은 긴 템플릿 타입에 읽기 쉬운 별칭을 준다. 템플릿 인자는 가능한 세 상태 타입이다.
using JobState = std::variant<Idle, Running, Failed>;

// 여러 람다를 하나의 방문자로 합치는 C++20 보조 클래스 템플릿이다.
template<class... Functions>
struct Overload : Functions... { using Functions::operator()...; };
template<class... Functions>
Overload(Functions...) -> Overload<Functions...>; // 추론 가이드가 템플릿 인자를 생성자 인수에서 알아낸다.

class Job { // class의 기본 접근은 private이므로 상태 표현을 외부에서 직접 깨뜨릴 수 없다.
public:
    // 생성자에는 반환형이 없다. explicit은 Job{3} 같은 직접 초기화만 허용하고 암시적 변환을 막는다.
    explicit Job(int target) : target_{target}, state_{Idle{"준비"}} {} // 멤버 초기화 목록에서 멤버를 직접 만든다.

    void start() { state_ = Running{0}; } // prvalue Running이 variant 안의 이전 객체를 끝내고 새 객체를 만든다.
    void advance() {
        // get_if<Running>(&state_)는 variant 주소 하나를 입력받아 활성 타입이 맞으면 Running*, 아니면 nullptr를 반환한다.
        // state_를 바꾸거나 예외를 던지지 않으며 반환 포인터는 state_의 대안 교체·소멸 시 무효화된다.
        if (auto* running{std::get_if<Running>(&state_)}) {
            ++running->completed; // ->로 포인터가 가리키는 멤버를 수정한다.
            if (running->completed >= target_) { state_ = Idle{"완료"}; } // 비교 뒤 조건 분기한다.
        }
    }

    [[nodiscard]] std::string describe() const { // const 멤버 함수는 논리적 상태를 바꾸지 않는다.
        // visit(visitor,state_)는 방문자와 variant를 입력받아 활성 대안용 람다를 호출하고 그 string 반환값을 돌려준다.
        return std::visit(Overload{
            [](const Idle& value) { return "idle:" + value.reason; }, // const lvalue 참조로 복사 없이 읽는다.
            // to_string(int)는 정수 하나를 입력받아 새 소유 string을 반환하고 value는 바뀌지 않는다.
            [](const Running& value) { return "running:" + std::to_string(value.completed); },
            [](const Failed& value) { return "failed:" + std::to_string(value.error_code); }
        }, state_); // std::visit은 활성 타입에 맞는 함수 호출을 선택한다.
    }

private:
    int target_{}; // 목표 작업 수를 객체가 소유한다.
    JobState state_{}; // variant가 활성 상태 객체의 수명을 직접 관리한다.
};

int main() { // 운영체제에 종료 코드를 반환하는 진입 함수다.
    Job job{2}; // 직접 초기화이며 job은 이름 있는 lvalue다.
    job.start(); // 멤버 함수 호출은 숨은 this 포인터를 통해 객체를 찾는다.
    job.advance();
    job.advance();
    const std::string text{job.describe()}; // 반환 prvalue로 목적지 객체를 직접 초기화해 복사 생략될 수 있다.
    std::cout << text << '\n';
    // 로드·비교·조건 분기·함수 호출과 visit의 분기 구현은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return text == "idle:완료" ? 0 : 1; // == 결과에 따라 삼항 연산자가 성공/실패 코드를 고른다.
}
