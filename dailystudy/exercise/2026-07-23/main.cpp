#include <chrono>      // std::chrono::milliseconds로 대기 시간을 표현한다.
#include <iostream>    // std::cout 표준 출력 스트림을 사용한다.
#include <memory>      // std::unique_ptr와 std::make_unique로 독점 소유권을 나타낸다.
#include <stop_token>  // std::stop_token으로 협력적 중지 요청을 관찰한다.
#include <thread>      // C++20 std::jthread와 std::this_thread::sleep_for를 사용한다.
#include <utility>     // std::move로 lvalue를 xvalue로 변환한다.

// struct의 기본 접근은 public이다. 단순 결과 데이터는 공개 멤버가 자연스럽다.
struct JobResult {
    int completed_steps{};  // int 기본 타입 멤버를 중괄호로 0 초기화한다.
    bool stopped{};         // bool은 참/거짓을 저장하며 여기서는 false로 초기화된다.
};

// class의 기본 접근은 private이다. public 인터페이스와 구현을 접근 지정자로 나눈다.
class IJob {
public:
    virtual ~IJob() = default;  // 기반 클래스 포인터로 파괴할 때 파생 소멸자를 호출한다.

    // int가 반환형, current가 값 매개변수다. = 0은 파생 클래스가 구현할 순수 가상 함수다.
    [[nodiscard]] virtual int step(int current) const = 0;
};

class CountingJob final : public IJob {
public:
    // 생성자에는 반환형이 없다. explicit은 int 하나가 객체로 암시 변환되는 일을 막는다.
    explicit CountingJob(int limit)
        : limit_{limit} {}  // 생성자 매개변수 limit로 private 멤버를 직접 초기화한다.

    [[nodiscard]] int step(int current) const override {
        // + 연산자는 두 정수를 더한다. const 함수이므로 limit_를 바꾸지 않는다.
        return current < limit_ ? current + 1 : current;
    }

private:
    int limit_;  // 멤버 변수는 작업이 도달할 상한을 저장한다.
};

// using은 긴 템플릿 타입에 별칭을 붙인다. 소유권 의미는 unique_ptr 그대로다.
using JobOwner = std::unique_ptr<IJob>;

class JobRunner {
public:
    // 생성자에는 반환형이 없고, JobOwner 매개변수를 값으로 받아 소유권을 이전받는다.
    explicit JobRunner(JobOwner job)
        : job_{std::move(job)} {}  // std::move(job)은 xvalue이며 unique_ptr 이동 생성이 일어난다.

    // JobRunner는 독점 소유자를 복사할 수 없으므로 복사를 명시적으로 금지한다.
    JobRunner(const JobRunner&) = delete;
    JobRunner& operator=(const JobRunner&) = delete;
    JobRunner(JobRunner&&) noexcept = default;
    JobRunner& operator=(JobRunner&&) noexcept = default;

    [[nodiscard]] JobResult run() {
        JobResult result{};  // 이름 있는 지역 객체 result는 lvalue이며 두 멤버가 0/false가 된다.

        // &result와 this 캡처는 기존 객체를 비소유로 참조한다. worker가 합류하기 전까지 살아 있어야 한다.
        // jthread 생성자는 호출할 람다를 입력받고 새 실행 스레드를 소유한다. 람다가 stop_token을 첫 인자로 받을 수 있어 자동 전달된다.
        // 생성자는 반환값이 없고 스레드 생성 실패 시 system_error가 가능하다. worker 소멸자는 중지 요청 후 join한다.
        std::jthread worker{[this, &result](std::stop_token token) {
            // *job_은 IJob lvalue를 만든다. const 참조는 그 기존 객체에 바인딩되며 소유하지 않는다.
            const IJob& job{*job_};

            // &&는 왼쪽부터 평가하고 거짓이면 오른쪽 비교를 생략한다.
            // stop_requested()는 인자가 없고 요청 여부 bool을 반환하며 token이나 작업을 중단시키지는 않는다.
            while (!token.stop_requested() && result.completed_steps < 5) {
                // 가상 함수 호출은 실제 CountingJob::step을 선택하는 간접 호출이 될 수 있다.
                result.completed_steps = job.step(result.completed_steps);
                // sleep_for(duration)는 1ms 시간값 하나를 입력받고 void를 반환한다. 최소 그 기간 현재 스레드를 쉬게 하지만 더 늦게 깰 수 있다.
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }
            result.stopped = token.stop_requested();  // 함수 호출 결과를 bool 멤버에 저장한다.
        }};

        // request_stop()은 인자 없이 중지 상태를 처음 요청했는지 bool을 반환한다. 여기서는 반환값을 버리며 강제 종료하지 않는다.
        worker.request_stop();
        // join()은 인자·반환값이 없고 작업 스레드 종료까지 기다린다. 자기 자신 join 등 오류에서는 system_error가 가능하다.
        worker.join();
        return result;          // 반환 prvalue 초기화에는 복사 생략 또는 작은 결과 복사가 적용된다.
    }

private:
    JobOwner job_;  // private 멤버가 구체 작업 객체의 수명과 독점 소유권을 가진다.
};

JobRunner make_runner() {
    // <CountingJob>은 make_unique의 템플릿 인자다. explicit 생성자는 직접 초기화로 올바르게 호출된다.
    // make_unique<CountingJob>(5)는 정수 5를 생성자 인자로 전달해 객체를 만들고 unique_ptr<CountingJob> prvalue를 반환한다.
    return JobRunner{std::make_unique<CountingJob>(5)};
}

int main() {
    JobRunner runner{make_runner()};  // 반환 prvalue로 직접 초기화되어 불필요한 복사를 피한다.
    const JobResult result{runner.run()};  // const 지역 결과는 초기화 뒤 변경할 수 없다.

    // << 연산자는 값을 스트림에 차례로 보내고, 조건 연산자 ?:는 두 문자열 중 하나를 고른다.
    std::cout << "완료 단계: " << result.completed_steps
              << ", 중지 요청 관찰: " << (result.stopped ? "예" : "아니오") << '\n';
    return 0;  // main의 int 반환값 0은 정상 종료를 뜻한다.
}
