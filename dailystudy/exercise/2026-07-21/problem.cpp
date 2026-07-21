#include <expected>  // 성공값 또는 오류값 하나를 저장하는 std::expected를 쓴다.
#include <iostream>  // 검증 메시지를 표준 출력에 보낸다.
#include <string>    // 오류 설명의 수명을 소유하는 문자열이다.

struct DivideError {
    // struct 멤버는 기본 public이어서 failure.error().reason으로 접근할 수 있다.
    std::string reason;  // 호출이 끝나도 오류 문자가 남도록 직접 소유한다.
};

// using은 긴 expected 타입의 별칭이다. 새 타입을 별도로 정의하지는 않는다.
// 첫 템플릿 인자 int는 성공값, 둘째 DivideError는 실패값의 타입이다.
using DivideResult = std::expected<int, DivideError>;

// 반환형은 성공 int 또는 실패 DivideError다. 두 매개변수는 값으로 복사된다.
[[nodiscard]] DivideResult safe_divide(int numerator, int denominator) {
    // [[nodiscard]]는 나눗셈의 성공/실패 결과를 버리는 실수를 경고하게 한다.
    // TODO 연습: denominator가 0이면 std::unexpected와 한글 오류를 반환하라.
    if (denominator == 0) {  // 비교 결과에 따라 나눗셈 여부를 조건 분기한다.
        // unexpected prvalue가 DivideError를 expected의 오류 저장소에 구성한다.
        return std::unexpected(DivideError{"0으로 나눌 수 없습니다."});
    }

    // 나눗셈 식의 결과는 prvalue이며 expected의 성공 저장소를 초기화한다.
    return numerator / denominator;
}

int main() {
    // const 지역 변수는 초기화 뒤 다른 expected 상태로 대입할 수 없다.
    // 중괄호 안 함수 반환값은 prvalue이며 각 지역 객체를 초기화한다.
    const DivideResult success{safe_divide(20, 4)};
    const DivideResult failure{safe_divide(20, 0)};

    // TODO 연습: 아래 검증을 가린 뒤 value(), error(), operator bool을 직접 사용해 보라.
    if (!success || *success != 5) {
        // 논리 부정 !는 성공 여부를 뒤집고, ||는 어느 한 조건만 참이어도 참이다.
        std::cout << "[검증 실패] 성공 경로를 확인하세요.\n";
        return 1;
    }

    if (failure || failure.error().reason.empty()) {
        // failure가 성공이면 왼쪽만으로 참이라 오른쪽 error 접근을 실행하지 않는다.
        // empty()는 문자열 길이가 0인지 bool로 알려주는 const 멤버 함수다.
        std::cout << "[검증 실패] 오류 경로를 확인하세요.\n";
        return 1;
    }

    // *success는 const int&로 얻는 lvalue 식이며 값을 새로 만들지 않는다.
    std::cout << "20 / 4 = " << *success << '\n';
    // '\n'은 줄바꿈 문자 하나이며 endl과 달리 스트림 강제 flush를 요구하지 않는다.
    std::cout << "오류: " << failure.error().reason << '\n';
    std::cout << "[검증 완료] 성공값과 오류값을 모두 확인했습니다.\n";

    // 이 작은 함수도 최적화 시 인라인될 수 있다. 특정 나눗셈/분기 명령을 쓴다고
    // 단정할 수 없으며 CPU·ABI·컴파일러·옵션에 따라 생성 코드가 달라진다.
    return 0;
}
