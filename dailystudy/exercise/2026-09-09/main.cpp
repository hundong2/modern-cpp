// <cstddef>는 컨테이너의 원소 수와 페이지 offset을 표현할 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 출력 객체 std::cout, std::ostream과 정수·문자 삽입 연산을 선언한다.
#include <iostream>
// <ranges>는 두 반복자 사이를 비소유 범위로 묶는 C++20 std::ranges::subrange를 선언한다.
#include <ranges>
// <utility>는 이름 있는 객체를 xvalue로 바꾸어 이동 생성을 선택하게 하는 std::move를 선언한다.
#include <utility>
// <vector>는 Record를 연속 저장소에 소유하는 std::vector와 그 const_iterator를 선언한다.
#include <vector>

// struct는 기본 접근이 public이다. 검증을 마친 단순 데이터 전달 객체(DTO)는 이름 있는 필드를
// 직접 읽는 편이 자연스럽다. int는 기본 정수 타입이고 {}는 빠진 값을 0으로 값 초기화한다.
struct Record {
    int id{};
    int amount{};
};

// 계산 결과도 불변식을 숨길 필요가 없는 DTO라 struct로 둔다.
struct PageSummary {
    std::size_t count{};
    int total{};
};

// class는 기본 접근이 private이다. 저장소 소유권과 "항상 범위 안인 page" 규칙은 구현에 숨기고,
// 호출자에게는 읽기 전용 비소유 Page만 공개한다.
class RecordBook {
public:
    // using은 새 클래스를 만드는 것이 아니라 긴 구체 타입에 읽기 쉬운 별칭을 붙인다.
    // Storage의 템플릿 인자 Record는 vector가 소유하고 파괴할 원소 타입이다.
    using Storage = std::vector<Record>;
    using ConstIterator = Storage::const_iterator;
    using Difference = Storage::difference_type;
    // subrange의 I와 S 템플릿 인자는 모두 ConstIterator다. 같은 random-access 반복자의 차이로
    // 거리를 구할 수 있으므로 기본 kind는 sized이고, Page는 Record 자체를 소유하지 않는다.
    using Page = std::ranges::subrange<ConstIterator>;

    // 생성자는 반환형이 없다. explicit은 Storage 하나가 RecordBook으로 뜻밖에 암시 변환되는 것을 막는다.
    // 값 매개변수 records는 lvalue 호출자에게는 독립 복사를, xvalue 호출자에게는 이동 경계를 제공한다.
    // 멤버 초기화 목록은 생성자 본문 전에 records_를 곧바로 구성한다.
    // [호출 계약: std::move(records)와 Storage 이동 생성]
    // (1) move에는 수신 객체가 없고 records는 생성이 끝난 size>=0의 Storage 값 매개변수 lvalue이며,
    //     목적 records_는 아직 수명이 시작되지 않았다.
    // (2) 선택 함수는 template<class T> remove_reference_t<T>&& move(T&&) noexcept에서 T=Storage&이고,
    //     이어 Storage의 vector(vector&&) 이동 생성자가 선택된다.
    // (3) 유일한 인자 식 records는 Storage lvalue다. 모든 유효한 vector를 허용하고, move 자체는 빌린
    //     객체를 소유하지 않으며 후속 생성자가 연속 저장소 소유권을 넘겨받도록 값 범주만 xvalue로 바꾼다.
    // (4) move는 같은 records를 가리키는 Storage&&를 반환해 records_ 구성에 즉시 사용한다. 생성자에는
    //     반환값이 없고 성공하면 완성된 RecordBook 객체가 생긴다.
    // (5) records_가 원소를 소유하고 records는 살아 있지만 유효한 값 미지정 상태가 된다. 기존 원소
    //     관찰자가 있었다면 이동된 원소를 계속 가리키지만 이제 그 소유자는 records_다.
    // (6) move와 기본 allocator vector 이동은 O(1), 무할당·noexcept다. 수명을 연장하지 않고 원소를
    //     복사하지 않는다. 유효한 records라는 전제 외 UB 조건은 없으며 동시 접근 동기화는 제공하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    // 대표 문서: ../standard-library/containers-and-views.md
    explicit RecordBook(Storage records) noexcept
        : records_{std::move(records)} {}

    // [호출 계약: const Storage::size()]
    // (1) 수신자는 수명이 유효한 const Storage lvalue records_이며 완성된 Record 원소들을 소유한다.
    // (2) 선택 overload는 Storage::size_type size() const noexcept이고 템플릿 원소 타입은 Record다.
    // (3) 데이터 인자는 없다. 숨은 this가 const라 원소를 수정하거나 소유권을 이전하지 않는다.
    // (4) 반환형 Storage::size_type(기본 allocator에서는 std::size_t)의 prvalue는 호출자에게 그대로 반환된다.
    // (5) 호출 뒤 크기·용량·원소·반복자 상태는 모두 그대로다.
    // (6) O(1), 무할당·비무효화·noexcept다. 살아 있는 vector여야 하며 동시 쓰기가 있다면 외부 동기화가
    //     필요하지만, 이 예제는 생성이 끝난 객체를 한 실행 흐름에서 읽기만 한다.
    // 대표 문서: ../standard-library/containers-and-views.md
    [[nodiscard]] std::size_t size() const noexcept {
        return records_.size();
    }

    // offset과 requested_count는 기본 타입 값 매개변수라 호출 때 복사된다. const 멤버 함수이므로 저장소를
    // 바꾸지 않는다. offset이 끝을 넘으면 빈 끝 범위, count가 남은 원소보다 크면 끝까지만 반환한다.
    [[nodiscard]] Page page(std::size_t offset, std::size_t requested_count) const {
        const std::size_t current_size{records_.size()};
        const std::size_t bounded_offset{offset < current_size ? offset : current_size};
        const std::size_t remaining{current_size - bounded_offset};
        const std::size_t bounded_count{
            requested_count < remaining ? requested_count : remaining};

        // [호출 계약: Storage::cbegin()과 const_iterator::operator+]
        // (1) 수신자 records_는 current_size개 원소를 소유한 const Storage lvalue이고 아직 구조 변경이 없다.
        // (2) cbegin() const noexcept -> ConstIterator와 random-access iterator의
        //     operator+(Difference) const -> ConstIterator가 차례로 선택된다.
        // (3) cbegin에는 데이터 인자가 없다. +의 인자는 [0,current_size]인 bounded_offset을 Difference로
        //     바꾼 prvalue이며, 저장소를 빌려 위치만 계산하고 원소 소유권은 옮기지 않는다.
        // (4) cbegin 반환 iterator는 +의 수신 값으로 쓰이고, +가 반환한 iterator prvalue로 first를 구성한다.
        // (5) records_와 원소는 그대로이고 first는 첫 페이지 원소 또는 past-the-end를 가리킨다.
        // (6) 두 연산은 O(1), 무할당·비무효화이며 이 반복자 구현의 단순 복사/덧셈은 예외를 내지 않는다.
        //     같은 배열의 [begin,end] 안에서만 더해야 한다는 전제를 clamp가 보장한다. records_의 재할당·
        //     관련 원소 삭제·파괴 뒤에는 iterator가 무효이며, 동시 변경과 함께 사용하면 데이터 경쟁이다.
        // 대표 문서: ../standard-library/containers-and-views.md
        const ConstIterator first{
            records_.cbegin() + static_cast<Difference>(bounded_offset)};
        // 같은 random-access + 계약으로 first에서 남은 범위 안의 bounded_count만큼 전진한다.
        const ConstIterator last{first + static_cast<Difference>(bounded_count)};

        // [생성 계약: std::ranges::subrange<ConstIterator>(first, last)]
        // (1) 아직 결과 Page 객체는 없고 first와 last는 같은 records_ 저장소의 유효한 경계 반복자다.
        // (2) I=S=ConstIterator인 sized subrange의 대표 생성자 subrange(I i, S s)가 선택되어 두 반복자를
        //     값으로 보관한다. 세 번째 kind 템플릿 인자는 반복자 차이를 지원하므로 sized다.
        // (3) first와 last는 ConstIterator const lvalue로 복사되며 first<=last이고 둘 다 [cbegin,cend] 안이다.
        //     반복자는 RecordBook을 빌릴 뿐 원소나 저장소의 소유권을 얻지 않는다.
        // (4) 생성자 자체 반환값은 없다. Page prvalue가 page()의 같은 타입 결과 객체에 직접 구성되어 사용된다.
        // (5) records_는 바뀌지 않고 결과는 정확히 bounded_count개 Record를 읽는 반열린 [first,last) 범위다.
        // (6) O(1), 무할당·비무효화다. 생성 시점부터 [first,last)가 유효한 범위, 즉 last가 first에서
        //     도달 가능한 같은 저장소 경계여야 한다. 위반은 생성자 전제조건 위반이며 미정의 동작이고,
        //     여기서는 clamp와 같은 vector가 보장한다. 원본 파괴나 재할당 후 Page는 댕글링하며 수명을
        //     연장하지 않는다. 예외·스레드 동기화를 추가하지 않는다. 같은 타입 prvalue 반환은 C++17부터
        //     보장 복사 생략되어 별도 subrange 복사/이동이 없다(이름 있는 지역의 선택적 NRVO와 구별한다).
        // 대표 문서: ../standard-library/algorithms-and-ranges.md
        return Page{first, last};
    }

private:
    Storage records_; // 이 class가 연속 저장소와 네 Record의 수명을 독점 관리한다.
};

// 같은 타입의 이름 없는 prvalue를 반환하므로 결과 RecordBook에 직접 구성된다. 흔히 RVO라고 부르지만,
// 이름 있는 지역에 선택적으로 적용되는 NRVO가 아니라 C++17 이후의 보장된 prvalue 복사 생략이다.
[[nodiscard]] RecordBook make_record_book() {
    // [생성 계약: Storage initializer_list 생성과 RecordBook 직접 반환]
    // (1) Storage와 반환 RecordBook 목적 객체는 아직 없고 네 Record 값은 완전한 aggregate prvalue다.
    // (2) Storage는 vector(initializer_list<Record>, allocator 기본값) overload를 선택하고, 그 Storage
    //     prvalue는 explicit RecordBook(Storage)의 값 매개변수를 직접 구성한다.
    // (3) initializer_list에는 유효한 Record 네 개 {{1,10},{2,20},{3,30},{4,40}}가 const 원소로 노출되어
    //     vector가 각각 복사한다. 목록 저장소는 full-expression까지만 빌리고 최종 vector가 원소를 소유한다.
    // (4) 두 생성자 자체 반환값은 없다. 완성된 RecordBook prvalue는 함수 결과 객체로 즉시 사용된다.
    // (5) 결과 RecordBook의 records_가 순서대로 네 값을 독립 소유한다. 임시 목록과 값 매개변수는 이후
    //     파괴되고, 외부 원소 관찰자는 아직 없다.
    // (6) vector 구성은 시간·공간 O(4)이고 저장소 할당이나 원소 복사 실패 예외가 가능하다. Record의 int
    //     복사는 예외를 내지 않는다. 길이는 표현 가능하고 새 객체라 무효화 대상이 없으며 공유 접근도 없다.
    // 대표 문서: ../standard-library/containers-and-views.md
    return RecordBook{RecordBook::Storage{{1, 10}, {2, 20}, {3, 30}, {4, 40}}};
}

[[nodiscard]] PageSummary summarize(const RecordBook::Page& page) {
    // [호출 계약: const sized subrange::size()]
    // (1) 수신자는 원본 RecordBook의 유효한 반복자 쌍을 보관한 const RecordBook::Page lvalue다.
    // (2) 선택 멤버는 sized subrange의 size() const이며 데이터 인자 없이 unsigned 반복자 차이를 반환한다.
    // (3) 데이터 인자와 소유권 이동은 없다. 두 경계가 여전히 같은 살아 있는 vector를 가리켜야 한다.
    // (4) 반환된 크기 prvalue를 std::size_t count 구성에 사용한다.
    // (5) Page, 반복자, 원본 원소의 상태는 모두 그대로다.
    // (6) random-access 경계의 차이이므로 O(1), 무할당·비무효화다. 댕글링/서로 다른 범위 반복자의 차이는
    //     미정의 동작이지만 호출 중 원본이 살아 있고 바뀌지 않는다. 예외·스레드 동기화를 추가하지 않는다.
    // 대표 문서: ../standard-library/algorithms-and-ranges.md
    const std::size_t count{page.size()};
    int total{};

    // [호출 계약: subrange begin/end와 const_iterator 순회 연산]
    // (1) range-for의 숨은 수신자는 유효한 const Page lvalue page이고 두 ConstIterator는 같은 원본을 본다.
    // (2) begin() const/end() const가 경계 iterator 값을 반환하고, ConstIterator의 !=, 전위 ++, 단항 *
    //     연산이 각각 비교·전진·const Record& 역참조를 수행한다.
    // (3) begin/end에는 데이터 인자가 없다. !=는 두 iterator를 빌리고, ++는 지역 begin iterator를
    //     수정하며, *는 end가 아닌 iterator만 허용한다. Record 소유권은 이동하지 않는다.
    // (4) begin/end 반환값은 숨은 지역 반복자에 저장되고 != bool은 반복 분기에, *의 const Record&는
    //     record 참조 바인딩에 사용된다. ++는 같은 iterator&를 반환하지만 range-for가 별도 저장하지 않는다.
    // (5) 매 반복에서 지역 iterator만 다음 위치로 바뀌고 page와 Record는 그대로이며 total만 amount만큼 는다.
    // (6) 각 iterator 연산은 O(1), 전체는 O(count), 무할당·비무효화다. 원본 수명과 iterator 유효성,
    //     end 비역참조가 전제다. 읽는 동안 다른 실행 흐름이 vector/원소를 쓰면 데이터 경쟁이므로 금지한다.
    // 대표 문서: ../standard-library/algorithms-and-ranges.md
    for (const Record& record : page) {
        total += record.amount;
    }

    // count와 total은 기본 타입 값을 복사해 DTO prvalue를 만들며 같은 타입 함수 결과에 직접 구성된다.
    return PageSummary{count, total};
}

int main() {
    // make_record_book() 호출은 RecordBook prvalue다. const original 결과 객체에 직접 구성되며 중간
    // RecordBook 복사/이동이 없다. 이름이 생긴 original 식은 이후 lvalue이고 전체 저장소를 소유한다.
    const RecordBook original{make_record_book()};

    // original은 const lvalue, 1과 2는 int prvalue가 std::size_t 값 매개변수로 변환된다. 반환 Page는
    // original을 빌리는 prvalue이며 page 결과 객체에 직접 구성된다. original이 더 오래 살아야 한다.
    const RecordBook::Page page{original.page(1, 2)};

    // [생성 계약: RecordBook 복사 생성과 Storage 복사 생성]
    // (1) 목적 independent_copy는 아직 없고 original은 네 Record를 소유한 살아 있는 const lvalue다.
    // (2) 컴파일러가 만든 RecordBook(const RecordBook&)이 선택되고, 멤버에는
    //     Storage::vector(const Storage&) 복사 생성자가 선택된다.
    // (3) 인자 original은 const RecordBook lvalue로 생성 중 빌리며 소유권을 넘기지 않는다. 모든 Record는
    //     int 두 개를 가진 복사 가능한 값이고 original과 그 page는 호출 내내 유효하다.
    // (4) 생성자 반환값은 없다. 성공한 independent_copy는 별도 vector 저장소를 소유한다.
    // (5) original과 page는 그대로이고 copy는 같은 네 값의 독립 원소를 갖는다. 어느 한 vector의 후속
    //     재할당도 다른 vector의 반복자에는 영향을 주지 않는다.
    // (6) 시간·추가 공간 O(4), 새 저장소 할당 실패 예외가 가능하며 실패 시 목적 객체는 완성되지 않는다.
    //     원본 관찰자는 무효화되지 않고 수명도 늘지 않는다. 복사 중 original을 동시에 쓰면 안 된다.
    // 대표 문서: ../standard-library/containers-and-views.md
    RecordBook independent_copy{original};

    // [호출 계약: std::move(independent_copy), RecordBook/Storage 이동 생성]
    // (1) move에는 수신 객체가 없고 independent_copy는 네 Record를 독립 소유한 유효한 lvalue이며,
    //     목적 moved_owner는 아직 없다.
    // (2) move<RecordBook&>(RecordBook&) -> RecordBook&&와 컴파일러가 만든
    //     RecordBook(RecordBook&&), 그 안의 Storage::vector(Storage&&)가 선택된다.
    // (3) 유일한 인자 식 independent_copy는 lvalue다. xvalue 변환 뒤 그 복사본 저장소의 소유권만
    //     moved_owner로 넘기며 original의 저장소나 page는 전혀 건드리지 않는다.
    // (4) move의 RecordBook&& 반환은 이동 생성에 즉시 사용하고 생성자 반환값은 없다.
    // (5) moved_owner가 네 원소를 소유한다. independent_copy는 살아 있지만 값 미지정이므로 size를
    //     가정하지 않는다. original의 Page는 계속 유효하다.
    // (6) move와 기본 allocator vector 이동은 O(1), 무할당·noexcept이며 원본 Page 관찰자를 무효화하지
    //     않는다. moved-from 객체는 파괴/대입 가능한 상태다. 공유 객체가 없어 데이터 경쟁도 없다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    // 대표 문서: ../standard-library/containers-and-views.md
    RecordBook moved_owner{std::move(independent_copy)};

    // const lvalue Page를 빌려 순회한다. 원본 RecordBook은 여전히 살아 있으므로 반환 참조·반복자가 유효하다.
    const PageSummary summary{summarize(page)};

    // [호출 계약: std::ostream 정수/문자 삽입 연쇄]
    // (1) 최초 수신자는 출력 가능한 std::cout/std::ostream lvalue이고 summary와 moved_owner는 살아 있다.
    // (2) count와 moved_owner.size()에는 size_t의 실제 기반 unsigned 정수 멤버 overload, total에는 int
    //     멤버 overload, 공백과 개행에는 operator<<(std::ostream&, char) 비멤버 overload가 선택된다.
    // (3) 피연산자는 summary.count const lvalue의 값, char prvalue 세 개, summary.total const int 값,
    //     moved_owner.size()의 std::size_t prvalue다. 모두 읽기 입력이고 소유권을 넘기지 않는다.
    // (4) 각 삽입은 같은 std::ostream&를 반환해 다음 삽입의 수신자로 쓰며 마지막 반환만 버린다.
    // (5) 성공하면 `2 50 4\n` 문자가 순서대로 출력 버퍼에 추가되고 입력 객체는 바뀌지 않는다.
    // (6) 전체 복잡도와 점근 상한은 locale·숫자 변환·버퍼·장치에 의존한다. 버퍼 준비 중 할당, 실패 상태 비트,
    //     설정된 예외 마스크의 예외가 가능하다. 참조 수명은 호출을 덮고 무효화는 없다. 여러 실행 흐름의
    //     레코드 단위 비혼합은 보장하지 않지만 이 프로그램은 단일 실행 흐름에서만 cout을 쓴다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cout << summary.count << ' ' << summary.total << ' ' << moved_owner.size() << '\n';

    // 기계 실행 관점에서는 vector가 보관한 크기/주소 load, offset 비교와 조건 분기, iterator 주소 덧셈,
    // amount load와 total store, 스트림 버퍼를 향한 간접 호출이 생길 수 있다. subrange는 보통 반복자 두
    // 개뿐이지만 표현을 고정하지 않는다. 실제 load/store/분기 제거·복사 생략·인라인·할당·가상 간접 호출은
    // CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라 특정 어셈블리로 단정할 수 없다.

    return 0;
}
