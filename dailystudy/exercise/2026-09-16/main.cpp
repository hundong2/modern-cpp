#include <any>           // std::any/any_cast: 구체 타입을 지운 채 값을 소유하고, typeid가 일치하는 타입으로 안전하게 조회한다.
#include <iostream>      // std::cout: 예제의 네 가지 조회 결과를 표준 출력에 기록한다.
#include <string>        // std::string: 메타데이터 키와 trace 문자열의 문자 저장소를 값으로 소유한다.
#include <unordered_map> // std::unordered_map: 문자열 키에서 타입 소거 값으로 평균 O(1) 조회하는 저장소다.
#include <utility>       // std::move: 이름 있는 값을 xvalue로 바꾸어 소유권 이전을 허용한다.

// MetadataBag은 HTTP/RPC 프레임워크의 핵심 요청 타입이 모든 부가 타입을 알지 않아도 되게 하는 경계다.
// std::any는 저장한 구체 객체를 값으로 소유한다. 따라서 이 예제의 string은 원래 인자의 수명과 독립적이다.
// 다만 저장 타입은 복사 생성 가능해야 하며, 꺼낼 때는 cv를 제외한 "정확히 같은 타입"을 요청해야 한다.
// struct는 기본 접근이 public인 반면 class는 기본 접근이 private이다. 여기서는 저장소를 숨기기 위해 class를 쓴다.
class MetadataBag {
public:
    // using은 새 타입을 만드는 문법이 아니라 긴 기존 타입에 문맥 이름을 붙이는 별칭이다.
    // 첫 템플릿 인자 std::string은 key_type, 둘째 std::any는 mapped_type이다.
    using Storage = std::unordered_map<std::string, std::any>;
    using size_type = Storage::size_type;

    // explicit은 정수 하나가 MetadataBag으로 뜻밖에 암시 변환되는 것을 막는다.
    // size_type은 음수가 없는 컨테이너 크기 타입이고, expected_entries는 예약 힌트일 뿐 논리 원소 수가 아니다.
    explicit MetadataBag(size_type expected_entries) {
        // [호출 계약: std::unordered_map::reserve]
        // (1) 수신 객체의 정확한 타입은 Storage, 즉 unordered_map<string, any>이며 기본 생성 직후 size()==0인 유효한 lvalue다.
        // (2) 선택 시그니처는 `void reserve(size_type count)`이고 템플릿 인자는 클래스의 string/any 및 기본 hash/equal/allocator다.
        // (3) 인자 expected_entries는 size_type lvalue 식이지만 값으로 복사되며, 예상 저장 수 이상의 모든 표현 가능한 값을 허용한다.
        //     이 호출은 입력 변수의 소유권을 받지 않고 현재 값만 읽는다.
        // (4) 반환형은 void라 사용할 반환값이 없다.
        // (5) 성공 뒤 max_load_factor를 넘지 않도록 bucket 수가 늘 수 있지만 size는 0이고 expected_entries는 변하지 않는다.
        // (6) 평균 O(size), 최악 O(size^2) 재해시 비용이며 지금은 빈 저장소다. bucket 할당 실패/길이 오류가 예외가 될 수 있다.
        //     실제 재해시가 일어나면 반복자는 무효화되지만 원소 참조·포인터는 무효화되지 않는다. 현재는 원소 관찰자가 없다.
        //     map 수명은 MetadataBag 수명과 같고, 같은 객체를 다른 스레드가 동시에 변경하려면 외부 동기화가 필요하다.
        values_.reserve(expected_entries);
    }

    // T는 호출자가 명시하는 저장 타입 템플릿 인자다. key/value를 값으로 받아 이 함수가 소유권 sink가 된다.
    // lvalue를 넘기면 매개변수 생성에서 복사하고, prvalue/rvalue를 넘기면 직접 구성 또는 이동할 수 있다.
    template <class T>
    void put(std::string key, T value) {
        // 이름이 있는 key와 value 식은 선언 타입과 무관하게 둘 다 lvalue다. std::move가 xvalue를 복원한다.
        // [호출 계약: std::move, std::any 값 생성자, std::unordered_map::insert_or_assign]
        // (1) 수신 values_의 정확한 타입은 Storage이며 살아 있는 유효한 map이다. key는 소유 문자열 lvalue,
        //     value는 살아 있는 T lvalue이고, T는 any가 저장할 수 있도록 복사 생성 가능한 객체 타입이어야 한다.
        // (2) `move<U>(U&&) noexcept`는 key에서 U=string&, value에서 U=T&로 추론되어 각각 string&&/T&&를 반환한다.
        //     `any(ValueType&&)`는 ValueType=T로 선택되어 decay된 T를 저장한다. 이어
        //     `insert_or_assign(key_type&&, M&&)`은 M=any로 선택되어 rvalue key와 mapped value를 받는다.
        // (3) 두 move의 인자는 비어 있어도 유효한 lvalue이며, xvalue 표시는 뒤 생성자에 소비 권한을 준다.
        //     insert 인자는 이동 가능한 key와 any prvalue다. key는 어떤 문자열도 허용하며 같은 키는 교체 의미다.
        // (4) move가 반환한 두 rvalue reference는 즉시 생성/삽입에만 사용한다. any 생성자는 반환값이 없다.
        //     insert_or_assign은 iterator와 삽입 여부 bool의 pair를 값으로 반환하지만 여기서는 의도적으로 버린다.
        // (5) 새 키면 map이 T를 소유한 node를 하나 얻고, 기존 키면 기존 any와 그 payload를 파괴·교체한다.
        //     성공 뒤 key string은 유효하지만 값은 미지정이다. value는 계속 살아 있으나 이동 뒤 상태는 T의 계약에
        //     따른다(오늘의 string은 유효·미지정, int는 같은 값). 두 매개변수는 함수 종료 시 파괴된다.
        // (6) move는 별도 표준 복잡도 절 없이 참조 변환만 하며 무할당·noexcept다. any 값 생성의 비용/예외는
        //     T의 생성과 구현의 저장 전략에 따르고 표준은 이 any 연산에 별도 복잡도 상한을 두지 않는다.
        //     map 수정은 평균 O(1)·최악 O(n)이고 node/bucket 할당이 실패할 수 있다. 재해시는 반복자를 무효화하지만
        //     다른 원소의 참조·포인터는 보존한다. 같은 키 교체는 과거 payload 포인터를 무효화한다.
        //     map이 payload를 소유하며 MetadataBag 파괴/교체/erase 전까지 산다. 동시 put/read에는 외부 동기화가 필요하다.
        values_.insert_or_assign(std::move(key), std::any{std::move(value)});
    }

    // 반환 포인터는 소유하지 않는 관찰자다. nullptr은 "키 없음" 또는 "타입 불일치"를 예외 없이 표현한다.
    // 호출자는 둘을 구분하려면 먼저 key 존재 여부 API를 따로 두거나, 이 예제처럼 서로 다른 조회로 검사할 수 있다.
    template <class T>
    [[nodiscard]] const T* get(const std::string& key) const {
        // [호출 계약: std::unordered_map::find/end, const_iterator 비교]
        // (1) 수신 객체는 const Storage lvalue이며 이 호출 동안 변경되지 않는다. key는 살아 있는 const string lvalue다.
        // (2) `const_iterator find(const key_type&) const`, `const_iterator end() const`와 두 const_iterator의
        //     동등 비교 operator==가 선택된다. 클래스 템플릿 인자는 string/any 및 기본 정책 타입이다.
        // (3) find 인자 key는 빌려 읽고 소유권을 넘기지 않는다. end와 iterator 비교에는 사용자 인자가 없다.
        // (4) find는 같은 키 node 또는 end iterator를 반환해 found에 저장한다. end 반환은 비교에 쓰이고,
        //     비교 bool은 if 분기를 결정한다.
        // (5) map/key/원소는 모두 그대로다. 키가 없으면 즉시 nullptr를 반환하고 iterator를 역참조하지 않는다.
        // (6) find는 평균 O(1), 최악 O(n), end/비교는 상수 시간이며 할당·무효화가 없다. hash/equality가 던질 수
        //     있으므로 이 함수는 noexcept가 아니다. 반복자는 map보다 오래 살 수 없고, 동시 const 조회끼리는 가능하지만
        //     같은 map의 동시 변경은 동기화해야 한다.
        const auto found{values_.find(key)};
        if (found == values_.end()) {
            return nullptr;
        }

        // [호출 계약: std::any_cast<T>(const std::any*)와 const_iterator::operator->]
        // (1) found는 유효한 Storage::const_iterator이고 operator->가 가리키는 node의 second는 살아 있는 const any다.
        // (2) iterator의 `operator->() const` 뒤 `template<class T> const T* any_cast(const any*) noexcept`가 선택된다.
        // (3) 인자는 found->second의 주소인 const any* prvalue다. null이 아니며 소유권을 넘기지 않는다.
        //     T는 string 또는 int처럼 cv를 제외한 저장 타입과 정확히 비교 가능한 객체 타입이어야 한다.
        // (4) payload 타입과 typeid(T)가 일치하면 그 const T*를, 다르면 nullptr를 반환한다. typeid는 최상위 cv를
        //     구별하지 않으므로 저장 int에 T=const int를 요청하는 것은 성공할 수 있으며, 값 타입 변환은 하지 않는다.
        // (5) any와 payload는 바뀌지 않는다. 반환 포인터는 payload를 별칭할 뿐 수명을 연장하거나 소유하지 않는다.
        // (6) iterator 접근은 직접 node 관찰이고, pointer-form any_cast에는 별도 표준 복잡도 절이 없다.
        //     any_cast는 무할당·noexcept라 bad_any_cast를 던지지 않는다.
        //     포인터는 해당 key 교체/삭제, map/MetadataBag 파괴 때 무효가 된다. 재해시만으로 node 참조는 무효화되지 않는다.
        //     같은 payload에 겹치는 쓰기가 없어야 하며 const 조회끼리는 외부 변경이 없을 때 안전하다.
        return std::any_cast<T>(&found->second);
    }

private:
    // [생성 계약: std::unordered_map<std::string, std::any> 기본 생성]
    // (1) 생성될 수신 객체의 정확한 타입은 Storage이고 아직 수명이 시작되지 않은 values_ subobject다.
    // (2) 선택 생성자는 `unordered_map()`이며 key/mapped 템플릿 인자는 string/any, 나머지는 기본 hash/equal/allocator다.
    // (3) 명시적 인자는 없고 기본 정책 객체들이 값으로 구성된다.
    // (4) 생성자는 반환값이 없으며 완성된 빈 map이 MetadataBag의 private 멤버가 된다.
    // (5) 성공 뒤 values_.size()==0이고 어떤 key도 소유하지 않는다. MetadataBag 소멸 시 모든 node/payload도 함께 파괴된다.
    // (6) 상수 시간이며 구현이 초기 bucket 저장소를 할당할지는 고정하지 않는다. 정책/할당 실패 예외가 나면
    //     MetadataBag 생성도 실패하고 완성 객체·관찰자는 없다. 반복자 무효화 대상도 없고 생성 중 공유 접근은 허용되지 않는다.
    Storage values_{};
};

// RequestContext는 실제 요청 수명 동안 MetadataBag을 값 멤버로 소유하는 상위 경계다.
// class의 private 기본 접근과 public 접근 지정자가 불변식 경계를 만든다.
class RequestContext {
public:
    using size_type = MetadataBag::size_type;

    // explicit은 `RequestContext context = 2U;` 같은 의미가 불분명한 암시 변환을 금지한다.
    // 멤버 초기화 목록은 생성자 본문의 실행보다 먼저 metadata_를 직접 구성한다.
    explicit RequestContext(size_type expected_metadata_entries)
        : metadata_{expected_metadata_entries} {}

    // 반환형의 &는 복사 없이 기존 bag을 별칭하는 lvalue reference다. null일 수 없고 소유권도 이전하지 않는다.
    [[nodiscard]] MetadataBag& metadata() noexcept {
        return metadata_;
    }

    // 뒤의 const는 *this를 변경하지 않는 overload임을 뜻하며, 반환 const reference로 호출자의 변경도 막는다.
    [[nodiscard]] const MetadataBag& metadata() const noexcept {
        return metadata_;
    }

private:
    MetadataBag metadata_;
};

// 실행 관점에서 map 조회는 보통 key 문자 load, hash 계산, bucket/node 주소 load, 문자열 비교와 조건 분기를 만든다.
// any_cast는 보통 타입 식별 정보 비교와 null/성공 분기를 포함하고, any의 생성·파괴는 구현의 type-erased manager를
// 간접 호출할 수 있다. 그러나 표준은 vtable, 가상 호출, 작은 객체 버퍼, heap 사용이나 특정 명령어를 강제하지 않는다.
// 실제 load/store/분기/간접 호출은 CPU, 표준 라이브러리 구현, 컴파일러와 최적화 옵션에 따라 달라진다.

// 반환형 int는 기본 정수 타입이다. 0은 성공, 0이 아닌 값은 자체 검증 실패를 운영체제에 알린다.
int main() {
    // 2U는 unsigned int prvalue이고 중괄호 초기화는 위험한 narrowing을 거부한다.
    // context는 이름이 있으므로 lvalue이며, main의 끝까지 metadata_와 그 payload를 소유한다.
    RequestContext context{2U};
    MetadataBag& writable_metadata{context.metadata()};

    // [생성 계약: std::string(const char*)]
    // (1) 각 수신 string 임시 객체는 아직 수명이 시작되지 않았다. 문자열 리터럴은 정적 수명의 const char 배열 lvalue다.
    // (2) 선택 시그니처는 `basic_string(const char* s, const Allocator& = Allocator())`이고 char/기본 traits/allocator를 쓴다.
    // (3) 배열-포인터 변환된 const char* prvalue가 입력이며 모두 NUL 종료된 유효한 범위를 가리킨다. 소유권은 빌리지 않는다.
    // (4) 생성자는 반환값이 없고 각 string prvalue가 put의 string/T 값 매개변수를 직접 초기화한다.
    // (5) 생성된 string은 문자를 독립 소유하고 리터럴은 변하지 않는다. put 뒤 임시는 소비되고 full-expression 끝에 파괴된다.
    // (6) 각 문자열 길이에 선형이고 작은 문자열 최적화 여부는 구현별이다. 할당/길이 예외가 가능하며 실패 시 완성 string이 없다.
    //     외부 관찰자가 없어 무효화 문제도 없고, 서로 다른 임시 객체 생성에는 공유 가변 상태가 없다.
    writable_metadata.put<std::string>(std::string{"trace"}, std::string{"req-42"});
    writable_metadata.put<int>(std::string{"retries"}, 3);

    // const lvalue reference는 새 소유 객체를 만들지 않고 context를 읽기 전용으로 별칭한다.
    // 참조 자체는 null일 수 없고 context보다 오래 살면 안 된다. 이 바인딩은 복사/이동 호출이 아니다.
    const RequestContext& readonly_context{context};
    const MetadataBag& readable_metadata{readonly_context.metadata()};

    // 템플릿 인자 string/int가 기대 payload 타입을 컴파일 타임에 정한다. 네 포인터는 모두 비소유 관찰자다.
    // string{...}는 prvalue이고 get의 const reference 매개변수에 full-expression 동안 바인딩되어 안전하다.
    const std::string* const trace{
        readable_metadata.get<std::string>(std::string{"trace"})};
    const int* const retries{
        readable_metadata.get<int>(std::string{"retries"})};
    const std::string* const missing{
        readable_metadata.get<std::string>(std::string{"missing"})};
    const int* const wrong_type{
        readable_metadata.get<int>(std::string{"trace"})};

    // 포인터와 nullptr 비교는 소유권이나 수명을 바꾸지 않는 기본 연산이다.
    // 첫 두 성공 포인터가 null이면 아래 역참조가 미정의 동작이므로 먼저 분기해 차단한다.
    const bool missing_key{missing == nullptr};
    const bool wrong_type_detected{wrong_type == nullptr};
    if (trace == nullptr || retries == nullptr || !missing_key || !wrong_type_detected) {
        return 1;
    }

    // [호출 계약: std::cout의 문자열/정수/문자 삽입 operator<<]
    // (1) 정확한 수신 타입은 std::ostream인 전역 std::cout lvalue이며 사용 가능한 상태를 기대한다.
    //     trace/retries는 각각 살아 있는 const string/int를 가리키고 두 bool은 true다.
    // (2) const char*, const string&, int, char에 대응하는 basic_ostream 삽입 overload가 왼쪽부터 선택된다.
    // (3) 리터럴은 정적 수명의 const char 배열에서 변환된 포인터, *trace는 const string lvalue,
    //     *retries는 const int lvalue, '\n'은 char prvalue다. 삼항식 결과도 정적 리터럴 포인터이며 소유권 이전이 없다.
    // (4) 각 operator<<는 같은 ostream&를 반환해 다음 삽입의 수신자로 사용하고 마지막 반환 참조만 버린다.
    // (5) 성공하면 요구된 네 줄과 마지막 개행이 순서대로 버퍼에 기록되며 metadata와 payload는 변하지 않는다.
    // (6) 출력 문자 수/locale/장치에 따른 비용이고 stream 내부 버퍼 할당이 가능하다. 실패는 상태 비트에 기록되며
    //     exception mask 설정 시 예외가 날 수 있다. payload 포인터는 출력 동안 유효해야 한다. 표준 stream의 동시 호출은
    //     data race를 피하도록 동기화될 수 있어도 여러 호출의 문자가 섞일 수 있으므로 한 레코드의 원자성은 보장되지 않는다.
    std::cout << "trace=" << *trace << '\n'
              << "retries=" << *retries << '\n'
              << "missing=" << (missing_key ? "true" : "false") << '\n'
              << "wrong-type=" << (wrong_type_detected ? "true" : "false") << '\n';

    // 이 파일은 값을 반환하는 factory가 없어 NRVO 대상은 없다. 대신 위 string prvalue가 값 매개변수를 직접
    // 초기화하는 C++17 이후의 보장된 prvalue 구체화 규칙 덕분에 불필요한 중간 string 복사/이동이 생기지 않는다.
    return 0;
}
