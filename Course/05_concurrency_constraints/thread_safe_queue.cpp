#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

class ThreadSafeQueue {
public:
    void push(int value) {
        {
            std::lock_guard<std::mutex> lock{mutex_};
            queue_.push(value);
        }
        condition_.notify_one();
    }

    std::optional<int> wait_and_pop() {
        std::unique_lock<std::mutex> lock{mutex_};

        condition_.wait(lock, [this] {
            return closed_ || !queue_.empty();
        });

        if (queue_.empty()) {
            return std::nullopt;
        }

        const int value = queue_.front();
        queue_.pop();
        return value;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock{mutex_};
            closed_ = true;
        }
        condition_.notify_all();
    }

private:
    // queue_, closed_는 여러 스레드가 공유하는 메모리입니다.
    // mutex_를 잡지 않고 접근하면 데이터 경합이 발생할 수 있습니다.
    std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<int> queue_;
    bool closed_{false};
};

int main() {
    ThreadSafeQueue queue;

    std::thread producer{[&queue] {
        for (int value = 1; value <= 5; ++value) {
            queue.push(value);
        }
        queue.close();
    }};

    std::thread consumer{[&queue] {
        while (true) {
            const auto value = queue.wait_and_pop();
            if (!value) {
                break;
            }
            std::cout << "consume: " << *value << '\n';
        }
    }};

    producer.join();
    consumer.join();
}

