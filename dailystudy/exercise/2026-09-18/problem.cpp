#include <flat_map> // std::flat_map: 정렬된 연속 키/값 저장소로 작은 기능 표를 소유한다.
#include <iostream> // std::cout: 복사본과 배포본의 차이를 출력한다.
#include <string>   // std::string: 기능 이름의 문자를 자신의 저장소로 소유한다.
#include <utility>  // std::move: 명명된 lvalue를 이동 입력인 xvalue로 바꾼다.

// 직접 해보기: copy 뒤 live만 고치면 before는 왜 그대로인가? 이동 뒤 live의 내용은 왜 검사하면 안 되는가?
// struct는 기본 public이다. 조회 결과의 두 bool을 값으로 복사하므로 비소유 iterator를 밖에 내보내지 않는다.
struct FlagCheck {
    bool known{};   // {}는 false다. 키가 없음을 별도의 값으로 표현한다.
    bool enabled{}; // 알려진 키의 on/off 상태다.
};

// class는 기본 private이다. 읽기 중심 표의 변경 경로를 set 하나로 모은다.
class FeatureSet {
public:
    // 별칭은 새 타입이 아니다. string 키를 소유하고 int 0/1로 기능 상태를 표현한다.
    // GCC 16의 기본 mapped vector<bool> proxy는 flat_map iterator와 충돌하므로 int를 택한다.
    using Storage = std::flat_map<std::string, int>;

    // explicit은 Storage가 FeatureSet으로 뜻밖에 암시 변환되는 것을 막는다.
    // [호출 계약: std::move와 flat_map 이동 생성]
    // (1) 수신 대상 flags_는 아직 생성 전, 입력 flags는 유효한 Storage 값 매개변수 lvalue다.
    // (2) move<Storage&>(Storage&) -> Storage&&, flat_map(flat_map&&) 생성자를 선택한다.
    // (3) flags는 키/값을 소유하고 xvalue는 그 저장소를 이전하도록 허용한다.
    // (4) move 반환 참조는 생성에 쓰이고 생성자는 별도 반환값이 없다.
    // (5) flags_가 내용을 소유하며 flags는 유효하지만 값 미지정 상태로 파괴된다.
    // (6) 복잡도/예외는 기반 컨테이너·비교자의 이동에 의존한다. 과거 원소 관찰자는 수명과
    //     무효화 경계를 재확인해야 하며 공유 중 변경은 동기화해야 한다.
    explicit FeatureSet(Storage flags) : flags_{std::move(flags)} {}

    // 값 매개변수 key는 호출에서 복사/이동되어 함수 안에서 소유된다. bool은 작은 기본 타입 값이다.
    void set(std::string key, bool enabled) {
        // [호출 계약: std::move와 flat_map::insert_or_assign]
        // (1) 수신 flags_는 정렬된 유효 Storage, key는 소유 문자열 lvalue, enabled는 bool 값이다.
        // (2) move<string&>(string&) -> string&&; insert_or_assign(key_type&&, M&&), M=int다.
        // (3) key xvalue는 표에 이전 가능하고 조건식의 int prvalue 0/1을 값으로 저장한다.
        // (4) pair<iterator,bool>는 삽입 여부를 포함하지만 이 예제에서는 반환값을 버린다.
        // (5) 기존 키는 값이 대체되고 새 키는 정렬 위치에 삽입된다; key는 유효하나 값 미지정이다.
        // (6) 탐색 O(log n), 삽입 위치 이동은 O(n), 할당·비교·이동 예외 가능. 삽입/재배치는
        //     iterator·참조를 무효화할 수 있어 set 전의 관찰자를 재사용하지 않는다. 예외 시 세부 보장은
        //     기반 컨테이너와 요소 연산에 의존한다. 같은 표의 동시 읽기/쓰기는 동기화해야 한다.
        flags_.insert_or_assign(std::move(key), enabled ? 1 : 0);
    }

    // const 함수는 표를 바꾸지 않고 key를 호출 동안만 빌린다. 반환 struct는 독립 값이다.
    // 포인터로 원소를 돌려줬다면 null과 재배치 뒤 댕글링이 문제이므로 여기서는 값으로 복사한다.
    [[nodiscard]] FlagCheck check(const std::string& key) const {
        // [호출 계약: flat_map::find]
        // (1) 수신 flags_는 유효한 const Storage, key는 살아 있는 const string lvalue다.
        // (2) const_iterator find(const key_type&) const; key_type=std::string을 선택한다.
        // (3) key는 정확한 문자열 내용으로 읽기만 하며 소유권 이전은 없다.
        // (4) iterator는 일치 원소 또는 end를 뜻하며 found에 보관한다.
        // (5) 표와 key는 변하지 않는다; 반환 iterator는 표 변경·파괴 뒤 무효가 될 수 있다.
        // (6) 이진 탐색 O(log n), 무할당; 비교자 예외는 전파한다. 호출 중 객체가 살아 있어야 하고
        //     변경 없는 동시 읽기만 허용한다.
        const auto found{flags_.find(key)};

        // [호출 계약: flat_map::end와 iterator 비교]
        // (1) flags_는 유효한 const Storage이고 found는 같은 표에서 얻은 iterator다.
        // (2) const_iterator end() const noexcept와 같은 범위 iterator의 operator==를 쓴다.
        // (3) end의 인자는 없고 found는 비교에만 빌린다.
        // (4) end는 마지막 다음 위치, 비교 bool은 if 분기에 사용된다.
        // (5) 객체와 iterator는 변하지 않으며 end를 역참조하지 않는다.
        // (6) O(1), 무할당·무효화 없음; 유효한 같은 범위 iterator만 비교한다.
        if (found == flags_.end()) {
            return FlagCheck{false, false};
        }

        // [호출 계약: flat_map const_iterator::operator->]
        // (1) found는 end가 아님을 확인한 유효 iterator다.
        // (2) operator->() const는 현재 원소의 읽기 전용 관찰자를 준다.
        // (3) 인자는 없고 second int lvalue를 0과 비교한다; 소유권 이전은 없다.
        // (4) 비교 bool을 FlagCheck에 넣어 반환하고 iterator 관찰자는 밖으로 내보내지 않는다.
        // (5) 표/iterator는 변하지 않으며 결과는 독립 값이다.
        // (6) O(1), 무할당; end 역참조는 미정의 동작이라 위 분기가 필수다.
        //     표 변경·파괴 뒤 원소 관찰자는 무효이며 동시 변경은 동기화해야 한다.
        return FlagCheck{true, found->second != 0};
    }

private:
    Storage flags_; // 멤버 선언 순서대로 생성되고 FeatureSet이 파괴될 때 함께 파괴된다.
};

[[nodiscard]] FeatureSet make_flags() {
    // [생성 계약: flat_map 기본 생성]
    // (1) 대상 flags는 아직 생성 전이며 수신 객체는 없다.
    // (2) flat_map()에서 key_type=string, mapped_type=int, 기본 비교자를 선택한다.
    // (3) 데이터 인자는 없고 외부 저장소를 빌리지 않는다.
    // (4) 생성자 반환은 없고 빈 정렬 표를 만든다.
    // (5) flags가 이후 키/값을 소유하며 초기 size는 0이다.
    // (6) 빈 구성은 O(1); allocator/비교자 구성 예외 가능성을 일반적으로 배제하지 않는다.
    //     객체는 완성된 뒤에만 관찰하고 공유 중 변경은 동기화한다.
    FeatureSet::Storage flags{};

    // [생성/호출 계약: string 생성과 flat_map::insert_or_assign]
    // (1) flags는 빈 유효 Storage, string 대상은 아직 생성 전이다.
    // (2) basic_string(const char*) 및 insert_or_assign(key_type&&, M&&), M=int를 선택한다.
    // (3) "cache" 배열은 정적 수명 lvalue이며 문자를 새 string이 복사해 소유한다.
    //     string prvalue와 int prvalue 1은 값으로 표에 전달된다.
    // (4) string 생성자 반환은 없고 insert_or_assign의 pair<iterator,bool>는 버린다.
    // (5) 새 키가 삽입되어 크기가 1이 되고 임시는 전체 식 끝에 파괴된다.
    // (6) 문자열 구성 O(length)·할당 가능, 탐색 O(log n)+중간 삽입 O(n)·할당 가능.
    //     iterator/참조 무효화 및 비교·이동·할당 예외에 주의하고 동시 변경은 동기화한다.
    flags.insert_or_assign(std::string{"cache"}, 1);
    flags.insert_or_assign(std::string{"audit"}, 1);

    // [호출 계약: std::move와 FeatureSet 생성]
    // (1) flags는 두 키를 소유한 Storage lvalue, 반환 목적 객체는 아직 없다.
    // (2) move<Storage&>(Storage&) -> Storage&&; explicit FeatureSet(Storage)를 선택한다.
    // (3) xvalue로 소유권 이전을 허용하되 호출자 데이터를 빌리지 않는다.
    // (4) 생성자는 별도 반환값이 없고 결과 prvalue를 return에 사용한다.
    // (5) 반환 객체가 내용을 소유, flags는 유효하지만 값 미지정 후 파괴된다.
    // (6) 구성 비용/예외는 Storage 이동에 의존; 같은 타입 prvalue return에는 C++17 보장 복사 생략이
    //     적용된다. 지역 FeatureSet 이름을 그대로 반환한다면 NRVO는 선택적이다.
    return FeatureSet{std::move(flags)};
}

int main() {
    // prvalue 결과로 live를 직접 만든다. 반환 객체는 이 함수 끝까지 살아 있다.
    FeatureSet live{make_flags()};

    // [생성 계약: FeatureSet/flat_map 복사 생성]
    // (1) 대상 before는 아직 생성 전, live는 두 원소가 든 유효 FeatureSet lvalue다.
    // (2) 암시적 FeatureSet(const FeatureSet&)가 멤버 flat_map(const flat_map&)을 복사한다.
    // (3) live를 읽기만 하며 string 키와 int 값을 새 저장소로 복사해 독립 소유한다.
    // (4) 생성자는 반환값이 없고 before가 스냅샷 역할을 한다.
    // (5) live는 그대로, before는 독립된 키/값 저장소를 갖는다.
    // (6) O(n+전체 키 문자 수) 및 할당·복사 예외 가능; 실패 시 부분 대상은 정리된다.
    //     두 객체의 수명은 독립이며 공유 데이터 변경은 별도 동기화가 필요하다.
    FeatureSet before{live};

    // [생성 계약: std::string 리터럴 생성]
    // (1) 임시 string 대상은 아직 생성 전, "cache" 배열은 정적 수명이다.
    // (2) basic_string(const char*)를 선택한다.
    // (3) NUL 전 문자를 복사해 소유하고 set의 값 매개변수로 이동/직접 구성될 수 있다.
    // (4) 생성자는 별도 반환값이 없고 임시는 set 호출의 key 입력이다.
    // (5) live만 수정되며 before의 독립 복사본은 바뀌지 않는다.
    // (6) 문자열 길이에 선형·할당/예외 가능, 유효 NUL 종료 배열이 전제다.
    live.set(std::string{"cache"}, false);

    // [호출/생성 계약: std::move와 FeatureSet 이동 생성]
    // (1) live는 수정된 키/값을 소유한 유효 FeatureSet lvalue, deployed는 아직 생성 전이다.
    // (2) move<FeatureSet&>(FeatureSet&) -> FeatureSet&& 및 암시적 FeatureSet(FeatureSet&&)이다.
    // (3) xvalue 입력은 표 저장소 이전을 허용한다; before는 관여하지 않는다.
    // (4) move 참조는 생성에 쓰이고 생성자는 별도 반환값이 없다.
    // (5) deployed는 cache=0을 소유한다. live는 유효하지만 값 미지정이라 더 조회하지 않는다.
    // (6) 비용/예외/관찰자 무효화는 기반 컨테이너 이동 계약에 의존한다.
    //     독립 객체의 수명은 main 끝까지이고 동시 접근은 없다.
    FeatureSet deployed{std::move(live)};

    const FlagCheck prior{before.check(std::string{"cache"})};
    const FlagCheck current{deployed.check(std::string{"cache"})};
    const FlagCheck absent{deployed.check(std::string{"missing"})};

    // [호출 계약: ostream 삽입 연산자]
    // (1) 수신 std::cout은 유효한 ostream lvalue; 세 FlagCheck 값은 살아 있다.
    // (2) operator<<(ostream&,const char*)와 operator<<(ostream&,char)를 연쇄한다.
    // (3) 리터럴 배열과 조건 연산자의 const char*를 읽고 '\n' char 값을 출력한다; 소유권 이전은 없다.
    // (4) 각 삽입은 ostream&을 반환해 연쇄하고 마지막 참조는 버린다.
    // (5) 출력 버퍼/상태 비트만 바뀌고 FeatureSet과 결과는 불변이다.
    // (6) 비용은 출력 문자 수/장치에 의존; 실패 비트 또는 exception mask 예외가 가능하다.
    //     stream 수명은 유효하고 동일 stream의 동시 변경은 동기화해야 한다.
    std::cout << "before=" << (prior.enabled ? "on" : "off") << '\n';
    std::cout << "deployed=" << (current.enabled ? "on" : "off") << '\n';
    std::cout << "missing=" << (absent.known ? "known" : "unknown") << '\n';
    // 기계 관점: 탐색은 키 로드·비교·조건 분기, set은 연속 원소 이동/저장으로 나타날 수 있다.
    // 실제 명령·할당·간접 호출 여부는 CPU·표준 라이브러리·컴파일러·최적화 옵션에 달려 있다.
}
