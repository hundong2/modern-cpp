#include "modern_cpp/domain/task.hpp"  // 이 번역 단위가 구현할 공개 클래스 선언을 가장 먼저 포함합니다.

#include <algorithm>  // 중복 태그를 찾는 std::ranges::find를 제공합니다.

namespace modern_cpp::domain {  // 헤더 선언과 같은 이름 공간에서 멤버 함수를 정의합니다.

Task::Task(TaskId id, std::string title, Priority priority, std::vector<std::string> tags)  // 호출자가 만든 소유 값들을 받습니다.
    : id_{id},                    // 작은 TaskId는 객체 내부 멤버로 복사합니다.
      title_{std::move(title)},   // string의 버퍼 포인터를 이동해 문자 배열의 깊은 복사를 피합니다.
      priority_{priority},        // 1바이트 enum 값을 멤버에 복사합니다.
      tags_{std::move(tags)} {    // vector의 힙 배열 소유권을 상수 시간에 넘깁니다.
}  // 생성자 본문이 끝나면 매개변수는 파괴되지만 이동된 자원은 멤버가 계속 소유합니다.

void Task::add_tag(std::string tag) {  // 태그 소유권을 받을 수 있도록 값 매개변수를 사용합니다.
    if (tag.empty()) {                 // 빈 문자열은 도메인 데이터로 보관할 의미가 없으므로 걸러냅니다.
        return;                        // 아직 vector를 변경하지 않았으므로 강한 예외 안전성을 유지합니다.
    }  // 빈 태그 검사 분기를 닫습니다.

    if (std::ranges::find(tags_, tag) != tags_.end()) {  // 기존 문자열을 선형 탐색해 중복인지 확인합니다.
        return;                                           // 중복이면 힙 재할당 없이 즉시 종료합니다.
    }  // 중복 검사 분기를 닫습니다.

    tags_.push_back(std::move(tag));  // 필요하면 vector가 더 큰 연속 배열을 할당하고 기존 원소를 이동한 뒤 새 태그를 소유합니다.
}  // 지역 매개변수는 파괴되며 이동되지 않은 경우에도 자신의 버퍼를 자동 해제합니다.

TaskSnapshot make_snapshot(const Task& task) {  // Task의 수명에 종속되지 않는 읽기 값을 만듭니다.
    return TaskSnapshot{                       // 반환 값 최적화로 임시 객체 복사가 생략될 수 있습니다.
        .id = task.id(),                        // 작은 ID를 값 복사합니다.
        .title = task.title(),                  // std::string을 깊게 복사해 독립 버퍼를 확보합니다.
        .priority = task.priority(),            // enum 값을 복사합니다.
        .completed = task.completed(),          // bool 상태를 복사합니다.
        .tags = task.tags(),                    // vector와 각 string을 깊게 복사합니다.
    };  // 지정 초기화자는 멤버 이름을 보여 주어 필드 순서 실수를 줄입니다.
}  // 스냅샷을 호출자에게 값으로 반환합니다.

}  // namespace modern_cpp::domain: Task 구현 이름 공간을 닫습니다.
