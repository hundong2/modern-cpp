// <filesystem>은 문자열 결합 대신 경로 구성 요소 단위 연산을 제공한다.
#include <filesystem>
// <iostream>은 검증 결과를 표준 출력하는 std::cout을 제공한다.
#include <iostream>
// <optional>은 안전한 경로가 없을 수 있음을 std::optional로 표현한다.
#include <optional>
// <utility>는 생성자 값 매개변수에서 멤버로 이동하는 std::move를 제공한다.
#include <utility>

// 반복되는 긴 namespace 이름을 fs라는 별칭으로 줄인다.
namespace fs = std::filesystem;

// 경로 검증과 저장 루트 결합 책임을 한 객체에 캡슐화한다.
class SafePathResolver {
public:
    // 생성자는 반환형이 없고 explicit은 fs::path의 우발적 암시 변환을 금지한다.
    explicit SafePathResolver(fs::path storage_root)
        // std::move 식은 이름 있는 lvalue를 xvalue로 바꾸어 path 이동 생성의 후보가 되게 한다.
        : storage_root_{std::move(storage_root)} {}

    // optional<path>는 성공 경로 또는 값 없음이라는 두 상태를 타입에 드러낸다.
    [[nodiscard]] std::optional<fs::path> resolve(const fs::path& user_path) const {
        // is_absolute()가 true면 사용자가 저장 루트를 무시할 수 있으므로 즉시 거부한다.
        if (user_path.empty() || user_path.is_absolute()) {
            // nullopt는 optional에 안전한 결과가 없음을 나타내는 표준 태그다.
            return std::nullopt;
        }

        // 범위 기반 for는 경로를 루트 이름, 디렉터리, 파일명 같은 구성 요소로 순회한다.
        for (const fs::path& part : user_path) {
            // == 연산자로 상위 디렉터리 구성 요소를 비교해 lexical 루트 탈출을 거부한다.
            if (part == "..") {
                return std::nullopt;
            }
        }

        // / 연산자는 경로 구분자를 직접 이어 쓰지 않고 두 path를 이식 가능하게 결합한다.
        // lexically_normal은 파일 시스템에 접근하지 않고 . 및 중복 구분자를 정리한다.
        return (storage_root_ / user_path).lexically_normal();
    }

private:
    // private 멤버는 resolver가 저장 루트 값을 소유하고 외부가 임의로 바꾸지 못하게 한다.
    fs::path storage_root_{};
};

// main은 작은 직접 실행 검증을 수행하고 int 종료 코드를 반환한다.
int main() {
    // path prvalue를 explicit 생성자에 직접 전달해 안전한 저장 루트를 구성한다.
    const SafePathResolver resolver{fs::path{"uploads"}};
    // resolve 반환 prvalue가 optional 목적 객체를 직접 초기화할 수 있다.
    const std::optional<fs::path> safe{resolver.resolve(fs::path{"images/avatar.png"})};
    // 위험 입력은 .. 구성 요소를 포함하므로 값 없는 optional이 되어야 한다.
    const std::optional<fs::path> escaped{resolver.resolve(fs::path{"../secret.txt"})};

    // optional의 bool 변환을 &&와 ! 조건에 사용해 두 검증을 한 번에 확인한다.
    if (safe && !escaped) {
        // *safe는 optional 내부 path를 가리키는 const lvalue이며 generic_string()으로 표시한다.
        std::cout << safe->generic_string() << '\n';
        return 0;
    }

    // 기대와 다르면 1을 반환해 CTest가 실패를 감지하게 한다.
    return 1;
}
