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
        // 멤버 초기화 목록은 생성자 본문 전에 shared_ptr 소유권을 current_로 이동한다.
        : current_{std::move(initial)} {}

    // 값 매개변수는 호출자가 복사 또는 이동 중 원하는 소유권 전달 방식을 고르게 한다.
    void publish(std::shared_ptr<const AppConfig> next) {
        // release 저장은 새 AppConfig 생성 중 일어난 쓰기가 acquire 독자에게 보이게 한다.
        // std::move(next)는 lvalue인 매개변수를 xvalue로 바꿔 shared_ptr 소유권 한 몫을 옮긴다.
        current_.store(std::move(next), std::memory_order_release);
    }

    // override는 기반 클래스의 가상 함수 서명과 정확히 일치하는지 컴파일러가 검사하게 한다.
    [[nodiscard]] std::shared_ptr<const AppConfig> snapshot() const override {
        // acquire 로드는 대응하는 release 게시 이후 완성된 불변 객체를 안전하게 관찰한다.
        // 반환 shared_ptr은 객체 수명을 연장하므로 다음 publish 뒤에도 기존 스냅샷이 유효하다.
        return current_.load(std::memory_order_acquire);
    }

// private 접근 지정자 아래 구현 상태는 클래스 밖에서 직접 읽거나 바꿀 수 없다.
private:
    // class 멤버는 기본 private이며 atomic 특수화가 shared_ptr 자체의 동시 load/store를 보호한다.
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
    // AppConfig{...}는 prvalue이고 make_shared가 const AppConfig를 한 번 할당해 직접 생성한다.
    auto initial{std::make_shared<const AppConfig>(AppConfig{3, "api-v1"})};
    // initial은 이름 있는 lvalue이며 std::move(initial)은 xvalue로 바뀌어 저장소에 소유권을 넘긴다.
    AtomicConfigStore store{std::move(initial)};
    // store는 lvalue이고 RetryService의 const 참조 매개변수 및 멤버에 바인딩된다.
    const RetryService service{store};

    // publish 인자는 make_shared가 만든 shared_ptr prvalue에서 이동되어 불필요한 참조 횟수 증가를 줄인다.
    store.publish(std::make_shared<const AppConfig>(AppConfig{5, "api-v2"}));

    // 함수 호출은 acquire load, shared_ptr 수명 확보, 가상 호출을 거쳐 현재 설정 값을 얻는다.
    // 실제 로드·저장·비교·간접 호출 명령은 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라진다.
    std::cout << "retry_limit=" << service.retry_limit() << '\n';
    // 0은 정상 종료를 나타내는 int prvalue다.
    return 0;
}
