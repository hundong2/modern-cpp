#include <condition_variable> // worker를 busy waiting 없이 재운다.
#include <functional>         // 타입 지운 작업 std::function<void()>를 제공한다.
#include <iostream>           // 실행 결과를 출력한다.
#include <mutex>              // queue와 종료 상태를 보호한다.
#include <queue>              // FIFO 작업 대기열을 제공한다.
#include <stdexcept>          // 종료 후 제출 오류를 표현한다.
#include <thread>             // worker 실행 흐름을 제공한다.
#include <vector>             // worker 객체들을 소유한다.

class ThreadPool {                                      // 제한된 worker가 작업을 소비하는 풀이다.
public:                                                 // 생성·제출 API를 공개한다.
    explicit ThreadPool(std::size_t count) {            // 생성 시 고정 worker 수를 받는다.
        for (std::size_t i = 0; i < count; ++i) {       // 요청한 수만큼 thread를 만든다.
            workers_.emplace_back([this] { run(); });   // this는 소멸자 join 전까지 유효하다.
        }                                               // 모든 worker가 대기 루프에 들어간다.
    }                                                   // pool이 완전히 생성된다.

    ~ThreadPool() {                                     // worker보다 pool 상태가 먼저 파괴되지 않게 정리한다.
        {                                               // 잠금 수명을 짧게 제한하는 블록이다.
            std::lock_guard<std::mutex> lock(mutex_);   // stopping_ 변경을 queue와 동기화한다.
            stopping_ = true;                           // 새 제출을 막고 drain 후 종료를 요청한다.
        }                                               // notify 전에 mutex를 풀어 경쟁을 줄인다.
        ready_.notify_all();                            // 잠든 모든 worker가 종료 조건을 보게 한다.
        for (auto& worker : workers_) {                 // 생성한 모든 thread를 순회한다.
            worker.join();                              // run이 끝날 때까지 기다려 안전한 파괴를 보장한다.
        }                                               // 모든 worker가 끝났다.
    }                                                   // 이제 queue/mutex를 파괴해도 안전하다.

    void submit(std::function<void()> task) {           // 반환값 없는 교육용 작업을 값으로 받는다.
        {                                               // queue 변경 임계 구역이다.
            std::lock_guard<std::mutex> lock(mutex_);   // submit끼리와 worker pop을 직렬화한다.
            if (stopping_) throw std::runtime_error("pool stopped"); // 종료 후 제출을 거부한다.
            tasks_.push(std::move(task));               // callable의 소유권을 queue로 이동한다.
        }                                               // worker를 깨우기 전에 mutex를 푼다.
        ready_.notify_one();                            // 작업 하나를 처리할 worker 하나를 깨운다.
    }                                                   // 제출자는 작업 완료를 기다리지 않는다.

private:                                                // worker 구현과 공유 상태를 감춘다.
    void run() {                                        // 각 worker가 반복 실행할 함수다.
        while (true) {                                  // stop+empty까지 작업을 계속 소비한다.
            std::function<void()> task;                 // 잠금 밖에서 실행할 작업 소유자다.
            {                                           // queue 접근 임계 구역을 시작한다.
                std::unique_lock<std::mutex> lock(mutex_); // condition_variable이 unlock할 수 있는 lock이다.
                ready_.wait(lock, [this] { return stopping_ || !tasks_.empty(); }); // spurious wakeup을 predicate로 막는다.
                if (stopping_ && tasks_.empty()) return; // 남은 작업을 drain한 뒤 worker를 끝낸다.
                task = std::move(tasks_.front());       // 앞 작업을 지역 변수로 이동한다.
                tasks_.pop();                           // queue에서 소유권 없는 빈 항목을 제거한다.
            }                                           // 긴 작업 실행 전에 mutex를 반드시 푼다.
            task();                                     // 서로 다른 worker가 작업을 병렬 실행할 수 있다.
        }                                               // 다음 작업 또는 종료 신호를 기다린다.
    }                                                   // worker loop 정의를 끝낸다.

    std::mutex mutex_;                                  // tasks_와 stopping_을 함께 보호한다.
    std::condition_variable ready_;                     // 상태 변경을 worker에게 알린다.
    std::queue<std::function<void()>> tasks_;           // 아직 실행하지 않은 작업을 소유한다.
    std::vector<std::thread> workers_;                  // join 책임이 있는 thread들을 소유한다.
    bool stopping_ = false;                             // mutex_ 아래에서만 읽고 쓰는 종료 상태다.
};                                                      // pool 정의를 끝낸다.

int main() {                                            // 작은 pool의 drain 동작을 시험한다.
    std::mutex output_mutex;                            // std::cout 복합 출력을 보호한다.
    {                                                   // pool 소멸 시점을 명확히 하는 블록이다.
        ThreadPool pool{2};                             // worker 두 개를 시작한다.
        for (int id = 0; id < 4; ++id) {               // 네 작업을 제출한다.
            pool.submit([id, &output_mutex] {           // id는 값, mutex는 유효한 참조로 캡처한다.
                std::lock_guard<std::mutex> lock(output_mutex); // 한 줄 로그가 섞이지 않게 한다.
                std::cout << "task " << id << '\n';   // 실행 순서는 보장되지 않음을 관찰한다.
            });                                         // callable 소유권이 pool queue로 이동한다.
        }                                               // 제출을 끝낸다.
    }                                                   // 소멸자가 네 작업을 drain하고 worker를 join한다.
    return 0;                                           // 정상 종료한다.
}                                                       // output mutex를 파괴한다.
