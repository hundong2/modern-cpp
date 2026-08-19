// <iostream>은 직접 실습 결과를 쓰는 std::cout과 삽입 연산자를 선언한다.
#include <iostream>

// struct의 기본 접근은 public이다. 설정 파일을 읽은 직후의 단순 DTO를 집합체로 표현한다.
struct RawRetryOptions {
    int max_attempts{1}; // 생략된 지정 초기화 멤버에는 이 기본값이 적용된다.
    int delay_ms{};      // int{}는 0으로 값 초기화한다.
};

// class의 기본 접근은 private이며, 검증을 통과한 재시도 정책의 내부 상태를 숨긴다.
class RetryPolicy {
public:
    // 생성자에는 반환형이 없다. explicit은 RawRetryOptions에서 RetryPolicy로의 뜻밖의 암시 변환을 막는다.
    explicit RetryPolicy(RawRetryOptions options)
        // 멤버 초기화 목록은 생성자 본문보다 먼저 두 int 멤버를 직접 초기화한다.
        : max_attempts_{options.max_attempts}, delay_ms_{options.delay_ms} {}

    // 뒤 const는 숨은 this 포인터가 const RetryPolicy*처럼 동작해 멤버를 바꾸지 못한다는 뜻이다.
    [[nodiscard]] bool allows(int completed_attempts) const noexcept {
        // &&는 왼쪽이 거짓이면 오른쪽 비교를 생략하며, 두 조건을 모두 만족한 bool prvalue를 반환한다.
        return completed_attempts >= 0 && completed_attempts < max_attempts_;
    }

    [[nodiscard]] int delay_ms() const noexcept { return delay_ms_; }

private:
    int max_attempts_{}; // private 멤버는 클래스 외부 대입으로 불변식을 깨뜨릴 수 없다.
    int delay_ms_{};
};

int main() {
    // 지정자는 선언 순서를 지켜야 한다. 생략된 delay_ms는 멤버 기본값 0을 사용한다.
    const RawRetryOptions raw{.max_attempts = 3};
    // explicit 생성자는 `RetryPolicy policy = raw;`를 금지하고 이 직접 초기화는 허용한다.
    const RetryPolicy policy{raw};

    // RetryPolicy::allows(int) const는 policy const lvalue를 수신 객체로, int prvalue 2를 값 매개변수로 받아 복사하고 bool prvalue를 반환한다.
    // policy/인자는 바뀌지 않고 할당·예외가 없으며 O(1)이다. 삼항 연산자는 반환값에 따라 두 정적 문자열 리터럴 중 하나를 고른다.
    // delay_ms() const는 명시 인자 없이 int prvalue 0을 반환하고 policy를 유지하며 O(1), 할당·예외가 없다.
    // operator<<는 std::cout 수신 객체에 문자를 쓰고 같은 ostream&를 반환하며, 반환 참조는 연쇄 후 저장하지 않는다.
    std::cout << (policy.allows(2) ? "allowed" : "blocked")
              << " delay=" << policy.delay_ms() << '\n';

    // raw/policy는 이름 있는 lvalue이고 allows 결과 bool은 prvalue다. 비교와 조건 분기가 예상되지만 실제 명령은 CPU·ABI·최적화에 따라 다르다.
    return policy.allows(2) && policy.delay_ms() == 0 ? 0 : 1;
}
