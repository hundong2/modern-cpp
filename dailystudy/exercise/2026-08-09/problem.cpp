// <algorithm>은 최댓값을 찾는 std::max_element를 제공한다.
#include <algorithm>
// <span>은 포인터와 길이를 안전하게 묶은 비소유 범위 뷰를 제공한다.
#include <span>
// <vector>는 데이터를 소유하는 동적 배열을 제공한다.
#include <vector>

// using은 긴 템플릿 타입에 읽기 쉬운 별칭을 붙인다.
using Readings = std::vector<int>;

// const 원소 span을 값으로 받으면 뷰 자체는 복사하되 원소는 복사하거나 수정하지 않는다.
[[nodiscard]] int peak(std::span<const int> values) {
    // if 조건문은 empty()가 반환한 bool을 검사해 빈 범위의 역참조를 막는다.
    if (values.empty()) {
        return 0; // 빈 입력에 대한 이 예제의 계약은 0이다.
    }
    // max_element는 [begin, end) 반복자 범위를 순회하며 최댓값 위치를 반환한다. 시간 O(N), 추가 공간 O(1)이다.
    return *std::max_element(values.begin(), values.end()); // * 연산자는 유효한 반복자가 가리키는 int를 읽는다.
}

int main() {
    Readings readings{4, 9, 2}; // vector 소유 객체를 prvalue 초기화 목록으로 직접 생성한다.
    const int answer{peak(readings)}; // readings lvalue의 저장소를 span이 호출 동안 관찰한다.
    // readings가 파괴되거나 재할당되면 저장해 둔 span은 댕글링된다. 여기서는 span을 함수 밖에 보관하지 않는다.
    return answer == 9 ? 0 : 1;
}
