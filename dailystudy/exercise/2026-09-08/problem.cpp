// <iostream>은 연습 결과를 출력하는 std::cout과 삽입 연산자를 선언한다.
#include <iostream>
// <mutex>는 재시도 가능한 일회 초기화 도구 std::once_flag와 std::call_once를 선언한다.
#include <mutex>
// <optional>은 Report가 아직 없음/있음을 표현하는 std::optional을 선언한다.
#include <optional>
// <utility>는 이름 있는 Report를 xvalue로 표현하는 std::move를 선언한다.
#include <utility>

// 초보자 연습 목표: 여러 번 읽어도 계산은 한 번만 하고, 그 뒤 호출은 같은 결과를 재사용하게 만든다.
// struct는 기본 접근이 public이므로 외부에 공개해도 되는 단순 결과 DTO에 적합하다.
struct Report {
    int value{}; // 기본 타입 int를 {}로 값 초기화하면 0이다.
};

// class의 기본 접근은 private이다. public 가상 함수를 통해 구현 세부와 호출자를 분리한다.
class ReportSource {
public:
    virtual ~ReportSource() = default;
    [[nodiscard]] virtual Report fetch() = 0;
};

class CountingReportSource final : public ReportSource {
public:
    [[nodiscard]] Report fetch() override {
        ++attempts_;
        // Report{73}은 prvalue다. 반환형과 같은 타입의 prvalue가 함수 결과 객체를 직접 구성하므로
        // C++17 이후 불필요한 임시 복사/이동이 필요 없다. 이름 있는 지역의 선택적 NRVO와 구분한다.
        return Report{73};
    }

    [[nodiscard]] int attempts() const noexcept { return attempts_; }

private:
    int attempts_{};
};

class LazyReportCache {
public:
    // explicit은 source가 cache로 암시 변환되는 것을 막는다. ReportSource&는 비소유 lvalue 참조이고
    // source 객체가 cache보다 오래 살아야 한다. 생성자는 반환형이 없으며 멤버 초기화 목록을 사용한다.
    explicit LazyReportCache(ReportSource& source) noexcept : source_{source} {}

    [[nodiscard]] const Report& read() const {
        // [호출 계약: std::call_once(flag_, lambda)]
        // (1) 수신 객체 없는 자유 함수이고 flag_는 미완료 또는 성공 완료 상태의 mutable std::once_flag다.
        // (2) call_once<lambda>(once_flag&, Callable&&) 인스턴스이며 추가 Args는 없고 반환형은 void다.
        // (3) 첫 인자는 flag lvalue 비소유 참조, 둘째는 this 포인터를 값 캡처한 lambda prvalue다. this 대상과
        //     source_는 호출 전체보다 오래 살며 lambda는 Report를 cache_에 소유시킨다.
        // (4) void라 반환값을 사용하지 않는다. 성공 실행 하나만 returning이고 이후 호출은 passive다.
        // (5) 예외 active 실행은 오류를 그대로 전파하고 flag를 미완료로 두며, 성공 실행은 cache 쓰기를
        //     이후 passive 반환에 동기화한다. 예외 전 외부 부수 효과는 자동 rollback되지 않는다.
        // (6) 표준 점근 복잡도·할당·lock-free·공정성·대기 상한은 없다. callable/system 오류가 가능하다.
        //     같은 flag의 수명과 source/cache의 동시 접근 규율을 지켜 데이터 경쟁과 댕글링을 배제한다.
        // 대표 문서: ../standard-library/concurrency-time-filesystem.md
        std::call_once(flag_, [this] {
            // [호출 계약: std::optional<Report>::emplace(source_.fetch())]
            // (1) 수신자는 최초에 빈 optional<Report> lvalue cache_이고 source_ 대상은 살아 있다.
            // (2) template<class... Args> Report& emplace(Args&&...)에서 Args=Report가 선택된다.
            // (3) 유일한 인자는 가상 fetch가 반환한 Report prvalue이고 contained 객체가 값을 소유한다.
            // (4) Report&를 반환하지만 쓰지 않는다.
            // (5) fetch가 던지면 emplace 전이라 빈 상태가 유지되고, 성공하면 cache_가 Report 하나를 포함한다.
            // (6) Report 한 번 구성 비용 O(1), optional 자체 별도 할당 없음이다. 구성 예외면 빈 상태이며 과거
            //     contained 참조가 있었다면 무효지만 여기에는 없다. call_once 직렬화 밖 동시 변경은 금지한다.
            // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
            cache_.emplace(source_.fetch());
        });

        // mutable 멤버 이름은 const 함수 안에서도 non-const lvalue이므로, 읽기 전용 게시 경계를
        // 실제 overload 선택에 반영하도록 const optional lvalue reference를 만든다.
        const std::optional<Report>& published_cache{cache_};

        // [호출 계약: const std::optional<Report>::value]
        // (1) 수신자는 성공 call_once 뒤 값을 가진 const published_cache lvalue다.
        // (2) const Report& value() const & 오버로드이며 데이터 인자는 없다.
        // (3) 소유권 입력은 없고 contained 값 존재 불변식이 전제다.
        // (4) Report의 const lvalue 참조를 반환해 read 호출 결과로 사용한다.
        // (5) 상태는 변하지 않고 참조는 cache 수명과 다음 emplace/reset 전까지 유효하다. 여기서는 다시 안 바뀐다.
        // (6) O(1)·무할당·비무효화이며 빈 상태면 bad_optional_access지만 성공 뒤라 배제된다. 읽기만 공유한다.
        return published_cache.value();
    }

private:
    ReportSource& source_;

    // [생성 계약: std::once_flag()]
    // (1) 목적 flag_ 수명 시작 전이며 수신·데이터 인자는 없다.
    // (2) constexpr once_flag() noexcept 기본 생성자이고 반환값이 없다.
    // (3) {}는 아직 성공 callable이 없다는 상태를 만들며 소유권을 받지 않는다.
    // (4) 별도 반환 없이 flag_ 멤버를 구성한다.
    // (5) 미완료 상태가 되고 복사/대입은 삭제되어 LazyReportCache도 복사·이동할 수 없다.
    // (6) noexcept이며 생성 자체는 동기화가 아니다. 복잡도·할당 방식은 명시되지 않고 사용 전 안전하게
    //     게시해야 하며 모든 call_once가 끝날 때까지 수명을 유지한다. 외부 참조를 무효화하지 않는다.
    mutable std::once_flag flag_{};

    // [생성 계약: std::optional<Report>()]
    // (1) 아직 없는 cache_가 목적 객체이고 Report가 템플릿 인자다.
    // (2) constexpr optional() noexcept 기본 생성자, 인자·반환값 없음이다.
    // (3) 초기 contained 소유권은 없고 {}로 빈 상태를 선택한다.
    // (4) 반환 없이 optional 멤버의 수명을 시작한다.
    // (5) emplace 전까지 비어 있고 cache 파괴 시 contained Report도 함께 파괴된다.
    // (6) O(1)·무할당·noexcept, 오류·기존 관찰자 무효화 없음이며 동시성은 call_once 계약에 맡긴다.
    mutable std::optional<Report> cache_{};
};

int main() {
    CountingReportSource source{};
    const LazyReportCache cache{source};

    // 첫 호출만 active이고 둘째 호출은 passive라 fetch를 다시 부르지 않는다.
    const Report& first{cache.read()};
    const Report& second{cache.read()};

    // first는 const lvalue이므로 Report snapshot{first}는 암시적 복사 생성자를 선택한다.
    Report snapshot{first};

    // [호출 계약: std::move(snapshot)와 Report 이동 생성]
    // (1) 자유 함수의 입력 snapshot은 살아 있는 Report lvalue이고 moved 목적 객체는 아직 없다.
    // (2) move<Report&>(Report&) noexcept가 Report&&를 반환하고 암시적 Report(Report&&)가 선택된다.
    // (3) 인자는 snapshot 하나이며 xvalue로 표현한다. Report는 int만 가져 이동도 값 복사와 같은 결과다.
    // (4) Report&& 반환을 moved 구성에 즉시 사용하며 생성자는 반환값이 없다.
    // (5) moved.value는 73이고 기본 타입만 가진 snapshot도 값 73을 유지하지만 일반 타입에는 기대하면 안 된다.
    // (6) O(1)·무할당·noexcept이고 수명·참조를 무효화하지 않는다. 공유 접근이나 오류 전제는 없다.
    Report moved{std::move(snapshot)};

    // [호출 계약: std::ostream 삽입 연쇄]
    // (1) 수신자는 정상 상태의 std::cout/std::ostream lvalue이고 정수/참조 대상은 모두 살아 있다.
    // (2) int와 bool 멤버 삽입 및 operator<<(std::ostream&, char)가 순서대로 선택된다.
    // (3) 피연산자는 attempts()의 int prvalue, moved.value int lvalue, 비교 bool prvalue와 char prvalue다.
    // (4) 매 호출이 같은 ostream&를 반환해 다음 수신자로 쓰고 마지막 반환만 버린다.
    // (5) `1 73 1\n`을 버퍼에 추가하고 source/cache/Report 상태와 참조 수명은 변하지 않는다.
    // (6) 복잡도와 비용은 자릿수·locale·버퍼/장치에 의존하고 내부 할당 가능성이 있다. 실패는 상태 비트/설정 예외로
    //     나타나며 컨테이너 무효화는 없다. 여러 스레드의 한 레코드 비혼합은 자동 보장하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cout << source.attempts() << ' ' << moved.value << ' ' << (&first == &second) << '\n';

    // 기계 실행 관점: flag와 optional 상태를 load해 비교·조건 분기하고 active 실행은 Report 값을 store한다.
    // source_.fetch()는 가상 간접 호출일 수 있다. 실제 원자 명령·잠금·분기·가상화 제거 여부는 CPU, ABI,
    // 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라지며 특정 어셈블리로 단정할 수 없다.
    return 0;
}
