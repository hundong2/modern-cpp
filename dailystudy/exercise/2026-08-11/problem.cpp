// <functional>은 객체를 소유하지 않는 참조 래퍼 std::reference_wrapper를 제공한다.
#include <functional>
// <iostream>은 표준 출력 기능을 제공한다.
#include <iostream>
// <optional>은 실패 가능한 결과 타입을 제공한다.
#include <optional>
// <string>은 문자열 소유 타입을 제공한다.
#include <string>
// <vector>는 연속 컨테이너를 제공한다.
#include <vector>

// 템플릿 인자 T는 호출 시 vector 원소 타입으로 정해진다.
template <class T>
[[nodiscard]] std::optional<std::reference_wrapper<const T>> find_first(const std::vector<T>& values, const T& target) {
    // values와 target은 const lvalue 참조라 소유권을 가져오지 않고 호출 중에만 읽는다.
    for (const T& value : values) {
        if (value == target) { // == 연산자가 두 T 값을 비교한다.
            // std::cref 함수 호출은 const 참조를 복사 가능한 래퍼로 만들며, 원본보다 오래 사용하면 안 된다.
            return std::cref(value);
        }
    }
    return std::nullopt; // 못 찾았음을 특별한 T 값 대신 타입의 빈 상태로 표현한다.
}

int main() {
    const std::vector<std::string> names{"Ada", "Bjarne"}; // const 소유 객체의 수명은 main 끝까지이다.
    const auto found{find_first(names, std::string{"Ada"})}; // 임시 string은 prvalue이고 호출이 끝날 때 파괴된다.
    // found가 참인 동안 get()은 names 안의 살아 있는 string lvalue를 돌려준다.
    std::cout << (found ? found->get() : std::string{"missing"}) << '\n';
    return found && found->get() == "Ada" ? 0 : 1; // 비교·조건 분기는 기계 코드가 되지만 구체 명령은 환경과 최적화에 따라 달라진다.
}
