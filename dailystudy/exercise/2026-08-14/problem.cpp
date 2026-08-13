// 출력, 단일 소유권, 이동을 위한 표준 헤더다.
#include <iostream>
#include <memory>
#include <utility>

// struct는 기본 public이며 단순 결과 묶음에 알맞다.
struct Report { int value{}; };

class Doubler final {
public:
    [[nodiscard]] Report run(int input) const { return Report{input * 2}; } // prvalue 반환은 복사 생략 대상이다.
};

class ReportService final {
public:
    // 생성자에는 반환형이 없고 explicit은 뜻밖의 암시 변환을 막는다.
    explicit ReportService(std::unique_ptr<Doubler> worker) : worker_{std::move(worker)} {}
    [[nodiscard]] Report execute(int input) const { return worker_->run(input); }
private:
    std::unique_ptr<Doubler> worker_{}; // private 멤버가 소유권과 수명을 가진다.
};

int main() {
    auto worker{std::make_unique<Doubler>()}; // make_unique 템플릿 호출 결과는 prvalue다.
    ReportService service{std::move(worker)}; // xvalue로 소유권을 복사 없이 넘긴다.
    const Report report{service.execute(21)}; // 반환값으로 목적 객체를 직접 초기화한다.
    std::cout << report.value << '\n';
    // 연습: move를 지우고 unique_ptr 복사 금지 진단을 읽어 본다.
    return report.value == 42 && !worker ? 0 : 1;
}
