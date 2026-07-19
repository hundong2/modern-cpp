#include <cassert>
#include <iostream>
#include <string>
#include <variant>

struct Loading {};
struct Success {
    std::string value;
};
struct Failure {
    int error_code{};
};

using Result = std::variant<Loading, Success, Failure>;

std::string message(const Result& result) {
    // 연습: 아래 방문자에 Success와 Failure 처리를 직접 추가해 보세요.
    // 완성 예시는 CHECKPOINT.md의 정답을 푼 뒤 확인하세요.
    return std::visit(
        [](const auto& current) -> std::string {
            using T = std::decay_t<decltype(current)>;
            if constexpr (std::is_same_v<T, Loading>) {
                return "불러오는 중";
            } else if constexpr (std::is_same_v<T, Success>) {
                return "성공: " + current.value;
            } else {
                return "실패: 오류 " + std::to_string(current.error_code);
            }
        },
        result);
}

int main() {
    Result result{Loading{}};
    assert(message(result) == "불러오는 중");

    result = Success{"설정 저장됨"};
    assert(message(result) == "성공: 설정 저장됨");

    result = Failure{404};
    assert(message(result) == "실패: 오류 404");

    std::cout << "[검증 완료] Result 연습 문제가 통과했습니다.\n";
}
