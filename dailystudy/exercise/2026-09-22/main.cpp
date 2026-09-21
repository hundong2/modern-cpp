#include <algorithm> // std::ranges::fold_left: 범위를 왼쪽부터 하나의 소유 결과 값으로 축약한다.
#include <cstddef>   // std::size_t: 원소 개수처럼 음수가 될 수 없는 크기를 표현한다.
#include <iostream>  // std::cout: 완성된 요약 스냅숏을 표준 출력에 기록한다.
#include <string>    // std::string: 원장 항목과 요약의 마지막 출처 문자열을 소유한다.
#include <utility>   // std::move: 이름 있는 누산 객체를 xvalue로 표시한다.
#include <vector>    // std::vector: 원장 항목을 연속 저장소에 소유한다.

// struct는 기본 접근이 public이다. LedgerEntry는 별도 불변식보다 값 묶음 역할이 중요하다.
struct LedgerEntry {
    std::string source; // 문자열 메모리는 각 항목이 소유한다.
    long long delta_cents{}; // 넓은 부호 정수 기본 타입이며 빈 중괄호는 0으로 값 초기화한다.
};

// using은 새 타입을 만들지 않고 긴 타입에 별칭을 붙인다. vector의 템플릿 인자는 LedgerEntry다.
using Ledger = std::vector<LedgerEntry>;

// class는 기본 접근이 private이다. 집계 결과가 항상 생성자를 거치도록 상태를 감춘다.
class BatchSummary {
public:
    // explicit은 `BatchSummary s = {500, 3, "refund"};` 같은 copy-list 암시 변환을 막는다.
    // 값 매개변수 last_source는 호출자가 복사 또는 이동해 함수 안으로 넘긴 독립 문자열이다.
    explicit BatchSummary(
        long long net_cents,
        std::size_t applied_count,
        std::string last_source)
        // [첫 호출 계약: std::move와 std::string 이동 생성]
        // (1) 수신 대상 last_source_는 아직 생성 전이고, 매개변수 last_source는 유효한
        //     std::string lvalue다. 이 생성 중 두 객체를 다른 스레드가 동시에 만지지 않는다.
        // (2) move<std::string&>(std::string&) -> std::string&&와
        //     basic_string(basic_string&&) 오버로드를 선택한다.
        // (3) 인자 last_source는 lvalue, move 결과는 xvalue다. 문자의 소유권을 멤버로
        //     이전해도 된다는 뜻이며 move 자체는 데이터를 옮기지 않는다.
        // (4) move는 std::string&&를 반환해 멤버 초기화에 사용하고, 생성자는 반환값이 없다.
        // (5) 성공하면 last_source_가 문자를 소유한다. 매개변수는 유효하지만 값 미지정이다.
        // (6) allocator 인자 없는 문자열 이동 생성은 상수 시간이고 noexcept다. 표준은 별도 할당의
        //     부재를 명시하지 않으며 원본 문자 관찰자는 영향을 받을 수 있다. 수명 종료 뒤 접근은 UB다.
        : net_cents_{net_cents},
          applied_count_{applied_count},
          last_source_{std::move(last_source)} {}

    // && 참조 한정자는 곧 소비할 임시/xvalue 요약에서만 다음 스냅숏을 만들게 한다.
    // const 참조 entry는 원장 항목을 호출 동안 읽기만 빌리며 소유하지 않는다.
    [[nodiscard]] BatchSummary include(const LedgerEntry& entry) && {
        // [생성 계약: std::string 복사 생성과 BatchSummary prvalue]
        // (1) 새 BatchSummary와 그 string 멤버는 아직 없고 entry.source는 살아 있는
        //     const std::string lvalue다. `net_cents_ + entry.delta_cents`가 long long 범위에
        //     표현 가능해야 하며, 현재 *this는 호출 뒤 더 쓰지 않을 xvalue다.
        // (2) BatchSummary(long long,size_t,std::string)의 셋째 값 매개변수에
        //     basic_string(const basic_string&) 복사 생성자를 선택한다.
        // (3) 세 인자는 long long prvalue, size_t prvalue, const string lvalue다. entry는 빌릴 뿐 소유권을
        //     넘기지 않고, 결과가 마지막 출처 문자를 깊게 복사해 독립 소유한다.
        // (4) 문자열 생성자는 반환값이 없고 BatchSummary{...} prvalue가 함수 결과로 쓰인다.
        // (5) 성공하면 새 요약만 합계·건수·마지막 출처를 소유하며 entry와 *this는 불변이다.
        // (6) 문자열 길이에 선형이고 할당 실패 예외가 날 수 있다. 실패 시 입력은 그대로다.
        //     반환 prvalue는 C++17부터 목적 객체에 직접 구성되며 공유 상태·무효화는 없다.
        return BatchSummary{
            net_cents_ + entry.delta_cents,
            applied_count_ + 1U,
            entry.source};
    }

    // const 멤버 함수는 관찰만 한다. 기본 타입은 값으로 반환하므로 빌린 수명이 없다.
    [[nodiscard]] long long net_cents() const noexcept {
        return net_cents_;
    }

    [[nodiscard]] std::size_t applied_count() const noexcept {
        return applied_count_;
    }

    // 반환 참조는 *this가 소유한 문자열을 빌린다. 요약 객체보다 오래 저장하면 댕글링된다.
    [[nodiscard]] const std::string& last_source() const noexcept {
        return last_source_;
    }

private:
    long long net_cents_{};
    std::size_t applied_count_{};
    std::string last_source_;
};

// fold가 호출할 reducer다. 누산기는 값으로 받아 이번 단계가 소유하고, 항목은 읽기만 빌린다.
[[nodiscard]] BatchSummary fold_entry(BatchSummary summary, const LedgerEntry& entry) {
    // [호출 계약: std::move와 && 한정 include]
    // (1) 수신 객체 summary는 이 함수가 소유하는 유효한 BatchSummary lvalue이고 entry도 유효하다.
    // (2) move<BatchSummary&>(BatchSummary&) -> BatchSummary&& 뒤 include(const LedgerEntry&) &&를 택한다.
    // (3) move의 유일한 인자는 summary lvalue이고 결과 xvalue는 소유권 소비 의도를 표시한다.
    //     include 인자 entry는 const lvalue 참조로 빌린다. 합이 long long에 표현 가능해야 하며
    //     fold 안의 처리 횟수는 Ledger의 size_type 범위를 넘지 않는다.
    // (4) move 반환 참조는 즉시 수신 식으로만 쓰고, include가 BatchSummary prvalue를 반환한다.
    // (5) 새 결과가 갱신 상태를 소유한다. summary는 곧 소멸하며 이후 관찰하지 않는다.
    // (6) move 자체는 상수 시간·무할당·noexcept다. include는 문자열 복사에서 선형 시간과
    //     할당 예외가 가능하다. 외부 별칭/반복자 무효화는 없고 동시 호출은 입력이 읽기 전용이면 안전하다.
    return std::move(summary).include(entry);
}

// const 참조 매개변수는 호출 동안 vector를 빌린다. 반환값은 입력과 독립된 소유 스냅숏이다.
[[nodiscard]] BatchSummary summarize(const Ledger& entries) {
    // [생성 계약: BatchSummary identity와 std::string 기본 생성]
    // (1) initial과 그 string 멤버는 아직 없고 별도 수신 상태도 없다.
    // (2) basic_string() 기본 생성 뒤 BatchSummary(long long,size_t,string)를 선택한다.
    // (3) long long{0}과 size_t{0} prvalue를 값으로 받고 string 기본 생성에는 명시 인자·외부
    //     소유권이 없다.
    // (4) 두 생성자는 별도 반환값이 없고 완성된 initial 객체를 다음 fold 호출에서 사용한다.
    // (5) initial은 합계 0, 건수 0, size 0 문자열을 독립 소유한다.
    // (6) string 기본 생성은 상수 시간이고 기본 allocator에서는 noexcept다. 표준은 별도 할당이
    //     반드시 없다고 명시하지 않는다. 무효화 대상은 없고 initial 수명은 함수 블록 끝까지다.
    BatchSummary initial{0LL, std::size_t{0}, std::string{}};

    // [첫 호출 계약: std::ranges::fold_left]
    // (1) ranges 알고리즘 함수 객체 호출이라 사용자 데이터 수신 객체는 없다. entries는 유효한
    //     const std::vector<LedgerEntry>이며 순회 중 구조/원소가 바뀌지 않는다. 모든 누적 합은
    //     long long에 표현 가능해야 하고 initial은 유효한 0/0/빈 문자열 상태다.
    // (2) ranges 알고리즘 함수 객체의 3인자 range 호출 연산을 선택한다. 대응 형식의 추론형은
    //     R=const vector<LedgerEntry>&, T=BatchSummary,
    //     F=BatchSummary(*)(BatchSummary,const LedgerEntry&)다.
    // (3) 첫 인자 entries는 const lvalue 범위라 빌린다. 둘째 std::move(initial)은 xvalue라 fold의
    //     값 매개변수로 상태를 이동 구성하고, 셋째 함수 포인터 prvalue는 프로그램 수명의 코드를
    //     비소유로 가리킨다. 빈 범위도 허용된다.
    // (4) 반환형은 BatchSummary다. 왼쪽부터 모든 원소를 반영한 값을 호출자가 소유해 사용한다.
    // (5) entries는 변하지 않고 initial은 유효하지만 값 미지정이 된다. 성공한 반환 객체는 최종
    //     합계·건수·마지막 출처를 독립 소유한다.
    // (6) N개 원소에 reducer N회로 O(N)이고 문자열 복사 총비용이 더해진다. 표준 의미상 초기값과
    //     매 단계 누산기는 xvalue로 reducer에 전달된다. 복사/할당 예외는 전파되고 입력 반복자는
    //     무효화되지 않으며 자체 동기화는 없다.
    return std::ranges::fold_left(entries, std::move(initial), &fold_entry);
}

// main은 운영체제에 int 종료 상태를 돌려주며 매개변수 없이 작은 실무 예제를 조립한다.
int main() {
    // [생성 계약: std::string과 std::vector initializer_list 생성]
    // (1) entries와 세 LedgerEntry/string 대상은 아직 없다. 문자열 리터럴은 정적 수명의
    //     NUL 종료 const char 배열이며 생성 동안 다른 실행 흐름과 공유 변경하지 않는다.
    // (2) 각 basic_string(const char*, const Allocator& = {})와
    //     vector(initializer_list<LedgerEntry>, const Allocator& = {})를 선택한다.
    // (3) 문자열 인자는 배열-포인터 변환으로 얻은 const char* prvalue이고 NUL 전 문자를 복사한다.
    //     vector 인자는 세 const LedgerEntry의 initializer_list를 빌리므로 원소를 복사 소유한다.
    // (4) 생성자는 반환값이 없다. 완성된 const vector는 entries라는 이름으로 사용된다.
    // (5) 성공하면 entries가 세 항목과 모든 문자열을 독립 소유하고 리터럴은 그대로다.
    // (6) 원소/문자 수에 선형이며 저장소 할당에서 bad_alloc/length_error가 가능하다. 실패하면
    //     완성된 entries가 없고 구성된 부분은 정리된다. 재할당 전 관찰자가 없어 무효화 문제도 없다.
    const Ledger entries{
        LedgerEntry{std::string{"gateway"}, 1'200LL},
        LedgerEntry{std::string{"database"}, -300LL},
        LedgerEntry{std::string{"refund"}, -400LL}};

    const BatchSummary summary{summarize(entries)};

    // 기계 관점에서 fold는 반복 끝 비교, 원소 load, 누산기 load/store와 reducer 호출이 될 수 있다.
    // 구체 함수 포인터는 인라인될 수도 있으며 실제 명령은 CPU·ABI·컴파일러·최적화에 따라 달라진다.

    // [첫 호출 계약: std::cout 스트림 삽입 연산자]
    // (1) 수신 std::cout은 오류가 없는 std::ostream 상태이고 summary와 그 문자열이 살아 있다.
    // (2) operator<<(const char*), long long용 ostream::operator<<와 size_t의 구현별 기반 부호 없는
    //     정수 타입에 대응하는 ostream::operator<<, operator<<(ostream&, const string&)를 선택한다.
    // (3) 리터럴의 배열-포인터 변환 결과는 비소유 const char* prvalue, 두 수는 값,
    //     last_source()는 const string lvalue다. 소유권을 넘기지 않고 문자열 수명이 호출을 덮는다.
    // (4) 각 호출은 같은 std::ostream&를 반환해 다음 삽입에 사용하고 최종 반환 참조는 버린다.
    // (5) summary는 그대로이고 cout 버퍼/상태가 갱신된다. flush 여부는 이 줄만으로 보장하지 않는다.
    // (6) 표준은 이 복합 삽입의 일반 복잡도·할당 상한을 정하지 않는다. I/O 실패는 기본 설정에서
    //     상태 비트로 남고 예외 마스크가 켜졌다면 예외가 가능하다. 외부 버퍼 관찰자 무효화 규칙은
    //     없으며 여러 실행 흐름이 만든 레코드 전체의 원자성은 보장하지 않는다.
    std::cout << "net=" << summary.net_cents()
              << ",count=" << summary.applied_count()
              << ",last=" << summary.last_source();
}
