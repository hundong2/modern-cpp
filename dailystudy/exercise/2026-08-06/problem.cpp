// <functional>은 std::function 호출 래퍼를 제공한다.
#include <functional>
// <iostream>은 결과를 표준 출력에 쓰는 std::cout을 제공한다.
#include <iostream>
// <utility>는 std::move를 제공한다.
#include <utility>
// <vector>는 동적 배열 std::vector를 제공한다.
#include <vector>

using Operation = std::function<int(int)>; // 함수 서명을 타입 별칭으로 표현한다.

[[nodiscard]] int apply_all(int value, const std::vector<Operation>& operations) {
    // const 참조 매개변수는 컨테이너를 복사하지 않으며 호출 동안 원본 수명이 유지되어야 한다.
    for (const Operation& operation : operations) { // 각 std::function lvalue를 읽기 전용 참조로 바인딩한다.
        value = operation(value); // ()가 저장된 호출 대상을 실행하고 반환 int를 다음 상태로 저장한다.
    }
    return value; // 기본 타입은 값으로 반환한다.
}

int main() {
    std::vector<Operation> operations{}; // 빈 벡터를 중괄호로 직접 초기화한다.
    Operation square{[](int value) { return value * value; }}; // 람다 prvalue를 std::function이 소유한다.
    operations.push_back(std::move(square)); // std::move는 square lvalue를 xvalue로 바꾸어 내부 호출 객체의 이동을 허용한다.
    operations.emplace_back([](int value) { return value - 1; }); // emplace_back은 람다에서 원소를 컨테이너 안에 직접 생성한다.
    const int answer{apply_all(5, operations)}; // 5는 prvalue이고 operations는 const 참조에 바인딩된다.
    std::cout << answer << '\n'; // 예상 결과 24를 출력한다.
    // square는 이동 후에도 유효하지만 비어 있을 수 있어 호출하지 않는다. 벡터가 호출 객체의 수명을 소유한다.
    // 간접 호출의 구체적 구현과 명령 선택은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return answer == 24 ? 0 : 1;
}
