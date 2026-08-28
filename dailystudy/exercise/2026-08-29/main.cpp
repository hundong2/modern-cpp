#include <cassert>    // assert 매크로로 학습 예제의 사후조건을 디버그 빌드에서 확인한다.
#include <iostream>   // std::cout 출력 객체와 operator<< 선언을 사용한다.
#include <semaphore>  // C++20 std::binary_semaphore의 release/acquire 동기화를 사용한다.
#include <string>     // endpoint 문자를 소유하는 std::string을 사용한다.
#include <thread>     // 소멸 시 합류하는 std::jthread를 사용한다.
#include <utility>    // std::move로 소유 문자열을 이동 가능한 xvalue 식으로 바꾼다.

// struct는 별도 접근 지정자가 없으면 멤버가 public이다. 단순 전달 자료 객체(DTO)에 알맞다.
struct ConfigUpdate {
    // int는 버전을 값으로 저장하는 기본 정수 타입이고 {}는 0으로 값 초기화한다.
    int version{};
    // string은 문자 버퍼를 독점 소유하며 ConfigUpdate의 수명과 함께 파괴된다.
    std::string endpoint{};
};

// class는 기본 접근이 private이다. 슬롯과 동기화 규칙을 외부에서 함부로 깨지 못하게 숨긴다.
class SingleUpdateMailbox {
public:
    // 생성자는 반환형이 없다. 멤버 초기화 목록에서 counter가 0인 닫힌 세마포어를 직접 초기화한다.
    SingleUpdateMailbox() : ready_{0}, slot_{} {}

    // 값 매개변수 update는 호출자가 준 객체의 독립 소유본이며, rvalue면 이동 생성될 수 있다.
    void publish(ConfigUpdate update) {
        // std::move(update)는 이름 있는 lvalue를 xvalue로 바꾼다. ConfigUpdate의 이동 대입이 endpoint 버퍼 소유권을 slot_로 넘긴다.
        // 반환형은 ConfigUpdate&지만 사용하지 않는다. 뒤에는 slot_가 새 값을 소유하고 update는 유효하지만 값이 미지정이다.
        slot_ = std::move(update);

        // 수신 객체는 counter 0인 std::binary_semaphore다. release(update=1)의 기본 인자 1을 쓰며 반환형은 void라 무시할 값이 없다.
        // counter는 1이 되고 대기자 하나가 진행할 수 있다. 이 호출 전 slot_ 쓰기는 성공한 acquire 뒤 읽기와 동기화한다.
        // counter가 이미 1일 때 다시 release하면 전제조건 위반이므로 이 클래스는 미수신 갱신 하나만 허용한다. 할당·공정성·특정 CPU 명령은 보장하지 않는다.
        ready_.release();
    }

    [[nodiscard]] ConfigUpdate wait() {
        // 수신 객체는 공유 중지 상태가 아닌 std::binary_semaphore다. acquire()는 데이터 인자와 반환값 없이 counter가 양수가 될 때까지 기다린다.
        // 성공하면 counter를 1 줄여 0으로 만들고 publish의 release와 동기화한다. 대기 시간 상한과 공정성은 보장하지 않는다.
        ready_.acquire();

        // slot_은 이름 있는 lvalue이고 move 결과는 xvalue다. 반환 객체가 문자열 버퍼를 이동 소유하며 slot_은 유효하지만 값이 미지정이다.
        // 반환 prvalue는 호출자의 결과 객체를 직접 초기화할 수 있어 불필요한 임시 복사를 피한다.
        return std::move(slot_);
    }

private:
    // binary_semaphore는 counting_semaphore<1>의 표준 별칭이며 이 객체는 슬롯 준비 여부만 소유한다.
    std::binary_semaphore ready_;
    // payload는 mailbox가 소유한다. publish와 wait를 동시에 여러 번 부르는 범용 큐가 아니라 단일 생산·단일 소비 예제다.
    ConfigUpdate slot_;
};

int main() {
    // {} 직접 초기화는 기본 생성자를 호출해 닫힌 mailbox를 만든다.
    SingleUpdateMailbox mailbox{};
    // 결과 객체는 main 스택에서 소유하고 worker가 join되기 전까지 살아 있다.
    ConfigUpdate received{};

    // jthread(F&&)는 prvalue lambda를 이동 소유하고 새 스레드에서 호출한다. lambda는 mailbox와 received를 비소유 참조로 캡처한다.
    // 생성된 worker는 실행 스레드 하나를 소유하며 실패하면 system_error를 던질 수 있다. 참조 대상은 join까지 살아 있어야 한다.
    std::jthread worker{[&mailbox, &received] {
        // wait() 반환 prvalue가 received의 이동 대입 입력이 된다. acquire 뒤라 slot_의 비원자 쓰기를 안전하게 관찰한다.
        received = mailbox.wait();
    }};

    // ConfigUpdate{...}는 prvalue다. publish의 값 매개변수를 직접 초기화하고 endpoint 문자열은 mailbox로 두 번 이동될 수 있다.
    mailbox.publish(ConfigUpdate{7, "edge.internal"});

    // 수신 객체는 joinable한 std::jthread worker다. join()은 인자·반환값 없이 작업 종료까지 기다리고 연결을 해제한다.
    // 성공 뒤 worker.joinable()==false이며 worker의 모든 쓰기는 이 줄 뒤 main의 읽기보다 먼저 일어난다. 자기 합류나 비joinable 호출은 system_error다.
    worker.join();

    // == 연산은 int 값을 비교해 bool prvalue를 만들며 assert는 거짓이면 진단 후 중단할 수 있다.
    assert(received.version == 7);
    // string과 문자열 리터럴의 operator==는 문자를 비교하며 객체를 바꾸지 않는다. 선형 시간이 들 수 있다.
    assert(received.endpoint == "edge.internal");

    // ostream operator<<(value)는 cout 버퍼에 값을 쓰고 같은 ostream&를 반환해 연쇄한다. 인자는 복사 소유되지 않고 호출 동안 읽힌다.
    // 출력 성공 뒤 received는 유지되며 I/O 실패는 스트림 상태 비트를 세운다. 기본 설정에서는 예외를 던지지 않는다.
    std::cout << received.version << ' ' << received.endpoint << '\n';
    return 0;
}
