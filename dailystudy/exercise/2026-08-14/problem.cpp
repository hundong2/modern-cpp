// <iostream>은 std::cout 출력 스트림을 제공한다.
#include <iostream>
// <memory>는 단일 소유권 포인터와 std::make_unique를 제공한다.
#include <memory>
// <utility>는 lvalue를 xvalue로 바꾸는 std::move를 제공한다.
#include <utility>

// struct는 기본 public이며 단순 결과 묶음에 알맞다.
struct Report { int value{}; }; // int 멤버를 중괄호로 0 초기화하며 public이라 호출자가 읽을 수 있다.

// final 클래스는 입력을 두 배로 만드는 작고 상태 없는 실무 처리 컴포넌트다.
class Doubler final {
public:
    [[nodiscard]] Report run(int input) const { return Report{input * 2}; } // prvalue 반환은 복사 생략 대상이다.
};

// 서비스는 작업자를 소유하고 호출을 위임해 생성·수명 관리와 계산 책임을 분리한다.
class ReportService final {
public:
    // 생성자에는 반환형이 없고 explicit은 뜻밖의 암시 변환을 막는다.
    explicit ReportService(std::unique_ptr<Doubler> worker) : worker_{std::move(worker)} {}
    [[nodiscard]] Report execute(int input) const { return worker_->run(input); } // ->로 소유 객체 함수를 호출하고 Report를 반환한다.
private:
    std::unique_ptr<Doubler> worker_{}; // private 멤버가 소유권과 수명을 가진다.
};

int main() {
    auto worker{std::make_unique<Doubler>()}; // make_unique 템플릿 호출 결과는 prvalue다.
    ReportService service{std::move(worker)}; // xvalue로 소유권을 복사 없이 넘긴다.
    const Report report{service.execute(21)}; // 반환값으로 목적 객체를 직접 초기화한다.
    std::cout << report.value << '\n'; // .으로 public 멤버를 읽고 << 연산자로 값과 줄바꿈을 출력한다.
    // 연습: move를 지우고 unique_ptr 복사 금지 진단을 읽어 본다.
    return report.value == 42 && !worker ? 0 : 1; // 계산값과 이동 후 빈 포인터를 검사해 성공(0) 또는 실패(1)를 반환한다.
}
