// <atomic>은 여러 스레드가 데이터 경쟁 없이 공유 상태를 게시하고 읽는 원자 연산을 제공한다.
#include <atomic>
// <iostream>은 예제 결과를 표준 출력으로 보내는 std::cout을 제공한다.
#include <iostream>
// <memory>는 공유 소유권을 표현하는 std::shared_ptr와 std::make_shared를 제공한다.
#include <memory>
// <string>은 문자 데이터를 직접 소유하는 std::string을 제공한다.
#include <string>
// <utility>는 이름 있는 객체에서 자원 이동 기회를 표시하는 std::move를 제공한다.
#include <utility>

// struct의 멤버는 기본 public이므로 함께 이동하는 단순 설정 값을 표현하기 좋다.
struct AppConfig {
    // int는 재시도 횟수를 저장하는 기본 정수 타입이고 {}는 0으로 값 초기화한다.
    int retry_limit{};
    // string은 endpoint 문자의 메모리를 소유하며 빈 중괄호는 빈 문자열로 초기화한다.
    std::string endpoint{};
};

// 읽기 포트를 추상화하면 서비스는 설정의 저장 방식이나 동기화 방식을 몰라도 된다.
class IConfigSnapshotSource {
// public 접근 지정자 뒤의 소멸자와 조회 함수는 포트를 사용하는 외부 코드가 호출할 수 있다.
public:
    // 가상 소멸자는 기반 클래스 포인터로 파생 객체를 지울 때 올바른 소멸을 보장한다.
    virtual ~IConfigSnapshotSource() = default;

    // const 멤버 함수는 포트 자체를 바꾸지 않고 불변 설정의 공유 소유권을 반환한다.
    [[nodiscard]] virtual std::shared_ptr<const AppConfig> snapshot() const = 0;
};

// final은 이 구현을 더 상속하지 않게 하며 원자적 불변 스냅샷 게시 책임을 한곳에 둔다.
class AtomicConfigStore final : public IConfigSnapshotSource {
// public 영역은 객체 생성, 새 설정 게시, 현재 스냅샷 조회라는 최소 API만 노출한다.
public:
    // 생성자는 반환형이 없고 explicit은 shared_ptr 하나가 저장소로 암시 변환되는 것을 막는다.
    explicit AtomicConfigStore(std::shared_ptr<const AppConfig> initial)
        // atomic<shared_ptr<...>> 생성자의 입력은 std::move(initial)이 넘긴 포인터 값이고 생성자는 반환값이 없다.
        // 멤버 초기화 뒤 current_가 공유 소유권 한 몫을 보관하며 값 매개변수 initial은 빈 유효 상태가 된다.
        : current_{std::move(initial)} {}

    // 값 매개변수는 호출자가 복사 또는 이동 중 원하는 소유권 전달 방식을 고르게 한다.
    void publish(std::shared_ptr<const AppConfig> next) {
        // 호출 계약: current_는 atomic<shared_ptr<const AppConfig>> 수신 객체이고 store(desired, order)를 호출한다.
        // 첫 인자 std::move(next)는 lvalue next를 xvalue로 바꿔 값 매개변수 desired에 공유 소유권 한 몫을 이동한다.
        // 둘째 인자 memory_order_release는 저장할 데이터가 아니라 이 store보다 앞선 AppConfig 쓰기의 게시 순서다.
        // 반환형은 void라 옛 shared_ptr를 돌려주지 않는다. current_는 next가 가리키던 새 값을 보관하고 next는 빈 유효 상태가 된다.
        // 이전 shared_ptr의 참조 횟수 감소·pointee 소멸은 값 교체 뒤 일어날 수 있으며 포인터가 가리키는 멤버를 원자화하지는 않는다.
        current_.store(std::move(next), std::memory_order_release);
    }

    // override는 기반 클래스의 가상 함수 서명과 정확히 일치하는지 컴파일러가 검사하게 한다.
    [[nodiscard]] std::shared_ptr<const AppConfig> snapshot() const override {
        // 호출 계약: load(order)의 수신 객체 current_는 읽히지만 저장값은 바뀌지 않으며 데이터 값 인자는 없다.
        // 유일한 인자 memory_order_acquire는 읽을 값이 아니라 메모리 순서이고, release 저장값을 읽을 때 앞선 쓰기와 동기화한다.
        // 반환형은 shared_ptr<const AppConfig>이며 호출 순간 관찰한 포인터 값의 소유권 한 몫을 복사해 prvalue로 반환한다.
        // 반환 스냅샷은 다음 publish 뒤에도 옛 AppConfig 수명을 연장한다. load는 예외를 던지지 않지만 lock-free는 보장되지 않는다.
        return current_.load(std::memory_order_acquire);
    }

// private 접근 지정자 아래 구현 상태는 클래스 밖에서 직접 읽거나 바꿀 수 없다.
private:
    // 템플릿 인자 shared_ptr<const AppConfig>가 원자적으로 교체되는 값 타입이며 atomic 객체는 복사할 수 없다.
    // 이 특수화는 shared_ptr 값의 동시 load/store를 보호할 뿐, pointee 멤버의 동시 변경까지 보호하지 않는다.
    std::atomic<std::shared_ptr<const AppConfig>> current_{};
};

// 애플리케이션 서비스는 저장 구현을 소유하지 않고 더 오래 사는 포트를 참조로 빌린다.
class RetryService {
// public 영역은 의존성 주입 생성자와 비즈니스 조회 동작을 외부에 제공한다.
public:
    // explicit은 IConfigSnapshotSource에서 RetryService로 뜻밖의 암시 변환을 막는다.
    explicit RetryService(const IConfigSnapshotSource& source)
        // 참조 멤버 source_는 새 객체를 만들지 않고 기존 lvalue 포트에 바인딩된다.
        : source_{source} {}

    // [[nodiscard]]는 계산 결과를 무시했을 때 실수를 경고하도록 의도를 표현한다.
    [[nodiscard]] int retry_limit() const {
        // snapshot()은 가상 간접 호출이며 반환 prvalue가 local shared_ptr를 직접 초기화한다.
        const std::shared_ptr<const AppConfig> config{source_.snapshot()};
        // -> 연산자는 shared_ptr가 가리키는 const AppConfig의 public 멤버를 읽는다.
        return config->retry_limit;
    }

// private 영역은 서비스 구현에만 필요한 비소유 의존성을 숨긴다.
private:
    // 비소유 참조는 source 객체가 RetryService보다 오래 살아야 한다는 수명 계약을 가진다.
    const IConfigSnapshotSource& source_;
};

// main은 운영체제가 호출하는 진입 함수이며 int 종료 코드를 반환한다.
int main() {
    // make_shared<const AppConfig>는 AppConfig 값을 생성자 인자로 받아 const 객체와 제어 블록을 만들고
    // shared_ptr<const AppConfig> prvalue를 반환한다. 보통 한 번 할당하지만 메모리 부족이면 예외가 날 수 있다.
    auto initial{std::make_shared<const AppConfig>(AppConfig{3, "api-v1"})};
    // move(initial)는 shared_ptr&& xvalue를 반환하고 저장소 생성자가 공유 소유권 한 몫을 이동한다. initial은 빈 유효 상태다.
    AtomicConfigStore store{std::move(initial)};
    // store는 lvalue이고 RetryService의 const 참조 매개변수 및 멤버에 바인딩된다.
    const RetryService service{store};

    // 두 번째 make_shared도 AppConfig prvalue 하나를 입력받아 const 객체와 제어 블록을 만들고 shared_ptr prvalue를 반환한다.
    // publish의 값 매개변수가 그 반환값을 직접 받고, 내부 store가 원자 저장소로 소유권을 이동한다.
    store.publish(std::make_shared<const AppConfig>(AppConfig{5, "api-v2"}));

    // 함수 호출은 acquire load, shared_ptr 수명 확보, 가상 호출을 거쳐 현재 설정 값을 얻는다.
    // 실제 로드·저장·비교·간접 호출 명령은 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라진다.
    // 각 operator<<는 std::ostream&를 반환해 연쇄되며 '\n'은 개행만 쓰고 강제 flush하지 않는다.
    std::cout << "retry_limit=" << service.retry_limit() << '\n';
    // 0은 정상 종료를 나타내는 int prvalue다.
    return 0;
}
