#pragma once  // 추상 저장소 계약이 한 번만 선언되도록 합니다.

#include "modern_cpp/domain/task.hpp"  // 저장하고 조회할 도메인 타입을 가져옵니다.

#include <expected>  // 성공 값 또는 오류 값을 명시적으로 표현하는 C++23 std::expected를 제공합니다.
#include <memory>    // 독점 소유권 이전을 나타내는 std::unique_ptr를 제공합니다.
#include <string>    // 오류 메시지의 저장 공간을 소유하는 std::string을 제공합니다.
#include <vector>    // 여러 스냅샷을 연속 메모리로 반환하는 std::vector를 제공합니다.

namespace modern_cpp::domain {  // 구현 기술에 의존하지 않는 저장소 포트를 도메인 이름 공간에 둡니다.

class TaskRepository {  // application 계층이 구체적인 map 저장 방식을 몰라도 되게 하는 추상 인터페이스입니다.
public:  // 다형적으로 호출할 공개 계약입니다.
    virtual ~TaskRepository() = default;  // 기반 포인터로 파괴할 때 파생 소멸자까지 호출되도록 가상 소멸자를 둡니다.

    virtual std::expected<void, std::string> save(std::unique_ptr<Task> task) = 0;  // unique_ptr 값 전달은 저장소로 소유권이 이동함을 타입으로 표현합니다.
    [[nodiscard]] virtual Task* find(TaskId id) noexcept = 0;  // raw pointer는 소유하지 않는 관찰자이며 없으면 nullptr을 반환합니다.
    [[nodiscard]] virtual const Task* find(TaskId id) const noexcept = 0;  // const 오버로드는 저장소와 Task를 수정하지 않는 조회를 보장합니다.
    [[nodiscard]] virtual std::vector<TaskSnapshot> all() const = 0;  // 내부 주소 대신 독립 수명의 값 목록을 반환합니다.
};  // 순수 가상 함수가 있으므로 직접 객체를 만들 수 없는 인터페이스 클래스입니다.

}  // namespace modern_cpp::domain: 저장소 계약의 이름 공간을 닫습니다.
