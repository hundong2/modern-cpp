// <functional>은 서로 다른 이동 전용 호출 대상을 한 타입으로 소유하는 std::move_only_function을 선언한다.
#include <functional>
// <iostream>은 결과를 쓰는 std::cout과 스트림 삽입 연산자를 선언한다.
#include <iostream>
// <memory>는 독점 소유 포인터 std::unique_ptr와 std::make_unique를 선언한다.
#include <memory>
// <string>은 서비스 이름의 문자 버퍼를 소유하는 std::string을 선언한다.
#include <string>
// <utility>는 lvalue 식을 xvalue로 바꾸는 std::move 함수 템플릿을 선언한다.
#include <utility>
// <vector>는 이동 전용 작업을 연속 저장소에 소유하는 std::vector를 선언한다.
#include <vector>

// struct는 기본 접근이 public이다. 배포 요청은 검증 뒤 값만 전달하는 단순 레코드다.
struct DeploymentRequest {
    std::string service{}; // string은 문자 버퍼를 독점 소유하고 레코드와 함께 파괴된다.
    int replicas{};        // int{}는 0으로 값 초기화되며 배포할 복제본 수를 저장한다.
};

// class는 기본 접근이 private이다. 적용된 총량 변경 규칙을 public 함수 뒤에 감춘다.
class DeploymentLedger {
public:
    // 생성자에는 반환형이 없다. explicit은 int 하나가 Ledger로 뜻밖에 암시 변환되는 일을 막는다.
    // initial_replicas는 int prvalue/lvalue에서 값 복사되고 멤버 초기화 목록이 본문 전에 total_replicas_를 초기화한다.
    explicit DeploymentLedger(int initial_replicas) : total_replicas_{initial_replicas} {}

    // const 참조는 요청을 빌릴 뿐 복사하거나 수명을 연장하지 않는다. 호출자는 호출 동안 객체가 살아 있음을 보장한다.
    void apply(const DeploymentRequest& request) {
        // +=는 현재 total_replicas_를 읽어 request.replicas와 더한 뒤 같은 int 객체에 저장한다.
        total_replicas_ += request.replicas;
        // string 복사 대입은 request.service의 문자를 last_service_가 독립 소유하도록 복사한다.
        // 길이에 선형이고 필요하면 할당하며, 재할당 시 last_service_의 기존 포인터·참조·반복자가 무효화된다.
        last_service_ = request.service;
    }

    // const 멤버 함수는 관찰만 하고 int 값을 복사 반환하므로 Ledger 소유권과 상태는 유지된다.
    [[nodiscard]] int total_replicas() const { return total_replicas_; }
    // 반환 const string&는 내부 문자열을 빌린다. Ledger 파괴나 이후 대입·재할당 뒤 보관한 관찰자는 유효하지 않을 수 있다.
    [[nodiscard]] const std::string& last_service() const { return last_service_; }

private:
    int total_replicas_{};       // private 멤버는 public 함수만 갱신한다.
    std::string last_service_{}; // 마지막 적용 서비스 이름을 Ledger가 소유한다.
};

// 이동 전용 명령을 받아 나중에 실행하는 작은 애플리케이션 계층이다.
class TaskQueue {
public:
    // using은 긴 클래스 템플릿 인스턴스에 별칭을 붙일 뿐 새 타입 계층이나 객체를 만들지 않는다.
    using Task = std::move_only_function<void()>;

    // 생성자는 반환형이 없다. explicit은 size_t 용량이 TaskQueue로 암시 변환되는 것을 막는다.
    explicit TaskQueue(std::size_t expected_count) : tasks_{} {
        // vector<Task>::reserve(size_type)는 expected_count 값을 복사해 최소 용량을 요청하고 void를 반환해 버린다.
        // 호출 전 tasks_는 빈 vector다. 성공 뒤 size는 0인 채 capacity만 커질 수 있고, O(N) 이동·할당 실패가 가능하다.
        // 현재 원소가 없어 무효화할 관찰자는 없으며 이후 expected_count개까지 push_back 재할당을 피한다.
        tasks_.reserve(expected_count);
    }

    // move_only_function이 복사 불가이므로 이를 담은 vector와 Queue도 자연스럽게 복사할 수 없다.
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;
    TaskQueue(TaskQueue&&) = default;
    TaskQueue& operator=(TaskQueue&&) = default;

    // 값 매개변수는 호출 대상 소유권을 함수 경계에서 인수하고, 반환 bool로 등록 성공을 알린다.
    [[nodiscard]] bool enqueue(Task task) {
        // move_only_function::operator bool() const noexcept는 인자 없이 호출 대상 보유 여부를 반환하고 task를 바꾸지 않는다.
        // bool prvalue를 조건에 즉시 쓰며 비어 있으면 저장소와 소유권을 건드리지 않고 false를 반환한다.
        if (!task) {
            return false;
        }
        // vector<Task>::push_back(Task&&)은 std::move(task) xvalue가 가진 호출 대상 소유권을 새 끝 원소로 이동한다.
        // void 반환은 버린다. 성공 뒤 size가 1 늘고 task는 유효하지만 비어 있을 수 있으며 상각 O(1)이다.
        // 예약 용량 안에서는 재할당이 없고, 부족하면 모든 기존 포인터·참조·반복자가 무효화되며 bad_alloc이 가능하다.
        tasks_.push_back(std::move(task));
        return true;
    }

    [[nodiscard]] std::size_t run_all() {
        std::size_t executed{}; // size_t{}는 0이고 실행한 작업 수를 음수 없이 센다.
        // 범위 for의 Task&는 vector 원소를 빌린 lvalue 참조다. 반복 중 vector 크기를 바꾸지 않아 참조가 안정적이다.
        for (Task& task : tasks_) {
            // move_only_function<void()>::operator()()은 데이터 인자 없이 저장된 호출 대상을 간접 호출하고 void를 반환한다.
            // task는 비어 있지 않다는 enqueue 불변식을 만족한다. 호출 대상 내부 상태는 바뀔 수 있고 예외는 그대로 전파된다.
            // 타입 소거 간접 호출·할당 여부는 구현과 최적화에 따라 달라지며 스레드 안전을 별도로 제공하지 않는다.
            task();
            ++executed; // 전위 ++는 실행 횟수를 1 저장한 뒤 갱신된 lvalue를 반환하지만 값은 사용하지 않는다.
        }
        // vector::clear()는 인자 없이 모든 Task를 파괴하고 size를 0으로 만들며 void를 반환한다.
        // O(N)이고 각 Task가 캡처한 unique_ptr 등 자원을 해제한다. capacity 반환은 보장하지 않으며 원소 관찰자는 모두 무효다.
        tasks_.clear();
        return executed; // size_t 값을 반환 prvalue로 전달하며 호출자 결과 객체를 직접 초기화할 수 있다.
    }

private:
    std::vector<Task> tasks_{}; // Queue가 등록된 호출 대상의 수명과 실행 순서를 독점 관리한다.
};

int main() {
    DeploymentLedger ledger{2}; // 직접 초기화는 explicit 생성자를 올바르게 선택해 초기 복제본 2를 저장한다.
    TaskQueue queue{2U};         // size_t로 변환 가능한 2U를 직접 초기화 인자로 넘겨 두 작업 용량을 예약한다.

    // make_unique<DeploymentRequest>(DeploymentRequest&&)는 집합체 prvalue를 동적 생성 객체로 이동 구성한다.
    // 반환 unique_ptr prvalue가 request를 직접 초기화하고 독점 소유한다. 한 번 할당하며 생성 실패 시 bad_alloc이 가능하다.
    auto request{std::make_unique<DeploymentRequest>(DeploymentRequest{std::string{"api"}, 3})};

    // std::move(request)는 unique_ptr lvalue를 xvalue로 바꾼다. 람다 init-capture가 독점 소유권을 이동해 request는 빈다.
    // 람다 prvalue는 move-only이고 Task 생성자 template<class F> move_only_function(F&&)가 이를 소유해 타입 소거한다.
    // Task prvalue는 enqueue(Task) 값 매개변수를 직접 초기화한다. 호출 전 queue size는 0이고 인자 소유권은 매개변수로 이동한다.
    // 반환 bool은 first_added에 저장한다. 성공 뒤 queue size는 1이고 ledger는 참조 캡처라 실행까지 살아 있어야 한다.
    const bool first_added{queue.enqueue(TaskQueue::Task{
        [owned = std::move(request), &ledger] { ledger.apply(*owned); }})};

    // 두 번째 make_unique는 worker 요청을 새로 소유하고 캡처 이동 뒤 임시 unique_ptr는 비게 된다.
    // enqueue 호출 전 size는 1이고 Task prvalue 소유권을 값 매개변수로 옮긴다. 반환 bool은 second_added에 저장한다.
    // 성공 뒤 size는 2이며 queue는 서로 다른 람다 타입을 같은 Task 타입으로 등록 순서대로 소유한다.
    const bool second_added{queue.enqueue(TaskQueue::Task{
        [owned = std::make_unique<DeploymentRequest>(
             DeploymentRequest{std::string{"worker"}, 4}),
         &ledger] { ledger.apply(*owned); }})};

    // run_all()은 데이터 인자 없이 두 Task를 등록 순서로 호출하고 지운 뒤 실행 횟수 size_t를 반환한다.
    // 반환값을 executed에 저장하며 호출 뒤 queue는 empty지만 예약 capacity는 남을 수 있고 ledger 총량은 9가 된다.
    const std::size_t executed{queue.run_all()};

    // operator<<는 각 인자를 형식화해 cout 버퍼·상태를 갱신하고 ostream&를 연쇄 반환한다. 최종 참조는 버린다.
    // last_service()가 반환한 const string&는 이 전체 식 동안 살아 있는 ledger 내부 문자열 "worker"를 빌린다.
    std::cout << executed << ' ' << ledger.total_replicas() << ' ' << ledger.last_service() << '\n';

    // unique_ptr 비멤버 operator==(const unique_ptr&, nullptr_t)는 request를 빌려 빈 상태 bool을 O(1)에 반환하고 바꾸지 않는다.
    // string과 const char* 비교 연산자는 last_service()의 내부 문자열을 읽어 길이에 선형으로 bool을 반환하며 두 입력은 유지된다.
    // &&는 왼쪽부터 단락 평가한다. 이름 있는 bool/size_t/ledger는 모두 lvalue이고 비교 결과는 bool prvalue다.
    return first_added && second_added && request == nullptr && executed == 2U &&
                   ledger.total_replicas() == 9 && ledger.last_service() == "worker"
               ? 0
               : 1;
}
