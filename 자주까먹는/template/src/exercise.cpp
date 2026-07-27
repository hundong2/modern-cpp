#include "modern_cpp/domain/task.hpp"          // 연습에 사용할 TaskSnapshot과 Priority를 가져옵니다.
#include "modern_cpp/domain/task_sorting.hpp"  // 완성된 정렬 함수를 비교 기준으로 가져옵니다.

#include <iostream>  // 결과를 눈으로 확인할 표준 출력 stream을 제공합니다.
#include <vector>    // 연습 데이터의 연속 메모리 컨테이너를 제공합니다.

int main() {  // 독립 실행 가능한 연습 프로그램의 진입점입니다.
    using modern_cpp::domain::Priority;      // 우선순위 enum 이름을 간결하게 사용합니다.
    using modern_cpp::domain::TaskId;        // 강한 ID 타입 이름을 간결하게 사용합니다.
    using modern_cpp::domain::TaskSnapshot;  // 스냅샷 타입 이름을 간결하게 사용합니다.

    std::vector<TaskSnapshot> tasks{  // initializer_list에서 세 값을 복사해 vector의 연속 힙 배열을 만듭니다.
        {.id = TaskId{1}, .title = "write tests", .priority = Priority::normal, .completed = false, .tags = {"test"}},
        {.id = TaskId{2}, .title = "fix production bug", .priority = Priority::high, .completed = false, .tags = {"urgent"}},
        {.id = TaskId{3}, .title = "read documentation", .priority = Priority::low, .completed = true, .tags = {"study"}},
    };  // 연습용 vector 초기화를 닫습니다.

    // TODO 1: 아래 함수를 주석 처리하고 std::ranges::sort와 projection으로 제목순 정렬을 직접 작성해 보세요.
    modern_cpp::domain::sort_by_priority_then_title(tasks);  // 현재는 high → normal → low 순서로 안전하게 정렬합니다.

    // TODO 2: 완료되지 않은 작업만 보여 주는 std::views::filter 파이프라인을 만들어 보세요.
    for (const TaskSnapshot& task : tasks) {  // 현재는 정렬된 모든 원소를 읽기 전용 참조로 순회합니다.
        std::cout << task.id.value << ": " << task.title << '\n';  // ID와 제목을 한 줄에 출력합니다.
    }  // 연습 목록 순회를 닫습니다.

    // 예측 질문: tasks를 함수에 값으로 넘기면 string과 vector 내부 버퍼에는 어떤 복사가 일어날까요?
    return 0;  // 모든 vector와 string이 자동 파괴된 뒤 성공 종료합니다.
}  // main의 스택 프레임과 그 안의 소유 객체 수명이 끝납니다.
