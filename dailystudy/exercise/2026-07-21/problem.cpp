#include <expected>  // 성공값 또는 오류값 하나를 저장하는 std::expected를 쓴다.
#include <iostream>  // 검증 메시지를 표준 출력에 보낸다.
#include <string>    // 오류 설명의 수명을 소유하는 문자열이다.

struct DivideError {
    std::string reason;  // 호출이 끝나도 오류 문자가 남도록 직접 소유한다.
};

using DivideResult = std::expected<int, DivideError>;

// 반환형은 성공 int 또는 실패 DivideError다. 두 매개변수는 값으로 복사된다.
[[nodiscard]] DivideResult safe_divide(int numerator, int denominator) {
    // TODO 연습: denominator가 0이면 std::unexpected와 한글 오류를 반환하라.
    if (denominator == 0) {  // 비교 결과에 따라 나눗셈 여부를 조건 분기한다.
        return std::unexpected(DivideError{"0으로 나눌 수 없습니다."});
    }

    // 나눗셈 식의 결과는 prvalue이며 expected의 성공 저장소를 초기화한다.
    return numerator / denominator;
}

int main() {
    const DivideResult success{safe_divide(20, 4)};
    const DivideResult failure{safe_divide(20, 0)};

    // TODO 연습: 아래 검증을 가린 뒤 value(), error(), operator bool을 직접 사용해 보라.
    if (!success || *success != 5) {
        std::cout << "[검증 실패] 성공 경로를 확인하세요.\n";
        return 1;
    }

    if (failure || failure.error().reason.empty()) {
        std::cout << "[검증 실패] 오류 경로를 확인하세요.\n";
        return 1;
    }

    // *success는 const int&로 얻는 lvalue 식이며 값을 새로 만들지 않는다.
    std::cout << "20 / 4 = " << *success << '\n';
    std::cout << "오류: " << failure.error().reason << '\n';
    std::cout << "[검증 완료] 성공값과 오류값을 모두 확인했습니다.\n";

    // 이 작은 함수도 최적화 시 인라인될 수 있다. 특정 나눗셈/분기 명령을 쓴다고
    // 단정할 수 없으며 CPU·ABI·컴파일러·옵션에 따라 생성 코드가 달라진다.
    return 0;
}
