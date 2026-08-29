#include <cassert>             // assert로 게시 전후의 값 계약을 검증한다.
#include <condition_variable>  // std::condition_variable로 결과 준비를 기다린다.
#include <iostream>            // std::cout 출력 객체를 사용한다.
#include <mutex>               // std::mutex와 RAII 잠금 객체를 사용한다.
#include <optional>            // 결과가 아직 없음을 std::optional로 표현한다.
#include <string>              // 결과 문자열 버퍼를 소유한다.
#include <thread>              // std::jthread로 생산자 작업 수명을 소유한다.
#include <utility>             // std::move로 결과 소유권을 슬롯에 넘긴다.

// class는 멤버가 기본 private라 result_와 ready_를 잠금 없이 건드리는 잘못된 상태를 숨긴다.
class ResultSlot {
public:
    // string 값 매개변수는 호출자와 독립된 소유본이며 prvalue/rvalue에서는 이동으로 초기화될 수 있다.
    void publish(std::string value) {
        {
            // scoped_lock 생성자는 mutex_ lvalue를 잠그고 소멸자는 예외가 있어도 해제한다. 반환값은 없다.
            std::scoped_lock lock{mutex_};
            // optional::emplace(Args&&...)는 string 생성자 인자로 value xvalue를 전달해 내부에 직접 객체를 만든다.
            // string&를 반환하지만 사용하지 않는다. 기존 값이 있으면 먼저 파괴되며 새 생성 실패 시 optional은 비어 있을 수 있다.
            result_.emplace(std::move(value));
            ready_ = true;
        }
        // notify_one은 인자·반환값 없이 대기자 하나를 깨운다. mutex로 게시된 result_를 직접 복사하거나 변경하지 않는다.
        ready_changed_.notify_one();
    }

    [[nodiscard]] std::string wait() {
        std::unique_lock<std::mutex> lock{mutex_};
        // wait(lock,predicate)는 lock을 원자적으로 풀고 기다린 뒤 다시 잠가 ready_를 검사한다. 허위 깨움은 반복 검사한다.
        // lambda의 this는 비소유 포인터이고 ResultSlot은 대기보다 오래 살아야 한다. 반환형은 void이며 성공 뒤 lock이 mutex를 소유한다.
        ready_changed_.wait(lock, [this] { return ready_; });
        // optional::value() &는 저장 string의 lvalue 참조를 반환하고, 빈 상태면 bad_optional_access를 던진다.
        // ready_ 불변식이 값 존재를 보장한다. move가 xvalue로 바꾸고 반환 string이 버퍼를 이어받아 slot 내부 string은 유효하지만 값이 미지정이다.
        return std::move(result_.value());
    }

private:
    // mutex/condition_variable 기본 생성은 잠기지 않고 대기자 없는 객체를, optional{}은 값 없는 객체를 만든다.
    // 각 객체는 ResultSlot이 소유하며 파괴 전 모든 접근 스레드가 join되어야 한다. 기본 생성의 별도 반환값은 없다.
    std::mutex mutex_{};
    std::condition_variable ready_changed_{};
    std::optional<std::string> result_{};
    bool ready_{};
};

int main() {
    // ResultSlot{}은 멤버를 기본 초기화한 prvalue로 slot을 직접 초기화하고 별도 복사 없이 이 scope에서 소유한다.
    ResultSlot slot{};
    // lambda prvalue는 slot을 비소유 참조로 캡처하고 jthread가 호출 객체와 실행 스레드를 소유한다.
    std::jthread producer{[&slot] {
        // string prvalue가 값 매개변수를 직접 초기화하고 publish 안에서 슬롯로 이동된다.
        slot.publish(std::string{"ready"});
    }};

    // wait의 string prvalue는 result를 직접 초기화할 수 있다. 호출은 준비될 때까지 블록될 수 있다.
    const std::string result{slot.wait()};
    // join()은 데이터 인자·반환값 없이 producer 종료를 기다리고 연결을 해제한다. 비joinable/자기 합류면 system_error다.
    // 성공한 join 뒤 producer는 joinable하지 않고 작업 평가가 이 줄 이후보다 먼저 일어나며 slot/result는 그대로 유지된다.
    producer.join();
    assert(result == "ready");

    // 출력 연산은 result를 읽을 뿐 소유권과 값은 유지하며 ostream& 반환값은 연쇄 뒤 버린다.
    std::cout << result << '\n';
    return 0;
}
