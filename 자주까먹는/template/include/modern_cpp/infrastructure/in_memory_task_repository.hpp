#pragma once  // 구체 저장소 클래스가 중복 정의되지 않게 합니다.

#include "modern_cpp/domain/task_repository.hpp"  // 구현해야 할 추상 저장소 계약을 가져옵니다.

#include <list>  // 삽입 순서를 노드 기반 이중 연결 리스트로 유지합니다.
#include <map>   // ID를 키로 정렬된 균형 트리 형태의 연관 컨테이너를 제공합니다.
#include <memory>  // map 값으로 Task를 독점 소유할 unique_ptr를 제공합니다.

namespace modern_cpp::infrastructure {  // 저장 기술과 컨테이너 선택을 infrastructure 계층에 격리합니다.

class InMemoryTaskRepository final : public domain::TaskRepository {  // 프로세스 메모리를 사용하는 저장소 어댑터입니다.
public:  // 추상 포트의 구체 구현을 공개합니다.
    std::expected<void, std::string> save(std::unique_ptr<domain::Task> task) override;  // 저장 성공 또는 중복 오류를 반환합니다.
    [[nodiscard]] domain::Task* find(domain::TaskId id) noexcept override;  // 수정 가능한 비소유 포인터 조회입니다.
    [[nodiscard]] const domain::Task* find(domain::TaskId id) const noexcept override;  // 읽기 전용 비소유 포인터 조회입니다.
    [[nodiscard]] std::vector<domain::TaskSnapshot> all() const override;  // 삽입 순서대로 독립 스냅샷을 만듭니다.

private:  // 컨테이너 조합은 인터페이스 이용자가 몰라도 되는 구현 세부 사항입니다.
    std::map<domain::TaskId, std::unique_ptr<domain::Task>> tasks_;  // 트리 노드마다 Task 소유 포인터를 보관해 ID 조회를 O(log N)에 수행합니다.
    std::list<domain::TaskId> insertion_order_;                     // 노드별 추가 할당을 사용하지만 중간 삽입·삭제 시 반복자가 안정적입니다.
};  // map의 unique_ptr들이 파괴되면서 모든 Task 힙 객체가 누수 없이 해제됩니다.

}  // namespace modern_cpp::infrastructure: 저장소 구현 이름 공간을 닫습니다.
