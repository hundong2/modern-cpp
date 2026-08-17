// <algorithm>은 std::ranges::sort 정렬 알고리즘을 제공한다.
#include <algorithm>
// <cstdint>는 파일 크기를 담을 고정 폭 부호 없는 정수 std::uintmax_t를 제공한다.
#include <cstdint>
// <filesystem>은 경로, 디렉터리 반복자, 파일 상태 API를 제공한다.
#include <filesystem>
// <iostream>은 결과를 표준 출력으로 보내는 std::cout을 제공한다.
#include <iostream>
// <string>은 파일 확장자를 소유하는 std::string을 제공한다.
#include <string>
// <system_error>는 비예외 오류 값을 담는 std::error_code를 제공한다.
#include <system_error>
// <utility>는 소유 자원 이동을 허용하는 std::move를 제공한다.
#include <utility>
// <vector>는 가변 길이 파일 레코드 목록 std::vector를 제공한다.
#include <vector>

// 긴 표준 타입을 짧게 읽기 위한 using 타입 별칭이며 fs는 namespace 별칭이다.
namespace fs = std::filesystem;

// struct의 멤버는 기본 public이므로 단순 결과 값을 묶는 값 객체에 알맞다.
struct FileRecord {
    // path는 파일 경로 문자열과 운영체제별 구분 규칙을 값으로 소유한다.
    fs::path path{};
    // uintmax_t는 표준 파일 크기 반환형과 같고 {}는 0 값 초기화를 뜻한다.
    std::uintmax_t bytes{};
};

// class의 멤버는 기본 private이며 public 함수로 파일 시스템 접근 경계를 제한한다.
class DirectoryCatalog {
public:
    // 생성자는 반환형이 없고 explicit은 path 하나가 Catalog로 암시 변환되는 일을 막는다.
    explicit DirectoryCatalog(fs::path root)
        // 값 매개변수 root를 xvalue로 바꾸어 멤버에 직접 이동 초기화한다.
        : root_{std::move(root)} {}

    // [[nodiscard]]는 계산한 목록을 실수로 버리면 컴파일러가 경고할 수 있게 한다.
    [[nodiscard]] std::vector<FileRecord> scan_regular_files() const {
        // vector<FileRecord>에서 FileRecord는 컨테이너 원소 타입인 템플릿 인자다.
        std::vector<FileRecord> records{};
        // error_code는 예외 대신 운영 실패 코드를 값으로 받아 호출자가 분기하게 한다.
        std::error_code error{};

        // directory_iterator(root_,error) 생성자는 시작 경로와 error_code& 출력 인자를 받으며 별도 반환값은 없다.
        // 성공하면 첫 항목을 가리키고, 실패하면 error에 코드가 기록되며 끝 반복자 상태가 된다.
        fs::directory_iterator current{root_, error};
        // 기본 생성된 반복자는 디렉터리 순회의 끝을 나타내는 센티널 역할을 한다.
        const fs::directory_iterator end{};

        // error가 없고 current가 end가 아닌 동안 한 항목씩 전위 ++로 진행한다.
        while (!error && current != end) {
            // * 연산자는 반복자가 가리키는 directory_entry lvalue에 접근한다.
            const fs::directory_entry& entry{*current};
            // is_regular_file(error)는 error_code& 출력 인자 하나를 받고 일반 파일 여부 bool을 반환한다.
            // 성공하면 error가 지워지고 entry는 바뀌지 않으며, 실패하면 false와 오류 코드로 보고한다.
            const bool regular{entry.is_regular_file(error)};

            // &&는 왼쪽이 참일 때만 오른쪽을 평가하며 일반 파일만 목록에 넣는다.
            if (!error && regular) {
                // file_size(error)는 error_code& 출력 인자를 받고 바이트 수 uintmax_t를 반환한다. 실패 값은 error로 판정해야 한다.
                const std::uintmax_t size{entry.file_size(error)};
                // 오류가 없다면 prvalue FileRecord를 vector 끝에 이동 또는 직접 생성한다.
                if (!error) {
                    // entry.path()는 인자 없이 path const&를 반환하고 push_back은 FileRecord prvalue를 이동 저장한 뒤 void를 반환한다.
                    // 성공하면 size가 1 늘며 vector 재할당 시 기존 관찰자가 무효화된다.
                    records.push_back(FileRecord{entry.path(), size});
                }
            }

            // increment(error)는 error_code& 출력 인자를 받고 현재 반복자를 다음 항목으로 변경한 뒤 자기 참조를 반환한다(여기서는 버림).
            current.increment(error);
        }

        // 람다는 두 const 참조를 빌리고 < 연산자로 path의 결정적 순서를 만든다.
        // ranges::sort(range,comparator)는 records와 엄격 약순서 술어를 입력받아 제자리 정렬하고 끝 반복자를 반환한다.
        // 시간 O(N log N)이며 반환 반복자는 여기서 사용하지 않는다.
        std::ranges::sort(records, [](const FileRecord& left, const FileRecord& right) {
            return left.path < right.path;
        });
        // 이름 있는 지역 vector는 lvalue지만 반환 시 복사 생략 또는 이동 대상으로 취급된다.
        return records;
    }

    // const 뒤의 함수는 root_ 멤버를 변경하지 않으며 const 참조로 빌려 준다.
    [[nodiscard]] const fs::path& root() const noexcept {
        // 반환 참조는 소유하지 않으므로 DirectoryCatalog 객체보다 오래 보관하면 안 된다.
        return root_;
    }

private:
    // class의 private 멤버는 카탈로그가 루트 경로의 수명과 불변식을 소유하게 한다.
    fs::path root_{};
};

// main은 운영체제와 프로그램을 연결하는 진입 함수이며 int 종료 코드를 반환한다.
int main() {
    // fs::path{"."} prvalue로 explicit 생성자를 직접 호출해 현재 디렉터리를 소유한다.
    DirectoryCatalog catalog{fs::path{"."}};
    // 함수 반환 prvalue가 records를 직접 초기화해 불필요한 복사를 피할 수 있다.
    const std::vector<FileRecord> records{catalog.scan_regular_files()};

    // generic_string()은 인자 없이 일반 구분자 형식의 새 string을 반환할 수 있고, size()는 원소 수를 O(1)에 반환한다.
    std::cout << "root=" << catalog.root().generic_string()
              << ", regular_files=" << records.size() << '\n';

    // empty()는 인자 없이 bool을 반환한다. front()는 비어 있지 않을 때 첫 FileRecord const&를 반환한다.
    if (!records.empty()) {
        // filename()은 마지막 경로 요소 path 값을, generic_string()은 그 문자의 소유 string을 반환하며 원본 path는 바뀌지 않는다.
        std::cout << "first=" << records.front().path.filename().generic_string()
                  << ", bytes=" << records.front().bytes << '\n';
    }

    // 0은 프로그램이 정상 종료했음을 운영체제에 알리는 int prvalue다.
    return 0;
}
