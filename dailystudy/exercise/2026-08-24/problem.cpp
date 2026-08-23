// <functional>은 복사 불가능한 람다도 소유할 수 있는 std::move_only_function을 선언한다.
#include <functional>
// <iostream>은 연습 결과를 출력하는 std::cout을 선언한다.
#include <iostream>
// <memory>는 std::make_unique와 std::unique_ptr를 선언한다.
#include <memory>
// <utility>는 std::move를 선언한다.
#include <utility>
// <vector>는 작업 목록을 소유하는 std::vector를 선언한다.
#include <vector>

// class의 기본 접근은 private이다. 작업 저장과 실행 순서를 외부가 직접 깨뜨리지 못하게 한다.
class CommandBuffer {
public:
    using Command = std::move_only_function<void()>; // void() 서명을 만족하는 이동 전용 호출 대상의 타입 소거 별칭이다.

    // 생성자에는 반환형이 없고 explicit은 unsigned 값 하나가 버퍼로 암시 변환되는 것을 막는다.
    explicit CommandBuffer(std::size_t capacity) : commands_{} {
        // reserve는 capacity 값을 복사 입력으로 받고 void를 반환한다. size는 0인 채 저장 용량만 확보할 수 있다.
        // 현재 원소가 없어 무효화 대상은 없고 할당 실패 예외가 가능하며 이후 capacity개 삽입의 재할당을 줄인다.
        commands_.reserve(capacity);
    }

    void add(Command command) {
        // push_back(Command&&)은 command xvalue의 호출 대상 소유권을 끝 원소로 이동하고 size를 1 늘린다.
        // 반환 void는 버리며 상각 O(1), 용량 초과 재할당 시 기존 관찰자 무효화와 bad_alloc 가능성이 있다.
        commands_.push_back(std::move(command));
    }

    void execute() {
        // Command&는 컨테이너 원소를 복사하지 않고 빌린다. 호출 중 목록 크기를 바꾸지 않는 것이 반복 불변식이다.
        for (Command& command : commands_) {
            // operator()()은 저장된 람다를 간접 호출하고 void를 반환한다. 모든 add 입력이 비어 있지 않다는 전제가 있다.
            // 호출 대상의 캡처 상태는 바뀔 수 있고 예외는 전파되며 Command 자체는 호출 뒤에도 소유 상태를 유지한다.
            command();
        }
        // clear는 모든 Command와 그 캡처를 파괴해 size를 0으로 만들고, capacity 반환은 보장하지 않는다.
        commands_.clear();
    }

private:
    std::vector<Command> commands_{}; // 서로 다른 람다 타입을 동일한 소유 타입으로 저장한다.
};

int main() {
    int total{};            // int{}는 0으로 값 초기화되며 작업 결과를 저장한다.
    CommandBuffer buffer{2U}; // 직접 초기화가 explicit 생성자를 선택한다.

    // make_unique<int>(7)는 int 7을 동적 생성하고 unique_ptr<int> prvalue를 반환해 seven이 독점 소유한다.
    auto seven{std::make_unique<int>(7)};
    // move_only_function 생성자는 std::move(seven)으로 이동 캡처한 람다 prvalue를 소유한다.
    // 람다는 unique_ptr 때문에 복사 불가지만 이동 가능하다. total은 참조로 빌리므로 execute까지 살아 있어야 한다.
    // add(Command)는 Task prvalue 소유권을 값 매개변수로 받아 vector 끝으로 이동하고 void를 반환한다. 뒤에 buffer size는 1이다.
    buffer.add(CommandBuffer::Command{[value = std::move(seven), &total] { total += *value; }});

    // 두 번째 호출 대상도 unique_ptr<int> 5의 소유권을 람다→Command→buffer 원소 순으로 이동하고 size를 2로 만든다.
    buffer.add(CommandBuffer::Command{
        [value = std::make_unique<int>(5), &total] { total += *value; }});

    // execute는 두 명령을 등록 순서로 호출해 total을 12로 만들고 저장 명령을 모두 파괴한다.
    buffer.execute();
    // operator<<는 total과 개행을 cout에 쓰고 ostream&를 반환해 연쇄하며 최종 참조는 버린다.
    std::cout << total << '\n';
    // unique_ptr operator==(nullptr)는 seven을 빌려 빈 상태 bool을 O(1)에 반환하고 포인터 상태·소유권을 바꾸지 않는다.
    return seven == nullptr && total == 12 ? 0 : 1; // 이동 뒤 seven이 비었고 두 작업 결과가 맞는지 검증한다.
}
