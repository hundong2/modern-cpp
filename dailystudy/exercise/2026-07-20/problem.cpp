/*
[기초 문법부터 읽는 순서]
1. 빈 구조체 Loading도 '로딩 중'이라는 서로 다른 상태 타입으로 사용할 수 있습니다.
2. Success와 Failure는 각 상태에서만 유효한 값과 오류 코드를 따로 보관합니다.
3. variant<Loading, Success, Failure>는 세 상태 중 하나만 저장합니다.
4. message 함수는 Result를 const 참조로 받아 원본을 복사·수정하지 않습니다.
5. std::visit는 현재 상태를 제네릭 람다의 current에 전달합니다.
6. is_same_v<T, Loading>은 두 타입이 같은지를 컴파일 시점 bool 값으로 알려 줍니다.
7. if constexpr/else if constexpr/else는 타입에 맞는 문구 하나를 반환합니다.
8. std::to_string은 int 오류 코드를 문자열로 바꾸고 + 연산자는 문자열을 연결합니다.
9. main에서 `result = ...`는 기존 상태를 새 상태로 바꾸며 assert가 각 결과를 검증합니다.
*/

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
