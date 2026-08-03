// <iostream>은 연습 결과 출력에 필요하다.
#include <iostream>
// <memory>는 unique_ptr와 make_unique로 정책의 수명을 관리한다.
#include <memory>
// <utility>는 std::move를 제공한다.
#include <utility>

// 공개 멤버가 자연스러운 단순 데이터 묶음이므로 기본 public인 struct를 쓴다.
struct Score { int value{}; }; // 짧은 선언에서 int와 중괄호 0 초기화를 함께 확인한다.

class BonusRule { // class의 멤버는 public: 전까지 기본 private다.
public:
    virtual ~BonusRule() = default; // 기반 포인터 삭제를 안전하게 하는 가상 소멸자다.
    [[nodiscard]] virtual Score apply(const Score& score) const = 0; // 순수 가상 함수가 포트 계약이다.
};

class AddBonus final : public BonusRule {
public:
    explicit AddBonus(int bonus) : bonus_{bonus} {} // 반환형 없는 생성자와 직접 멤버 초기화다.
    [[nodiscard]] Score apply(const Score& score) const override {
        return Score{score.value + bonus_}; // + 결과 prvalue로 Score를 직접 만들어 반환한다.
    }
private:
    int bonus_{}; // 외부에 숨긴 정책 상태다.
};

using RuleOwner = std::unique_ptr<BonusRule>; // unique_ptr<기반형>의 읽기 쉬운 타입 별칭이다.

[[nodiscard]] Score run_rule(RuleOwner rule, const Score& input) {
    // rule은 이 함수가 소유한다. -> 호출은 실제 AddBonus 구현으로 가상 간접 호출될 수 있다.
    return rule->apply(input);
} // 함수가 끝나면 unique_ptr 소멸자가 정책 객체를 자동 해제한다.

int main() {
    auto rule = std::make_unique<AddBonus>(7); // auto가 unique_ptr<AddBonus> 타입을 추론한다.
    // std::move(rule)는 xvalue이며 복사 불가능한 소유권을 값 매개변수로 이동한다.
    const Score result{run_rule(std::move(rule), Score{35})};
    std::cout << result.value << '\n';
    // 기계 관점에서는 값 로드·덧셈·비교·조건 분기·함수 호출이 생길 수 있으나 CPU·ABI·컴파일러·최적화에 따라 다르다.
    return result.value == 42 ? 0 : 1;
}
