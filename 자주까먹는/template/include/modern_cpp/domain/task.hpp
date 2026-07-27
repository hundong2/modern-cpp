#pragma once  // 헤더 중복 포함으로 같은 클래스가 재정의되는 일을 막습니다.

#include <compare>      // operator<=>로 값 타입의 전체 비교 연산을 생성합니다.
#include <cstdint>      // 플랫폼과 무관한 고정 폭 정수 std::uint64_t와 std::uint8_t를 제공합니다.
#include <string>       // 문자를 힙 버퍼에 소유하는 std::string을 제공합니다.
#include <string_view>  // 복사 없이 문자열을 관찰하는 std::string_view를 제공합니다.
#include <utility>      // 소유 자원을 복사하지 않고 넘기는 std::move를 제공합니다.
#include <vector>       // 연속 메모리에 가변 길이 원소를 저장하는 std::vector를 제공합니다.

namespace modern_cpp::domain {  // 업무 규칙을 외부 구현 세부 사항과 분리하는 도메인 계층입니다.

struct TaskId final {  // 원시 정수 대신 강한 타입을 사용해 다른 종류의 ID와 혼용되는 실수를 막습니다.
    std::uint64_t value{};  // 64비트 값은 객체 안에 직접 저장되며 기본 초기화 시 0이 됩니다.

    auto operator<=>(const TaskId&) const = default;  // map 정렬과 동등 비교에 필요한 비교 연산을 멤버 순서대로 생성합니다.
};  // TaskId는 힙 할당 없이 보통 레지스터 하나로 전달할 수 있는 작은 값 타입입니다.

enum class Priority : std::uint8_t {  // scoped enum은 이름 충돌과 정수로의 암시적 변환을 방지합니다.
    low = 1,                           // 명시한 기반 타입 덕분에 객체 표현 크기를 예측하기 쉽습니다.
    normal = 2,                        // 값의 순서는 아래 정렬 정책에서 우선순위 점수로 사용합니다.
    high = 3,                          // high는 사용자에게 먼저 보여 줄 가장 높은 우선순위입니다.
};  // enum class 블록을 닫습니다.

[[nodiscard]] constexpr std::string_view priority_name(Priority priority) noexcept {  // 컴파일 시간에도 실행 가능한 순수 변환 함수입니다.
    switch (priority) {  // 작은 enum 값은 최적화 시 비교 분기 또는 점프 테이블로 바뀔 수 있습니다.
        case Priority::low:     // 낮은 우선순위 분기입니다.
            return "low";       // 문자열 리터럴은 정적 저장 기간을 가지므로 반환한 view가 매달리지 않습니다.
        case Priority::normal:  // 보통 우선순위 분기입니다.
            return "normal";    // 동적 할당 없이 포인터와 길이만 반환합니다.
        case Priority::high:    // 높은 우선순위 분기입니다.
            return "high";      // noexcept 함수이므로 이 경로는 예외를 던지지 않습니다.
    }  // 모든 정상 enum 값을 처리한 switch를 닫습니다.
    return "unknown";  // 손상되거나 강제 변환된 알 수 없는 값에도 안전한 문자열을 반환합니다.
}  // priority_name 함수의 스택 프레임은 최적화 시 완전히 인라인될 수 있습니다.

class Task final {  // 상속을 의도하지 않는 도메인 엔티티임을 final로 명시합니다.
public:  // 호출자가 사용할 수 있는 안정적인 공개 계약을 시작합니다.
    Task(TaskId id, std::string title, Priority priority, std::vector<std::string> tags);  // 큰 소유 값은 값으로 받아 멤버로 이동합니다.

    [[nodiscard]] TaskId id() const noexcept { return id_; }  // 작은 ID는 복사 반환하며 객체 상태를 변경하지 않습니다.
    [[nodiscard]] const std::string& title() const noexcept { return title_; }  // const 참조는 문자열 버퍼 복사를 피하되 Task 수명보다 오래 보관하면 안 됩니다.
    [[nodiscard]] Priority priority() const noexcept { return priority_; }  // enum 값은 레지스터 크기이므로 값 반환이 적합합니다.
    [[nodiscard]] bool completed() const noexcept { return completed_; }  // bool 상태를 읽기만 하므로 const와 noexcept를 함께 표시합니다.
    [[nodiscard]] const std::vector<std::string>& tags() const noexcept { return tags_; }  // vector의 소유권은 Task에 남겨 둔 채 관찰만 허용합니다.

    void complete() noexcept { completed_ = true; }  // 이미 완료된 객체에 다시 호출해도 같은 값이 되는 멱등 연산입니다.
    void add_tag(std::string tag);  // 전달받은 문자열 버퍼를 vector 원소가 소유하도록 이동합니다.

private:  // 클래스 불변식을 외부에서 임의로 깨뜨릴 수 없도록 표현을 숨깁니다.
    TaskId id_{};                   // 객체 본체에 직접 저장되는 강한 ID 값입니다.
    std::string title_;             // 짧은 문자열 최적화 또는 힙 버퍼를 통해 제목 문자를 소유합니다.
    Priority priority_{Priority::normal};  // 기본 상태를 명시해 초기화 누락을 방지합니다.
    bool completed_{false};         // bool은 패딩이 생길 수 있으므로 sizeof(Task)는 멤버 크기의 단순 합과 다를 수 있습니다.
    std::vector<std::string> tags_; // vector 본체는 보통 포인터 3개이고 실제 원소 배열은 별도 힙 영역에 있습니다.
};  // Task의 멤버는 선언의 역순으로 자동 파괴되어 모든 소유 메모리를 회수합니다.

struct TaskSnapshot final {  // 저장소 내부 객체의 주소를 노출하지 않는 읽기용 값 객체입니다.
    TaskId id{};                     // ID를 값으로 복사해 원본 Task가 사라져도 유효합니다.
    std::string title;               // 문자열을 소유하므로 저장소 변경 뒤에도 안전합니다.
    Priority priority{Priority::normal};  // 정렬과 화면 표시에 필요한 값입니다.
    bool completed{false};           // 조회 시점의 완료 상태를 복사합니다.
    std::vector<std::string> tags;   // 태그도 깊은 복사되어 독립적인 수명을 갖습니다.
};  // 스냅샷은 안전성을 위해 복사 비용을 지불하는 데이터 전송 객체입니다.

[[nodiscard]] TaskSnapshot make_snapshot(const Task& task);  // 비소유 const 참조로 읽어 독립적인 스냅샷을 만듭니다.

}  // namespace modern_cpp::domain: 도메인 계층의 공개 선언을 닫습니다.
