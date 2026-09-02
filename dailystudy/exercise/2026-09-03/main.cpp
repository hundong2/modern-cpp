// <iostream>은 표준 출력 객체 std::cout과 산술값·문자 스트림 삽입의 기반을 선언한다.
#include <iostream>
// <memory>는 공유 소유권 타입 std::shared_ptr와 std::make_shared를 선언한다.
#include <memory>
// <string>은 자기 버퍼를 소유하는 std::string과 그 문자열 스트림 삽입 overload를 선언한다.
#include <string>
// <utility>는 이름 붙은 객체를 xvalue 식으로 바꾸는 std::move를 선언한다.
#include <utility>

// struct는 별도 접근 지정자가 없으면 멤버가 public이다. 이 타입은 이미 검증된 불변 스냅샷의 데이터 묶음이다.
struct ReportData {
    std::string title; // string은 제목 문자를 소유하며 ReportData와 함께 수명이 끝난다.
    // 생성자가 이 멤버를 초기화 목록에서 생략할 때만 int{} 기본 멤버 초기화자가 0을 만든다.
    // 아래 생성자는 revision{received_revision}을 명시하므로 여기의 {}는 실행되지도, 먼저 덮어쓰이지도 않는다.
    int revision{};

    // 생성자는 반환형이 없다. explicit은 두 인자의 `ReportData value = {"title", 1};` copy-list-init을 막지만
    // `ReportData value{"title", 1};` 직접 초기화는 허용한다.
    // std::move의 핵심 시그니처는 template<class T> constexpr remove_reference_t<T>&& move(T&& value) noexcept다.
    // received_title은 string lvalue라 T=string&로 추론되고, 유일한 인자는 비소유 전달 참조로 바인딩된다.
    // 반환은 같은 객체를 가리키는 string&& xvalue이며 호출 자체는 O(1)·무할당·noexcept이고 상태를 바꾸지
    // 않는다. 참조는 즉시 다음 생성에 쓰며 수명을 연장하거나 스레드 동기화를 제공하지 않는다.
    // 선택된 string 이동 생성자 basic_string(basic_string&& other) noexcept에서 새 title이 목적 객체이고,
    // 위 xvalue가 rvalue reference 인자다. 반환값은 없고 title은 이동 전 원본과 같은 값을 얻으며 원본은
    // 유효하지만 값이 미지정된다. 이 overload는 상수 시간·noexcept다. 구현은 heap 저장소를 넘길 수 있지만
    // SSO 문자열의 inline 문자를 다룰 수도 있으므로 버퍼 이전·할당 방식은 단정하지 않는다. 기존 문자 관찰자를
    // 저장하지 않고 이동 후 귀속에도 기대지 않으며, 두 객체의 동시 접근은 별도 동기화가 필요하다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    explicit ReportData(std::string received_title, int received_revision) noexcept
        : title{std::move(received_title)}, revision{received_revision} {}
};

// class는 기본 접근이 private이므로 바깥에서는 owner_를 교체해 스냅샷 불변식을 깨뜨릴 수 없다.
class ReportSnapshot {
private:
    std::shared_ptr<const ReportData> owner_; // const 데이터와 제어 블록의 공유 소유권을 가진다.

    // private 생성자라 바깥 코드는 빈 shared_ptr를 넣어 불변식을 깰 수 없고 create()만 이 경로를 사용한다.
    explicit ReportSnapshot(std::shared_ptr<const ReportData> owner) noexcept
        // std::move(T&&)에서 owner는 shared_ptr<const ReportData> lvalue라 T=shared_ptr<const ReportData>&다.
        // 유일한 비소유 인자에 바인딩해 같은 객체의 shared_ptr<const ReportData>&& xvalue를 반환한다. 호출
        // 자체는 O(1)·무할당·noexcept이고 상태·참조 횟수·수명을 바꾸지 않으며 동기화도 제공하지 않는다.
        // 선택된 shared_ptr(shared_ptr&& other) noexcept의 목적 객체는 아직 구성 전인 owner_, 인자는 위
        // xvalue다. 반환값 없이 stored pointer와 control-block 몫을 옮겨 owner는 empty, owner_는 유일한 몫을
        // 갖고 strong count는 유지된다. 상수 시간·무할당·noexcept이며 관리 객체/부분 객체 관찰자는 무효화되지
        // 않는다. 같은 핸들 객체를 다른 스레드에서 동시에 바꾸지 않는다는 전제가 있다.
        : owner_{std::move(owner)} {}

public:
    // 복사 생성은 유효한 owner_의 몫을 O(1)·무할당·noexcept로 하나 늘려 두 wrapper가 같은 const 데이터를
    // 소유하게 한다. 복사 대입이 이전 마지막 owner를 놓으면 그 객체의 소멸·해제 비용은 추가될 수 있다.
    // 이동은 원본을 empty로 만들어 accessor 불변식을 깨므로 막는다. 핸들 동시 변경에는 별도 동기화가 필요하다.
    ReportSnapshot(const ReportSnapshot&) noexcept = default;
    ReportSnapshot& operator=(const ReportSnapshot&) noexcept = default;
    ReportSnapshot(ReportSnapshot&&) = delete;
    ReportSnapshot& operator=(ReportSnapshot&&) = delete;

    [[nodiscard]] static ReportSnapshot create(std::string title, int revision) {
        // make_shared<const ReportData>(args...)는 수신 객체 없이 string xvalue와 int lvalue를 전달해 const
        // ReportData를 직접 만든다. std::move에서 title lvalue 때문에 T=string&이고 반환 string&&는 즉시
        // 생성 인자로 쓰인다. make_shared의 Args는 string과 int&로 추론되고 반환형은
        // shared_ptr<const ReportData> prvalue다. 성공 뒤 그 값이 객체·제어 블록을 소유하고 title은 유효하지만
        // 값이 미지정된다. 객체와 제어 블록은 보통 한 번 할당되고 비용은 구성에 따르며 bad_alloc 가능성이 있다.
        // 실패하면 부분 자원을 정리한다. 반환 ReportSnapshot prvalue는 호출자의 결과 객체에 직접 구성된다.
        return ReportSnapshot{std::make_shared<const ReportData>(std::move(title), revision)};
    }

    [[nodiscard]] std::shared_ptr<const std::string> title_handle() const noexcept {
        // operator->의 수신자는 비어 있지 않은 const shared_ptr<const ReportData> lvalue owner_이고 인자는 없다.
        // const ReportData*를 O(1)·무할당·noexcept로 반환하며 참조 횟수와 객체 상태는 그대로다. 아래 내장 ->와
        // &가 title의 const string*를 만든다. owner_가 비었다면 역참조는 UB지만 생성 불변식이 이를 배제한다.
        // 이어지는 aliasing 생성자 template<class Y> shared_ptr(const shared_ptr<Y>& r, element_type* p) noexcept에서
        // Y=const ReportData, element_type=const string이다. r은 owner_ const lvalue 참조, p는 title을 가리키는
        // 비소유 포인터 prvalue다. 새 핸들은 p를 저장하지만 r의 제어 블록을 공유해 참조 횟수를 1 늘린다.
        // 생성자는 반환값이 없고 전체 construction expression의 결과가 shared_ptr<const string> prvalue다.
        // 바깥 ReportData나 title 문자는 복사하지 않는다. 할당·예외 없이 O(1)이며,
        // 핸들이 하나라도 남아 있으면 바깥 ReportData와 그 부분 객체 title의 수명도 함께 유지된다. 같은 제어
        // 블록의 서로 다른 shared_ptr 조작은 안전하지만 가리키는 객체의 동시 변경까지 동기화하지는 않는다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        return std::shared_ptr<const std::string>{owner_, &owner_->title};
    }

    [[nodiscard]] int revision() const noexcept {
        // 위와 같은 operator->가 const ReportData*를 반환하고 내장 멤버 접근이 revision 값을 읽는다.
        // owner_와 참조 횟수는 유지되며 O(1)·무할당·noexcept다. 비어 있지 않다는 클래스 불변식이 전제다.
        return owner_->revision;
    }
};

int main() {
    // shared_ptr<const string>() 기본 생성자는 수신 객체·인자·반환값 없이 빈 핸들을 O(1)·무할당·noexcept로
    // 만든다. use_count는 0이고 저장 포인터는 null이며 관리 객체 수명이나 다른 핸들 상태를 바꾸지 않는다.
    std::shared_ptr<const std::string> title_handle{};

    {
        // basic_string(const char*, allocator)는 문자열 리터럴 lvalue가 변환된 non-null C 문자열 포인터를 읽고
        // 끝의 null 전까지 문자를 string prvalue에 복사한다. 생성자는 반환값이 없고 결과 객체가 문자를 소유한다.
        // O(n), 저장소 할당과 length_error/bad_alloc 가능성이 있다. prvalue가 create의 값 매개변수를 직접
        // 초기화하고, create가 반환한 ReportSnapshot prvalue도 snapshot에 직접 구성되어 중간 복사·이동이 없다.
        auto snapshot{ReportSnapshot::create(std::string{"release-notes"}, 7)};

        // title_handle()이 만든 shared_ptr<const string> prvalue는 aliasing 제어 블록 몫 하나를 소유한다.
        // 선택된 shared_ptr 이동 대입 operator=(shared_ptr&&) noexcept의 수신자는 현재 빈 title_handle lvalue,
        // 인자는 그 임시 객체가 바인딩된 rvalue 참조다. 기존 소유 대상은 없고 새 몫을 O(1)·무할당으로 넘겨받아
        // *this lvalue를 반환하지만 버린다. 인자 임시는 빈 유효 상태가 되고 소멸하며, title_handle은 title을
        // 가리키고 바깥 ReportData를 살려 둔다. 같은 객체에 대한 동시 비const 접근은 별도 동기화가 필요하다.
        title_handle = snapshot.title_handle();

        // use_count()의 수신자는 비어 있지 않은 const shared_ptr 관찰 상태이고 인자는 없다. 같은 제어 블록을
        // 공유하는 소유자 수 long을 O(1)·무할당·noexcept로 반환해 inside_owners에 저장한다. 수신자·대상·수명은
        // 변하지 않는다. 동시 복사/파괴가 있으면 값은 즉시 낡을 수 있으므로 동기화 판단에 쓰면 안 되지만 여기서는
        // 단일 스레드이며 snapshot.owner_와 title_handle 두 소유자를 관찰해 2다.
        const long inside_owners{title_handle.use_count()};

        // operator*의 수신자는 non-null shared_ptr<const string> lvalue이고 인자는 없다. const string&를 O(1),
        // 무할당·noexcept로 반환하고 소유 수와 객체를 바꾸지 않는다. 빈 포인터면 UB이며 반환 참조는 관리 객체가
        // 살아 있는 동안만 유효하다. 첫 비멤버 operator<<(ostream&, const string&)는 cout lvalue와 그 참조를 읽고
        // 같은 ostream&를 반환해 연쇄에 쓰며, int/long 멤버 삽입은 값을 복사하고 char 비멤버 삽입은 구분자를 쓴다.
        // 마지막 ostream&는 버린다. title·숫자는 유지되고 cout의 문자 위치·상태만 바뀐다. 표준은 전체 형식 출력의
        // 별도 상한을 두지 않으며 문자열 길이·locale·버퍼/장치에 따라 비용이 들고 실패는 기본적으로 상태 비트다.
        // 기본 synchronized 표준 스트림의 동시 호출은 data race를 막지만 이 << 연쇄 전체의 원자성은 보장하지 않는다.
        std::cout << *title_handle << ' ' << snapshot.revision() << ' ' << inside_owners << '\n';
    } // snapshot이 파괴되어도 aliasing 핸들 하나가 ReportData 전체를 계속 소유한다.

    // 동일 use_count() 계약으로 남은 소유자 하나를 관찰한다. 호출 뒤 상태·소유권·수명 변화는 없다.
    const long outside_owners{title_handle.use_count()};
    // 위와 동일한 operator*/string·long·char 삽입 계약이다. alias 덕분에 이 역참조는 안전하고 마지막 반환은 버린다.
    std::cout << *title_handle << ' ' << outside_owners << '\n';

    return 0;
}
