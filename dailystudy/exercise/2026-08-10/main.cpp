// <algorithm>은 범위 정렬 알고리즘 std::ranges::sort를 제공한다.
#include <algorithm>
// <iostream>은 표준 출력 객체 std::cout을 제공한다.
#include <iostream>
// <string>은 문자열을 소유하는 std::string을 제공한다.
#include <string>
// <utility>는 이동을 명시하는 std::move를 제공한다.
#include <utility>
// <vector>는 연속 메모리를 소유하는 가변 길이 컨테이너를 제공한다.
#include <vector>

// using은 긴 템플릿 타입에 읽기 쉬운 별칭을 붙인다. vector<string>은 문자열 객체들을 소유한다.
using Names = std::vector<std::string>;

// struct의 기본 접근은 public이며, 단순 데이터 묶음에 알맞다.
struct Report {
    std::string first{}; // 중괄호 초기화로 빈 문자열을 만든다.
    int count{};         // 기본 타입 int를 0으로 값 초기화한다.
};

// class의 기본 접근은 private이므로 공개 API와 구현 상태를 분리하기 좋다.
class NameCatalog {
public:
    // 생성자에는 반환형이 없다. explicit은 Names 한 개가 NameCatalog로 암시적으로 변환되는 것을 막는다.
    explicit NameCatalog(Names names) : names_{std::move(names)} {}

    // [[nodiscard]]는 반환값을 버리는 실수를 경고하며, const는 객체 상태를 바꾸지 않음을 뜻한다.
    [[nodiscard]] Report summarize() const {
        Names ordered{names_}; // names_는 lvalue이므로 복사 생성되어 원본 소유권은 유지된다.
        // ranges::sort(range)는 ordered 한 개를 입력받고 원소를 제자리 오름차순 변경한 뒤 끝 반복자를 반환한다(여기서는 버림).
        // 비교는 string의 사전식 operator<를 사용하며 시간 O(N log N), 추가 공간은 구현 의존이다.
        std::ranges::sort(ordered);
        // 조건 연산자는 비었으면 빈 문자열, 아니면 첫 원소의 복사본을 선택한다.
        // empty()는 인자 없이 bool을, front()는 비어 있지 않을 때 첫 string&를 반환한다. 둘 다 O(1)이고 컨테이너를 바꾸지 않는다.
        const std::string first{ordered.empty() ? std::string{} : ordered.front()};
        // Report{...}는 prvalue이며 결과 객체에 직접 만들어져 복사 생략이 보장된다.
        // size()는 인자 없이 원소 수 size_type을 반환한다. int 변환은 원소 수가 INT_MAX 이하라는 전제가 필요하다.
        return Report{first, static_cast<int>(ordered.size())};
    }

private:
    Names names_{}; // private 멤버가 문자열의 수명과 메모리 소유권을 관리한다.
};

int main() {
    Names names{"Cora", "Ada", "Bjarne"}; // 직접 목록 초기화로 소유 컨테이너를 만든다.
    // move(names)는 Names&& xvalue를 반환하고 NameCatalog 생성자가 vector 버퍼 소유권을 이동한다. names는 유효한 미지정 상태다.
    const NameCatalog catalog{std::move(names)};
    const Report report{catalog.summarize()}; // 함수 호출 결과 prvalue로 report를 직접 초기화한다.
    std::cout << report.first << ' ' << report.count << '\n'; // << 연산자로 값을 출력 스트림에 삽입한다.
    // 비교·논리 AND·조건 분기가 실행된다. 실제 로드/비교/분기 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return report.first == "Ada" && report.count == 3 ? 0 : 1;
}
