#include <flat_map> // std::flat_map: 정렬된 연속 저장소 두 개로 작은 설정 표를 소유한다.
#include <iostream> // std::cout: 예제의 확인 결과를 표준 출력으로 보낸다.
#include <string>   // std::string: 설정 키의 문자를 값으로 소유한다.
#include <utility>  // std::move: 이름 있는 객체를 이동 입력인 xvalue로 표시한다.

// struct는 기본 public이다. 두 기본 타입은 조회 결과를 값으로 전달하고 포인터 수명 문제를 없앤다.
struct LookupResult {
    bool found{}; // bool은 참/거짓이며 {}는 false로 초기화한다.
    int value{};  // int는 이 예제의 작은 양수 설정값에 충분하고 {}는 0이다.
};

// class는 기본 private이다. 저장 방식을 감추면 호출자는 정렬 불변식을 임의로 깨뜨릴 수 없다.
class ConfigTable {
public:
    // using은 새 타입이 아니라 기존 템플릿 특수화의 별칭이다. key=string, mapped=int다.
    using Storage = std::flat_map<std::string, int>;

    // explicit은 Storage 한 개를 ConfigTable로 암시 변환하는 경로를 막는다.
    // [호출 계약: std::move와 flat_map 이동 생성]
    // (1) 수신 대상 entries_는 아직 생성 전, entries는 유효한 Storage 값 매개변수 lvalue다.
    // (2) move<Storage&>(Storage&) -> Storage&&, flat_map(flat_map&&)를 선택한다.
    // (3) entries는 정렬 불변식을 가진 소유 표; move 결과 xvalue는 저장소 이전을 허용한다.
    // (4) move는 Storage&& 참조를 생성에 전달하고 생성자는 별도 반환값이 없다.
    // (5) entries_가 키/값을 소유하고 entries는 유효하지만 값이 미지정인 상태로 끝난다.
    // (6) 이동 복잡도와 noexcept 여부는 두 기반 컨테이너/비교자의 이동 계약에 의존한다.
    //     이동 전 원소 관찰자는 새 저장소의 수명/무효화 규칙을 확인해야 하며 공유 중 변경은 동기화한다.
    explicit ConfigTable(Storage entries) : entries_{std::move(entries)} {}

    // const 참조 key는 호출 동안 문자열을 빌린다. 반환 struct는 조회 결과를 값으로 소유한다.
    // 포인터도 비소유 주소이지만 null 가능성과 원본 파괴 뒤 댕글링을 따로 검사해야 하므로 밖으로 내보내지 않는다.
    [[nodiscard]] LookupResult lookup(const std::string& key) const {
        // [호출 계약: flat_map::find]
        // (1) 수신 entries_는 정렬된 유효한 const Storage, key는 살아 있는 const string lvalue다.
        // (2) const_iterator find(const key_type&) const; key_type=std::string 오버로드다.
        // (3) key는 정확히 이 문자열 내용의 키로 읽기 전용 대여하며 소유권 이전은 없다.
        // (4) const_iterator를 found에 보관한다; 존재하면 원소, 아니면 end를 가리킨다.
        // (5) 표와 key는 변하지 않고, iterator는 표가 바뀌거나 파괴될 때 무효가 될 수 있다.
        // (6) 정렬 연속 키에 대한 이진 탐색 O(log n), 무할당; 비교자가 던지면 전파한다.
        //     key/표는 호출 동안 살아야 하고 동시 변경 없이 읽기만 하는 경우에만 공유 가능하다.
        const auto found{entries_.find(key)};

        // [호출 계약: flat_map::end와 iterator 비교]
        // (1) 수신 entries_는 유효한 const Storage이고 found는 같은 표에서 나온 const_iterator다.
        // (2) const_iterator end() const noexcept; 같은 범위 iterator의 operator== 비교를 선택한다.
        // (3) end에는 인자가 없고 found는 비소유 iterator lvalue로 비교에만 사용한다.
        // (4) end는 마지막 다음 위치, 비교는 bool이며 if 분기에 사용한다.
        // (5) 표/iterator는 변하지 않는다. end를 역참조하지 않는다.
        // (6) end와 비교는 O(1), 무할당·무예외; 같은 유효 범위의 iterator만 비교한다.
        if (found == entries_.end()) {
            return LookupResult{false, 0}; // 실패를 값으로 반환하여 잘못된 iterator 역참조를 피한다.
        }

        // [호출 계약: flat_map const_iterator::operator->]
        // (1) found는 위 분기에서 end가 아님이 확인된 유효한 const_iterator다.
        // (2) operator->() const는 읽기 전용 key/value 관찰자를 돌려주는 iterator 연산이다.
        // (3) 명시적 인자는 없고 found의 위치를 빌린다. second는 저장된 int lvalue다.
        // (4) second를 int 값으로 복사해 LookupResult의 value로 사용한다.
        // (5) 표와 iterator는 변하지 않으며 반환 결과는 표와 독립적이다.
        // (6) O(1), 무할당; end 역참조는 미정의 동작이므로 앞 분기가 필수다.
        //     표의 변경/파괴 뒤 관찰자는 무효이며 동시 변경은 동기화해야 한다.
        return LookupResult{true, found->second};
    }

private:
    Storage entries_; // 멤버는 표의 키/값을 소유하고 ConfigTable과 함께 파괴된다.
};

// 함수 반환형은 ConfigTable 값이다. 반환 prvalue가 호출자의 목적 객체에 직접 구성된다.
[[nodiscard]] ConfigTable make_defaults() {
    // [생성 계약: flat_map 기본 생성]
    // (1) 대상 entries는 아직 생성 전이며 수신 객체는 없다.
    // (2) flat_map() 기본 생성자; key_type=string, mapped_type=int, 기본 비교자를 쓴다.
    // (3) 명시적 인자는 없고 외부 문자열/저장소를 빌리지 않는다.
    // (4) 생성자 반환은 없으며 빈 정렬 표를 entries에 만든다.
    // (5) entries가 키/값 저장소를 소유한다; 초기 size는 0이다.
    // (6) 빈 생성은 상수 시간, 구현에 따른 빈 컨테이너 할당 가능성을 단정하지 않는다.
    //     유효한 비교자·allocator가 전제이고 실패 시 완성 객체가 남지 않는다.
    ConfigTable::Storage entries{};

    // [생성/호출 계약: string 리터럴 생성과 flat_map::insert_or_assign]
    // (1) entries는 빈 유효 Storage; 첫 key는 아직 생성 전의 string 대상이다.
    // (2) basic_string(const char*)와 insert_or_assign(key_type&&, M&&), M=int를 선택한다.
    // (3) "retries" 배열 lvalue의 NUL 전 문자를 새 string이 복사해 소유한다.
    //     그 string prvalue와 int prvalue 3은 표에 값으로 들어가며 외부 자원은 빌리지 않는다.
    // (4) string 생성자 반환은 없고 insert_or_assign의 pair<iterator,bool>는 의도적으로 버린다.
    // (5) 키가 없으므로 삽입되어 크기가 1; 전달 임시는 식 끝에 파괴된다.
    // (6) 탐색 O(log n), 연속 저장소 중간 삽입 O(n) 이동·할당 가능; 삽입은 iterator/참조를
    //     무효화할 수 있다. 비교·할당·이동 예외는 전파되고 일부 연산의 강한 보장은 요소 타입에 의존한다.
    //     리터럴은 정적 수명이고 동시 변경은 동기화한다.
    entries.insert_or_assign(std::string{"retries"}, 3);
    entries.insert_or_assign(std::string{"timeout_ms"}, 250);

    // [호출 계약: std::move와 ConfigTable 생성]
    // (1) entries는 두 키를 소유한 유효 Storage lvalue, 반환 목적 객체는 아직 생성 전이다.
    // (2) move<Storage&>(Storage&) -> Storage&&; ConfigTable(Storage) explicit 생성자다.
    // (3) xvalue 입력으로 소유 표를 값 매개변수에 이전한다; key/value를 빌리지 않는다.
    // (4) 생성자 반환은 없고 ConfigTable prvalue를 return 값으로 사용한다.
    // (5) 반환 객체가 표를 소유하고 지역 entries는 유효하지만 값 미지정 후 파괴된다.
    // (6) 이동·구성 비용/예외는 Storage 구성에 의존; 같은 타입 prvalue 반환에는 C++17 보장 복사 생략이
    //     적용되어 추가 ConfigTable 복사/이동이 필요 없다. 지역 객체를 직접 return하면 NRVO는 선택적이다.
    return ConfigTable{std::move(entries)};
}

int main() {
    // make_defaults()는 prvalue다. config는 그 결과를 직접 소유하고 수명은 main 끝까지다.
    ConfigTable config{make_defaults()};

    // [생성 계약: std::string 리터럴 생성]
    // (1) 대상 임시 string은 아직 없고 리터럴 배열은 정적 수명이다.
    // (2) basic_string(const char*)를 두 조회 인자 각각에 선택한다.
    // (3) NUL 종료 문자열을 읽어 새 문자를 소유하며 호출 뒤 외부 버퍼를 빌리지 않는다.
    // (4) 생성자 반환은 없고 임시 const string&이 lookup 호출 동안 바인딩된다.
    // (5) 임시는 각 전체 식 끝에 파괴되며 config와 결과 값은 그대로다.
    // (6) 길이에 선형·할당/예외 가능; 유효 NUL 종료 배열이 전제다. 임시 수명은 호출을 포함한다.
    const LookupResult retries{config.lookup(std::string{"retries"})};
    const LookupResult missing{config.lookup(std::string{"unknown"})};

    // [호출 계약: ostream 삽입 연산자]
    // (1) 수신 std::cout은 유효한 std::ostream lvalue; retries/missing은 살아 있는 const 값이다.
    // (2) operator<<(ostream&,const char*), ostream::operator<<(int), operator<<(ostream&,char)를
    //     왼쪽부터 연쇄하며 첫 줄은 const char[9], int lvalue, char prvalue를 전달한다.
    // (3) 리터럴과 int를 읽기만 하고 소유권을 이전하지 않으며 '\n'은 한 문자를 출력한다.
    // (4) 각각 ostream&을 반환해 다음 삽입에 쓰고 마지막 반환 참조는 버린다.
    // (5) 출력 버퍼 위치/상태 비트가 바뀌고 입력 값은 변하지 않는다.
    // (6) 출력 문자 수에 선형(장치 비용 별도); 실패는 상태 비트 또는 exception mask의 예외로 나타난다.
    //     stream 수명은 프로그램 동안 유효하고 동일 stream의 동시 변경은 동기화가 필요하다.
    std::cout << "retries=" << retries.value << '\n';
    std::cout << "missing=" << (missing.found ? "present" : "absent") << '\n';
    // 기계 관점: 키 이진 탐색은 로드·문자 비교·분기, 출력은 버퍼 저장으로 이어질 수 있다.
    // 실제 명령과 메모리 배치는 CPU·라이브러리·컴파일러·최적화 옵션에 따라 달라진다.
}
