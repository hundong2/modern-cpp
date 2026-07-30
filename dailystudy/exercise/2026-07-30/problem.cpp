// <chrono>는 강한 시간 타입 duration과 시간 리터럴의 기반 기능을 제공한다.
#include <chrono>
// <iostream>은 결과를 화면에 쓰는 std::cout과 << 연산자를 제공한다.
#include <iostream>
// <string>은 문자를 소유하는 표준 라이브러리 타입 std::string을 제공한다.
#include <string>
// <utility>는 문자열의 자원을 이동할 때 쓰는 std::move를 제공한다.
#include <utility>

// 밀리초 타입에 짧은 별칭을 붙인다. duration의 템플릿 인자는 표현 타입과 단위 비율이다.
using Millis = std::chrono::milliseconds;

// struct의 기본 접근은 public이다. 공개할 단순 입력 데이터 묶음이다.
struct Job {
    int failures{};      // int 기본 정수 변수이며 {}는 0으로 값 초기화한다.
    Millis elapsed{};    // 경과 시간을 숫자만이 아니라 단위까지 포함해 보관한다.
};

// class의 기본 접근은 private라서 멤버 상태를 외부에서 함부로 바꿀 수 없다.
class RetryRule {
public:
    // 생성자는 반환형이 없다. explicit은 문자열 하나의 암시적 변환을 막는다.
    explicit RetryRule(std::string name, int limit, Millis delay)
        // 생성자 매개변수로 멤버를 직접 초기화하며, 문자열은 복사 대신 이동한다.
        : name_{std::move(name)}, limit_{limit}, delay_{delay} {}

    // 반환형 bool, const Job& 매개변수다. const 참조는 Job을 복사하지 않고 읽기만 한다.
    [[nodiscard]] bool expired(const Job& job) const {
        // TODO 연습: >=를 >로 바꿔 경계값 의미가 어떻게 달라지는지 확인한다.
        return job.elapsed >= delay_;
    }

    [[nodiscard]] bool may_retry(const Job& job) const {
        // &&는 두 조건이 모두 참인지 검사하며, 왼쪽이 거짓이면 expired 호출을 생략한다.
        if (job.failures < limit_ && expired(job)) {
            return true;
        }
        return false;
    }

    // std::string의 const 참조를 반환하므로 호출자는 소유권을 얻지 않고 내부 문자열을 빌린다.
    [[nodiscard]] const std::string& name() const { return name_; }

private:
    std::string name_; // 객체가 문자열 자원을 소유하며 객체 소멸 때 자동 해제한다.
    int limit_{};      // 허용 실패 횟수를 저장하는 멤버 변수다.
    Millis delay_{};   // 재시도 전 기다릴 시간을 저장하는 멤버 변수다.
};

int main() { // main 함수 호출은 프로그램 시작점이며 int 반환값은 종료 상태다.
    using namespace std::chrono_literals; // 1500ms 리터럴을 현재 범위에서 사용한다.

    // RetryRule{...} 직접 초기화는 explicit 생성자를 올바르게 호출한다.
    const RetryRule rule{"network", 3, 1500ms};
    // 이름 있는 job은 lvalue이고 Job{...}로 만든 초기값은 prvalue로 볼 수 있다.
    const Job job{2, 1500ms};

    // 포인터는 주소를 저장하며 &job은 lvalue 객체 job의 주소를 얻는 단항 연산자다.
    const Job* const pointer{&job};
    // 참조는 기존 객체에 바인딩한 별칭이다. job이 더 오래 살아 있으므로 수명이 안전하다.
    const Job& reference{job};

    // if는 비교 결과에 따라 조건 분기한다. ->는 포인터를 통한 멤버 접근이다.
    if (pointer != nullptr && rule.may_retry(reference)) {
        // << 연산자와 표준 라이브러리 함수 name(), count()를 실제 등장 위치에서 사용한다.
        std::cout << rule.name() << " retry after " << pointer->elapsed.count() << "ms\n";
    }

    // 반복문은 초기화된 int 변수 i를 비교하고 ++ 연산자로 증가시킨다.
    for (int i{0}; i < 2; ++i) {
        std::cout << "check " << i << '\n';
    }

    // may_retry 함수 호출 결과를 비교해 성공 0 또는 실패 1을 반환한다.
    return rule.may_retry(job) ? 0 : 1;
}
