#include "modern_cpp/infrastructure/in_memory_task_repository.hpp"  // 구현할 저장소 클래스 선언을 가져옵니다.

#include <utility>  // unique_ptr 소유권을 map으로 넘기는 std::move를 제공합니다.

namespace modern_cpp::infrastructure {  // infrastructure 계층 이름 공간에서 저장 방식을 정의합니다.

std::expected<void, std::string> InMemoryTaskRepository::save(  // 저장 성공 또는 오류를 반환합니다.
    std::unique_ptr<domain::Task> task) {                        // 호출자가 가진 Task 독점 소유권을 값으로 받습니다.
    if (!task) {                                                 // null unique_ptr는 저장할 객체가 없음을 뜻합니다.
        return std::unexpected{"cannot save a null task"};       // null 역참조 전에 안전한 오류 대안을 반환합니다.
    }  // null 검사 분기를 닫습니다.

    const domain::TaskId id = task->id();  // 소유권을 이동하기 전에 키 값을 작은 값 객체로 복사합니다.
    if (tasks_.contains(id)) {              // C++20 map::contains로 중복 키를 O(log N)에 검사합니다.
        return std::unexpected{"duplicate task id"};  // 함수 종료 시 아직 소유 중인 task가 자동 파괴됩니다.
    }  // 중복 검사 분기를 닫습니다.

    tasks_.emplace(id, std::move(task));  // 트리 노드를 할당하고 unique_ptr를 이동해 Task 소유권을 map에 넘깁니다.
    try {                                 // 두 컨테이너를 함께 갱신할 때 불일치 상태를 복구하기 위한 경계입니다.
        insertion_order_.push_back(id);   // list 노드를 별도 할당해 삽입 순서를 기록합니다.
    } catch (...) {                       // 메모리 부족 등 예상 밖 예외의 구체 타입을 바꾸지 않고 받습니다.
        tasks_.erase(id);                 // 방금 넣은 map 노드와 Task를 제거해 저장소 불변식을 되돌립니다.
        throw;                            // 원래 예외를 그대로 다시 던져 상위 장애 처리 계층에 전달합니다.
    }  // 두 컨테이너 갱신의 예외 안전 경계를 닫습니다.

    return {};  // expected<void, string>의 성공 상태를 반환합니다.
}  // task 매개변수는 이동 후 null이며 파괴 시 아무 객체도 삭제하지 않습니다.

domain::Task* InMemoryTaskRepository::find(domain::TaskId id) noexcept {  // 수정 가능한 관찰 포인터를 반환합니다.
    const auto iterator = tasks_.find(id);  // 트리를 따라 키를 찾아 map 반복자를 얻습니다.
    return iterator == tasks_.end()         // end는 일치하는 노드가 없음을 나타내는 센티널입니다.
        ? nullptr                           // 미발견이면 역참조할 수 없는 null을 반환합니다.
        : iterator->second.get();           // 발견하면 unique_ptr가 소유한 Task 주소만 빌려 줍니다.
}  // map이 변경되어 노드가 지워지면 반환 포인터는 무효가 되므로 오래 보관하면 안 됩니다.

const domain::Task* InMemoryTaskRepository::find(domain::TaskId id) const noexcept {  // 읽기 전용 관찰 포인터를 반환합니다.
    const auto iterator = tasks_.find(id);  // const map 조회는 const_iterator를 반환합니다.
    return iterator == tasks_.end()         // 노드 존재 여부를 검사합니다.
        ? nullptr                           // 없으면 null 포인터입니다.
        : iterator->second.get();           // get 자체는 소유권을 바꾸지 않고 주소만 반환합니다.
}  // const Task*를 통해서는 complete 같은 비const 멤버를 호출할 수 없습니다.

std::vector<domain::TaskSnapshot> InMemoryTaskRepository::all() const {  // 안전한 값 목록을 삽입 순서로 만듭니다.
    std::vector<domain::TaskSnapshot> snapshots;  // 연속 메모리를 소유할 빈 결과 vector입니다.
    snapshots.reserve(tasks_.size());             // 정확한 원소 수만큼 한 번에 할당해 주소 이동과 재할당을 줄입니다.

    for (const domain::TaskId id : insertion_order_) {  // list 노드를 포인터 추적으로 방문하며 ID를 값 복사합니다.
        const domain::Task* const task = find(id);       // map에서 실제 Task를 O(log N)에 찾습니다.
        if (task != nullptr) {                           // 두 컨테이너 불변식이 유지되었다면 항상 참입니다.
            snapshots.push_back(domain::make_snapshot(*task));  // 독립적인 깊은 복사 값을 vector 끝에 이동 삽입합니다.
        }  // 방어적인 null 검사를 닫습니다.
    }  // 삽입 순서 list 순회를 닫습니다.

    return snapshots;  // vector의 힙 배열 소유권을 값 반환으로 호출자에게 넘깁니다.
}  // 저장소 내부 map과 list는 const 함수이므로 변경되지 않습니다.

}  // namespace modern_cpp::infrastructure: 저장소 구현을 닫습니다.
