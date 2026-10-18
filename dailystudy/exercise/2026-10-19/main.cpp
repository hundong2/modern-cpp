// 오늘의 실무 패턴: 허용 사용자 목록은 한 객체가 값으로 소유하고, 판정 서비스는 그 객체를
// const pointer로 잠깐 관찰한다. std::ranges::contains는 반복자 쌍을 직접 노출하지 않고
// "이 ID가 정책 스냅숏에 있는가"라는 의도를 그대로 표현한다.

// <algorithm>은 C++23 범위 알고리즘 std::ranges::contains를 선언한다.
#include <algorithm>
// <iostream>은 표준 출력 객체 std::cout과 형식화 삽입 연산자를 선언한다.
#include <iostream>
// <utility>는 lvalue 식을 이동 가능 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>
// <vector>는 연속 저장소를 소유하는 동적 배열 std::vector를 선언한다.
#include <vector>

// using은 새 타입을 만들지 않고 기본 정수 타입 int에 도메인 이름을 붙인다.
using UserId = int;

// enum class는 정수로의 암시 변환을 막는 강한 열거형이다.
enum class Action {
    read,
    deploy,
};

// struct는 기본 접근이 public이다. 요청은 소유 자원 없이 두 작은 값을 모은다.
struct AccessRequest {
    UserId user_id{};       // {}는 기본 int를 0으로 값 초기화한다.
    Action action{Action::read};
};

// class는 기본 접근이 private이다. 외부에는 불변식을 지키는 생성·조회 함수만 공개한다.
class AllowList {
public:
    // explicit은 vector 하나가 AllowList로 뜻하지 않게 암시 변환되는 것을 막는다.
    // 값 매개변수 users는 호출자가 복사 또는 이동 비용을 명시적으로 선택하게 하고,
    // 멤버 초기화 목록은 생성자 본문 전에 users_를 만든다.
    explicit AllowList(std::vector<UserId> users)
        // [첫 호출 계약: std::move<std::vector<UserId>&> + vector 이동 생성]
        // (1) 자유 함수 move에는 수신 객체가 없고, users는 유효한 vector<UserId> lvalue이며
        //     목적 멤버 users_는 아직 생성되지 않았다.
        // (2) std::move<vector<UserId>&>(vector<UserId>&)가 vector<UserId>&&를 반환하고,
        //     vector(vector&&) overload가 T=UserId, Allocator=std::allocator<UserId>로 선택된다.
        // (3) users 식은 lvalue, move 결과는 xvalue다. 호출자는 매개변수의 버퍼 소유권을
        //     멤버로 이전해도 된다는 뜻을 표현하며 원소 참조를 빌려 저장하지 않는다.
        // (4) move 결과는 users_ 구성에 즉시 쓰고 별도 값으로 저장하지 않는다. 생성자는
        //     반환값이 없으며 완성된 users_가 저장소를 소유한다.
        // (5) 성공 뒤 users_는 이전된 원소를 보유하고 users는 유효하지만 내용은 미지정이다.
        // (6) 기본 allocator를 쓰는 vector 이동 생성은 표준상 O(1), noexcept다. users의
        //     원소 저장소를 새 owner에 인계하며 원소 관찰자는 그 owner 아래 이어질 수 있지만,
        //     표준 계약에 별도 내부 할당 횟수까지 덧붙여 단정하거나 users 자체 상태에 기대면 안 된다.
        //     users_ 파괴 때 저장소가 해제되며 같은 객체의 동시 쓰기에는 외부 동기화가 필요하다.
        : users_{std::move(users)} {}

    // 뒤 const는 이 관찰 함수가 AllowList의 논리 상태를 바꾸지 않음을 뜻한다.
    [[nodiscard]] bool contains(UserId user_id) const {
        // [첫 호출 계약: std::ranges::contains]
        // (1) 자유 range 알고리즘이라 사용자 수신 객체는 없다. 첫 인자 users_의 정확한 타입은
        //     살아 있는 const std::vector<UserId> lvalue이며 유효한 정수 원소를 소유한다.
        // (2) ranges::contains(R&&, const T&, Proj = {}) overload가 R=const vector<UserId>&,
        //     T=UserId, Proj=std::identity로 선택되고 반환형은 bool이다.
        // (3) users_는 빌린 lvalue range, user_id는 const UserId&에 결합되는 이름 있는 lvalue,
        //     projection은 생략되어 기본 prvalue다. 어느 인자도 소유권을 넘기지 않는다.
        // (4) 같은 값이 하나라도 있으면 true인 bool prvalue를 반환해 호출자에게 바로 돌려준다.
        // (5) vector, 원소, user_id, 반복자는 바뀌거나 무효화되지 않는다.
        // (6) 전제는 유효한 input_range와 == 비교 가능성이다. 최대 users_.size()번 비교하여
        //     O(n), 별도 동적 할당 없음이다. int 비교/기본 projection은 던지지 않는다.
        //     owner 파괴나 재할당 뒤 얻어 둔 반복자는 사용할 수 없고, 이 호출 자체는 반복자를
        //     밖으로 내보내지 않는다. 같은 vector를 다른 thread가 수정하면 데이터 경쟁이다.
        return std::ranges::contains(users_, user_id);
    }

private:
    // vector template 인자 UserId는 원소 타입이다. 컨테이너가 저장소와 원소 수명을 소유한다.
    std::vector<UserId> users_;
};

// 서비스는 정책 owner를 복사하지 않고 const pointer로 관찰한다. 포인터는 null일 수 있지만
// 생성자에서 유효한 참조의 주소만 받아 불변식상 null이 아니다.
class AccessGate {
public:
    // const AllowList&는 읽기 전용 비소유 참조다. &allow_list는 owner를 가리키는 pointer 값이며
    // 수명을 늘리지 않는다. allow_list가 gate보다 오래 살아야 한다.
    explicit AccessGate(const AllowList& allow_list, Action required) noexcept
        : allow_list_{&allow_list}, required_{required} {}

    [[nodiscard]] bool authorize(const AccessRequest& request) const {
        // &&는 왼쪽부터 평가하며 첫 조건이 false면 contains 호출을 생략하는 조건 분기다.
        return request.action == required_ && allow_list_->contains(request.user_id);
    }

private:
    const AllowList* allow_list_{}; // 비소유 관찰 pointer. 파괴 책임은 없다.
    Action required_{Action::read};
};

// 반환형은 소유 AllowList 값이다. 이름 있는 local을 반환하는 선택적 NRVO가 아니라,
// 아래 prvalue가 호출자의 결과 객체를 직접 초기화하는 C++17 보장 복사 생략 사례다.
[[nodiscard]] AllowList make_allow_list() {
    // [첫 생성 계약: std::vector<UserId>(initializer_list<UserId>)]
    // (1) 목적 vector는 아직 없고 {7,42,99} backing array의 세 const UserId가 유효하다.
    // (2) vector(initializer_list<value_type>, const Allocator& = Allocator()) overload가
    //     T=UserId, Allocator=std::allocator<UserId>로 선택된다.
    // (3) 세 정수 prvalue로 만든 backing array 원소를 읽어 복사한다. 기본 allocator 인자는
    //     생략되며 backing array의 소유권이나 참조를 보관하지 않는다.
    // (4) 생성자는 반환값이 없고 세 원소를 소유한 vector prvalue가 AllowList 값 매개변수를
    //     직접 초기화한 뒤 위 이동 생성으로 멤버에 소유권을 넘긴다.
    // (5) 완성된 AllowList의 vector 순서는 7,42,99이고 backing array와 수명이 독립이다.
    // (6) 시간/공간 O(3)이며 저장소 확보 중 length_error/bad_alloc 가능성이 있다. 표준은 내부
    //     할당 횟수를 이 계약에서 고정하지 않는다. 실패하면 완성된 AllowList가 없고 이미 만든
    //     원소는 정리된다. 구성 중에는 공유 접근이 없다.
    return AllowList{std::vector<UserId>{7, 42, 99}};
}

int main() {
    // make_allow_list() 결과 prvalue는 allow_list를 직접 초기화한다. 객체 수명은 main 끝까지다.
    // const owner로 선언해 gate가 살아 있는 동안 정책 저장소를 교체할 수 없는 읽기 스냅숏으로 쓴다.
    const AllowList allow_list{make_allow_list()};
    // owner를 먼저 선언했으므로 gate의 비소유 pointer는 gate가 쓰이는 동안 유효하다.
    AccessGate gate{allow_list, Action::deploy};

    // 중괄호 식은 AccessRequest prvalue다. const& 매개변수에 호출 full-expression 동안 결합된다.
    const bool allowed{gate.authorize(AccessRequest{42, Action::deploy})};
    const bool unknown_user{gate.authorize(AccessRequest{8, Action::deploy})};
    const bool wrong_action{gate.authorize(AccessRequest{42, Action::read})};

    // [첫 호출 계약: std::cout operator<< 연쇄]
    // (1) 왼쪽 수신 객체는 정상 상태의 std::ostream lvalue std::cout이고 한 thread만 쓴다.
    // (2) bool 값에는 basic_ostream<char>::operator<<(bool), 문자에는
    //     operator<<(basic_ostream<char>&, char)가 선택되며 각 호출은 ostream&를 반환한다.
    // (3) 세 bool은 이름 있는 lvalue에서 값으로 읽고 ' '/ '\n'은 char prvalue다. 소유권 이전은 없다.
    // (4) 각 반환 ostream&는 다음 <<의 왼쪽 피연산자로 사용하고 마지막 참조는 버린다.
    // (5) boolalpha를 켜지 않아 성공 시 "1 0 0\n"을 순서대로 기록한다. 정책 객체는 그대로다.
    // (6) 표준은 이 형식 출력 연쇄 전체에 하나의 이식 가능한 점근 복잡도를 정하지 않는다.
    //     locale 변환, 버퍼 할당과 장치 호출 비용은 구현별이다. 실패 시 failbit/badbit가 설정되고
    //     exceptions mask에 따라 ios_base::failure가 날 수 있다. 출력 자체는 다른 객체 참조를
    //     무효화하지 않으며 동시 레코드 원자성을 제공하지 않는다.
    std::cout << allowed << ' ' << unknown_user << ' ' << wrong_action << '\n';

    // 기계 관점에서 contains는 연속 원소 load, 정수 비교, 일치/끝 조건 분기를 만들 수 있고
    // pointer를 통한 호출은 주소 load를 포함할 수 있다. 이 타입에는 virtual 함수가 없어
    // 가상 간접 호출은 요구되지 않는다. 실제 인라이닝·분기 제거·명령 선택은 CPU, ABI,
    // 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라 특정 어셈블리로 단정하지 않는다.
    return 0;
}
