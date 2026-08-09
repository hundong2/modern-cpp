// <iostream>은 표준 출력 객체 std::cout을 제공한다.
#include <iostream>
// <numeric>은 범위의 합을 구하는 std::accumulate를 제공한다.
#include <numeric>
// <span>은 연속 메모리를 소유하지 않고 바라보는 std::span을 제공한다.
#include <span>
// <vector>는 크기가 실행 중 정해지는 연속 저장 컨테이너 std::vector를 제공한다.
#include <vector>

// struct의 기본 접근은 public이므로 단순 값 묶음에 알맞다.
struct ScoreSummary {
    int total{}; // int 기본 타입 멤버를 중괄호로 0 초기화한다.
    int count{}; // 멤버 변수는 객체 상태의 일부다.
};

// class의 기본 접근은 private이며 공개 API와 구현 상태를 분리한다.
class ScoreService {
public:
    // 생성자에는 반환형이 없다. explicit은 int 하나가 서비스로 암시적 변환되는 일을 막는다.
    explicit ScoreService(int bonus) : bonus_{bonus} {} // 멤버 초기화 목록으로 bonus_를 직접 초기화한다.

    // const span은 원소를 바꾸지 않는 비소유 뷰이고, 함수 뒤 const는 서비스 상태를 바꾸지 않음을 뜻한다.
    [[nodiscard]] ScoreSummary summarize(std::span<const int> scores) const {
        // accumulate의 템플릿 인자는 반복자 타입에서 추론되며 0은 합계 타입을 int로 정한다.
        const int raw{std::accumulate(scores.begin(), scores.end(), 0)};
        // 반환 식은 prvalue다. C++17 이후 결과 객체로 직접 만들어지는 복사 생략이 보장된다.
        return ScoreSummary{raw + bonus_, static_cast<int>(scores.size())};
    }

private:
    int bonus_{}; // private 멤버는 클래스 내부 함수만 직접 접근한다.
};

int main() {
    std::vector<int> owned{10, 20, 30}; // vector가 세 int의 수명과 메모리를 소유한다.
    const ScoreService service{5}; // explicit 생성자는 이처럼 직접 중괄호 초기화한다.
    // owned는 이름 있는 lvalue이고 span 매개변수에 비소유 참조가 바인딩된다. 호출 동안 owned가 살아 있다.
    const ScoreSummary result{service.summarize(owned)};
    std::cout << result.total << ' ' << result.count << '\n'; // << 연산자로 값과 문자를 출력한다.
    // 비교는 bool을 만들고 조건 연산자는 성공 0 또는 실패 1을 선택한다.
    // 로드·비교·조건 분기의 실제 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return result.total == 65 && result.count == 3 ? 0 : 1;
}
