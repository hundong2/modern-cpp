// <array>는 고정 크기 버퍼를 제공한다.
#include <array>
// <cstddef>는 std::byte를 제공한다.
#include <cstddef>
// <iostream>은 결과 출력에 사용한다.
#include <iostream>
// <memory_resource>는 주입 가능한 할당 자원과 pmr::vector를 제공한다.
#include <memory_resource>
// <utility>는 std::move를 제공한다.
#include <utility>
// <vector>는 pmr::vector 선언의 기반 컨테이너를 제공한다.
#include <vector>

using NumberList = std::pmr::vector<int>; // using은 긴 템플릿 형식에 읽기 쉬운 별칭을 붙인다.

[[nodiscard]] NumberList make_doubled(std::pmr::memory_resource* resource) {
    NumberList result{resource}; // 지역 객체가 주입된 자원을 참조한다.
    for (int value{1}; value <= 3; ++value) { // 초기화·조건·증가로 1, 2, 3을 반복한다.
        result.push_back(value * 2); // * 연산 결과 prvalue를 벡터 끝에 저장한다.
    }
    return result; // 반환값 최적화 또는 이동으로 복사를 피하며 자원은 계속 살아 있어야 한다.
}

int main() {
    std::array<std::byte, 512> storage{};
    std::pmr::monotonic_buffer_resource resource{storage.data(), storage.size()};
    NumberList numbers{make_doubled(&resource)}; // 함수 결과 prvalue로 목적지를 직접 초기화할 수 있다.
    NumberList moved{std::move(numbers)}; // std::move는 lvalue를 xvalue로 바꾸며 저장소 소유 상태를 이동한다.
    int sum{}; // 중괄호 초기화로 0이다.
    for (const int value : moved) { // 범위 for가 각 int를 읽기 전용 값으로 순회한다.
        sum += value; // +=는 기존 값을 읽고 더한 뒤 같은 변수에 저장한다.
    }
    std::cout << sum << '\n';
    // numbers는 유효하지만 이동 후 상태이므로 내용에 의존하지 않는다. 자원은 두 벡터보다 나중에 파괴된다.
    return sum == 12 ? 0 : 1;
}
