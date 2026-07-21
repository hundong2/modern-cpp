#include <cassert>   // assert 함수로 연습 결과가 기대값인지 실행 중 검증한다.
#include <concepts>  // std::integral concept로 정수 타입만 받게 제한한다.
#include <iostream>  // std::cout 표준 라이브러리 객체로 통과 메시지를 출력한다.
#include <span>      // std::span 비소유 연속 뷰를 사용한다.
#include <vector>    // std::vector가 테스트 데이터의 메모리와 수명을 소유한다.

// T는 템플릿 타입 인자이며 std::integral을 만족하는 기본 정수 타입이어야 한다.
template <std::integral T>
[[nodiscard]] T sum_non_negative(std::span<const T> values) {
    T sum{};  // T 타입 변수를 중괄호 값 초기화하여 정수 0으로 만든다.

    // const T&는 원소 lvalue에 읽기 전용 참조를 바인딩해 원소를 복사하지 않는다.
    for (const T& value : values) {
        // >= 비교 연산자가 bool을 만들고 if 조건문이 더할지 분기한다.
        if (value >= T{0}) {
            sum += value;  // +=는 sum을 로드하고 value를 더해 다시 저장하는 의미다.
        }
    }
    return sum;  // 지역 정수 값을 반환한다. 함수가 끝나면 지역 변수의 수명은 끝난다.
}

int main() {
    // vector가 원소를 소유한다. 이름 있는 numbers는 lvalue 식이다.
    const std::vector<int> numbers{3, -2, 7, 0};
    // span prvalue로 view를 직접 초기화한다. view는 numbers보다 먼저 파괴되어 안전하다.
    const std::span<const int> view{numbers};

    // 함수의 <int> 템플릿 인자는 인수 view에서 추론되므로 보통 생략할 수 있다.
    const int answer{sum_non_negative(view)};
    assert(answer == 10);  // == 연산자로 실제 값과 기대값을 비교한다.

    // data()의 반환 포인터는 소유하지 않는다. * 역참조 연산자로 첫 원소 lvalue를 읽는다.
    const int* first{view.data()};
    assert(first != nullptr && *first == 3);  // !=와 &&로 두 조건이 모두 참인지 검사한다.

    std::cout << "연습 통과: 합계 = " << answer << '\n';
    return 0;  // int 반환형 main의 정상 종료 코드다.
}
