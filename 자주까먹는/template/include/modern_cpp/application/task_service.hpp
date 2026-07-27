#pragma once  // 서비스 공개 계약이 여러 번 정의되지 않도록 합니다.

#include "modern_cpp/application/event_sink.hpp"  // 공유할 이벤트 출력 포트의 완전한 선언을 가져옵니다.
#include "modern_cpp/domain/task_repository.hpp"  // 서비스가 독점 소유할 저장소 포트를 가져옵니다.

#include <expected>  // 정상 결과와 업무 오류를 한 반환 타입에 담습니다.
#include <memory>    // unique_ptr와 shared_ptr의 소유권 타입을 제공합니다.
#include <optional>  // 값이 없을 수 있지만 오류는 아닌 조회 결과를 표현합니다.
#include <span>      // 연속 메모리 배열을 소유하지 않고 함수 호출 동안 관찰합니다.
#include <string>    // 생성 요청의 제목과 오류 메시지를 소유합니다.
#include <vector>    // 태그 입력과 목록 결과를 저장합니다.

namespace modern_cpp::application {  // 여러 도메인 연산을 조합하는 유스케이스 계층입니다.

class TaskService final {  // 저장소와 이벤트 포트를 조립해 애플리케이션 동작을 제공합니다.
public:  // main과 테스트가 사용할 서비스 API를 시작합니다.
    TaskService(  // 생성 시 필요한 의존성을 모두 받아 불완전한 객체 생성을 막습니다.
        std::unique_ptr<domain::TaskRepository> repository,  // 저장소 구현의 독점 소유권을 서비스로 이전합니다.
        std::shared_ptr<EventSink> event_sink);              // 여러 서비스가 같은 sink 수명을 공유할 수 있게 참조 계수를 보유합니다.

    [[nodiscard]] std::expected<domain::TaskId, std::string> create_task(  // 성공 시 강한 ID, 실패 시 설명 문자열을 반환합니다.
        std::string title,                                                // 소유할 입력은 값으로 받아 이동 가능하게 합니다.
        domain::Priority priority = domain::Priority::normal,             // 흔한 값을 기본 인수로 제공합니다.
        std::vector<std::string> tags = {});                              // 태그가 없을 때 빈 vector를 생성합니다.

    [[nodiscard]] std::expected<void, std::string> complete_task(domain::TaskId id);  // 값 없는 성공과 오류를 expected<void, E>로 표현합니다.
    [[nodiscard]] std::vector<std::string> complete_many(std::span<const domain::TaskId> ids);  // span은 vector와 array 모두 복사 없이 받습니다.
    [[nodiscard]] std::optional<domain::TaskSnapshot> find_task(domain::TaskId id) const;  // 미발견은 정상적인 빈 optional입니다.
    [[nodiscard]] std::vector<domain::TaskSnapshot> sorted_tasks() const;  // 정렬된 안전한 값 스냅샷을 반환합니다.

private:  // 의존성과 ID 상태를 외부에서 변경하지 못하게 숨깁니다.
    std::unique_ptr<domain::TaskRepository> repository_;  // 서비스가 파괴되면 구체 저장소도 정확히 한 번 자동 파괴됩니다.
    std::shared_ptr<EventSink> event_sink_;               // 마지막 공유 소유자가 사라질 때 제어 블록과 sink가 해제됩니다.
    std::uint64_t next_id_{1};                            // 단일 스레드 예제용 증가 ID이며 동시 접근에는 atomic 또는 잠금이 필요합니다.
};  // 멤버는 next_id_, event_sink_, repository_ 역순으로 파괴됩니다.

}  // namespace modern_cpp::application: 서비스 계약의 이름 공간을 닫습니다.
