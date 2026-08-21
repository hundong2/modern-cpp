// <iostream>은 검증 결과를 쓰는 std::cout과 스트림 삽입 연산자를 선언한다.
#include <iostream>
// <string>은 세션 ID와 소유자 이름을 독점 소유하는 std::string을 선언한다.
#include <string>
// <unordered_map>은 해시 레지스트리와 C++17 노드 핸들 API를 선언한다.
#include <unordered_map>
// <utility>는 lvalue를 xvalue로 바꾸는 std::move 함수 템플릿을 선언한다.
#include <utility>

// struct는 기본 접근이 public이다. 값만 묶는 단순 세션 레코드라 공개 집합체가 알맞다.
struct Session {
    std::string owner{}; // string이 문자 버퍼를 소유하며 Session과 함께 수명이 끝난다.
    int generation{};    // int{}는 0으로 값 초기화되며 키 재지정 횟수를 센다.
};

// enum class는 열거자 이름을 바깥 범위로 흘리지 않고 int로 암시 변환되지 않는 강한 결과 타입이다.
enum class RenameResult {
    renamed,
    missing_source,
    destination_exists,
};

// class는 기본 접근이 private이다. 해시 테이블 변경 규칙을 public 멤버 함수 뒤에 캡슐화한다.
class SessionRegistry {
public:
    // using은 긴 템플릿 인스턴스 이름에 별칭을 붙일 뿐 새 타입이나 객체를 만들지 않는다.
    using Table = std::unordered_map<std::string, Session>;

    // 생성자에는 반환형이 없다. explicit은 Table 하나가 Registry로 뜻밖에 암시 변환되는 일을 막는다.
    explicit SessionRegistry(Table sessions)
        // std::move<Table&>(sessions)는 이름 있는 값 매개변수 lvalue를 Table&& xvalue로 바꾼다.
        // Table 이동 생성자가 버킷·원소 소유권을 sessions_로 넘길 수 있고 원본은 유효하지만 값은 미지정이다.
        // 멤버 초기화 목록은 생성자 본문보다 먼저 private 멤버를 직접 초기화한다.
        : sessions_{std::move(sessions)} {}

    // 값 매개변수는 호출자가 준 두 ID를 함수가 독립적으로 소유하게 해 안전하게 이동할 수 있다.
    [[nodiscard]] RenameResult rename(std::string old_id, std::string new_id) {
        // string operator==는 두 lvalue 문자열을 읽어 같은 문자인지 비교하고 bool prvalue를 반환한다.
        // 두 문자열은 바뀌지 않으며 최악 O(L), 할당·무효화가 없다.
        if (old_id == new_id) {
            // unordered_map::contains(const key_type&) const는 old_id를 빌려 평균 O(1)에 존재 여부 bool을 반환한다.
            // sessions_의 크기·버킷·원소 수명은 유지되고 해시/동등 비교가 던질 수 있으며 동시 변경 안전을 제공하지 않는다.
            return sessions_.contains(old_id) ? RenameResult::renamed : RenameResult::missing_source;
        }

        // contains의 인자 new_id는 const string&로 빌려지고 소유권은 호출자 함수에 남는다.
        // 반환 bool을 즉시 조건 분기에 쓰며 true면 어떤 노드도 꺼내기 전에 충돌을 보고한다.
        if (sessions_.contains(new_id)) {
            return RenameResult::destination_exists;
        }

        // unordered_map::extract(const key_type&)는 old_id const lvalue를 검색 키로 빌려 평균 O(1)에 원소를 분리한다.
        // 반환 Table::node_type prvalue를 node가 이동 생성한다. 성공하면 sessions_ 크기는 1 줄고 node가 key/mapped를 소유한다.
        // 실패하면 빈 node다. 다른 원소의 참조는 유지되지만 해당 원소는 재삽입 전 컨테이너 소속이 아니며 해시/동등 비교가 던질 수 있다.
        Table::node_type node{sessions_.extract(old_id)};
        // node_type::empty() const noexcept는 인자 없이 소유 원소 유무를 bool로 반환하고 node를 바꾸지 않는 O(1) 관찰이다.
        if (node.empty()) {
            return RenameResult::missing_source;
        }

        // node_type::key()는 인자 없이 수정 가능한 string&를 반환한다. node는 비어 있지 않아 전제조건을 만족한다.
        // std::move(new_id)는 new_id lvalue를 string&& xvalue로 바꾸고 string 이동 대입이 새 키 버퍼를 이전할 수 있다.
        // 호출 뒤 key는 새 ID를 소유하고 new_id는 유효하지만 값은 미지정이며 sessions_의 다른 원소는 바뀌지 않는다.
        node.key() = std::move(new_id);
        // node_type::mapped()는 인자 없이 Session&를 반환한다. 참조로 generation을 직접 1 증가시키며 소유권은 node에 남는다.
        // 비어 있지 않은 node라는 전제조건이 필요하고 호출 자체는 O(1), 무할당이며 반환 참조는 node/재삽입 원소 수명에 묶인다.
        ++node.mapped().generation;

        // unordered_map::insert(node_type&&)는 node xvalue의 소유권을 sessions_에 이전한다.
        // 반환 insert_return_type의 inserted는 성공 bool, position은 원소 반복자, node는 실패 시 되돌아온 소유 노드다.
        // 성공하면 크기가 원래대로 늘고 node는 비며 평균 O(1)이다. 재해시 시 반복자는 무효화되지만 원소 참조는 유지된다.
        // 이 단일 스레드 객체는 목적지 부재를 먼저 확인했으므로 정상 계약에서는 inserted가 true다.
        Table::insert_return_type inserted{sessions_.insert(std::move(node))};
        return inserted.inserted ? RenameResult::renamed : RenameResult::destination_exists;
    }

    // 반환 포인터는 Registry가 소유한 Session을 빌릴 뿐 수명을 연장하지 않는다.
    [[nodiscard]] const Session* find(const std::string& id) const {
        // unordered_map::find(const key_type&) const는 id를 빌려 평균 O(1)에 const_iterator를 값으로 반환한다.
        // 수신 sessions_는 바뀌지 않고 반환 반복자는 재해시·해당 원소 삭제 뒤 무효다. 해시/동등 비교가 던질 수 있다.
        const auto iterator{sessions_.find(id)};
        // unordered_map::end() const는 인자 없이 과거-끝 const_iterator를 O(1)에 반환하며 상태를 바꾸지 않는다.
        // 반복자 비교 operator==의 bool 결과로 실패를 판정하고, 성공이면 second의 주소를 비소유 포인터로 반환한다.
        return iterator == sessions_.end() ? nullptr : &iterator->second;
    }

private:
    Table sessions_{}; // Registry가 모든 키와 Session 객체의 수명을 독점 관리한다.
};

int main() {
    // Table 기본 생성자는 빈 버킷 구조를 만들고 기본 hash/equal/allocator 객체를 값으로 보관한다.
    SessionRegistry::Table initial{};
    // unordered_map::emplace(args...)는 string prvalue와 Session prvalue를 원소 생성 인자로 완벽 전달한다.
    // 반환 pair<iterator,bool>은 무시한다. 성공하면 "alpha" 원소를 소유하고 평균 O(1), 할당/bad_alloc과 재해시가 가능하다.
    initial.emplace(std::string{"alpha"}, Session{std::string{"kim"}, 0});
    // 두 번째 emplace도 키·값 소유권을 컨테이너로 이동하며 기존 키와 달라 bool은 true다.
    initial.emplace(std::string{"occupied"}, Session{std::string{"lee"}, 4});

    // initial은 lvalue지만 std::move 결과 xvalue가 값 매개변수와 멤버로 이동되어 registry가 원소를 이어 소유한다.
    SessionRegistry registry{std::move(initial)};

    // 두 const char* 리터럴은 각각 string 값 매개변수를 직접 초기화한다. 함수는 old ID를 빌려 찾고 new ID 자원을 키로 옮긴다.
    // 반환 enum class 값을 renamed에 저장하며 registry의 alpha 키는 beta로 바뀌고 Session 객체 값은 유지된다.
    const RenameResult renamed{registry.rename("alpha", "beta")};
    // 목적지 키가 이미 있으므로 반환값은 destination_exists이고 beta/occupied 두 원소는 모두 유지된다.
    const RenameResult collision{registry.rename("beta", "occupied")};

    // string 임시 "beta"가 const string& 매개변수에 호출 전체 식 동안 바인딩된다.
    // 반환 const Session*은 registry 내부를 빌리므로 registry보다 오래 쓰거나 이후 삭제/재해시 뒤 쓰면 안 된다.
    const Session* const session{registry.find("beta")};
    // operator<<는 각 값을 출력 버퍼에 쓰고 ostream&를 연쇄 반환한다. 최종 참조는 버리며 I/O 실패는 상태 비트로 남는다.
    std::cout << (renamed == RenameResult::renamed) << ' '
              << (collision == RenameResult::destination_exists) << ' '
              << (session != nullptr ? session->owner : std::string{"missing"}) << ' '
              << (session != nullptr ? session->generation : -1) << '\n';

    // &&는 왼쪽부터 단락 평가한다. 포인터가 null이면 역참조 식은 실행되지 않는다.
    return renamed == RenameResult::renamed &&
                   collision == RenameResult::destination_exists && session != nullptr &&
                   session->owner == "kim" && session->generation == 1
               ? 0
               : 1;
}
