#include "modern_cpp/application/event_sink.hpp"               // 콘솔 이벤트 출력 구현을 가져옵니다.
#include "modern_cpp/application/task_service.hpp"             // 애플리케이션 유스케이스 서비스를 가져옵니다.
#include "modern_cpp/domain/task.hpp"                           // Priority와 TaskId 값 타입을 가져옵니다.
#include "modern_cpp/infrastructure/in_memory_task_repository.hpp"  // 메모리 저장소 구현을 가져옵니다.
#include "modern_cpp/version.h"                                // 컴파일 시간 프로젝트 메타데이터를 가져옵니다.

#include <array>     // 고정 길이 연속 메모리 컨테이너 std::array를 제공합니다.
#include <iostream>  // 예제 결과를 표준 출력에 기록합니다.
#include <map>       // 출력 통계를 키 순서로 보관할 std::map을 제공합니다.
#include <memory>    // make_unique와 make_shared 팩토리를 제공합니다.
#include <string>    // 통계 map의 소유 문자열 키를 제공합니다.

namespace {  // 이 번역 단위 밖에서 이름이 충돌하지 않도록 내부 연결 함수로 제한합니다.

void print_tasks(const std::vector<modern_cpp::domain::TaskSnapshot>& tasks) {  // 큰 vector는 const 참조로 빌려 복사를 피합니다.
    std::cout << "\n[sorted tasks]\n";  // 섹션 제목을 stream 버퍼에 씁니다.
    for (const auto& task : tasks) {    // 각 스냅샷을 const 참조로 읽어 string과 vector 복사를 피합니다.
        std::cout << '#' << task.id.value                    // 강한 ID 내부의 표시용 정수 값을 출력합니다.
                  << " [" << priority_name(task.priority)    // enum을 수명 안전한 string_view 이름으로 변환합니다.
                  << "] " << task.title                      // 스냅샷이 소유한 제목을 출력합니다.
                  << " completed=" << std::boolalpha         // 뒤따르는 bool을 true/false 문자열로 표시하게 합니다.
                  << task.completed << " tags=" << task.tags.size() << '\n';  // 완료 상태와 연속 배열 원소 수를 출력합니다.
    }  // 모든 스냅샷의 읽기 순회를 닫습니다.
}  // 함수는 소유권을 받지 않았으므로 tasks와 원소는 호출자에게 그대로 남습니다.

}  // namespace: 파일 내부 보조 함수 영역을 닫습니다.

int main() {  // 운영체제가 프로세스를 시작한 뒤 호출하는 프로그램 진입점입니다.
    using modern_cpp::domain::Priority;  // 반복되는 한정 이름을 줄이되 타입 출처는 명시합니다.

    std::cout << modern_cpp::project_name << " v"              // constexpr string_view는 실행 파일의 정적 데이터 영역을 가리킵니다.
              << modern_cpp::project_version_major << '.'      // 정수 상수는 컴파일러가 명령어 즉시값으로 넣을 수 있습니다.
              << modern_cpp::project_version_minor << '\n';    // 한 줄의 버전 출력을 끝냅니다.

    auto event_sink = std::make_shared<modern_cpp::application::ConsoleEventSink>();  // 객체와 참조 계수 제어 블록을 보통 한 번의 힙 할당으로 만듭니다.
    auto repository = std::make_unique<modern_cpp::infrastructure::InMemoryTaskRepository>();  // 저장소를 힙에 만들고 독점 소유합니다.
    modern_cpp::application::TaskService service{std::move(repository), event_sink};  // 저장소는 이동하고 sink는 복사해 실제 공유 소유를 만듭니다.

    const auto learn = service.create_task("read C++23 ownership code", Priority::high, {"cpp23", "memory"});  // expected가 성공 ID 또는 오류를 값으로 보관합니다.
    const auto review = service.create_task("review open-source CMake", Priority::normal, {"cmake"});          // 임시 vector의 문자열들이 서비스와 Task로 이동됩니다.
    const auto refactor = service.create_task("refactor repository adapter", Priority::high, {"architecture"});  // 같은 우선순위에서는 제목으로 정렬됩니다.

    if (!learn || !review || !refactor) {  // 세 생성 중 하나라도 실패하면 ID를 역참조하지 않습니다.
        std::cerr << "failed to seed demo data\n";  // 오류 stream은 일반 출력과 별도로 즉시 진단할 때 사용합니다.
        return 1;                                  // 0이 아닌 종료 코드는 셸과 CI에 실패를 알립니다.
    }  // 초기 데이터 생성 실패 경계를 닫습니다.

    const std::array ids_to_complete{*review, modern_cpp::domain::TaskId{999}};  // array는 스택 프레임 안에 두 ID를 연속 저장합니다.
    const auto errors = service.complete_many(ids_to_complete);                  // array가 span<const TaskId>로 암시적으로 변환되어 복사 없이 전달됩니다.
    for (const std::string& error : errors) {                                    // 소유 오류 문자열을 읽기 전용 참조로 순회합니다.
        std::cout << "[expected error] " << error << '\n';                       // 존재하지 않는 ID 오류가 값으로 처리되는 모습을 보여 줍니다.
    }  // 오류 목록 순회를 닫습니다.

    const auto tasks = service.sorted_tasks();  // 저장소와 독립적인 정렬 스냅샷 vector를 소유합니다.
    print_tasks(tasks);                         // const 참조로 목록을 빌려 출력합니다.

    std::map<std::string, std::size_t> counts_by_priority;  // 트리 기반 map이 우선순위 이름 키를 정렬 상태로 유지합니다.
    for (const auto& task : tasks) {                         // vector의 연속 원소를 캐시 친화적으로 순회합니다.
        ++counts_by_priority[std::string{priority_name(task.priority)}];  // 없는 키는 0으로 만들고 카운터를 증가시킵니다.
    }  // 통계 계산 순회를 닫습니다.

    std::cout << "\n[counts]\n";                              // 구조화된 바인딩 예제의 출력 제목입니다.
    for (const auto& [name, count] : counts_by_priority) {    // map의 pair<const Key, Value>를 읽기 전용 이름으로 분해합니다.
        std::cout << name << '=' << count << '\n';            // map 키 정렬 순서대로 통계를 출력합니다.
    }  // map 트리 노드 순회를 닫습니다.

    return 0;  // 지역 객체가 역순 파괴된 뒤 운영체제에 성공을 반환합니다.
}  // service가 sink 공유 소유권을 놓고 event_sink가 마지막 소유자로서 실제 객체를 파괴합니다.
