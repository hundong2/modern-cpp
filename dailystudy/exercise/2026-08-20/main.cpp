// <expected>는 성공값 DeploymentPlan 또는 오류 PlanError 중 하나를 소유하는 std::expected를 선언한다.
#include <expected>
// <iostream>은 결과를 쓰는 std::cout과 ostream 삽입 연산자를 선언한다.
#include <iostream>
// <string>은 서비스 이름을 소유하는 std::string과 empty 멤버를 선언한다.
#include <string>
// <string_view>는 복사 없는 오류 메시지 뷰 std::string_view를 선언한다.
#include <string_view>
// <utility>는 소유 자원을 이동 후보로 표시하는 std::move를 선언한다.
#include <utility>

// struct는 기본 접근이 public이다. 이 타입은 외부 입력을 그대로 담는 DTO라서 집합체로 유지한다.
struct RawDeployment {
    // string() 기본 생성자는 인자가 없고 빈 소유 문자열을 만든다. 반환값은 별도로 없으며 service가 목적 객체다.
    // 보통 할당 없이 O(1)이고 참조·반복자는 아직 없으며, 생성된 문자열 수명은 RawDeployment와 같다.
    std::string service{};
    int replicas{1};       // 중괄호 멤버 초기값은 지정하지 않은 필드의 기본값이 된다.
    bool canary{};         // bool{}은 false이므로 기본 배포는 카나리가 아니다.
};

// class는 기본 접근이 private이다. 유효한 복제 수만 표현하려는 도메인 값 타입이다.
class ReplicaCount {
public:
    // 생성자에는 반환형이 없다. explicit은 int가 ReplicaCount로 암시 변환되는 일을 막는다.
    explicit ReplicaCount(int value) : value_{value} {} // 멤버 초기화 목록이 본문 전에 value_를 직접 초기화한다.

    // 반환형 int와 뒤 const는 값을 복사해 돌려주며 객체 상태를 바꾸지 않는다는 뜻이다.
    [[nodiscard]] int value() const noexcept { return value_; }

private:
    int value_{}; // private 멤버는 클래스 바깥에서 직접 바꿀 수 없다.
};

// enum class는 열거자 이름을 타입 범위 안에 두고 정수와의 암시 변환을 막는다.
enum class PlanError {
    empty_service,
    invalid_replicas,
};

// class는 검증된 배포 계획의 소유권과 불변식을 한곳에 모은다.
class DeploymentPlan {
public:
    // 생성자는 반환형이 없고, 세 매개변수를 멤버 초기화 목록에서 직접 초기화한다.
    DeploymentPlan(std::string service, ReplicaCount replicas, bool canary)
        // std::move<T>(T&&)에서 T=std::string&로 추론되어 매개변수는 string&로 붕괴하고 반환은 string&&다.
        // service lvalue를 xvalue로 바꿀 뿐 실제 이동은 string 이동 생성자가 수행한다.
        // 인자는 함수가 소유한 문자열이고 service_가 버퍼를 넘겨받을 수 있다. 반환값은 없으며 이동 뒤 인자는 유효하지만 값은 미지정이다.
        // 일반적으로 O(1)이지만 구현의 작은 문자열 최적화 상태에서는 문자를 복사할 수 있고, service_ 수명은 DeploymentPlan과 같다.
        : service_{std::move(service)}, replicas_{replicas}, canary_{canary} {}

    // const string& 반환은 복사를 피하지만, 이 참조는 *this가 살아 있고 service_가 바뀌지 않는 동안만 유효하다.
    [[nodiscard]] const std::string& service() const noexcept { return service_; }
    [[nodiscard]] int replicas() const noexcept { return replicas_.value(); }
    [[nodiscard]] bool canary() const noexcept { return canary_; }

private:
    std::string service_{};   // 위와 같은 기본 생성 계약이며, 계획이 서비스 이름 메모리를 독점 소유한다.
    ReplicaCount replicas_{1}; // explicit 생성자를 올바른 직접 중괄호 초기화로 호출한다.
    bool canary_{};
};

// using은 긴 템플릿 인스턴스에 도메인 이름을 붙이며 새 타입을 만들지는 않는다.
using PlanResult = std::expected<DeploymentPlan, PlanError>;

// 서비스 클래스는 입력 DTO 검증과 도메인 객체 생성을 조율하는 애플리케이션 경계다.
class DeploymentPlanner {
public:
    // 매개변수를 값으로 받으므로 호출자는 복사할지 이동할지 선택하고, 함수는 자기 복사본을 안전하게 소비한다.
    [[nodiscard]] PlanResult make(RawDeployment request) const {
        // string::empty()의 대표 형태는 bool empty() const noexcept다. 수신 객체 request.service는 현재 소유 문자열이며
        // 인자는 없고 O(1)로 크기 0 여부를 반환한다. 문자열·용량·참조는 바뀌지 않고 예외나 할당도 없다.
        if (request.service.empty()) {
            // unexpected<PlanError>(PlanError&&)는 PlanError prvalue를 값으로 소유하고 그 임시값을 PlanResult의 오류 대안으로 이동한다.
            // 반환 prvalue는 호출 결과를 직접 초기화할 수 있으며 request는 그대로 파괴된다. O(1), 할당 없음, 예외 없음이다.
            return std::unexpected<PlanError>{PlanError::empty_service};
        }
        // ||는 두 비교 중 하나라도 참이면 오류이며, 단락 평가로 왼쪽이 참이면 오른쪽 비교를 생략한다.
        if (request.replicas < 1 || request.replicas > 100) {
            return std::unexpected<PlanError>{PlanError::invalid_replicas};
        }

        // std::move<T>(T&&)에서 T=std::string&이고 request.service lvalue를 string&& xvalue로 바꿔 계획에 소유권을 넘길 기회를 준다.
        // ReplicaCount{request.replicas}는 explicit 생성자를 직접 호출하며, 전체 DeploymentPlan prvalue가 expected의 성공값을 직접 초기화한다.
        return DeploymentPlan{std::move(request.service), ReplicaCount{request.replicas}, request.canary};
    }
};

// 오류를 비소유 문자열 뷰로 바꾸므로 정적 문자열 리터럴의 수명을 빌리며 할당하지 않는다.
[[nodiscard]] constexpr std::string_view message(PlanError error) noexcept {
    // switch는 enum 값을 비교해 한 분기로 이동하며 모든 열거자를 명시해 누락 경고를 받을 수 있다.
    switch (error) {
    case PlanError::empty_service:
        // string_view(const char*) 변환은 정적 배열의 첫 문자를 가리키고 길이를 계산해 뷰 prvalue를 만든다. 문자를 복사·소유하지 않는다.
        // 리터럴은 프로그램 끝까지 살아 있어 반환 뷰가 안전하며, 할당·예외가 없고 길이 계산은 문자열 길이에 선형이다.
        return "service name is empty";
    case PlanError::invalid_replicas:
        return "replicas must be 1..100";
    }
    return "unknown error"; // 손상된 값에 대한 방어 반환이며 정상 열거자에서는 도달하지 않는다.
}

int main() {
    // C++20 지정 초기화는 집합체 멤버를 선언 순서대로 이름 붙여 초기화한다.
    // string(const char*, const Allocator& = Allocator())는 리터럴 배열이 변환된 const char* prvalue를 첫 인자로 받고 기본 할당자를 사용한다.
    // 널 종료 전 문자를 request.service가 복사 소유한다. O(문자 수), 할당 가능, 실패 시 bad_alloc이고 리터럴 수명은 변하지 않는다.
    RawDeployment request{.service = "billing", .replicas = 3, .canary = true};
    const DeploymentPlanner planner{}; // 기본 생성된 무상태 서비스는 자동 저장 기간 동안 살아 있다.

    // move<T>(T&&)에서 T=RawDeployment&로 추론되어 RawDeployment&&를 반환하고 make의 값 매개변수 이동 생성을 선택한다.
    // planner.make의 PlanResult prvalue는 같은 타입 목적 객체 result를 직접 초기화해 별도 expected 복사·이동을 생략한다.
    // 이동 뒤 request는 유효하지만 service 값은 미지정이므로 이후 논리에 사용하지 않는다.
    PlanResult result{planner.make(std::move(request))};

    // expected::operator bool() const noexcept는 성공 상태를 bool로 반환하고 수신 객체를 바꾸지 않는다. O(1), 할당·무효화·예외가 없다.
    if (!result) {
        // 비const lvalue result에는 expected::error() & -> PlanError&가 선택되어 오류 상태라는 전제에서 내부 오류를 빌려준다.
        // 명시 인자는 없고 O(1), 상태·소유권·할당 변화가 없다. message의 값 매개변수로 PlanError를 복사하며 참조는 result 수명에 종속된다.
        // message는 정적 문자열 뷰를 반환한다. 각 operator<<는 같은 ostream&를 반환하며 여기서는 최종 참조를 저장하지 않는다.
        std::cout << "error: " << message(result.error()) << '\n';
        return 1;
    }

    // expected::value() & -> DeploymentPlan&는 성공값 참조를 O(1)에 반환한다. 오류 상태면 bad_expected_access를 던지지만 위 분기가 전제조건을 보장한다.
    // plan은 const lvalue 참조라 복사하지 않으며 result가 살아 있는 main 끝까지 내부 객체를 관찰한다.
    const DeploymentPlan& plan{result.value()};
    // 각 사용자 정의 접근자는 plan을 바꾸지 않고 값 또는 참조를 반환한다. ostream 삽입은 출력 상태만 바꾸고 plan/result의 소유권은 유지한다.
    std::cout << plan.service() << " replicas=" << plan.replicas()
              << " canary=" << (plan.canary() ? "true" : "false") << '\n';

    // 이름 있는 request/planner/result/plan은 lvalue, make와 DeploymentPlan{...} 결과는 prvalue, std::move(request)는 xvalue다.
    // 실행은 문자열 로드·조건 비교·분기·함수 호출을 포함할 수 있지만 실제 명령은 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.
    return 0;
}
