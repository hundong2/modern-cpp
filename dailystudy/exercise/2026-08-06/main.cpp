// <functional>은 호출 가능한 객체를 같은 형식으로 보관하는 std::function을 제공한다.
#include <functional>
// <iostream>은 표준 출력 객체 std::cout을 제공한다.
#include <iostream>
// <memory>는 단독 소유권을 나타내는 std::unique_ptr와 std::make_unique를 제공한다.
#include <memory>
// <string>은 소유하는 문자열 형식 std::string을 제공한다.
#include <string>
// <utility>는 이동을 요청하는 std::move를 제공한다.
#include <utility>
// <vector>는 크기가 동적으로 변하는 연속 컨테이너를 제공한다.
#include <vector>

using Command = std::function<int(int)>; // using 별칭은 int를 받아 int를 돌려주는 호출 계약을 표현한다.

// struct는 기본 접근이 public이므로 단순 실행 결과 묶음에 알맞다.
struct ExecutionResult {
    int value{}; // 기본 타입 int 멤버를 중괄호로 0 초기화한다.
    std::string trace{}; // 각 단계를 기록하는 소유 문자열이다.
};

// class는 기본 접근이 private이며 외부에는 필요한 포트만 공개한다.
class ResultPort {
public:
    virtual ~ResultPort() = default; // 가상 소멸자는 기반 포인터로 파생 객체를 안전하게 파괴한다.
    virtual void publish(const ExecutionResult& result) const = 0; // const 참조는 복사 없이 읽고 순수 가상 함수가 포트를 정의한다.
};

class ConsoleResultAdapter final : public ResultPort {
public:
    void publish(const ExecutionResult& result) const override {
        std::cout << result.trace << " = " << result.value << '\n'; // << 연산자가 문자열과 정수를 출력 스트림에 삽입한다.
    }
};

class CommandPipeline {
public:
    // 생성자는 반환형이 없다. explicit은 unique_ptr 하나가 파이프라인으로 암시 변환되는 일을 막는다.
    explicit CommandPipeline(std::unique_ptr<ResultPort> output)
        : output_{std::move(output)} {} // 멤버 초기화 목록이 xvalue에서 단독 소유권을 이동한다.

    void add(std::string name, Command command) {
        names_.push_back(std::move(name)); // 값 매개변수의 문자열 소유 저장소를 벡터로 이동한다.
        commands_.push_back(std::move(command)); // std::function이 감싼 호출 객체도 목적지로 이동한다.
    }

    [[nodiscard]] int run(int initial) const {
        ExecutionResult result{initial, "start"}; // 집계체를 값과 문자열로 직접 초기화한다.
        for (std::size_t index{}; index < commands_.size(); ++index) { // 크기 형식 인덱스로 모든 명령을 순회한다.
            result.value = commands_[index](result.value); // []로 명령을 찾고 ()로 간접 호출한 prvalue를 저장한다.
            result.trace += " -> " + names_[index]; // +=와 +가 단계 이름을 추적 문자열에 이어 붙인다.
        }
        output_->publish(result); // ->는 소유 포인터가 가리키는 포트를 호출하며 실제 어댑터로 가상 간접 호출될 수 있다.
        return result.value; // int prvalue를 호출자에게 반환한다.
    }

private:
    std::vector<std::string> names_; // 각 명령과 같은 인덱스에 표시 이름을 저장한다.
    std::vector<Command> commands_; // 템플릿 인자 Command인 소유 컨테이너가 실행 단계를 보관한다.
    std::unique_ptr<ResultPort> output_; // 파이프라인이 출력 포트 객체의 수명을 단독 소유한다.
};

int main() { // 프로그램 진입 함수는 성공 여부를 int 종료 코드로 반환한다.
    CommandPipeline pipeline{std::make_unique<ConsoleResultAdapter>()}; // 파생 객체 prvalue의 소유권이 기반 포인터로 이동한다.
    const int offset{3}; // const 지역 변수는 초기화 뒤 값을 바꿀 수 없다.
    pipeline.add("double", [](int value) { return value * 2; }); // 상태 없는 람다가 곱셈 명령으로 복사되어 저장된다.
    pipeline.add("add-three", [offset](int value) { return value + offset; }); // 캡처가 offset을 값으로 복사해 람다 수명과 분리한다.
    const int answer{pipeline.run(4)}; // 반환 prvalue로 목적지를 직접 초기화하여 불필요한 임시 복사를 피한다.
    // 실행에는 로드·저장·비교·조건 분기·함수/가상 간접 호출이 포함될 수 있으나 실제 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return answer == 11 ? 0 : 1; // == 비교와 ?: 조건 연산자로 성공 코드를 고른다.
}
