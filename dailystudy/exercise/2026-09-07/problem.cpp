// <iostream>은 결과를 확인할 std::cout과 출력 연산자를 선언한다.
#include <iostream>
// <string>은 서비스 이름 문자를 소유하는 std::string을 선언한다.
#include <string>
// <tuple>은 위치 기반 레코드 std::tuple과 이를 callable 인자로 펼치는 std::apply를 선언한다.
#include <tuple>
// <utility>는 string 소유권 이전 의도를 표현하는 std::move를 선언한다.
#include <utility>

// struct는 기본 접근이 public이라 검증을 마친 뒤 자유롭게 읽는 결과 DTO에 알맞다.
// class는 기본 private이므로 변환 규칙이나 불변식을 숨기는 mapper에 알맞다는 차이도 기억한다.
struct HealthSnapshot {
    std::string service; // 클래스 타입 string이 문자 저장소를 소유한다.
    int ready{};         // int{}는 0으로 값 초기화된다.
    int total{};

    // 생성자는 반환형이 없고 멤버 초기화 목록은 본문 전 각 멤버의 수명을 바로 시작한다.
    // explicit은 tuple 열 세 개가 HealthSnapshot으로 뜻밖에 암시 변환되는 것을 막는다.
    // [호출 계약: std::move(received_service)와 std::string 이동 생성]
    // (1) 수신 객체 없는 move의 입력은 살아 있는 std::string 값 매개변수 received_service lvalue이고,
    //     service 멤버는 아직 구성 전이다.
    // (2) move<std::string&>(std::string&)가 std::string&&를 반환하고 string(string&&)이 선택된다.
    // (3) 인자는 received_service 하나다. 모든 유효한 string을 허용하며 후속 생성자에 소유권을 넘길 의도다.
    // (4) move의 반환 참조는 service 구성에 즉시 사용하고 string 생성자 자체에는 반환값이 없다.
    // (5) service가 문자를 소유하며 received_service는 유효하지만 값 미지정 상태가 된다.
    // (6) move는 O(1)·무할당·noexcept이고 기본 allocator string 이동도 상수 시간·noexcept다. 원본 문자
    //     관찰자에 계속 기대면 안 되며 수명은 늘지 않는다. 한 스레드 안에서만 접근하고 UB 전제는 없다.
    // 감사 요약: 수신 객체 없이 received_service 인자를 받고 string&&를 반환해 즉시 사용한다.
    // move는 O(1)·무할당·noexcept이고 매개변수 수명은 유지되지만 이동 뒤 값은 미지정이다.
    explicit HealthSnapshot(std::string received_service,
                            int received_ready,
                            int received_total) noexcept
        : service{std::move(received_service)}, ready{received_ready}, total{received_total} {}
};

// using 별칭은 새 타입을 정의하지 않는다. tuple의 템플릿 인자 string/int/int와 열 순서는 외부 스키마다.
using HealthRow = std::tuple<std::string, int, int>;

class HealthRowMapper {
public:
    // const HealthRow&는 호출자가 소유한 tuple lvalue에 붙는 읽기 전용 비소유 별명이다. raw가 호출 내내
    // 살아 있어야 하며 참조 자체는 수명을 늘리지 않는다. const 멤버 함수는 mapper 상태를 바꾸지 않는다.
    [[nodiscard]] HealthSnapshot map(const HealthRow& raw) const {
        // [호출 계약: std::apply(lambda, raw)]
        // (1) 수신 객체는 없다. 첫 인자는 캡처 없는 lambda prvalue, 둘째 raw는 살아 있는 const HealthRow lvalue다.
        // (2) 선택 함수는 template<class F, class Tuple> constexpr decltype(auto) apply(F&&, Tuple&&)이며
        //     F는 lambda 타입, Tuple은 const HealthRow&다. tuple_size 3에 맞춰 세 인자를 전달한다.
        // (3) lambda에는 raw 원소가 각각 const std::string&, const int&, const int& 식으로 전달된다. lambda의
        //     string 매개변수는 const reference로 빌리고 int는 값 복사한다. tuple이나 문자열 소유권은 안 옮긴다.
        // (4) lambda가 만든 HealthSnapshot prvalue가 apply의 반환값이며 map의 같은 타입 결과로 즉시 사용된다.
        // (5) raw와 원소는 그대로이고 결과 Snapshot은 service 문자열의 독립 복사본을 소유한다.
        // (6) apply 자체는 원소 수 O(3), 무할당·비무효화다. string을 HealthSnapshot 값 매개변수로 복사하는
        //     과정은 문자 수에 선형이고 bad_alloc 가능하며 예외는 apply/map 밖으로 전파된다. tuple-like 조건이나
        //     호출 가능 서명이 맞지 않으면 컴파일되지 않는다. raw 수명은 호출을 덮고 외부 동시 변경이 없어
        //     데이터 경쟁이 없으며 런타임 인덱스 범위 UB는 없다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        // 감사 요약: 수신 객체 없이 lambda와 const tuple lvalue 두 인자를 받아 HealthSnapshot을 반환한다.
        // O(3), apply 자체 무할당·비무효화이고 raw 수명은 유지되며 string 복사 생성 예외는 전파된다.
        return std::apply(
            [](const std::string& service, int ready, int total) -> HealthSnapshot {
                // service는 raw 원소에 붙은 const lvalue 참조다. HealthSnapshot의 string 값 매개변수를 만들 때
                // 복사되며, 생성자 내부 move는 그 복사본만 결과 멤버로 옮긴다. raw의 원본은 유지된다.
                return HealthSnapshot{service, ready, total};
            },
            raw);
    }
};

int main() {
    // [호출 계약: std::string(const char*)와 HealthRow 원소 생성자]
    // (1) 새 row를 구성하기 전이며 "search"는 정적 수명의 const char[7] 문자열 리터럴이다.
    // (2) basic_string(const char*, allocator)와 tuple의 세 원소 forwarding 생성자가 선택된다. tuple 인자는
    //     정확히 std::string, int, int다.
    // (3) string 입력은 non-null 첫 문자 포인터이고 null 종료까지 유효하다. tuple에는 string prvalue와
    //     int prvalue 2, 3을 주며 tuple이 string 소유권을 받는다.
    // (4) 생성자는 반환값이 없고 성공한 객체는 이름 있는 HealthRow lvalue row가 된다.
    // (5) row가 "search", 2, 3을 소유하며 리터럴·숫자는 변하지 않는다.
    // (6) string은 문자 수 O(6), 할당·length_error·bad_alloc 가능성이 있고 tuple은 원소 구성 비용이 든다.
    //     null 포인터라면 string 생성이 UB지만 리터럴이 배제한다. 새 객체라 기존 관찰자 무효화는 없고 row
    //     파괴 때 원소도 파괴된다. 생성 중 공유 접근은 없다.
    const HealthRow row{std::string{"search"}, 2, 3};
    const HealthRowMapper mapper{};
    const HealthSnapshot snapshot{mapper.map(row)};

    // bool은 기본 타입이며 비교 결과 prvalue로 값 초기화된다. if로 바꾸어 두 분기를 직접 출력해 보아도 된다.
    const bool all_ready{snapshot.ready == snapshot.total};

    // [호출 계약: std::ostream 삽입 연쇄]
    // (1) 수신자는 정상 상태 std::cout/std::ostream lvalue이고 snapshot과 그 string 멤버는 살아 있다.
    // (2) string에는 비멤버 operator<<(ostream&, const string&), int/bool에는 ostream 멤버 overload,
    //     공백과 개행에는 비멤버 operator<<(ostream&, char)가 순서대로 선택된다.
    // (3) 피연산자는 const string lvalue, char prvalue 네 개, int lvalue 두 개의 값, bool lvalue의 값이다.
    //     각 값은 읽기만 하며 소유권을 전달하지 않는다.
    // (4) 매 호출은 같은 ostream&를 반환해 다음 호출의 수신자로 사용하며 마지막 반환 참조는 버린다.
    // (5) 성공하면 `search 2 3 0\n`이 출력 버퍼에 추가되고 snapshot·row·all_ready는 그대로다.
    // (6) 비용은 생성 문자, locale, 버퍼와 장치에 의존해 표준의 단일 점근 상한이 없다. 스트림 준비 중 할당
    //     가능성이 있고 실패는 상태 비트 또는 예외 마스크가 켜졌을 때 ios_base::failure로 나타난다. 참조·수명
    //     무효화는 없으며 여러 스레드 레코드 비혼합은 보장하지 않는다. 이 예제는 단일 스레드다.
    // 감사 요약: cout 수신자와 string/char/int/bool 피연산자를 받아 ostream& 반환을 연쇄 사용한다.
    // 출력 복잡도는 버퍼에 의존하고 상태 오류·예외 가능성이 있으며 객체 수명·관찰자를 무효화하지 않는다.
    std::cout << snapshot.service << ' ' << snapshot.ready << ' ' << snapshot.total
              << ' ' << all_ready << '\n';

    // 직접 해보기:
    // 1) map의 매개변수를 HealthRow 값으로 바꾸고 map(std::move(row))을 호출해 복사/이동 경계를 그린다.
    // 2) tuple에 네 번째 bool 열을 추가해 컴파일 오류를 읽고 lambda 서명과 스키마 결합을 설명한다.
    // 3) `const std::string&` lambda 매개변수를 std::string 값으로 바꾸고 원본/결과 소유권을 비교한다.
    // 초보자 검증: 실행 전에 네 출력 필드를 예측하고, row가 왜 map 뒤에도 원래 값을 보유하는지 말한다.

    // 기계 실행 관점에서는 tuple 원소를 load하고 string 복사를 위해 문자 저장소를 읽고 쓴 뒤 int 비교와
    // 출력 분기를 수행할 수 있다. 사용자 정의 가상 함수는 없어 가상 간접 호출은 요구되지 않지만 streambuf
    // 내부는 동적 다형성을 쓸 수 있다. apply/lambda의 인라인, 복사 제거, 실제 load/store/분기/할당은 CPU,
    // ABI, 컴파일러, 표준 라이브러리와 최적화 옵션에 따라 달라 특정 어셈블리로 단정하지 않는다.

    return 0;
}
