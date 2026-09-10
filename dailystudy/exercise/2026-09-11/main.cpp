// <iostream>은 표준 출력 객체 std::cout과 문자열·정수 삽입 연산자를 선언한다.
#include <iostream>
// <ranges>는 lazy filter_view/transform_view와 C++23 std::ranges::to를 선언한다.
#include <ranges>
// <string>은 각 행과 서비스 DTO가 문자를 독점 소유하도록 std::string을 선언한다.
#include <string>
// <utility>는 이름 있는 객체를 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>
// <vector>는 행과 materialized snapshot을 연속 저장하는 std::vector를 선언한다.
#include <vector>

// struct는 기본 접근 지정자가 public이라 단순 데이터 행을 표현하기 좋다. class는 기본 private라
// 아래 저장소처럼 불변식을 감추기 좋다. 둘은 접근 기본값 외에는 같은 종류의 사용자 정의 타입이다.
struct UserRow {
    // int는 여기서 서비스 식별자와 점수를 나타내는 기본 정수 타입이다.
    int id;
    // std::string은 문자 버퍼를 소유한다. string_view와 달리 원본 행이 사라져도 자신의 문자는 산다.
    std::string name;
    int score;
    bool active;
};

struct UserCard {
    int id;
    std::string name;
};

// using은 새 타입을 만들지 않고 긴 템플릿 인스턴스에 읽기 쉬운 별칭을 붙인다.
// 템플릿 인자 UserRow/UserCard가 vector가 생성·파괴·연속 저장할 원소 타입을 결정한다.
using UserRows = std::vector<UserRow>;
using UserCards = std::vector<UserCard>;

class UserRepository {
public:
    // explicit은 UserRows 한 개가 필요한 자리에 UserRepository가 암시적으로 생기는 실수를 막는다.
    // 매개변수를 값으로 받으면 호출자가 복사할지 이동할지 경계에서 선택하고, 저장소는 그 값을 채택한다.
    // [호출 계약: std::move(rows)와 std::vector<UserRow> 이동 생성]
    // (1) 수신 목적 객체 rows_는 아직 생성 전이고, 매개변수 rows는 완전히 생성된 UserRows lvalue로
    //     행·문자 버퍼를 독점 소유한다. 두 vector는 서로 호환되는 기본 allocator를 사용한다.
    // (2) std::move는 template<class T> remove_reference_t<T>&& move(T&&) noexcept에서 T=UserRows&를
    //     고르고, rows_는 vector(vector&&) 이동 생성자를 선택한다.
    // (3) rows 식은 이름 때문에 lvalue지만 std::move(rows)가 UserRows&& xvalue를 반환한다. 이는 소유권을
    //     옮길 수 있다는 허가일 뿐이며 실제 버퍼 이전은 vector 이동 생성자가 수행한다.
    // (4) std::move 반환 참조는 rows_ 초기화에 즉시 사용한다. 생성자는 반환값이 없고 rows_가 버퍼 owner가 된다.
    // (5) 성공 뒤 rows_는 모든 행을 소유하고 rows는 유효하지만 내용이 명시되지 않은 moved-from 상태다.
    //     전제조건을 만족하는 연산은 가능하지만 이전 크기나 원소가 남았다고 가정하면 안 된다.
    // (6) 기본 allocator의 vector 이동은 O(1), 무할당이며 noexcept다. 기존 원소 관찰자는 새 owner의
    //     원소를 계속 가리키지만 rows 객체 자체 참조는 그대로다. 외부 동기화 없이 한 실행 흐름에서만 쓴다.
    explicit UserRepository(UserRows rows) : rows_{std::move(rows)} {}

    // 반환형 auto는 길고 구현 세부적인 filter_view 타입을 컴파일러가 정확히 추론하게 한다.
    // const 멤버 함수이므로 숨은 this는 const UserRepository*이고 행을 바꿀 수 없다.
    [[nodiscard]] auto active_with_minimum(const int minimum_score) const {
        // 값 캡처는 함수가 끝나도 predicate 내부에 독립된 int가 남게 한다. 매개변수 row는 const 참조라
        // 복사 없이 읽고, 반환 bool은 filter가 해당 행을 통과시킬지 뜻한다.
        const auto predicate = [minimum_score](const UserRow& row) {
            return row.active && row.score >= minimum_score;
        };

        // [호출 계약: std::views::filter(rows_, predicate)]
        // (1) 첫 인자/수신 범위 rows_는 살아 있는 저장소가 소유한 const UserRows lvalue이고 모든 행은
        //     완성 상태다. predicate는 int 임계값을 가진 복사 가능한 lambda lvalue다.
        // (2) range adaptor std::views::filter의 (R&&, Pred&&) 호출이 선택되며 R=const UserRows&,
        //     Pred=const lambda&다. 결과는 ref_view<const UserRows>를 바탕으로 한 filter_view 계열이다.
        // (3) rows_는 비소유 참조 view로 빌리고 predicate는 결과 view 안으로 복사한다. 원소·버퍼 소유권은
        //     이동하지 않으며 minimum_score의 값만 closure에 들어 있다.
        // (4) lazy filter view prvalue를 반환하며 함수 결과 view에 직접 구성된다. 아직 행도 predicate도 읽지 않는다.
        // (5) 저장소와 rows_는 변하지 않는다. 오늘처럼 새 view를 한 번 처음 순회하면 그 시점의
        //     active/score를 검사한다. forward_range용 filter_view는 첫 통과 반복자를 cache할 수 있으므로
        //     순회를 시작한 같은 view를 predicate 관련 변경 뒤 재사용하지 말고 새 query view를 만든다.
        // (6) view 구성은 O(1), 원소 할당과 즉시 무효화가 없다. 순회는 본 원소 수에 선형이다. 저장소 파괴는
        //     항상 view를 dangling으로 만든다. vector 재할당은 이미 얻었거나 cache한 반복자를 무효화하므로
        //     시작한 view 재사용이 위험하다. 순회와 같은 저장소의 동시 변경은 data race가 될 수 있다.
        // 대표 문서: ../standard-library/algorithms-and-ranges.md
        return std::views::filter(rows_, predicate);
    }

    // 반환형 void는 성공값을 만들지 않는 명령임을 나타낸다. id는 작은 정수 값을 복사해 받는다.
    void deactivate(const int id) {
        // [호출 계약: range-for가 숨겨 호출하는 vector::begin/end와 iterator 연산]
        // (1) 수신 rows_는 완성된 UserRows lvalue이고 size 3의 유효한 연속 저장소다. 반복 전에
        //     구조를 바꾸는 실행 흐름은 없으며, 만들어질 row는 현재 원소를 빌리는 UserRow&다.
        // (2) range-for 전개는 iterator begin() noexcept와 iterator end() noexcept를 고르고, 같은
        //     vector의 random-access iterator 비교, operator*, 전위 operator++를 반복한다.
        // (3) 명시 인자는 없고 숨은 this로 rows_를 빌린다. 역참조는 소유권 없는 UserRow&를 본문에 주며
        //     id 비교와 active 대입만 하고 iterator/end를 외부에 저장하지 않는다.
        // (4) begin/end는 시작/과거-끝 iterator 값을, 비교는 bool 문맥에서 검사할 결과를, 역참조는
        //     UserRow&를, 전위 증가는 iterator&를 반환하고 모두 loop 제어 또는 row 바인딩에 사용된다.
        // (5) 성공 뒤 size/capacity와 iterator 유효성은 그대로이고, 찾은 행의 active만 false가 될 수 있다.
        // (6) 각 숨은 연산 O(1), 전체 O(n), 무할당·비무효화다. 과거-끝을 역참조하거나 무효 iterator를
        //     비교/증가하면 미정의 동작이며, 순회 중 구조 변경이나 다른 스레드의 무동기 접근은 금지한다.
        // 대표 문서: ../standard-library/containers-and-views.md
        // range-for는 rows_의 시작부터 끝까지 UserRow lvalue를 하나씩 참조한다. 포인터와 달리 참조는
        // null 상태를 표현하지 않으며 row 별칭은 현재 반복에서만 사용한다.
        for (UserRow& row : rows_) {
            if (row.id == id) {
                row.active = false;
                return;
            }
        }
    }

private:
    // private 접근 지정자는 서비스가 행 저장소를 직접 변경하지 못하게 하고 메서드 경계를 강제한다.
    UserRows rows_;
};

class UserService {
public:
    // 멤버 초기화 목록은 함수 본문 전에 reference member를 반드시 바인딩한다. explicit은 저장소 하나가
    // 서비스로 암시 변환되는 것을 막는다. reference는 소유하지 않으므로 repository가 서비스보다 오래 살아야 한다.
    explicit UserService(const UserRepository& repository) : repository_{repository} {}

    [[nodiscard]] UserCards load_cards(const int minimum_score) const {
        // eligible은 저장소를 빌린 lazy view다. 여기까지는 행을 읽거나 UserCard를 만들지 않았다.
        auto eligible = repository_.active_with_minimum(minimum_score);

        // [생성 계약: std::string 복사 생성이 포함된 UserCard aggregate 초기화]
        // (1) 각 호출 때 row는 filter를 통과한 살아 있는 const UserRow lvalue이고 row.name은 유효한
        //     const std::string lvalue다. 새 UserCard와 그 name member는 아직 생성 전이다.
        // (2) UserCard{row.id, row.name}의 둘째 멤버는 basic_string(const basic_string&) 복사 생성자를 고른다.
        // (3) id 값은 복사하고 name은 const 참조로 읽어 별도 문자 버퍼에 복사한다. 원본 소유권은 유지된다.
        // (4) 생성자는 반환값이 없고 lambda는 독립 소유하는 UserCard prvalue를 반환한다.
        // (5) 원본 row는 그대로이며 결과 card의 name은 저장소 문자열과 수명을 공유하지 않는다.
        // (6) 이름 길이에 선형이고 문자 저장소 할당과 std::bad_alloc 가능성이 있다. 실패 시 미완성 card는
        //     정리되고 원본은 변하지 않는다. 성공한 card는 원본 vector 재할당과 무관하게 유효하다.
        const auto projector = [](const UserRow& row) {
            return UserCard{row.id, row.name};
        };

        // [호출 계약: std::views::transform(eligible, projector)]
        // (1) eligible은 repository_를 간접 참조하는 유효한 filter_view lvalue이고 projector는 비상태
        //     복사 가능한 lambda lvalue다. repository_는 load_cards 호출 동안 살아 있다.
        // (2) std::views::transform의 (R&&, F&&) overload에서 R은 eligible의 lvalue reference,
        //     F는 const lambda&이며 결과는 filter view와 함수 객체를 저장한 transform_view 계열이다.
        // (3) view/함수 객체를 결과 view 표현 안으로 복사한다. 저장소 행 소유권은 옮기지 않고 각 원소는
        //     나중에 const UserRow&로 projector에 전달된다.
        // (4) lazy transform view prvalue를 projected에 직접 초기화하며 UserCard는 아직 하나도 만들지 않는다.
        // (5) eligible과 저장소는 변하지 않는다. projected를 순회할 때 통과한 행마다 새 UserCard prvalue가 생긴다.
        // (6) view 구성 O(1), 무할당이다. 전체 순회는 행 수+복사 문자 수에 선형이고 predicate/projector
        //     예외를 전달한다. 기반 저장소 수명과 반복자 무효화 규칙을 그대로 가진다.
        // 대표 문서: ../standard-library/algorithms-and-ranges.md
        auto projected = std::views::transform(eligible, projector);

        // [호출 계약: std::ranges::to<UserCards>(projected)]
        // (1) 입력 projected는 아직 소비하지 않은 유효한 transform_view lvalue이고 목적 컨테이너 타입은
        //     정확히 std::vector<UserCard>다. repository_와 모든 기반 행이 순회 끝까지 살아 있다.
        // (2) C++23 ranges::to<C>(R&&, Args&&...)에서 C=UserCards, R=decltype(projected)&,
        //     추가 생성자 인자 pack은 비어 있고 vector의 from-range 생성 경로를 선택한다.
        // (3) projected는 forward_range라 구현은 거리를 구하는 순회 뒤 원소 구성 순회를 할 수 있다. 원본
        //     저장소 소유권은 안 옮기며 두 번째 순회의 UserCard/name 복사가 snapshot 소유권을 만든다.
        // (4) 모든 통과 원소를 소유한 UserCards prvalue를 반환해 함수 반환 객체에 직접 구성한다.
        // (5) 원본 저장소 행은 그대로이고 결과 vector의 size는 통과 행 수다. 순회는 filter view의
        //     내부 첫 통과 iterator cache를 갱신할 수 있지만, 함수 종료로 두 view가 파괴돼도 결과의
        //     UserCard와 string은 독립적으로 살아 있다.
        // (6) 시간 O(n+k+총 문자 길이), 공간 O(k+문자)다. forward_range용 vector 생성은 원소 저장소를
        //     재할당하지 않지만 술어는 거리·구성 순회에서 반복 평가될 수 있고 projector는 구성 순회에서
        //     평가된다. 저장소/string 할당·복사 예외 시 부분 결과는 파괴되고 원본은 유지된다.
        // 대표 문서: ../standard-library/algorithms-and-ranges.md
        return std::ranges::to<UserCards>(projected);
    }

private:
    const UserRepository& repository_;
};

int main() {
    // [생성 계약: std::string(const char*)와 vector initializer_list 생성]
    // (1) 세 문자열·seed 목적 객체는 아직 없고 문자열 literal은 프로그램 끝까지 사는 null 종료 char 배열이다.
    // (2) 각 std::string{"..."}은 basic_string(const char*, const Allocator& = {})를, seed는
    //     vector(initializer_list<UserRow>, const Allocator& = {})를 선택한다.
    // (3) char pointer는 null이 아니고 종단 문자가 있다. UserRow prvalue 세 개는 initializer_list의 const
    //     원소가 된 뒤 vector로 복사되며 외부 버퍼 소유권을 빌리지 않는다.
    // (4) 생성자는 반환값이 없다. seed는 세 UserRow와 각 name 문자 버퍼를 최종적으로 독점 소유한다.
    // (5) literal은 변하지 않고 seed.size()==3이며 입력 순서를 보존한다. 임시/list 원소는 식 끝에 파괴된다.
    // (6) 총 문자 수와 원소 수에 선형이고 vector/string 할당 실패·길이 초과 예외가 가능하다. 생성 실패 시
    //     완성된 부분은 역순 정리된다. 새 컨테이너라 외부 반복자 무효화나 공유 스레드 상태는 없다.
    UserRows seed{
        UserRow{1, std::string{"Ada"}, 91, true},
        UserRow{2, std::string{"Bjarne"}, 72, true},
        UserRow{3, std::string{"Linus"}, 88, true},
    };

    // std::move(seed)는 이름 있는 seed lvalue를 xvalue로 바꿔 값 매개변수로 이동 구성하게 한다. 위의
    // std::move/vector 이동 계약이 적용된다. repository 생성자의 멤버 초기화에서 다시 최종 owner로 옮긴다.
    UserRepository repository{std::move(seed)};
    UserService service{repository};

    // 반환 식은 UserCards prvalue다. C++17 이후 같은 타입 결과 객체에 직접 초기화되어 중간 vector의
    // 필수 복사/이동이 없다. 이를 보장 복사 생략이라 하며 first가 모든 원소 수명을 소유한다.
    UserCards first{service.load_cards(80)};

    // materialize 뒤 저장소 행을 바꿔도 first는 이미 깊게 복사한 이름과 원소를 보존한다.
    repository.deactivate(1);
    UserCards second{service.load_cards(80)};

    // [호출 계약: const vector::size()]
    // (1) 수신자는 각각 2개/1개 UserCard를 소유하는 const가 아닌 UserCards lvalue first/second다.
    // (2) size_type size() const noexcept가 숨은 const this만 받아 선택된다.
    // (3) 명시 인자와 소유권 이동은 없고 컨테이너를 읽기만 한다.
    // (4) 각 원소 수 size_type prvalue를 반환해 first_count/second_count에 값으로 저장한다.
    // (5) vector의 size/capacity/원소와 모든 참조는 변하지 않는다.
    // (6) O(1), 무할당·비무효화·noexcept다. 다른 스레드가 같은 vector를 동시에 변경하지 않아야 한다.
    const auto first_count{first.size()};
    const auto second_count{second.size()};

    // [호출 계약: vector::operator[]]
    // (1) 수신 first는 size 2, second는 size 1인 UserCards lvalue이고 아래 인덱스 0/1은 모두 유효하다.
    // (2) 비-const vector의 reference operator[](size_type) overload가 선택된다.
    // (3) 0/1은 int prvalue에서 size_type으로 변환되며 소유권을 옮기지 않는다.
    // (4) UserCard&를 반환하고 즉시 name lvalue를 출력에 빌린다. 반환 참조를 저장하지 않는다.
    // (5) 컨테이너와 card/string 내용은 바뀌지 않는다.
    // (6) O(1), 무할당·비무효화다. 범위 검사가 없어 인덱스가 size 이상이면 미정의 동작이지만 위 size
    //     불변식이 전제조건을 증명한다. vector가 재할당되면 참조는 무효지만 출력 중 변경은 없다.
    const std::string& first_name{first[0].name};
    const std::string& first_other_name{first[1].name};
    const std::string& second_name{second[0].name};

    // [호출 계약: std::ostream 정수/size_type/string/char 삽입 operator<<]
    // (1) 왼쪽 수신은 정상 상태의 전역 std::cout std::ostream lvalue이고 모든 string 참조는 살아 있다.
    // (2) size_type/문자열에는 대응 산술 overload와 operator<<(ostream&, const string&), 구분자에는
    //     operator<<(ostream&, char)가 왼쪽부터 차례로 선택된다.
    // (3) 숫자·char 값은 복사하고 string은 const 참조로 빌린다. 출력 버퍼로 소유권을 이전하지 않는다.
    // (4) 각 호출은 같은 ostream&를 반환해 다음 삽입의 왼쪽 피연산자로 쓰며 마지막 반환은 버린다.
    // (5) `2 Ada Linus 1 Linus\n`이 출력 버퍼에 순서대로 추가되고 snapshot은 그대로다.
    // (6) 출력 문자 수에 선형이며 스트림 버퍼 할당, 상태 비트 설정, 예외 mask에 따른 ios_base::failure가
    //     가능하다. 컨테이너 참조는 무효화하지 않고 같은 stream에 동시 무동기 쓰기는 하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cout << first_count << ' ' << first_name << ' ' << first_other_name << ' '
              << second_count << ' ' << second_name << '\n';

    // 기계 실행 관점: materialize 때 거리 계산 순회 뒤 결과 vector 저장소를 확보하고, 다시 행을 load해
    // bool/score 비교·조건 분기와 선택된 이름의 copy/store를 수행할 수 있다. 이 경로의 vector 원소
    // 저장소 재할당은 없지만 string은 별도 할당할 수 있다. 실제 inlining, 분기 제거, SIMD, 할당 횟수와
    // 명령은 CPU·표준 라이브러리·컴파일러·최적화 옵션에 따라 달라 특정 형태로 단정하지 않는다.
    return 0;
}
