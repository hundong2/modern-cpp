// <iostream>은 결과를 관찰할 표준 출력 객체와 산술값·문자 삽입의 기반을 선언한다.
#include <iostream>
// <memory>는 std::shared_ptr와 예외 안전한 std::make_shared를 선언한다.
#include <memory>
// <string>은 endpoint 문자를 소유하는 std::string과 문자열 삽입 overload를 선언한다.
#include <string>
// <utility>는 소유권 이동 의도를 표현하는 std::move를 선언한다.
#include <utility>

// 직접 연습: 큰 불변 설정 객체에서 endpoint 부분만 안전하게 공개하는 별칭 핸들을 만든다.
struct ServiceConfig {
    std::string endpoint; // struct의 기본 public 멤버이며 객체가 문자열 저장소를 소유한다.
    // 생성자가 이 멤버를 초기화 목록에서 생략할 때만 int{} 기본 멤버 초기화자가 0을 만든다.
    // 아래 timeout_seconds{timeout}이 선택되면 이 {}는 무시되며 0을 먼저 쓴 뒤 덮는 과정이 아니다.
    int timeout_seconds{};

    // explicit은 `ServiceConfig value = {"api", 30};` copy-list-init을 막고 직접 초기화만 허용한다.
    explicit ServiceConfig(std::string received_endpoint, int timeout) noexcept
        // 수신 없는 template<class T> constexpr remove_reference_t<T>&& std::move(T&& value) noexcept에서 이름 붙은
        // received_endpoint는 string lvalue라 T=string&다. 유일한 인자에 비소유 바인딩해 같은 객체의 string&&
        // xvalue를 반환한다. O(1)·무할당·noexcept이고 호출 자체는 상태를 바꾸지 않으며 수명 연장·동기화도 없다.
        // 선택된 basic_string(basic_string&& other) noexcept의 목적 객체는 새 endpoint, 유일한 인자는 그
        // xvalue다. 반환값 없이 이동 전 원본과 같은 값을 얻고 원본은 유효하지만 값이 미지정된다. 이 overload는
        // 상수 시간·noexcept다. heap 저장소를 넘길 수도, SSO inline 문자를 다룰 수도 있어 버퍼 이전·할당 방식은
        // 구현별이므로 단정하지 않는다. 기존 문자 관찰자는 저장하지 않고 두 객체의 동시 접근은 하지 않는다.
        // 대표 문서: ../standard-library/io-parsing-and-utilities.md
        : endpoint{std::move(received_endpoint)}, timeout_seconds{timeout} {}
};

class ConfigLease { // class 기본 private로 소유 포인터 교체를 캡슐화한다.
private:
    std::shared_ptr<const ServiceConfig> owner_;

    // private이라 외부에서 empty owner로 만들 수 없고 create()가 성공한 make_shared 결과만 전달한다.
    explicit ConfigLease(std::shared_ptr<const ServiceConfig> owner) noexcept
        // std::move(T&&)에서 owner lvalue 때문에 T=shared_ptr<const ServiceConfig>&이고, 유일한 비소유 인자와
        // 같은 객체의 shared_ptr<const ServiceConfig>&& xvalue를 반환한다. O(1)·무할당·noexcept이고 상태·
        // 수명을 직접 바꾸지 않는다. 이어 shared_ptr(shared_ptr&& other) noexcept가 아직 구성 전인 owner_를
        // 목적 객체로 삼아 stored pointer/control-block 몫을 옮긴다. 생성자는 반환값이 없고 owner는 empty,
        // owner_는 non-null이 되며 count는 유지된다. 상수 시간·무할당·noexcept이고 관찰자 무효화가 없으며
        // 같은 shared_ptr 객체의 동시 변경은 별도 동기화가 필요하다.
        : owner_{std::move(owner)} {}

public:
    // 유효 owner의 복사 생성은 O(1)·무할당·noexcept로 strong count를 늘린다. 복사 대입이 이전 마지막 owner를
    // 놓으면 그 객체의 소멸·해제 비용은 추가될 수 있다. 원본을 empty로 만들 수 있는 wrapper 이동은 삭제한다.
    ConfigLease(const ConfigLease&) noexcept = default;
    ConfigLease& operator=(const ConfigLease&) noexcept = default;
    ConfigLease(ConfigLease&&) = delete;
    ConfigLease& operator=(ConfigLease&&) = delete;

    [[nodiscard]] static ConfigLease create(std::string endpoint, int timeout) {
        // make_shared<const ServiceConfig>(args...)는 수신 객체 없이 std::move가 만든 string&& xvalue와 timeout
        // int lvalue를 전달한다. move의 T=string&, make_shared의 Args=string,int&이고 반환형은
        // shared_ptr<const ServiceConfig> prvalue다. 성공하면 불변 객체·제어 블록을 소유하고 endpoint는 유효하지만
        // 값이 미지정된다. 구성에 선형 시간이 들 수 있고 보통 한 할당, bad_alloc 가능성이 있으며 실패 시 부분
        // 자원을 정리한다. ConfigLease prvalue는 호출 결과에 직접 구성되어 중간 wrapper 복사·이동이 없다.
        return ConfigLease{std::make_shared<const ServiceConfig>(std::move(endpoint), timeout)};
    }

    [[nodiscard]] std::shared_ptr<const std::string> endpoint_handle() const noexcept {
        // shared_ptr::operator->는 비어 있지 않은 owner_에서 const ServiceConfig*를 O(1)·무할당·noexcept로
        // 반환하고 상태를 유지한다. 내장 멤버 접근과 &가 endpoint의 const string*를 만든다. aliasing 생성자
        // shared_ptr<const string>(const shared_ptr<const ServiceConfig>& r, const string* p) noexcept는 owner_
        // lvalue의 제어 블록을 공유하고 p를 관찰 포인터로 저장한다. 생성자 반환값은 없고 전체 expression이
        // shared_ptr<const string> prvalue다. outer/string 문자를 복사하지 않으며 참조 횟수는 1 증가한다.
        // 별도 할당·예외는 없고 O(1)이다. 결과가 살아 있는 동안 ServiceConfig와 endpoint
        // 수명이 함께 유지된다. owner_가 빈 상태이거나 p가 유효한 부분 객체가 아니라면 이후 역참조는 잘못이다.
        // 공유 횟수 조작은 서로 다른 핸들 사이 안전하지만 ServiceConfig의 동시 변경을 자동 보호하지는 않는다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        return std::shared_ptr<const std::string>{owner_, &owner_->endpoint};
    }

    [[nodiscard]] int timeout() const noexcept {
        // 같은 operator-> 계약으로 const ServiceConfig*를 받아 int 값을 복사한다. 소유권 변화 없이 O(1)이다.
        return owner_->timeout_seconds;
    }
};

int main() {
    // basic_string(const char*, allocator)는 non-null·null 종료된 문자열 리터럴 포인터에서 6자를 복사해 소유한다.
    // 목적 객체는 새 endpoint이고 생성자는 반환값이 없다. 성공 뒤 endpoint가 버퍼를 소유하고 main scope 끝까지
    // 산다. O(n), 할당 및 length_error/bad_alloc 가능성이 있고 원본 문자열 리터럴은 유지된다.
    const std::string endpoint{"api-v2"};

    // ConfigLease::create(std::string,int)의 첫 값 매개변수는 endpoint const string lvalue에서 선택된
    // basic_string(const basic_string& other)로 복사 구성된다. 새 매개변수가 문자를 독립 소유하고 생성자 반환은
    // 없으며 원본은 유지된다. O(n), 할당과 bad_alloc 가능성이 있고 실패 시 lease는 생성되지 않는다. 둘째 int
    // prvalue 30은 값 복사된다. create가 반환한 ConfigLease prvalue는 lease에 직접 구성되어 이동 호출이 없다.
    ConfigLease lease{ConfigLease::create(endpoint, 30)};

    // aliasing construction expression이 만든 prvalue로 auto가 shared_ptr<const string> 값을 직접 초기화한다.
    // C++17 이후 이
    // 동일 타입 prvalue는 목적 객체에 직접 구성되어 중간 shared_ptr 복사·이동이 필요 없다(RVO와 같은 복사 생략).
    const auto endpoint_handle{lease.endpoint_handle()};

    // operator*는 non-null shared_ptr<const string>에서 const string&를 O(1)·무할당·noexcept로 반환하며 핸들·
    // 참조 횟수를 유지한다. use_count()는 인자 없이 같은 제어 블록 소유자 수 long을 O(1)·noexcept로 반환한다.
    // 첫 string 삽입은 cout lvalue와 const string&를, 정수/long 삽입은 값을, char 삽입은 구분자 prvalue를 받는다.
    // 각 호출은 같은 ostream&를 반환해 연쇄하고 마지막 반환은 버린다. 입력 값·소유권은 유지되며 cout 위치·상태만
    // 바뀐다. 빈 핸들 역참조는 UB지만 lease 불변식이 배제한다. 출력 비용은 문자 수·locale·버퍼/장치에 의존하고
    // 실패는 기본적으로 예외가 아니라 스트림 상태 비트로 기록된다. 기본 synchronized 표준 스트림의 동시 호출은
    // data race를 만들지 않지만 이 여러 번의 << 연쇄 전체가 원자적인 한 레코드가 된다는 보장은 없다.
    std::cout << *endpoint_handle << ' ' << lease.timeout() << ' ' << endpoint_handle.use_count() << '\n';

    return 0;
}
