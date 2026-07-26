// <algorithm>은 정렬 알고리즘 std::sort를 제공한다.
#include <algorithm>
// <compare>는 C++20 비교 범주와 삼방향 비교를 제공한다.
#include <compare>
// <iostream>은 결과를 화면에 출력하는 std::cout을 제공한다.
#include <iostream>
// <string>은 문자열을 소유하는 std::string을 제공한다.
#include <string>
// <utility>는 두 값을 묶는 std::pair를 제공한다.
#include <utility>
// <vector>는 여러 원소를 소유하는 동적 배열 std::vector를 제공한다.
#include <vector>

// class는 기본 접근이 private이므로 값 검증을 통과한 경로만 public으로 연다.
class Score {
public:
    // 생성자에는 반환형이 없다. explicit은 Score score = 80; 같은 암시적 변환을 막는다.
    explicit Score(int value)
        // 멤버 초기화 목록과 중괄호 초기화로 value_를 0~100 범위로 보정한다.
        : value_{value < 0 ? 0 : (value > 100 ? 100 : value)} {}

    // const 함수는 객체를 변경하지 않는다. 반환형 int는 점수 값을 복사해 돌려준다.
    [[nodiscard]] int value() const { return value_; }

    // TODO 1: = default를 지우고 value_를 사용한 operator<=> 본문을 직접 작성해 보자.
    auto operator<=>(const Score&) const = default;
    bool operator==(const Score&) const = default;

private:
    // 멤버 변수는 Score 객체 수명 동안 객체 안에 값으로 존재하며 별도 힙 소유권이 없다.
    int value_;
};

// struct는 기본 접근이 public이다. 연습에서는 이름과 점수를 공개 데이터로 묶는다.
struct Student {
    std::string name;
    Score score;
};

// using 별칭은 pair의 의미를 읽기 쉽게 하지만 새로운 별도 타입을 만들지는 않는다.
using ScoreRange = std::pair<Score, Score>;

// 템플릿 인자 T는 비교 가능한 실제 타입으로 치환된다.
template <typename T>
[[nodiscard]] bool inside(const T& value, const T& low, const T& high) {
    // &&는 왼쪽이 거짓이면 오른쪽을 계산하지 않는 단락 평가 논리 연산자다.
    return !(value < low) && !(high < value);
}

// 함수 반환형 ScoreRange는 두 Score를 소유하는 pair 값을 돌려준다.
[[nodiscard]] ScoreRange passing_range() {
    // 두 prvalue Score{...}로 pair 결과 객체를 만든다. prvalue 반환은 목적지에 직접 만들어질 수 있다.
    return ScoreRange{Score{60}, Score{100}};
}

int main() {
    // vector가 Student와 내부 문자열을 소유한다. 각 Score는 explicit 생성자를 직접 중괄호로 호출한다.
    std::vector<Student> students{{"민수", Score{72}}, {"지수", Score{95}}, {"준호", Score{55}}};

    // 람다 매개변수의 const Student&는 원소를 복사하지 않는 비소유 참조다.
    std::sort(students.begin(), students.end(), [](const Student& left, const Student& right) {
        // TODO 2: 높은 점수가 먼저 오게 하려면 < 를 > 로 바꾸고 결과를 관찰하자.
        return left.score < right.score;
    });

    const ScoreRange range{passing_range()};
    // 반복할 때 student는 vector 원소에 바인딩된 lvalue const 참조다.
    for (const Student& student : students) {
        // 함수 호출에서 템플릿 인자 Score가 추론된다.
        if (inside(student.score, range.first, range.second)) {
            std::cout << student.name << " 합격: " << student.score.value() << '\n';
        } else {
            std::cout << student.name << " 재도전\n";
        }
    }

    // 임시 Score{80}은 prvalue다. const Score&는 이 완전 표현식 동안 임시 객체에 바인딩된다.
    const Score& temporary_lifetime_extended{Score{80}};
    std::cout << "참조로 수명이 연장된 점수: " << temporary_lifetime_extended.value() << '\n';

    // 비교/분기는 로드·비교·조건 선택으로 구현될 수 있지만 명령은 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return 0;
}
