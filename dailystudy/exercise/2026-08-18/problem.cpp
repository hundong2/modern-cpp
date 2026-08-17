// <iostream>은 학습 결과를 출력하는 std::cout을 제공한다.
#include <iostream>
// <memory>는 Impl의 독점 소유권을 표현하는 unique_ptr와 make_unique를 제공한다.
#include <memory>
// <utility>는 std::move와 std::swap 함수 템플릿을 제공한다.
#include <utility>
// <vector>는 재시도 지연 시간을 연속 저장하는 std::vector를 제공한다.
#include <vector>

// RetrySchedule은 Pimpl 내부를 깊게 복사해 값처럼 동작하는 실무 설정 객체다.
class RetrySchedule {
public:
    // explicit은 vector가 RetrySchedule로 암시 변환되는 것을 막고 직접 중괄호 초기화는 허용한다.
    explicit RetrySchedule(std::vector<int> delays);
    // 생성자와 달리 소멸자는 ~가 붙고 반환형이 없으며, Impl 완전 정의 뒤에서 구현한다.
    ~RetrySchedule();

    // 복사 생성자와 복사 대입은 각 객체가 서로 다른 Impl을 소유하도록 직접 정의한다.
    RetrySchedule(const RetrySchedule& other);
    RetrySchedule& operator=(const RetrySchedule& other);
    // 이동 연산은 unique_ptr 소유권만 O(1)에 옮기고 예외를 던지지 않는다.
    RetrySchedule(RetrySchedule&&) noexcept;
    RetrySchedule& operator=(RetrySchedule&&) noexcept;

    // 새 지연 시간을 내부 vector 끝에 추가한다.
    void add_delay(int milliseconds);
    // [[nodiscard]]는 계산 결과를 실수로 버렸을 때 경고할 기회를 준다.
    [[nodiscard]] int total_delay() const;

    // friend는 private impl_에 접근하지만 멤버 함수는 아니며 두 값의 구현 소유권을 교환한다.
    friend void swap(RetrySchedule& left, RetrySchedule& right) noexcept;

private:
    // 불완전 타입 선언으로 vector 등 구현 헤더가 공개 인터페이스에 퍼지는 것을 줄인다.
    class Impl;
    // unique_ptr 기본 생성자는 인자 없이 빈 포인터를 noexcept로 만들고 할당하지 않는다. 실제 완성 객체는
    // 생성자 초기화 목록에서 make_unique 결과로 직접 초기화되며 이동 후 원본만 다시 빈 상태가 될 수 있다.
    std::unique_ptr<Impl> impl_{};
};

class RetrySchedule::Impl {
public:
    // vector 값을 받아 멤버 초기화 목록에서 이동하므로 호출자는 복사 또는 명시적 이동을 선택한다.
    explicit Impl(std::vector<int> delays)
        // std::move는 delays lvalue를 xvalue로 바꾸고 vector 이동 생성자가 보통 버퍼 소유권을 O(1)에 넘겨받는다.
        // 할당자 조건에 따라 선형 이동일 수 있으며 이동 후 delays는 유효하지만 원소 값은 미지정이다.
        : delays_{std::move(delays)} {}

    // vector<int>가 모든 지연 값을 소유하며 Impl 복사는 vector 원소를 깊게 복사한다. {} 기본 생성은
    // 인자·원소·할당 없이 빈 vector를 만들지만 실제 Impl 생성에서는 이동 생성자가 이 멤버를 초기화한다.
    std::vector<int> delays_{};
};

RetrySchedule::RetrySchedule(std::vector<int> delays)
    // make_unique<Impl>은 xvalue vector 한 개로 Impl을 만들고 unique_ptr prvalue를 반환한다.
    // 성공 후 impl_만 새 Impl을 소유하며 할당 실패 시 예외가 나고 생성 중인 RetrySchedule은 완성되지 않는다.
    : impl_{std::make_unique<Impl>(std::move(delays))} {}

RetrySchedule::~RetrySchedule() = default;

RetrySchedule::RetrySchedule(const RetrySchedule& other)
    // other.impl_ 역참조 식은 const Impl lvalue다. make_unique<Impl>이 암시적 Impl 복사 생성자를 호출해
    // vector까지 깊게 복사하고 새 unique_ptr을 반환한다. other와 그 소유권은 그대로 유지된다.
    // 복잡도와 추가 공간은 지연 개수에 선형이고 할당 실패 시 이 새 객체만 생성되지 않는다.
    : impl_{std::make_unique<Impl>(*other.impl_)} {}

RetrySchedule& RetrySchedule::operator=(const RetrySchedule& other) {
    // this는 현재 객체를 가리키는 포인터이고 &other와 같으면 자기 대입이므로 상태를 유지한다.
    if (this == &other) {
        return *this;
    }

    // other lvalue를 복사 생성해 임시 독립 소유권을 먼저 완성한다. 실패하면 *this는 바뀌지 않는다.
    RetrySchedule copy{other};
    // swap은 두 RetrySchedule lvalue 참조를 받아 unique_ptr 소유권을 교환하고 반환값은 없다.
    // 호출 뒤 *this는 새 복사본, copy는 옛 구현을 소유하며 두 인자 자체의 수명은 유지된다.
    swap(*this, copy);
    // 함수 끝에서 copy가 파괴되며 이전 구현을 RAII로 정리해 강한 예외 보장을 만든다.
    return *this;
}

RetrySchedule::RetrySchedule(RetrySchedule&&) noexcept = default;
RetrySchedule& RetrySchedule::operator=(RetrySchedule&&) noexcept = default;

void swap(RetrySchedule& left, RetrySchedule& right) noexcept {
    // std::swap<unique_ptr<Impl>>(left.impl_, right.impl_)가 선택되어 두 독점 포인터를 O(1)에 교환한다.
    // 반환형은 void이고 두 pointee는 파괴되지 않으며 참조·포인터로 관찰하던 Impl 주소도 그대로다.
    // noexcept이며 별도 할당도 없지만, 호출 뒤 어느 RetrySchedule이 어느 Impl을 소유하는지는 뒤바뀐다.
    std::swap(left.impl_, right.impl_);
}

void RetrySchedule::add_delay(int milliseconds) {
    // delays_는 vector<int> 수신 객체다. push_back(const int&)가 milliseconds lvalue를 복사해 끝에 추가하고
    // 반환형 void라 결과를 저장하지 않는다. 성공 뒤 크기가 1 늘며 상각 O(1), 추가 용량은 O(1)이다.
    // 재할당되면 기존 원소 포인터·참조·반복자가 모두 무효화되고 bad_alloc이면 vector는 원래 상태를 유지한다.
    impl_->delays_.push_back(milliseconds);
}

int RetrySchedule::total_delay() const {
    int total{}; // int 기본 타입을 중괄호로 0 초기화한다.
    // 범위 for는 delays_에 begin/end를 얻어 모든 int를 const 참조로 순회한다. 컨테이너를 바꾸지 않으므로
    // 반복자는 순회 동안 유효하고, 시간 O(N)·추가 공간 O(1)이며 빈 vector면 본문을 한 번도 실행하지 않는다.
    for (const int& delay : impl_->delays_) {
        total += delay; // +=는 현재 합에 delay를 더해 total lvalue에 다시 저장한다.
    }
    return total; // int 값을 반환하며 작은 기본 타입의 복사 비용은 상수다.
}

int main() {
    // std::vector<int>{100,200,400}은 initializer_list<int> 생성자를 선택해 세 값을 선형 복사하고,
    // 새 연속 저장소를 소유한 vector prvalue를 반환한다. 할당 실패 시 예외가 나며 참조할 기존 원소는 없다.
    // 이 prvalue는 RetrySchedule 값 매개변수로 이동될 수 있다.
    RetrySchedule original{std::vector<int>{100, 200, 400}};
    // original은 lvalue이므로 사용자 정의 복사 생성자가 별도 Impl과 vector를 깊게 복사한다.
    RetrySchedule changed{original};
    // 일반 멤버 함수 호출은 changed 내부만 바꾸고 original 불변식을 유지한다.
    changed.add_delay(800);

    const int original_total{original.total_delay()}; // 반환 prvalue로 700을 직접 초기화한다.
    const int changed_total{changed.total_delay()};   // 독립 복사본은 1500을 반환한다.

    // ostream operator<< 연쇄는 같은 std::cout 참조에 두 정수와 문자를 순서대로 기록한다.
    // 각 반환 std::ostream&는 다음 삽입에 사용되고 최종 참조는 버린다. 객체와 입력 정수는 바뀌지 않는다.
    // 출력 오류는 기본 설정에서 상태 비트로 남고, 시간은 변환된 문자 수와 장치 비용에 비례한다.
    std::cout << original_total << ' ' << changed_total << '\n';

    // &&는 왼쪽이 거짓이면 오른쪽 비교를 생략하며, 원본과 복사본의 독립성을 검증한다.
    return original_total == 700 && changed_total == 1500 ? 0 : 1;
}
