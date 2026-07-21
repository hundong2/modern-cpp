#include <cassert>
#include <iostream>
#include <string>
#include <variant>

struct Loading {};
// 빈 구조체도 서로 다른 상태를 타입으로 구분하며 멤버는 기본 public이다.
struct Success {
    std::string value;
};
struct Failure {
    int error_code{};
};

using Result = std::variant<Loading, Success, Failure>; // 동시에 한 후보 객체의 수명만 활성 상태다.
// using은 새 클래스를 만들지 않고 긴 variant 타입에 Result라는 별칭을 붙인다.

std::string message(const Result& result) {
    // const lvalue 참조로 받아 variant와 내부 string을 복사하지 않고 읽는다.
    // 연습: 아래 방문자에 Success와 Failure 처리를 직접 추가해 보세요.
    // 완성 예시는 CHECKPOINT.md의 정답을 푼 뒤 확인하세요.
    return std::visit(
        // 람다 자체는 컴파일러가 만드는 익명 클래스 객체이며 operator()를 가진다.
        [](const auto& current) -> std::string {
            using T = std::decay_t<decltype(current)>; // const T&에서 실제 상태 타입 T만 얻는다.
            if constexpr (std::is_same_v<T, Loading>) {
                return "불러오는 중";
            } else if constexpr (std::is_same_v<T, Success>) {
                return "성공: " + current.value;
            } else {
                return "실패: 오류 " + std::to_string(current.error_code); // 반환 임시 string은 prvalue다.
            }
        },
        result);
}

int main() {
    // Success{...}는 prvalue이며 Result 내부의 활성 저장소를 초기화한다.
    Result result{Loading{}}; // 오른쪽 빈 객체는 prvalue, result는 저장 공간을 가진 지역 lvalue다.
    assert(message(result) == "불러오는 중");

    // Success prvalue의 string을 variant 내부로 이동할 수 있어 문자 버퍼 복사를 줄일 수 있다.
    result = Success{"설정 저장됨"};
    assert(message(result) == "성공: 설정 저장됨");

    result = Failure{404};
    assert(message(result) == "실패: 오류 404");

    std::cout << "[검증 완료] Result 연습 문제가 통과했습니다.\n";
}
