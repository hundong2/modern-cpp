#include <cassert>             // assert 매크로로 큐의 종료 후 결과 불변식을 검증한다.
#include <condition_variable>  // std::condition_variable의 술어 대기와 깨우기를 사용한다.
#include <cstddef>             // std::size_t로 용량과 원소 수를 표현한다.
#include <deque>               // std::deque가 양끝 삽입·삭제 가능한 작업 저장소를 소유한다.
#include <iostream>            // std::cout과 출력 operator<< 선언을 사용한다.
#include <mutex>               // std::mutex와 std::unique_lock으로 공유 상태를 보호한다.
#include <optional>            // std::optional<Job>으로 정상 종료와 작업 반환을 구분한다.
#include <string>              // std::string이 작업 payload 문자를 독점 소유한다.
#include <thread>              // std::jthread가 소비자 스레드 수명을 RAII로 소유한다.
#include <utility>             // std::move로 문자열과 작업 소유권을 이전한다.

// struct는 기본 접근이 public이다. 검증된 작업 payload만 묶는 단순 자료 객체에 알맞다.
struct Job {
    // int는 작업 식별자를 값으로 저장하고 {}는 0으로 값 초기화한다.
    int id{};
    // string은 문자 버퍼를 소유하며 Job의 수명과 함께 정리된다.
    std::string payload{};
};

// class는 기본 접근이 private이다. mutex와 조건 변수의 사용 순서를 외부에서 깨지 못하게 감춘다.
class BlockingJobQueue {
public:
    // 생성자는 반환형이 없다. explicit은 정수 하나가 큐로 암시 변환되는 것을 막고 BlockingJobQueue{3}만 허용한다.
    explicit BlockingJobQueue(std::size_t capacity) : capacity_{capacity} {
        // > 연산은 size_t 값을 비교해 bool prvalue를 만든다. 용량 0이면 생산자는 영원히 진행할 수 있으므로 금지한다.
        assert(capacity_ > 0U);
    }

    // Job 값 매개변수는 호출자로부터 독립된 소유본이다. rvalue 호출은 payload 버퍼를 이동할 수 있다.
    [[nodiscard]] bool push(Job job) {
        // unique_lock<mutex>(mutex&)는 mutex_ lvalue를 비소유 참조해 잠근다. lock은 이 함수 scope에서 잠금 소유권을 가진다.
        // mutex 잠금은 실패 시 system_error를 던질 수 있고, 소멸자는 잠겼다면 해제한다. 공정성·대기 상한·특정 CPU 명령은 보장하지 않는다.
        std::unique_lock<std::mutex> lock{mutex_};

        // 수신 객체는 not_full_ condition_variable이고 첫 인자는 잠금을 소유한 unique_lock lvalue다.
        // wait<Predicate>(lock, pred)는 pred가 true일 때까지 원자적으로 mutex를 풀어 대기하고 깨면 다시 잠가 검사한다.
        // lambda는 this를 비소유 포인터로 캡처해 closed_와 jobs_를 읽는다. 반환형은 void이며 깨어난 뒤 lock은 mutex를 소유한다.
        // 허위 깨움은 술어 재검사로 흡수한다. 큐 객체는 대기보다 오래 살아야 하며 예외 시 lock RAII가 mutex를 해제한다.
        not_full_.wait(lock, [this] {
            // deque::size()는 size_type을 O(1)에 반환하고 수신 deque를 바꾸지 않는다. 잠금 아래라 동시 접근과 경쟁하지 않는다.
            return closed_ || jobs_.size() < capacity_;
        });

        if (closed_) {  // 닫힌 뒤 새 작업을 받지 않는 종료 불변식을 조건 분기로 지킨다.
            return false;
        }

        // 수신 객체는 잠금 아래의 deque<Job> jobs_다. push_back(Job&&)의 인자는 std::move(job) xvalue다.
        // 반환형은 void이고 job의 payload 소유권을 새 끝 원소로 옮긴다. 뒤에서 job은 유효하지만 값이 미지정이다.
        // 평균 O(1)이며 새 블록 할당이 실패하면 bad_alloc을 던질 수 있다. deque 삽입은 기존 반복자를 무효화할 수 있으나 기존 원소 참조는 유지된다.
        jobs_.push_back(std::move(job));

        // 수신 condition_variable에는 데이터 인자·반환값이 없다. 소비자 하나를 깨울 수 있지만 즉시 실행·공정성은 보장하지 않는다.
        // 공유 상태 변경은 mutex가 게시하므로 notify 자체가 payload를 소유하거나 큐를 바꾸지 않는다. 잠금 해제 뒤에도 호출할 수 있지만 여기서는 단순성을 택한다.
        not_empty_.notify_one();
        return true;
    }

    [[nodiscard]] std::optional<Job> pop() {
        std::unique_lock<std::mutex> lock{mutex_};

        // wait(lock,pred)의 pred는 인자 없이 bool을 반환한다. 작업이 생기거나 close된 경우에만 반환해 허위 깨움과 lost wakeup을 막는다.
        // 반환 뒤 mutex를 다시 소유하므로 jobs_.front/pop_front와 close 상태 판정을 한 임계 구역에서 수행한다.
        not_empty_.wait(lock, [this] {
            // empty()는 deque가 비었는지 bool로 O(1)에 돌려주며 상태를 바꾸지 않는다.
            return closed_ || !jobs_.empty();
        });

        if (jobs_.empty()) {
            // 빈 optional prvalue는 “닫혔고 남은 작업 없음”을 나타낸다. 반환 객체는 호출자 저장소에 직접 생성될 수 있다.
            return std::nullopt;
        }

        // front()는 첫 Job lvalue 참조를 O(1)에 반환한다. 비어 있지 않다는 전제조건은 바로 위 분기가 보장한다.
        // move는 그 lvalue를 xvalue로 바꾸고 local Job이 payload를 이어받는다. 실제 이동은 Job 이동 생성자가 한다.
        Job job{std::move(jobs_.front())};
        // pop_front()는 반환값 없이 이동된 첫 원소를 파괴하고 크기를 1 줄인다. 그 원소의 참조/포인터는 무효가 되며 다른 원소는 유지된다.
        jobs_.pop_front();
        // 공간을 기다리던 생산자 하나를 깨운다. 큐 상태와 인자 객체는 이 호출 자체로 더 바뀌지 않는다.
        not_full_.notify_one();
        // Job prvalue가 optional<Job> 반환값을 직접 초기화한다. 지역 job의 payload는 반환 객체로 이동될 수 있다.
        return job;
    }

    void close() {
        {
            // scoped_lock(mutex&)는 mutex_를 즉시 잠그고 scope 끝에서 해제하는 이동·복사 불가 RAII 객체다.
            std::scoped_lock lock{mutex_};
            closed_ = true;  // bool 저장은 잠금 아래에서 이루어져 대기 술어와 데이터 경쟁하지 않는다.
        }
        // notify_all()은 데이터 인자와 반환값 없이 모든 대기자를 깨워 종료 술어를 다시 검사하게 한다.
        // 큐·closed_를 직접 바꾸지 않고, 깨운 스레드가 언제 mutex를 얻는지나 실행 순서는 보장하지 않는다.
        not_empty_.notify_all();
        not_full_.notify_all();
    }

private:
    const std::size_t capacity_{};  // const 멤버는 생성 뒤 바뀌지 않는 용량 불변식을 표현한다.
    // mutex와 condition_variable의 인자 없는 기본 생성은 잠기지 않고 대기자 없는 객체를 만들며 반환값·동적 할당 보장은 없다.
    std::mutex mutex_{};            // 아래 공유 상태와 술어 평가를 하나의 잠금으로 보호한다.
    std::condition_variable not_empty_{};
    std::condition_variable not_full_{};
    // deque<Job>{}는 크기 0인 소유 컨테이너, bool{}은 false를 만든다. deque 내부 할당 시점은 구현 세부다.
    std::deque<Job> jobs_{};
    bool closed_{};
};

int main() {
    // size_t prvalue 3으로 explicit 생성자를 직접 호출한다.
    BlockingJobQueue queue{3U};
    // string{} 기본 생성자는 길이 0인 소유 문자열을 만들며 별도 반환값이 없다. 구현의 small-string 저장 여부는 보장되지 않는다.
    std::string consumed{};

    // jthread(F&&)는 lambda prvalue를 이동 소유하고 새 실행 스레드를 시작한다. queue/consumed는 비소유 참조 캡처다.
    // 생성 실패 시 system_error가 가능하고, 두 참조 대상은 join 완료까지 살아 있어야 한다.
    std::jthread worker{[&queue, &consumed] {
        // pop 결과 optional prvalue를 job에 직접 초기화한다. 조건식은 has_value와 동등하게 값 존재를 검사한다.
        // optional<Job>(Job&&)은 pop의 Job 소유값을 이동 보관한다. 조건 변환은 has_value를 검사하고 optional을 바꾸지 않는다.
        while (std::optional<Job> job{queue.pop()}) {
            // append(const string&)는 job->payload lvalue를 읽어 consumed 뒤에 복사한다. 반환 string&는 사용하지 않는다.
            // 선형 시간이며 필요 시 재할당해 consumed의 기존 포인터·참조·반복자를 무효화할 수 있고, 실패하면 bad_alloc을 던질 수 있다.
            consumed.append(job->payload);
        }
    }};

    // Job{...}은 prvalue이고 push 값 매개변수를 직접 초기화한다. bool 반환값을 assert가 검사한다.
    assert(queue.push(Job{1, "alpha"}));
    assert(queue.push(Job{2, "+beta"}));
    queue.close();

    // join()은 인자·반환값 없이 worker 종료를 기다리고 스레드 연결을 해제한다. 성공 뒤 joinable()==false다.
    // 자기 합류/비joinable이면 system_error이며, 성공한 join은 worker의 consumed 쓰기를 이후 읽기와 동기화한다.
    worker.join();
    assert(consumed == "alpha+beta");

    // ostream operator<<(value)는 consumed를 복사 소유하지 않고 출력 버퍼에 기록한 뒤 같은 ostream&를 반환한다.
    // consumed는 유지되고 I/O 실패는 상태 비트를 세우며 기본 설정에서는 예외를 던지지 않는다.
    std::cout << consumed << '\n';
    return 0;
}
