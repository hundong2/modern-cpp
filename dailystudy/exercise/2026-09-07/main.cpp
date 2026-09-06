// <iostream>은 표준 출력 객체 std::cout과 형식 삽입 연산자를 선언한다.
#include <iostream>
// <string>은 서비스 이름 문자를 소유하는 std::string을 선언한다.
#include <string>
// <tuple>은 서로 다른 열 타입을 위치로 묶는 std::tuple과 std::apply를 선언한다.
#include <tuple>
// <utility>는 이름 있는 객체를 xvalue 식으로 바꾸는 std::move를 선언한다.
#include <utility>

// class는 별도 접근 지정자가 없으면 멤버가 private이다. 반대로 struct의 기본 접근은 public이므로,
// 무조건 공개해도 되는 단순 데이터 묶음에는 struct가 어울리고 불변식을 숨길 값 객체에는 class가 어울린다.
class DeploymentPlan {
public:
    // 생성자는 반환형이 없다. explicit은 세 열을 DeploymentPlan으로 암시 변환하는 copy-list-initialization을
    // 막고 `DeploymentPlan plan{"api", 3, true};` 같은 의도적인 직접 초기화를 요구한다.
    // [호출 계약: std::move(service)와 std::string 이동 생성]
    // (1) 수신 객체 없는 함수 std::move의 입력 service는 살아 있는 std::string 값 매개변수이며 이름 있는
    //     lvalue다. service_는 아직 수명이 시작되지 않은 std::string 멤버다.
    // (2) 선택 함수는 template<class T> remove_reference_t<T>&& move(T&&) noexcept에서 T=std::string&인
    //     인스턴스다. 이어서 string(string&&) 이동 생성자가 service_를 구성한다.
    // (3) 유일한 인자는 service lvalue에 바인딩된 전달 참조이며 소유권은 move 호출이 아니라 뒤의 생성자가
    //     넘겨받는다. move 결과는 같은 객체를 나타내는 std::string&& xvalue다.
    // (4) move는 같은 service를 가리키는 xvalue 참조를 반환해 즉시 사용하고, string 생성자는 반환값이 없다.
    // (5) service_가 문자 값을 소유하며 service는 유효하지만 값이 미지정된 상태가 된다. 두 객체 수명은
    //     서로 독립이고 생성자 종료 때 매개변수 service가 먼저 파괴된다.
    // (6) move 자체는 O(1)·무할당·noexcept·비무효화다. 기본 allocator의 string 이동 생성도 상수 시간이고
    //     noexcept이며, service를 가리키던 문자 관찰자는 이동 결과에 따라 더는 원본 관찰자로 쓰면 안 된다.
    //     외부 동시 접근을 동기화하지 않으며 범위 위반이나 UB를 만드는 입력은 없다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    // 감사 요약: 수신 객체는 없고 인자는 service lvalue, 반환은 string&&이며 O(1)·무할당·noexcept이고
    // 실제 이동 뒤 원본 수명은 유지되지만 값은 미지정이다.
    explicit DeploymentPlan(std::string service, int replicas, bool canary) noexcept
        : service_{std::move(service)}, replicas_{replicas}, canary_{canary} {}

    // 반환형 const std::string&는 멤버를 복사하지 않는 읽기 전용 lvalue 참조다. 호출자는 객체를 소유하지
    // 않으며 이 DeploymentPlan이 살아 있는 동안만 참조를 써야 한다. const 멤버 함수는 논리 상태를 바꾸지 않는다.
    [[nodiscard]] const std::string& service() const noexcept { return service_; }

    // 기본 타입 int를 값으로 반환하므로 호출자는 독립 복사본을 받는다.
    [[nodiscard]] int replicas() const noexcept { return replicas_; }

    // bool 값은 canary 배포 여부를 나타낸다. 반환 prvalue는 조건식이나 출력에 바로 쓸 수 있다.
    [[nodiscard]] bool canary() const noexcept { return canary_; }

private:
    std::string service_; // 클래스 타입은 서비스 이름 문자 저장소를 단독 소유한다.
    int replicas_{};      // 기본 타입 int의 {} 값 초기화는 0이며 이후 멤버 초기화 목록 값으로 구성된다.
    bool canary_{};       // bool{}은 false다.
};

// using은 새 클래스를 만들지 않고 긴 구체 타입에 읽기 쉬운 별칭을 붙인다. std::tuple의 템플릿 인자는
// 차례대로 std::string, int, bool이며, 위치 0/1/2가 각각 서비스명/복제 수/canary 여부라는 외부 스키마다.
using RawDeploymentRow = std::tuple<std::string, int, bool>;

// 이 어댑터는 위치 기반 인프라 자료를 이름 있는 도메인 값으로 바꾸는 경계를 한곳에 가둔다.
class DeploymentRowMapper {
public:
    // row를 값으로 받으면 호출자가 lvalue를 주면 보존용 복사, xvalue를 주면 소유권 이동이 일어난다.
    // const 멤버 함수이므로 mapper 자체에는 숨은 상태 변경이 없다.
    [[nodiscard]] DeploymentPlan map(RawDeploymentRow row) const {
        // [호출 계약: std::move(row)]
        // (1) 수신 객체는 없고 row는 세 원소를 소유하는 유효한 RawDeploymentRow lvalue다.
        // (2) 선택 인스턴스는 move<RawDeploymentRow&>(RawDeploymentRow&) noexcept이며 반환형은
        //     RawDeploymentRow&&다.
        // (3) 유일한 인자는 함수가 소유한 row lvalue다. 허용값 제한은 없고, 소유권 이전 가능성을 표현한다.
        // (4) 같은 tuple을 나타내는 xvalue 참조를 반환해 아래 apply의 두 번째 인자로 즉시 사용한다.
        // (5) 호출 자체는 row를 바꾸지 않는다. 실제 원소 이동은 apply가 lambda의 값 매개변수를 구성할 때 난다.
        // (6) O(1)·무할당·noexcept이며 수명을 늘리거나 관찰자를 무효화하지 않는다. row는 apply가 끝날 때까지
        //     살아 있고 이 함수 안에서만 접근하므로 데이터 경쟁이 없다.
        // [호출 계약: std::apply(lambda, std::move(row))]
        // (1) 수신 객체는 없다. 첫 피연산자는 새 lambda prvalue, 둘째는 유효한 tuple xvalue다.
        // (2) 선택 함수는 template<class F, class Tuple> constexpr decltype(auto) apply(F&&, Tuple&&)에서
        //     F가 lambda 타입, Tuple=RawDeploymentRow다. tuple_size가 3이므로 callable 인자는 세 개다.
        // (3) lambda는 소유 캡처가 없는 prvalue이며, tuple xvalue의 string/int/bool 원소가 순서대로 xvalue로
        //     전달되어 값 매개변수 service/replicas/canary를 구성한다. string 소유권은 service로 이동한다.
        // (4) 반환형은 lambda가 만든 DeploymentPlan prvalue다. map의 같은 타입 반환값으로 즉시 사용한다.
        // (5) 결과 Plan이 문자열을 소유하고 row의 string 원소는 유효하지만 값 미지정이다. int/bool 값은 같다.
        // (6) apply 자체의 오버헤드는 tuple 원소 수 O(3), 무할당·비무효화이며 callable 호출의 예외를 그대로
        //     전달한다. 이 callable의 기본 allocator string 이동은 noexcept다. tuple-like 요구를 만족하지 않으면
        //     컴파일되지 않으며, 실행 중 범위 인덱싱이나 UB는 없다. 공유 객체 동기화는 제공하지 않는다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        // 감사 요약: 수신 객체 없이 lambda와 tuple xvalue 두 인자를 받고 DeploymentPlan을 반환해 사용한다.
        // O(3)·apply 자체 무할당이며 row 수명은 유지되고 callable 예외는 전파된다.
        return std::apply(
            [](std::string service, int replicas, bool canary) -> DeploymentPlan {
                // [호출 계약: std::move(service)와 DeploymentPlan의 string 값 매개변수 구성]
                // (1) service는 apply가 tuple 원소에서 이동 구성한 유효한 std::string lvalue다.
                // (2) move<std::string&>가 std::string&&를 반환하고, 그 xvalue로 생성자의 string 값
                //     매개변수를 이동 구성한다.
                // (3) 인자는 service lvalue 하나이며 소유권을 결과 Plan 쪽으로 넘길 의도를 표시한다.
                // (4) 반환 참조는 DeploymentPlan 직접 초기화에 즉시 쓰며 별도로 저장하지 않는다.
                // (5) Plan 생성 과정이 문자열 소유권을 넘겨받고 service는 유효하지만 값 미지정이 된다.
                // (6) move와 기본 allocator string 이동은 O(1)·무할당·noexcept다. 수명을 늘리지 않으며
                //     외부 관찰자나 동시 접근은 없다. lambda의 같은 타입 prvalue 반환은 C++17부터 결과
                //     객체에 직접 구성되어 불필요한 복사/이동을 요구하지 않는다.
                return DeploymentPlan{std::move(service), replicas, canary};
            },
            std::move(row));
    }
};

int main() {
    // [호출 계약: std::string(const char*)와 RawDeploymentRow 원소 생성자]
    // (1) 아직 raw 객체는 없고 문자열 리터럴 "billing"은 프로그램 끝까지 사는 const char[8]이다.
    // (2) string은 basic_string(const char*, const Allocator& = Allocator())를, tuple은 세 원소로부터 각
    //     원소를 구성하는 forwarding 생성자를 선택하며 템플릿 인자는 string/int/bool이다.
    // (3) string 인자는 null이 아닌 첫 문자 포인터로 null 종료까지 읽는다. tuple 입력은 string prvalue,
    //     int prvalue 4, bool prvalue true이며 각각 허용 범위이고 tuple이 string 소유권을 받는다.
    // (4) 두 생성자는 반환값이 없고 완성된 tuple은 raw라는 이름의 lvalue가 된다.
    // (5) raw가 "billing", 4, true를 독립 소유한다. 리터럴과 정수/불 값은 바뀌지 않는다.
    // (6) string 구성은 문자 수 O(7)이며 할당·length_error·bad_alloc 가능, tuple 조립은 세 원소 구성 비용이다.
    //     새 객체라 기존 참조 무효화는 없다. tuple 파괴 때 모든 원소 수명은 끝나지만 표준은 특정 인덱스
    //     파괴 순서를 보장하지 않으므로 그 순서에 기대면 안 된다. null 포인터 전달은 UB지만 리터럴이
    //     배제하며, 생성 중 외부 동시 접근은 없다.
    RawDeploymentRow raw{std::string{"billing"}, 4, true};
    const DeploymentRowMapper mapper{}; // {} 값 초기화로 상태 없는 class 객체의 수명을 시작한다.

    // [호출 계약: std::move(raw)와 map 값 매개변수의 tuple 이동 생성]
    // (1) raw는 세 원소를 소유한 유효한 lvalue이고 mapper는 const 객체다.
    // (2) move<RawDeploymentRow&>는 RawDeploymentRow&&를 반환하며 map(RawDeploymentRow) 호출의 값
    //     매개변수는 tuple 이동 생성자로 구성된다.
    // (3) 유일한 move 인자는 raw lvalue다. 전체 raw의 소유 자원을 mapper 경계로 넘기려는 xvalue 표현이다.
    // (4) xvalue 참조는 map 인자 구성에 즉시 사용되고 map은 DeploymentPlan prvalue를 반환해 plan을 구성한다.
    // (5) raw의 string 원소는 유효하지만 값 미지정이고 plan은 독립 문자열과 숫자를 소유한다.
    // (6) move 자체와 tuple의 각 기본 이동은 상수 원소 수에 O(1), 기본 string allocator에서 noexcept다.
    //     raw의 수명은 main 끝까지지만 이동 전 값에 기대면 안 된다. 외부 관찰자·동시 접근은 없다.
    const DeploymentPlan plan{mapper.map(std::move(raw))};

    // const char*는 문자를 소유하지 않는 주소 값이다. 두 문자열 리터럴은 정적 수명이므로 댕글링되지 않는다.
    const char* rollout_lane{"stable"};
    // if는 bool 값을 비교해 한 분기로만 들어간다. 가상 호출은 없고 canary()는 bool prvalue를 반환한다.
    if (plan.canary()) {
        rollout_lane = "canary";
    }

    // [호출 계약: std::ostream 삽입 연쇄]
    // (1) 최초 수신자는 정상 상태의 std::cout/std::ostream lvalue다. plan과 rollout_lane 대상은 살아 있다.
    // (2) string에는 operator<<(basic_ostream&, const basic_string&), 공백/개행 char에는
    //     operator<<(basic_ostream&, char), int/bool에는 ostream 멤버 operator<<(int/bool), C 문자열에는
    //     operator<<(basic_ostream&, const char*)가 차례로 선택된다.
    // (3) 인자는 service()의 const string lvalue, char prvalue 네 개, replicas() int prvalue,
    //     canary() bool prvalue, non-null const char* lvalue가 가리키는 null 종료 문자열이다. 소유권 이전은 없다.
    // (4) 각 호출은 같은 std::ostream&를 반환하고 다음 삽입의 수신자로 사용한다. 마지막 반환은 버린다.
    // (5) 성공하면 `billing 4 1 canary\n` 문자가 버퍼에 순서대로 추가되고 Plan과 포인터는 변하지 않는다.
    // (6) 표준은 전체 형식 출력의 단일 점근 상한을 두지 않는다. locale·변환·버퍼/장치 비용이 들며 상태
    //     준비 중 할당 가능성이 있다. 실패는 상태 비트, 예외 마스크가 켜졌다면 ios_base::failure로 나타난다.
    //     C 문자열 포인터가 null이면 전제 위반으로 UB이므로 여기서는 리터럴로 배제한다. 여러 스레드의
    //     레코드 단위 비혼합은 보장하지 않고, 이 프로그램은 단일 스레드다.
    // 감사 요약: cout 수신자와 string/char/int/bool/C 문자열 피연산자를 받아 ostream& 반환을 연쇄 사용한다.
    // 출력 복잡도는 버퍼에 의존하고 상태 오류·예외 가능성이 있으며 객체 수명·관찰자는 무효화하지 않는다.
    std::cout << plan.service() << ' ' << plan.replicas() << ' ' << plan.canary()
              << ' ' << rollout_lane << '\n';

    // 기계 실행 관점: tuple 원소 주소/값을 읽어 lambda 인자와 Plan 멤버에 저장하고, bool을 비교해 조건
    // 분기한 뒤 스트림의 가상 streambuf 경계를 통해 문자를 기록할 수 있다. apply가 반드시 런타임 간접 호출을
    // 만든다고 단정할 수 없으며 lambda는 인라인될 수 있다. 실제 load/store/분기/간접 호출·SSO·할당은
    // CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라진다.

    return 0;
}
