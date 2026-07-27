#include "modern_cpp/application/task_service.hpp"  // 구현할 서비스 계약과 의존 타입을 가져옵니다.

#include "modern_cpp/domain/task_sorting.hpp"  // 헤더 전용 concept 기반 정렬 함수를 가져옵니다.

#include <algorithm>   // 빈 태그를 제거하는 std::erase_if를 제공합니다.
#include <stdexcept>   // 생성자 전제 조건 위반에 사용할 std::invalid_argument를 제공합니다.
#include <utility>     // unique_ptr와 string을 이동할 std::move를 제공합니다.

namespace modern_cpp::application {  // application 계층 이름 공간에서 서비스 동작을 정의합니다.

TaskService::TaskService(  // 모든 필수 의존성을 생성 시점에 주입받습니다.
    std::unique_ptr<domain::TaskRepository> repository,  // 호출자로부터 저장소 독점 소유권을 받습니다.
    std::shared_ptr<EventSink> event_sink)               // 이벤트 sink의 공유 소유권에 참여합니다.
    : repository_{std::move(repository)},                // unique_ptr는 복사할 수 없으므로 명시적으로 이동합니다.
      event_sink_{std::move(event_sink)} {                // shared_ptr 이동은 참조 계수 증가를 피하고 소유권 슬롯만 옮깁니다.
    if (!repository_) {                                   // null 저장소라면 이후 모든 유스케이스가 실패하므로 즉시 검사합니다.
        throw std::invalid_argument{"TaskService requires a repository"};  // 생성 실패 시 이미 초기화된 멤버는 자동 파괴됩니다.
    }  // 저장소 전제 조건 검사를 닫습니다.
    if (!event_sink_) {  // null sink를 역참조하는 정의되지 않은 동작을 미리 차단합니다.
        throw std::invalid_argument{"TaskService requires an event sink"};  // 불완전한 서비스 객체는 호출자에게 노출되지 않습니다.
    }  // 이벤트 sink 전제 조건 검사를 닫습니다.
}  // 성공적으로 생성된 서비스가 두 의존성의 수명을 관리합니다.

std::expected<domain::TaskId, std::string> TaskService::create_task(  // 정상 ID와 예상 가능한 입력 오류를 분리합니다.
    std::string title,                                                // 호출자가 임시 string을 넘기면 버퍼를 이동할 수 있습니다.
    domain::Priority priority,                                        // 작은 enum은 값으로 전달합니다.
    std::vector<std::string> tags) {                                  // 태그 배열의 소유권도 값으로 받습니다.
    if (title.empty()) {                                               // 비어 있는 제목은 저장 전에 거부해 도메인 불변식을 지킵니다.
        return std::unexpected{"title must not be empty"};             // expected의 오류 대안을 직접 생성합니다.
    }  // 제목 검증 분기를 닫습니다.

    std::erase_if(tags, [](const std::string& tag) { return tag.empty(); });  // remove-erase 세부 코드 없이 빈 태그를 제거합니다.
    const domain::TaskId id{next_id_};                                      // 성공하기 전에는 증가시키지 않아 ID 공백을 피합니다.
    auto task = std::make_unique<domain::Task>(                              // Task만 힙에 만들고 지역 unique_ptr 객체가 그 주소를 독점 소유합니다.
        id, std::move(title), priority, std::move(tags));                    // 소유 버퍼들을 Task 멤버로 연속 이동합니다.

    auto saved = repository_->save(std::move(task));  // 저장소 포트에 Task의 독점 소유권을 넘깁니다.
    if (!saved) {                                      // expected가 오류 대안을 보유하는지 검사합니다.
        return std::unexpected{std::move(saved.error())};  // 저장 오류 문자열을 호출자 결과로 이동합니다.
    }  // 저장 실패 처리를 닫습니다.

    ++next_id_;  // 저장에 성공한 뒤 다음 ID를 증가시킵니다. 이 예제는 단일 스레드 전제입니다.
    const std::string message{"created task #" + std::to_string(id.value)};  // publish 호출 동안 유효한 소유 문자열을 만듭니다.
    event_sink_->publish(message);  // 공유된 sink 객체를 가상 호출하며 동적 디스패치가 실제 구현 주소를 선택합니다.
    return id;                      // 작은 성공 값을 expected 내부에 직접 저장해 반환합니다.
}  // 지역 객체는 역순 파괴되고 저장된 Task만 저장소 힙에 남습니다.

std::expected<void, std::string> TaskService::complete_task(domain::TaskId id) {  // 성공 값이 필요 없는 명령입니다.
    domain::Task* const task = repository_->find(id);  // 저장소 소유권을 얻지 않는 관찰 포인터를 잠시 빌립니다.
    if (task == nullptr) {                              // nullptr은 해당 ID가 없다는 명시적 신호입니다.
        return std::unexpected{"task not found: #" + std::to_string(id.value)};  // 호출자가 처리할 업무 오류를 반환합니다.
    }  // 미발견 분기를 닫습니다.

    task->complete();  // 저장소가 소유한 객체의 bool 상태를 제자리에서 변경합니다.
    const std::string message{"completed task #" + std::to_string(id.value)};  // 이벤트 메시지 메모리를 함수가 소유합니다.
    event_sink_->publish(message);  // publish가 반환된 뒤 string_view를 보관하지 않는다는 계약에 의존합니다.
    return {};                      // expected<void, E>의 성공 대안을 기본 생성합니다.
}  // 관찰 포인터는 파괴 개념이 없으며 Task 소유권은 계속 저장소에 있습니다.

std::vector<std::string> TaskService::complete_many(std::span<const domain::TaskId> ids) {  // 다양한 연속 컨테이너를 빌려 받습니다.
    std::vector<std::string> errors;  // 실패 메시지만 소유하는 연속 배열을 만듭니다.
    errors.reserve(ids.size());       // 최악의 경우를 미리 예약해 반복 중 재할당 횟수를 줄입니다.

    for (const domain::TaskId id : ids) {  // span의 각 작은 ID를 값으로 읽으며 원본 배열을 변경하지 않습니다.
        auto result = complete_task(id);   // 각 명령의 expected 결과를 지역 객체로 받습니다.
        if (!result) {                     // 실패만 모으고 나머지 ID 처리는 계속합니다.
            errors.push_back(std::move(result.error()));  // 오류 string 버퍼를 vector 원소로 이동합니다.
        }  // 개별 실패 처리를 닫습니다.
    }  // 모든 ID를 순회한 범위 for를 닫습니다.

    return errors;  // 반환 값 최적화 또는 vector 이동으로 힙 버퍼 소유권을 호출자에게 넘깁니다.
}  // span은 소유하지 않으므로 원본 ID 배열은 호출자가 계속 관리합니다.

std::optional<domain::TaskSnapshot> TaskService::find_task(domain::TaskId id) const {  // 미발견을 예외가 아닌 빈 값으로 나타냅니다.
    const domain::Task* const task = repository_->find(id);  // const 서비스에서는 const 저장소 오버로드가 선택됩니다.
    if (task == nullptr) {                                    // 저장된 객체가 없으면 안전하게 빈 optional을 반환합니다.
        return std::nullopt;                                  // 별도 힙 할당 없이 optional의 상태 비트만 비어 있게 설정합니다.
    }  // 미발견 분기를 닫습니다.
    return domain::make_snapshot(*task);  // 포인터를 유효한 참조로 역참조하고 독립 소유 스냅샷을 만듭니다.
}  // 관찰 포인터가 범위를 벗어나며 저장소 내부 객체에는 변화가 없습니다.

std::vector<domain::TaskSnapshot> TaskService::sorted_tasks() const {  // 호출자가 소유할 정렬 목록을 만듭니다.
    auto tasks = repository_->all();                                  // 저장소에서 삽입 순서 스냅샷 vector를 값으로 받습니다.
    domain::sort_by_priority_then_title(tasks);                        // concept로 검증된 ranges::sort가 vector 원소를 이동·교환합니다.
    return tasks;                                                      // 정렬된 vector 버퍼 소유권을 호출자에게 반환합니다.
}  // 지역 vector는 보통 NRVO 덕분에 별도 파괴 후 복사 없이 결과가 됩니다.

}  // namespace modern_cpp::application: 서비스 구현 이름 공간을 닫습니다.
