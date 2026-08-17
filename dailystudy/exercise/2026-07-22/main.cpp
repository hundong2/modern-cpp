#include <concepts>  // std::convertible_to concept로 정책 반환형을 검사한다.
#include <cstddef>   // std::size_t: 크기와 인덱스를 나타내는 부호 없는 정수 타입이다.
#include <iostream>  // std::cout으로 결과를 표준 출력에 쓴다.
#include <span>      // std::span: 연속 메모리를 소유하지 않고 바라보는 C++20 뷰다.
#include <utility>   // std::move: lvalue 식을 xvalue 식으로 바꾸는 표준 함수다.
#include <vector>    // std::vector: 동적 배열의 메모리와 원소 수명을 소유한다.

// struct는 기본 접근이 public이다. 단순 결과 묶음은 공개 데이터가 자연스럽다.
struct Summary {
    double average{};       // double 기본 타입 멤버를 중괄호로 0.0 초기화한다.
    std::size_t count{};    // 멤버 변수 count는 포함된 점수 개수를 저장한다.
};

// class는 기본 접근이 private다. 저장소의 소유 데이터를 외부에서 직접 바꾸지 못하게 한다.
class ScoreStore {
public:
    // 생성자에는 반환형이 없다. explicit은 vector 하나가 ScoreStore로 암시 변환되는 일을 막는다.
    explicit ScoreStore(std::vector<int> initial)
        // 생성자 매개변수 initial을 xvalue로 만들어 멤버 초기화 목록에서 vector를 이동 생성한다.
        : scores_{std::move(initial)} {}

    // void는 반환값이 없다는 반환형이고, int value는 값으로 받는 매개변수다.
    void add(int value) {
        // 표준 호출 계약: scores_는 vector<int> 수신 객체이고 push_back(const int&)에 value를 입력한다.
        // 반환형은 void이고 성공하면 원소 수가 1 늘어난다. 상각 O(1), 재할당 시 기존 span·포인터·참조가 무효가 된다.
        scores_.push_back(value);
    }

    // const 멤버 함수는 *this를 통해 scores_를 수정하지 않겠다고 약속한다.
    [[nodiscard]] std::span<const int> scores() const {
        // prvalue span을 반환한다. 원소 복사는 없고 scores_가 원소 수명을 계속 소유한다.
        return scores_;
    }

private:
    std::vector<int> scores_;  // private 멤버 변수: 점수 메모리의 단일 소유자다.
};

// 정책 타입 P가 accept(int)를 호출할 수 있고 결과가 bool로 변환 가능해야 한다.
template <typename P>
concept ScorePolicy = requires(const P& policy, int value) {
    { policy.accept(value) } -> std::convertible_to<bool>;
};

struct PositiveOnly {
    // [[nodiscard]]는 반환값을 버린 실수를 컴파일러가 경고할 수 있게 한다.
    [[nodiscard]] bool accept(int value) const {
        return value > 0;  // > 비교 연산자는 bool을 만들며 이후 조건 분기에 사용된다.
    }
};

// using은 긴 타입을 새 별명으로 읽기 쉽게 만든다. 소유권은 바꾸지 않는다.
using ScoreView = std::span<const int>;

// template의 P는 컴파일 때 정해지는 타입 인자이며 ScorePolicy 요구를 만족해야 한다.
template <ScorePolicy P>
class ScoreService {
public:
    // 생성자에는 반환형이 없고, 정책을 값으로 받아 서비스가 소유하도록 이동한다.
    explicit ScoreService(P policy) : policy_{std::move(policy)} {}

    [[nodiscard]] Summary summarize(ScoreView values) const {
        int total{0};             // int 기본 타입 변수를 중괄호로 0 초기화한다.
        std::size_t count{0};     // 지역 변수 count는 함수 호출 동안만 살아 있다.

        // const int value는 각 원소를 읽기 전용 값으로 복사한다. 반복문은 끝까지 반복한다.
        for (const int value : values) {
            // &&는 왼쪽부터 평가하는 논리곱이다. 정책 함수 호출 결과로 조건 분기한다.
            if (policy_.accept(value) && value <= 100) {
                total += value;  // += 연산자는 기존 total을 읽고 더한 뒤 새 값을 저장한다.
                ++count;         // 전위 ++ 연산자는 count를 1 증가시킨다.
            }
        }

        // == 비교로 0 나눗셈을 피한다. ?: 조건 연산자는 둘 중 한 값을 고른다.
        const double average{count == 0
                                 ? 0.0
                                 : static_cast<double>(total) / static_cast<double>(count)};
        return Summary{average, count};  // prvalue 결과 객체는 보통 복사 생략으로 직접 만들어진다.
    }

private:
    P policy_;  // 템플릿 인자 P 타입의 정책 멤버를 서비스가 값으로 소유한다.
};

// 함수 반환형 ScoreStore와 빈 매개변수 목록 ()을 가진 팩토리 함수다.
ScoreStore make_store() {
    // explicit 생성자는 ScoreStore{...}처럼 올바른 직접 초기화로 호출한다.
    return ScoreStore{std::vector<int>{80, -5, 100, 70}};
}

int main() {
    ScoreStore store{make_store()};  // 반환 prvalue로 store를 직접 초기화하며 복사 생략될 수 있다.
    store.add(90);                   // . 연산자로 멤버 함수 add를 호출한다.

    const ScoreView view{store.scores()};  // 임시 span을 복사하지만 원소는 store가 계속 소유한다.
    // span::front()는 인자가 없고 첫 int의 const 참조를 O(1)에 반환한다. 비어 있지 않아야 하며 view는 바뀌지 않는다.
    const int& first{view.front()};
    // span::data()는 인자 없이 첫 원소 const int*를 반환한다. 소유권은 없고 store 재할당·소멸 시 무효가 된다.
    const int* pointer{view.data()};

    ScoreService<PositiveOnly> service{PositiveOnly{}};  // <> 안의 템플릿 타입 인자를 명시한다.
    const Summary result{service.summarize(view)};       // const 결과는 초기화 뒤 바꿀 수 없다.

    // << 연산자는 값을 출력 스트림에 차례로 보내고 '\n' 문자는 줄을 바꾼다.
    std::cout << "첫 점수: " << first << ", 포인터가 가리킨 값: " << *pointer << '\n';
    std::cout << "유효 점수 " << result.count << "개의 평균: " << result.average << '\n';

    // 0 반환은 운영체제에 정상 종료를 알린다.
    return 0;
}
