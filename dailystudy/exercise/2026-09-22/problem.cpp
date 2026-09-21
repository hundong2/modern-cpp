#include <algorithm> // std::ranges::fold_left: 점검 범위를 하나의 보고서 값으로 축약한다.
#include <cstddef>   // std::size_t: 점검 개수처럼 음수가 아닌 크기를 표현한다.
#include <iostream>  // std::cout: 학습 예제의 보고서를 출력한다.
#include <string>    // std::string: 점검 이름과 첫 실패 이름을 소유한다.
#include <utility>   // std::move: 초기 보고서를 fold에 xvalue로 전달한다.
#include <vector>    // std::vector: 점검 입력 배치를 연속 메모리에 소유한다.

// 직접 해보기 문제: 외부 점검 목록을 받아 통과/실패 수와 첫 실패 이름을 스냅숏으로 만든다.
// struct는 기본 public이라 입력 레코드처럼 단순한 값 묶음에 어울린다.
struct HealthCheck {
    std::string name;
    bool passed{}; // bool은 참/거짓 기본 타입이고 {}는 false로 값 초기화한다.
};

// using은 새 타입이 아닌 별칭이며, std::vector의 템플릿 인자 HealthCheck가 원소 타입을 정한다.
using CheckBatch = std::vector<HealthCheck>;

// class는 기본 private이다. 보고서 갱신 규칙을 observe라는 좁은 공개 경계 뒤에 둔다.
class AuditReport {
public:
    // 기본 생성된 size_t는 0, string은 빈 소유 문자열이다. 생성자는 암시 변환을 받지 않는다.
    AuditReport() = default;

    // & 참조 한정자는 이름 있는 누산기만 제자리 갱신한다. check는 호출 동안 읽기만 빌린다.
    void observe(const HealthCheck& check) & {
        if (check.passed) { // if는 bool 값을 비교해 통과/실패 제어 흐름 중 하나로 분기한다.
            ++passed_count_;
            return;
        }

        ++failed_count_;

        // 이름이 빈 문자열이어도 첫 실패를 보존하도록 문자열 내용을 sentinel로 쓰지 않는다.
        if (failed_count_ == std::size_t{1}) {
            // [첫 호출 계약: std::string 복사 대입]
            // (1) 수신 first_failure_는 비어 있는 유효한 std::string이고 check.name은 유효한
            //     const std::string lvalue다. 두 문자열은 서로 다른 객체다.
            // (2) basic_string<char>::operator=(const basic_string&) -> basic_string&를 선택한다.
            // (3) 유일한 인자 check.name은 const lvalue로 빌린다. 문자를 깊게 복사하고 원본
            //     소유권이나 check의 수명은 가져오지 않는다.
            // (4) 갱신된 수신 string&를 반환하지만 이 대입문에서는 반환 참조를 사용하지 않는다.
            // (5) 성공하면 first_failure_가 같은 문자를 독립 소유하고 check.name은 그대로다.
            // (6) 문자 수에 선형이며 할당과 bad_alloc/length_error가 가능하다. 예외 시 수신에는
            //     다른 효과가 없는 강한 보장이다. 성공하면 재할당 여부와 무관하게 수신의 기존
            //     포인터·참조·반복자는 무효화될 수 있지만 source 관찰자는 이 호출로 무효화되지 않는다.
            first_failure_ = check.name;
        }
    }

    [[nodiscard]] std::size_t passed_count() const noexcept {
        return passed_count_;
    }

    [[nodiscard]] std::size_t failed_count() const noexcept {
        return failed_count_;
    }

    // 이 참조는 AuditReport가 소유한 문자만 빌린다. report보다 오래 보관하면 안 된다.
    [[nodiscard]] const std::string& first_failure() const noexcept {
        return first_failure_;
    }

private:
    std::size_t passed_count_{};
    std::size_t failed_count_{};
    std::string first_failure_;
};

// 누산기를 값으로 받아 이 호출이 소유한다. 바깥 보고서를 변경하지 않는 functional-core 경계다.
[[nodiscard]] AuditReport fold_check(AuditReport report, const HealthCheck& check) {
    report.observe(check);

    // [호출 계약: 암시적 AuditReport 이동 생성과 std::string 이동 생성]
    // (1) 반환 목적 AuditReport는 아직 없고 report는 유효한 이름 있는 함수 매개변수다.
    // (2) C++23 move-eligible 규칙으로 식을 xvalue 취급해 암시적 AuditReport(AuditReport&&)를
    //     선택하고, 그 안에서 basic_string(basic_string&&)와 size_t 복사가 일어난다. NRVO 대상은 아니다.
    // (3) 유일한 원본 report는 xvalue로 소비되며 그 문자열 소유권을 반환 객체로 넘길 수 있다.
    // (4) 생성자는 별도 반환값이 없고, 완성된 AuditReport가 함수의 값 반환 결과다.
    // (5) 결과가 카운트와 문자열을 소유하고 report는 유효하지만 값 미지정인 뒤 소멸한다.
    // (6) allocator 인자 없는 string 이동은 상수 시간·noexcept라 전체 암시적 이동도 그러하다.
    //     표준은 별도 할당의 부재를 명시하지 않는다. 원본 문자열 관찰자는 영향을 받을 수 있고
    //     결과 수명은 호출자가 소유하며 공유 변경의 동기화는 없다.
    return report;
}

[[nodiscard]] AuditReport summarize_checks(const CheckBatch& checks) {
    // [생성 계약: AuditReport와 std::string 기본 생성]
    // (1) initial과 그 string 멤버는 아직 없고 별도 수신 상태도 없다.
    // (2) AuditReport()와 그 안의 basic_string() 기본 생성자를 인자 없이 선택한다.
    // (3) 명시 인자·외부 저장소·소유권 이전은 없다. size_t 멤버는 기본 멤버 초기화로 0이 된다.
    // (4) 생성자는 반환값이 없고 완성된 initial 객체를 다음 fold 호출에서 사용한다.
    // (5) initial은 0/0/size 0 문자열을 소유하며 다른 객체나 관찰자를 바꾸지 않는다.
    // (6) string 기본 생성은 상수 시간이고 기본 allocator에서는 noexcept다. 표준은 별도 할당이
    //     반드시 없다고 명시하지 않는다. 실패 상태·무효화 대상은 없고 initial 수명은 함수 블록 끝까지다.
    AuditReport initial{};

    // [첫 호출 계약: std::ranges::fold_left와 std::move]
    // (1) 둘 다 사용자 데이터 수신 객체는 없다. checks는 순회 동안 바뀌지 않을 유효한
    //     const std::vector<HealthCheck>이고 initial은 0/0/빈 문자열인 유효한 AuditReport다.
    // (2) fold는 ranges 알고리즘 함수 객체의 3인자 range 호출 연산을 선택하며 추론형은
    //     R=const vector<HealthCheck>&, T=AuditReport,
    //     F=AuditReport(*)(AuditReport,const HealthCheck&)다. move는
    //     template<class T> remove_reference_t<T>&& move(T&&) noexcept에서 T=AuditReport&다.
    // (3) checks는 const lvalue 범위로 빌린다. move의 인자 initial은 AuditReport lvalue로 빌리고,
    //     그 AuditReport&& xvalue를 fold의 값 매개변수 생성에 사용해 상태를 이동할 수 있게 한다.
    //     함수 포인터 prvalue는 프로그램 수명의 fold_check 코드를 비소유로 가리키며 빈 범위도 허용된다.
    // (4) move는 initial을 가리키는 AuditReport&&를 반환해 즉시 둘째 인자로 쓰고, fold는 모든 원소를
    //     왼쪽부터 반영한 AuditReport 값을 반환해 호출자가 사용한다.
    // (5) move 자체는 initial을 바꾸지 않지만 fold의 값 인자 구성 뒤 initial은 유효한 값 미지정 상태다.
    //     checks는 그대로이고 성공한 결과는 입력과 독립된 카운트와 첫 실패 문자열을 소유한다.
    // (6) move는 O(1)·무할당·noexcept이고 참조 수명을 연장하지 않는다. fold는 reducer N회로 O(N)에
    //     실패 이름 복사 비용이 더해지며 할당 예외를 전파할 수 있다. 입력 iterator는 무효화되지 않고
    //     지역 initial은 공유하지 않지만, 다른 흐름이 checks를 동기화 없이 바꾸면 데이터 경쟁이다.
    return std::ranges::fold_left(checks, std::move(initial), &fold_check);
}

int main() {
    // [생성 계약: std::string과 std::vector initializer_list 생성]
    // (1) checks와 네 string 대상은 아직 없고 각 리터럴은 정적 수명의 NUL 종료 배열이다.
    // (2) basic_string(const char*, allocator)와
    //     vector(initializer_list<HealthCheck>, allocator) 생성자를 선택한다.
    // (3) 리터럴 문자를 각 string이 복사 소유하고 initializer_list의 const 원소들을 vector가
    //     다시 복사한다. bool prvalue는 값으로 저장되며 외부 소유권을 빌리지 않는다.
    // (4) 생성자는 반환값이 없고 완성된 const vector를 checks 이름으로 사용한다.
    // (5) 성공하면 checks가 네 레코드와 문자열을 독립 소유하고 리터럴은 변하지 않는다.
    // (6) 전체 원소/문자 수에 선형이며 할당 실패·길이 초과 예외가 가능하다. 실패 시 부분
    //     원소는 정리되고 외부 관찰자는 없다. 완성 뒤 const라 구조 변경/반복자 무효화가 없다.
    const CheckBatch checks{
        HealthCheck{std::string{"gateway"}, true},
        HealthCheck{std::string{"database"}, false},
        HealthCheck{std::string{"cache"}, false},
        HealthCheck{std::string{"checkout"}, true}};

    const AuditReport report{summarize_checks(checks)};

    // 기계 관점에서 각 점검은 bool load와 조건 분기, 카운터 store를 만들 수 있다. reducer/이동이
    // 인라인되거나 사라질지는 CPU·ABI·컴파일러·최적화 옵션에 따라 달라 특정 명령으로 단정하지 않는다.

    // [첫 호출 계약: std::cout 스트림 삽입 연산자]
    // (1) 수신 std::cout은 유효한 std::ostream이고 report와 반환 참조의 owner가 살아 있다.
    // (2) operator<<(const char*), size_t에 대응하는 부호 없는 정수용 ostream::operator<<,
    //     operator<<(ostream&, const string&) 오버로드를 피연산자 순서대로 고른다.
    // (3) 리터럴은 비소유 const char* prvalue, 카운트는 size_t prvalue, 실패 이름은 const string
    //     lvalue다. 인자는 출력 동안만 읽고 소유권을 이전하지 않는다.
    // (4) 각 삽입은 같은 std::ostream&를 반환해 체인에 사용하며 마지막 반환 참조는 버린다.
    // (5) report는 불변이고 cout의 버퍼와 상태 비트만 갱신된다. 자동 flush는 보장하지 않는다.
    // (6) 표준은 이 복합 삽입의 일반 복잡도·할당 상한을 정하지 않는다. 기본은 오류를 상태 비트로
    //     남기며 예외 마스크가 켜졌다면 예외가 가능하다. 수명/반복자 무효화는 없고 여러 실행
    //     흐름이 만든 레코드 전체의 원자성은 보장하지 않는다.
    std::cout << "passed=" << report.passed_count()
              << ",failed=" << report.failed_count()
              << ",first=" << report.first_failure();
}
