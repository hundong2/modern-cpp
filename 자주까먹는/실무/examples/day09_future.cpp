#include <future>    // std::packaged_task와 std::future를 제공한다.
#include <iostream>  // 비동기 결과를 출력한다.
#include <thread>    // 교육용 worker thread를 제공한다.
#include <utility>   // std::move를 제공한다.

int main() {                                        // future shared state 수명을 관찰한다.
    std::packaged_task<int()> task([] {             // int 결과 또는 예외를 저장할 작업을 만든다.
        return 6 * 7;                               // worker가 계산해 shared state에 42를 기록한다.
    });                                             // callable이 packaged_task 안에 소유된다.
    std::future<int> result = task.get_future();    // 결과를 한 번 소비할 future를 얻는다.
    std::thread worker(std::move(task));            // 복사 불가 task를 worker로 이동해 실행한다.
    worker.join();                                  // thread 객체의 수명을 안전하게 끝낸다.
    const int value = result.get();                 // 준비된 값 또는 저장된 예외를 한 번 가져온다.
    std::cout << value << '\n';                    // 42를 출력한다.
    return value == 42 ? 0 : 1;                    // 자체 검사 결과를 반환한다.
}                                                  // 이미 소비한 future와 worker 객체를 파괴한다.
