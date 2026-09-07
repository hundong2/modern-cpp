// <iostream>은 결과를 기록하는 std::cout과 형식 삽입 연산자를 선언한다.
#include <iostream>
// <mutex>는 한 번만 성공하는 초기화 상태 std::once_flag와 함수 std::call_once를 선언한다.
#include <mutex>
// <optional>은 아직 없음/초기화 완료를 값으로 표현하는 std::optional을 선언한다.
#include <optional>
// <string>은 설정 endpoint 문자를 독점 소유하는 std::string을 선언한다.
#include <string>
// <thread>는 범위 끝에서 자동 join하는 C++20 스레드 소유자 std::jthread를 선언한다.
#include <thread>
// <utility>는 이름 있는 객체를 xvalue로 표현하는 std::move를 선언한다.
#include <utility>

// Config는 성공적으로 읽은 뒤 응용 계층에 건네는 이름 있는 값 객체다.
// class는 기본 접근이 private이라 endpoint와 세대 번호를 생성자 검증 경계 뒤에 숨긴다.
class Config {
public:
    // 생성자는 반환형이 없다. explicit은 두 값으로 하는 copy-list-initialization에서 이 생성자를
    // 암시적으로 사용하는 경로를 막고 직접 초기화 의도를 드러낸다.
    // 멤버 초기화 목록은 생성자 본문보다 먼저 선언 순서대로 각 멤버의 수명을 시작한다.
    // [호출 계약: std::move(endpoint)와 std::string 이동 생성]
    // (1) 자유 함수라 수신 객체는 없고 endpoint는 유효한 std::string 값 매개변수 lvalue다.
    // (2) 선택 인스턴스는 move<std::string&>(std::string&) noexcept이고 이어 string(string&&)이 선택된다.
    // (3) 인자는 endpoint 하나이며 모든 유효한 string을 허용한다. xvalue 표현을 통해 소유권 이전을 허용한다.
    // (4) move는 같은 객체를 가리키는 std::string&&를 반환해 endpoint_ 구성에 사용하고 생성자는 반환값이 없다.
    // (5) endpoint_가 문자를 소유하며 매개변수는 수명은 남지만 유효하고 값이 미지정된 상태가 된다.
    // (6) move 자체는 O(1)·무할당·noexcept·비무효화다. 기본 allocator의 string 이동도 O(1)·noexcept이나
    //     원본 문자 관찰자에 계속 의존하면 안 된다. 외부 동시 접근은 없고 예외가 난 인자 구성은 호출 전 단계다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    explicit Config(std::string endpoint, int generation)
        : endpoint_{std::move(endpoint)}, generation_{generation} {}

    // const std::string& 반환은 저장소를 복사하지 않는 비소유 lvalue 참조다. 이 Config보다 오래 보관하면 안 된다.
    [[nodiscard]] const std::string& endpoint() const noexcept { return endpoint_; }

    // int는 기본 타입이며 값으로 반환하면 호출자가 독립적인 prvalue를 받는다.
    [[nodiscard]] int generation() const noexcept { return generation_; }

private:
    std::string endpoint_; // string 객체가 endpoint 문자 저장소를 RAII로 소유한다.
    int generation_{};     // int{}는 0으로 값 초기화되며 생성 시 전달값으로 직접 구성된다.
};

// 인터페이스는 지연 초기화 정책과 실제 I/O 공급원을 분리한다. 참조를 통한 load 호출은 동적 타입의
// override를 고르는 가상 호출이며, 기반 포인터/참조로 파괴할 수 있도록 소멸자도 virtual이다.
class ConfigSource {
public:
    virtual ~ConfigSource() = default;
    [[nodiscard]] virtual Config load() = 0;
};

// 호출 횟수를 기록하는 test double이다. 실제 서비스에서는 파일·DB·원격 설정 포트가 이 역할을 맡는다.
class CountingConfigSource final : public ConfigSource {
public:
    [[nodiscard]] Config load() override {
        ++attempts_;

        // [호출 계약: std::string(const char*)와 Config 직접 초기화]
        // (1) 새 std::string 목적 객체는 아직 없고 리터럴은 정적 수명의 const char 배열이다.
        // (2) basic_string(const char*, const Allocator& = Allocator())가 선택되고 Config(string,int)가 이어진다.
        // (3) 인자는 null이 아닌 첫 문자 포인터와 기본 allocator다. null 종료까지 읽어 문자를 복사 소유한다.
        // (4) 생성자 반환값은 없고 Config prvalue가 load의 반환 객체를 직접 구성한다.
        // (5) 결과가 "config.internal"을 독립 소유하며 source 상태는 attempts_ 증가 외에는 변하지 않는다.
        // (6) 문자 수 O(15), 할당·length_error·bad_alloc 가능성이 있다. null 포인터면 UB이나 리터럴이 배제한다.
        //     새 객체라 기존 관찰자 무효화는 없고 C++17의 같은 타입 prvalue 반환은 불필요한 복사/이동이 없다.
        return Config{std::string{"config.internal"}, 42};
    }

    [[nodiscard]] int attempts() const noexcept { return attempts_; }

private:
    int attempts_{};
};

// 이 class는 “처음 성공한 구성만 게시”하는 응용 경계다. source는 소유하지 않고 빌리므로 source가
// provider보다 오래 살아야 한다. mutable은 const get이 논리적 결과는 바꾸지 않으면서 캐시 상태를 채우게 한다.
class LazyConfigProvider {
public:
    // explicit은 ConfigSource가 Provider로 암시 변환되는 것을 막는다. 참조 멤버는 생성 때 한 번 바인딩되고
    // 다른 source를 가리키도록 재대입할 수 없으며 대상 수명을 연장하지 않는다.
    explicit LazyConfigProvider(ConfigSource& source) noexcept : source_{source} {}

    [[nodiscard]] const Config& get() const {
        // [호출 계약: std::call_once(init_flag_, lambda)]
        // (1) 수신 객체 없는 함수다. init_flag_는 아직 성공 실행이 없거나 이미 완료된 mutable std::once_flag다.
        // (2) call_once<lambda>(once_flag&, Callable&&)가 선택되며 Args 팩은 비어 있고 반환형은 void다.
        // (3) 첫 인자는 살아 있는 flag lvalue 비소유 참조, 둘째는 this를 값 캡처한 lambda prvalue다. lambda는
        //     provider 수명 안에서 즉시/대기 뒤 호출되며 source_와 cache_에 접근한다. 별도 소유권 이전은 없다.
        // (4) void라 사용할 반환값이 없다. 성공 active 실행 하나만 returning이고 이후 호출은 passive다.
        // (5) callable이 반환하면 flag는 완료 상태가 되고 cache_ 쓰기가 모든 passive 반환에 동기화된다.
        //     callable이 던지면 예외를 전파하고 flag는 미완료라 다음 호출이 재시도한다.
        // (6) 표준은 점근 복잡도·할당·lock-free를 보장하지 않고 경쟁 시 막힐 수 있다. callable 예외와 필요한
        //     system_error가 가능하다. flag/provider/source가 모든 호출보다 오래 살고 같은 cache_ 접근은 이
        //     동기화 경계를 지나야 한다. 재귀적으로 같은 flag를 다시 쓰지 않으며 데이터 경쟁을 만들지 않는다.
        // 대표 문서: ../standard-library/concurrency-time-filesystem.md
        // 감사 요약: 수신 객체 없이 flag lvalue와 lambda prvalue 두 인자를 받고 void를 반환한다. 복잡도와
        // 대기 상한은 미규정이며 예외·수명·동기화 계약은 위와 같고 외부 관찰자를 직접 무효화하지 않는다.
        std::call_once(init_flag_, [this] {
            // [호출 계약: std::optional<Config>::emplace(source_.load())]
            // (1) 수신자는 처음에는 비어 있는 cache_ lvalue다. source_는 살아 있는 ConfigSource lvalue다.
            // (2) template<class... Args> Config& optional<Config>::emplace(Args&&...)에서 Args=Config가 선택된다.
            // (3) 유일한 인자는 가상 load가 반환한 Config prvalue이며 결과 객체 소유권을 optional 안으로 옮긴다.
            // (4) 새 contained Config의 lvalue 참조를 반환하지만 여기서는 사용하지 않는다.
            // (5) 성공하면 cache_가 값을 포함한다. load가 먼저 던지면 emplace는 시작되지 않아 비어 있고,
            //     포함 객체 생성이 던지면 optional은 비어 있어 call_once가 안전하게 다음 시도를 허용한다.
            // (6) 기존 값이 있으면 파괴한 뒤 한 번 구성하므로 Config 구성/소멸 비용이다. 별도 allocation은
            //     optional이 요구하지 않지만 string은 할당할 수 있다. 과거 contained 참조는 무효가 되며,
            //     이 코드의 복잡도는 Config 이동 구성에 따르고 성공 초기화 한 번이라 과거 참조가 없다.
            //     생성 예외는 call_once 밖으로 전파된다.
            // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
            // 감사 요약: 빈 optional 수신자와 Config prvalue 입력 인자를 받아 Config&를 반환하지만 버린다.
            cache_.emplace(source_.load());
        });

        // mutable 멤버 이름은 const 함수 안에서도 non-const lvalue다. 읽기 전용 게시 경계를 실제
        // overload 선택에 반영하도록 cache_를 const optional lvalue reference에 먼저 바인딩한다.
        const std::optional<Config>& published_cache{cache_};

        // [호출 계약: const std::optional<Config>::value]
        // (1) 수신자는 call_once의 returning/passive 반환 뒤 값을 포함하는 const published_cache lvalue다.
        // (2) const Config& value() const & 오버로드가 선택되며 데이터 인자는 없다.
        // (3) 매개변수·소유권 이전은 없고 contained Config가 반드시 존재한다는 사후 불변식을 사용한다.
        // (4) cache_ 안 Config의 const lvalue 참조를 반환해 get 호출자에게 그대로 전달한다.
        // (5) optional과 Config는 변하지 않으며 참조는 provider가 파괴되기 전까지 유효하다.
        // (6) O(1)·무할당·비무효화다. 빈 상태면 bad_optional_access를 던지지만 성공 call_once 뒤라 배제된다.
        //     반환 참조의 동시 읽기는 안전하나 Config를 다른 경로로 변경하면 별도 동기화가 필요하다.
        return published_cache.value();
    }

private:
    ConfigSource& source_; // 비소유 lvalue 참조: provider보다 긴 source 수명이 전제다.

    // [생성 계약: std::once_flag()]
    // (1) 목적 객체 init_flag_는 아직 수명이 시작되지 않았고 수신 객체·데이터 인자는 없다.
    // (2) constexpr once_flag() noexcept 기본 생성자가 선택되며 반환값은 없다.
    // (3) 인자와 소유권 이전은 없고 {}는 default member initializer다.
    // (4) 별도 반환값 없이 멤버 객체의 수명을 시작한다.
    // (5) call_once가 아직 callable을 실행하지 않은 초기 상태가 된다. 생성 자체는 동기화 연산이 아니다.
    // (6) 표준은 복잡도·저장 방식·할당을 정하지 않지만 noexcept다. 복사/대입은 삭제되며 멤버 수명 중
    //     다른 스레드에 안전하게 게시한 뒤 사용해야 한다. 컨테이너 관찰자나 외부 객체는 무효화하지 않는다.
    mutable std::once_flag init_flag_{};

    // [생성 계약: std::optional<Config>()]
    // (1) cache_ 목적 객체는 아직 없고 템플릿 인자 Config가 contained 타입이다.
    // (2) constexpr optional() noexcept 기본 생성자이며 명시적 데이터 인자와 반환값이 없다.
    // (3) {}는 빈 상태 초기화를 뜻하고 Config 소유권을 아직 받지 않는다.
    // (4) 생성자는 값을 반환하지 않고 cache_ 수명을 시작한다.
    // (5) 성공 뒤 contained Config가 없는 상태이고 provider 파괴 때 optional 수명이 끝난다.
    // (6) O(1)·무할당·noexcept이고 기존 참조 무효화나 오류가 없다. 동시 접근 안전성은 call_once가 제공한다.
    mutable std::optional<Config> cache_{};
};

int main() {
    // source를 provider보다 먼저 만들면 블록을 나갈 때 provider가 먼저 파괴되어 빌린 참조가 안전하다.
    CountingConfigSource source{};
    const LazyConfigProvider provider{source};

    const Config* first{};  // raw pointer는 Config를 소유하지 않고 주소만 관찰한다. {}는 null 초기화다.
    const Config* second{};
    {
        // [호출 계약: std::jthread(callable) 두 번과 범위 끝 소멸]
        // (1) 아직 없는 두 jthread가 목적 객체이고 provider/source와 두 포인터는 바깥 범위에서 살아 있다.
        // (2) template<class F, class... Args> explicit jthread(F&&, Args&&...)에서 각 F는 lambda 타입,
        //     Args는 비어 있다. lambda가 stop_token 인자를 받지 않아 token 없는 호출 경로가 선택되고,
        //     소멸자는 joinable이면 stop 요청 후 join한다.
        // (3) 각 인자는 `[&]` lambda prvalue다. 참조 캡처 대상을 빌리며 소유하지 않고 각기 다른 pointer를 쓴다.
        // (4) 생성자/소멸자는 반환값이 없다. 새 실행 스레드는 provider.get()의 참조 주소를 저장한다.
        // (5) 두 스레드 중 하나만 call_once active이고 다른 하나는 passive다. 각 thread 완료는 소멸자 내부의
        //     성공 join 반환과 동기화되므로 블록 뒤 main은 두 포인터와 attempts_ 쓰기를 안전하게 읽는다.
        //     두 포인터가 같은 cache Config를 가리키며 source load 횟수는 1이다.
        // (6) 생성 복잡도·비용은 OS/구현 의존이고 system_error 가능, 성공 뒤 joinable이다. 캡처 수명은 join까지
        //     유지되며 서로 다른 포인터 객체에 써 경쟁하지 않는다. callable은 stop_token을 받지 않아 소멸자의
        //     stop 요청을 관찰하지 않고 join은 자연 종료까지 상한 없이 막힐 수 있다. get에서 빠져나간 미처리
        //     예외는 thread entry에서 terminate를 부르며, noexcept 소멸자 안 join 실패도 terminate로 이어진다.
        // 대표 문서: ../standard-library/concurrency-time-filesystem.md
        std::jthread first_reader{[&] { first = &provider.get(); }};
        std::jthread second_reader{[&] { second = &provider.get(); }};
    }

    // [호출 계약: std::string을 포함한 Config 복사 생성]
    // (1) 목적 snapshot은 아직 없고 *first는 join 뒤 provider cache에 붙은 유효한 const Config lvalue다.
    // (2) 암시적 Config(const Config&)가 선택되고 그 안에서 string(const string&)이 endpoint를 복사한다.
    // (3) 유일한 입력은 비소유 const lvalue 참조이며 provider 소유권은 유지된다.
    // (4) 생성자는 반환값 없이 독립 snapshot을 만든다.
    // (5) 두 Config가 같은 문자 값을 각자 소유하고 first/cache는 변하지 않는다.
    // (6) 문자 수 선형, 할당·bad_alloc 가능성이 있고 실패 시 snapshot은 완성되지 않는다. 참조 무효화는 없다.
    Config snapshot{*first};

    // [호출 계약: std::move(snapshot)와 Config/std::string 이동 생성]
    // (1) 자유 함수 move의 입력 snapshot은 살아 있는 Config lvalue이고 transferred는 아직 없다.
    // (2) move<Config&>는 Config&&를 반환하고 암시적 Config(Config&&), string(string&&)이 선택된다.
    // (3) 인자는 snapshot 하나이며 xvalue로 표현해 endpoint 소유권 이전을 허용한다.
    // (4) 반환 Config&&는 transferred 구성에 즉시 쓰고 생성자에는 반환값이 없다.
    // (5) transferred가 endpoint를 소유하며 snapshot은 유효하지만 endpoint 값은 미지정이다. int는 복사된다.
    // (6) move 자체와 기본 allocator string 이동은 O(1)·무할당·noexcept다. snapshot 수명은 main 끝까지며
    //     이동 전 endpoint 관찰자는 transferred 쪽 자원을 볼 수 있어 원본 관찰자로 사용하면 안 된다.
    Config transferred{std::move(snapshot)};

    // [호출 계약: std::ostream 삽입 연쇄와 char 삽입]
    // (1) 수신자는 앞 출력 뒤에도 유효한 std::cout/std::ostream lvalue이고 모든 피연산자 대상은 살아 있다.
    // (2) string 비멤버 삽입, int/bool 멤버 삽입, operator<<(std::ostream&, char)가 순서대로 선택된다.
    // (3) 인자는 const string lvalue, char prvalue 넷, int prvalue 둘, bool prvalue 하나다. 소유권은 이동하지 않는다.
    // (4) 각 호출은 같은 std::ostream&를 반환해 다음 호출 수신자로 사용하고 마지막 반환만 버린다.
    // (5) `config.internal 42 1 1\n`이 추가되고 Config/source/provider는 변하지 않는다.
    // (6) 복잡도와 비용은 문자화·locale·버퍼·장치에 의존한다. 실패는 상태 비트/설정 예외로 나타나며 할당 가능,
    //     객체·참조 수명과 cache 관찰자는 무효화하지 않는다. 여러 스레드 레코드 비혼합은 보장하지 않는다.
    std::cout << transferred.endpoint() << ' ' << transferred.generation() << ' '
              << source.attempts() << ' ' << (first == second) << '\n';

    // 기계 실행 관점: call_once 구현은 flag load/비교/조건 분기, 원자 연산·잠금·OS 대기를 조합할 수 있고,
    // source_.load()는 가상 간접 호출이 될 수 있다. 성공 뒤 optional 상태와 Config 값을 load해 출력한다.
    // 실제 load/store 수, 문자열 할당·SSO, 인라인·가상화 제거와 분기 모양은 CPU, ABI, 표준 라이브러리,
    // 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리나 lock-free 구현으로 단정하지 않는다.

    return 0;
}
