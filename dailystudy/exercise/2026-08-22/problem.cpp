// <iostream>은 연습 결과를 쓰는 std::cout을 선언한다.
#include <iostream>
// <string>은 자산 키와 파일 이름을 소유하는 std::string을 선언한다.
#include <string>
// <unordered_map>은 평균 상수 시간 조회와 노드 핸들 키 변경을 제공한다.
#include <unordered_map>
// <utility>는 std::move를 선언한다.
#include <utility>

// struct는 기본 public이다. 렌더링 자산의 작은 값 레코드를 집합체 초기화하기에 알맞다.
struct Asset {
    std::string file{}; // 파일 이름 문자를 이 객체가 독점 소유한다.
    int reloads{};      // 키가 바뀔 때마다 갱신 횟수를 1 올린다.
};

// class는 기본 private이다. 키 재지정 절차를 한 함수에 두어 호출자가 중간 상태를 보지 못하게 한다.
class AssetCatalog {
public:
    using Table = std::unordered_map<std::string, Asset>;

    // 생성자는 반환형이 없으며 값 매개변수의 map 소유권을 멤버로 옮긴다.
    explicit AssetCatalog(Table assets) : assets_{std::move(assets)} {}

    [[nodiscard]] bool rename(std::string old_key, std::string new_key) {
        // unordered_map::contains(const key_type&) const는 new_key를 빌려 평균 O(1)에 bool을 반환한다.
        // true면 수신 map과 두 인자를 바꾸지 않고 false를 반환해 기존 목적지 원소를 보호한다.
        if (assets_.contains(new_key)) {
            return false;
        }

        // extract(const key_type&)는 old_key를 빌려 찾은 원소를 소유 node_type prvalue로 분리한다.
        // 성공하면 map 크기가 1 줄고, 실패하면 빈 노드다. 평균 O(1), 해시/동등 비교 예외 가능성이 있다.
        Table::node_type node{assets_.extract(old_key)};
        // empty()는 인자·상태 변화 없이 bool을 반환하는 noexcept O(1) 관찰이다.
        if (node.empty()) {
            return false;
        }

        // key()의 string&에 new_key xvalue를 이동 대입한다. 새 키가 노드에 소유되고 new_key 값은 미지정이 된다.
        node.key() = std::move(new_key);
        // mapped()의 Asset&로 멤버를 직접 갱신한다. 노드가 비어 있지 않다는 전제조건 아래 O(1)이며 수명은 노드/삽입 원소에 묶인다.
        ++node.mapped().reloads;

        // insert(node_type&&)는 node 소유권을 map으로 되돌리고 insert_return_type을 값으로 반환한다.
        // 목적지 부재를 먼저 확인했으므로 inserted가 true여야 한다. 성공 시 map 크기는 원래대로이고 node는 빈다.
        Table::insert_return_type result{assets_.insert(std::move(node))};
        return result.inserted;
    }

    [[nodiscard]] const Asset* find(const std::string& key) const {
        // find는 key를 빌려 const_iterator를 반환하고 map을 바꾸지 않는다. 평균 O(1), 최악 O(N)이다.
        const auto iterator{assets_.find(key)};
        // end() 반환 반복자와 비교해 실패면 nullptr, 성공이면 저장 Asset의 비소유 주소를 반환한다.
        return iterator == assets_.end() ? nullptr : &iterator->second;
    }

private:
    Table assets_{};
};

int main() {
    AssetCatalog::Table initial{};
    // emplace의 두 인자는 string/Asset prvalue이며 컨테이너 원소 생성으로 이동된다.
    // pair<iterator,bool> 반환은 버리고, 평균 O(1), 할당 실패와 재해시 가능성을 받아들인다.
    initial.emplace(std::string{"hero"}, Asset{std::string{"hero.png"}, 0});
    AssetCatalog catalog{std::move(initial)}; // initial의 소유 버킷과 원소를 catalog 값 매개변수로 이동한다.

    // 리터럴에서 두 string 값 매개변수를 만들고 hero 원소의 키를 player로 바꾼다.
    const bool renamed{catalog.rename("hero", "player")};
    // 임시 string은 find 호출 동안 const 참조에 바인딩되고 반환 포인터는 catalog 내부를 빌린다.
    const Asset* const asset{catalog.find("player")};

    // 스트림 삽입은 bool·문자열·int를 쓰고 같은 ostream&를 연쇄 반환하며 최종 참조는 버린다.
    std::cout << renamed << ' ' << (asset != nullptr ? asset->file : std::string{"missing"}) << ' '
              << (asset != nullptr ? asset->reloads : -1) << '\n';

    // 포인터 null 검사를 먼저 두어 뒤의 멤버 접근이 유효할 때만 평가되게 한다.
    return renamed && asset != nullptr && asset->file == "hero.png" && asset->reloads == 1 ? 0 : 1;
}
