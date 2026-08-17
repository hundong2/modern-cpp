// <iostream>은 표준 출력 객체 std::cout과 출력 연산자 <<를 제공한다.
#include <iostream>
// <memory>는 단독 소유 스마트 포인터 unique_ptr와 생성 함수 make_unique를 제공한다.
#include <memory>
// <numbers>는 C++20의 타입별 수학 상수 std::numbers::pi_v<T>를 제공한다.
#include <numbers>
// <utility>는 lvalue를 이동 가능한 xvalue로 표현하는 std::move를 제공한다.
#include <utility>

// using은 새 타입을 만드는 것이 아니라 긴 기존 타입에 읽기 쉬운 별칭을 붙인다.
using Scalar = double;

// struct의 기본 접근은 public이다. 단순 결과 묶음은 공개 데이터로 표현하기 알맞다.
struct AreaResult {
    Scalar square_units{}; // double 기본 실수형 멤버를 중괄호로 값 초기화하면 0.0이다.
    bool valid{};          // bool 멤버를 중괄호로 값 초기화하면 false이다.
};

// class의 기본 접근은 private이다. Radius는 유효한 상태를 생성자를 통해서만 만든다.
class Radius {
public:
    // 생성자는 반환형이 없다. explicit은 double이 Radius로 몰래 암시적 변환되는 것을 막는다.
    // 올바른 사용은 Radius radius{2.0};처럼 의도를 드러낸 직접 초기화이다.
    explicit Radius(Scalar value)
        // 멤버 초기화 목록은 생성자 본문 전에 매개변수 value로 멤버를 직접 초기화한다.
        : value_{value} {}

    // 반환형은 Scalar, 매개변수는 없고, 뒤의 const는 객체 상태를 바꾸지 않는다는 약속이다.
    [[nodiscard]] Scalar value() const { return value_; }

    // 반환형 bool인 멤버 함수다. > 비교 연산자의 참/거짓 결과를 호출자에게 돌려준다.
    [[nodiscard]] bool is_valid() const { return value_ > 0.0; }

private:
    Scalar value_{}; // private 멤버 변수에는 클래스 외부가 직접 접근할 수 없다.
};

// 정책 포트다. struct 기본 public 덕분에 public:을 생략했지만 인터페이스는 외부에 공개된다.
struct AreaPolicy {
    // 기반 포인터로 파생 객체를 지울 때 올바른 소멸자가 호출되도록 가상 소멸자를 둔다.
    virtual ~AreaPolicy() = default;
    // 순수 가상 함수: 반환형 AreaResult, 매개변수는 복사 없는 const 참조다.
    [[nodiscard]] virtual AreaResult calculate(const Radius& radius) const = 0;
};

// final은 이 구현 어댑터를 더 상속하지 않겠다는 설계 의도를 나타낸다.
class CircleAreaPolicy final : public AreaPolicy {
public:
    // override는 기반 클래스의 가상 함수 서명과 정확히 일치하는지 컴파일러가 검사하게 한다.
    [[nodiscard]] AreaResult calculate(const Radius& radius) const override {
        // if 조건문은 is_valid 함수 호출 결과를 검사해 거짓이면 실패 경로로 분기한다.
        if (!radius.is_valid()) { // ! 연산자는 bool 값을 반대로 뒤집는다.
            return AreaResult{0.0, false}; // prvalue 결과가 반환 목적지에 직접 생성될 수 있다.
        }

        // pi_v<Scalar>의 <Scalar>는 상수의 실수 타입을 고르는 템플릿 인자다.
        const Scalar r{radius.value()}; // const 지역 변수는 초기화 뒤 다시 저장할 수 없다.
        // pi_v<Scalar>는 함수가 아니라 Scalar 타입으로 준비된 컴파일 시간 상수 값이며 입력·반환 호출이 없다.
        // * 연산은 원의 넓이 πr²를 계산하고 radius나 상수를 바꾸지 않는다.
        return AreaResult{std::numbers::pi_v<Scalar> * r * r, true};
    }
};

class AreaService {
public:
    // 생성자는 반환형이 없고, unique_ptr 매개변수는 정책의 단독 소유권을 받겠다는 뜻이다.
    explicit AreaService(std::unique_ptr<AreaPolicy> policy)
        // std::move 결과는 xvalue이며 unique_ptr 이동 생성자가 소유권을 policy_로 옮긴다.
        : policy_{std::move(policy)} {}

    // const Radius&는 기존 객체나 임시 Radius에 복사 없이 바인딩되는 비소유 참조다.
    [[nodiscard]] AreaResult run(const Radius& radius) const {
        // ->는 포인터가 가리키는 객체의 멤버에 접근한다. 실제 구현은 가상 호출로 선택될 수 있다.
        return policy_->calculate(radius);
    }

private:
    // 템플릿 인자 AreaPolicy는 포인터가 소유할 기반 타입이다.
    std::unique_ptr<AreaPolicy> policy_;
};

int main() { // main의 반환형 int는 운영체제에 전달할 종료 상태 코드다.
    // make_unique<CircleAreaPolicy>()는 생성자 인자 없이 객체를 만들고 unique_ptr<CircleAreaPolicy> prvalue를 반환한다.
    // 할당 실패 시 bad_alloc이 가능하고 반환 포인터가 객체 수명을 단독 소유한다.
    auto policy{std::make_unique<CircleAreaPolicy>()};
    // move(policy)는 policy를 unique_ptr&& xvalue로 캐스팅해 service 생성자가 소유권을 이동하게 한다. 함수 자체는 이동하지 않는다.
    AreaService service{std::move(policy)};

    // Radius{2.0}은 prvalue이며 run의 const 참조에 바인딩되어 호출이 끝날 때까지 살아 있다.
    const AreaResult result{service.run(Radius{2.0})};
    // &&는 왼쪽이 참일 때만 오른쪽을 평가하는 단축 논리 AND 연산자다.
    if (result.valid && result.square_units > 12.0) {
        // <<는 값들을 출력 스트림에 차례로 보내고 '\n'은 줄을 바꾸는 문자다.
        std::cout << "circle area = " << result.square_units << '\n';
    }

    // for는 초기화, 계속 조건, 증가식으로 구성된다. int는 기본 정수 타입이다.
    for (int check{0}; check < 2; ++check) {
        std::cout << "check " << check << '\n';
    }

    // == 비교와 ?: 조건 연산자로 성공이면 0, 실패이면 1을 반환한다.
    return result.valid == true ? 0 : 1;
}
