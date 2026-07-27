#pragma once  // 헤더 전용 템플릿 정의의 중복 포함을 막습니다.

#include "modern_cpp/domain/task.hpp"  // 정렬 대상 TaskSnapshot과 Priority를 가져옵니다.

#include <algorithm>  // 제약 조건이 있는 std::ranges::sort 알고리즘을 제공합니다.
#include <concepts>   // 템플릿 요구 사항을 표현할 std::same_as concept를 제공합니다.
#include <ranges>     // range_value_t와 random_access_range concept를 제공합니다.

namespace modern_cpp::domain {  // 도메인 값에 적용되는 재사용 알고리즘을 같은 계층에 둡니다.

template <typename Range>  // 호출 시 실제 컨테이너 타입으로 치환되는 함수 템플릿을 선언합니다.
concept SnapshotRandomAccessRange =  // 긴 컴파일 오류 대신 의도가 드러나는 제약 이름을 만듭니다.
    std::ranges::random_access_range<Range> &&  // sort에 필요한 임의 접근 반복자를 요구합니다.
    std::same_as<std::ranges::range_value_t<Range>, TaskSnapshot>;  // 원소 값 타입이 정확히 TaskSnapshot인지 검사합니다.

template <SnapshotRandomAccessRange Range>  // 위 concept를 만족하는 vector 계열 범위만 인스턴스화할 수 있습니다.
void sort_by_priority_then_title(Range& tasks) {  // 비const 참조는 호출자의 컨테이너 순서를 제자리에서 변경함을 표시합니다.
    std::ranges::sort(tasks, [](const TaskSnapshot& left, const TaskSnapshot& right) {  // 반복자 쌍 대신 범위 전체를 전달합니다.
        if (left.priority != right.priority) {  // 우선순위가 다를 때 제목 비교를 생략하는 빠른 분기입니다.
            return left.priority > right.priority;  // enum의 기반 정수 값이 큰 high 항목을 앞으로 보냅니다.
        }  // 첫 번째 정렬 키 비교를 닫습니다.
        return left.title < right.title;  // 같은 우선순위에서는 문자열의 사전식 순서를 사용합니다.
    });  // introsort 계열 구현은 보통 O(N log N) 비교를 수행하지만 구체 구현은 표준이 강제하지 않습니다.
}  // 템플릿 정의는 사용하는 번역 단위에서 보여야 하므로 .cpp가 아니라 헤더에 둡니다.

}  // namespace modern_cpp::domain: 헤더 전용 정렬 알고리즘의 이름 공간을 닫습니다.
