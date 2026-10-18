// 직접 해보기: HTTP 상태 코드 중 재시도 가능한 값을 owner가 보관하고, 읽기 전용 함수가
// std::ranges::contains로 정책을 질의한다. 호출자가 컨테이너의 반복자 수명에 의존하지 않는다.

// <algorithm>은 C++23 std::ranges::contains를 선언한다.
#include <algorithm>
// <iostream>은 std::cout과 출력 연산자를 선언한다.
#include <iostream>
// <utility>는 std::move를 선언한다.
#include <utility>
// <vector>는 원소를 연속 저장하고 소유하는 std::vector를 선언한다.
#include <vector>

using StatusCode = int; // 별칭은 새 타입이 아니라 int에 업무 의미를 붙인다.

// class의 기본 접근은 private이고 struct의 기본 접근은 public이다. 둘 다 데이터와 함수를 가질 수
// 있지만, 여기서는 상태 코드 저장소를 숨기고 읽기 함수만 public으로 공개하려고 class를 선택한다.
class RetryPolicy {
public:
    // explicit은 vector에서 RetryPolicy로의 의도치 않은 암시 변환을 차단한다.
    // 값 매개변수는 호출자가 lvalue를 주면 복사, rvalue를 주면 이동할 수 있고, 아래 멤버 초기화
    // 목록은 생성자 본문보다 먼저 retryable_을 만든다. 실제 초기화 순서는 멤버 선언 순서다.
    explicit RetryPolicy(std::vector<StatusCode> retryable)
        // [첫 호출 계약: std::move + std::vector<StatusCode> 이동 생성]
        // (1) move는 자유 함수라 수신 객체가 없고 retryable은 저장소를 소유한 vector lvalue다.
        // (2) std::move<vector<StatusCode>&>가 vector&&를 반환하고 vector(vector&&)가 선택된다.
        // (3) 이름 있는 값 매개변수 lvalue를 xvalue로 표시해 멤버로 버퍼 소유권을 넘긴다.
        // (4) move 결과는 retryable_ 구성에 사용하며 생성자 자체 반환값은 없다.
        // (5) 성공 뒤 멤버가 원소를 소유하고 매개변수는 유효하지만 내용 미지정 상태다.
        // (6) 기본 allocator의 vector 이동 생성은 표준상 O(1), noexcept이고 원소 저장소를 멤버에
        //     인계한다. 표준 계약에 별도 내부 할당 횟수까지 덧붙여 단정하지 않는다. 멤버 파괴 때
        //     저장소를 해제하며 같은 객체를 동시에 쓰는 동작은 외부에서 동기화해야 한다.
        : retryable_{std::move(retryable)} {}

    // bool은 반환형, StatusCode는 복사되는 작은 값 매개변수다. 뒤 const는 이 함수가 멤버 상태를
    // 바꾸지 않는다는 뜻이다. pointer나 참조를 밖으로 반환하지 않아 내부 저장소 수명을 노출하지 않는다.
    [[nodiscard]] bool should_retry(StatusCode code) const {
        // [첫 호출 계약: std::ranges::contains]
        // (1) 수신 없는 자유 range 알고리즘이고 retryable_은 살아 있는 const vector<int> lvalue다.
        // (2) ranges::contains(R&&, const T&, Proj={})에서 R=const vector<int>&,
        //     T=int, Proj=identity가 선택되며 bool을 반환한다.
        // (3) range와 code lvalue를 읽기만 하고 기본 projection 인자는 생략한다. 소유권 이전은 없다.
        // (4) 같은 상태 코드가 있으면 true를 반환하며 그 prvalue를 호출자에게 바로 전달한다.
        // (5) 컨테이너·code·원소와 관찰자는 바뀌지 않는다.
        // (6) 유효 범위와 == 비교 가능성이 전제다. 최악 O(n) 비교, 무할당이며 int 비교는
        //     비투척이다. owner가 살아 있는 동안만 내부 순회가 유효하고 동시 수정은 데이터 경쟁이다.
        return std::ranges::contains(retryable_, code);
    }

private:
    // template 인자 StatusCode는 원소 타입이다. vector가 원소를 소유하며 policy와 함께 파괴한다.
    std::vector<StatusCode> retryable_;
};

int main() {
    // [첫 생성 계약: std::vector<StatusCode>(initializer_list)]
    // (1) 목적 vector는 아직 없고 세 int backing-array 원소가 유효하다.
    // (2) initializer_list 생성자에서 T=StatusCode, Allocator=std::allocator<int>다.
    // (3) {408,429,503} 값을 복사하고 backing array를 빌려 저장하지 않는다.
    // (4) 생성자는 반환값 없이 소유 vector prvalue를 만들고 RetryPolicy 값 매개변수를 초기화한다.
    // (5) policy 멤버는 같은 세 값을 순서대로 소유하며 초기화 목록과 독립 수명이다.
    // (6) O(3) 시간/공간이며 저장소 확보 중 length_error/bad_alloc 가능하다. 표준은 내부 할당
    //     횟수를 고정하지 않는다. 성공 뒤 관찰자는 재할당/파괴 전까지 유효하다. 구성은 한 thread에서
    //     하며 완성 뒤 같은 vector를 수정하면서 동시에 읽으려면 외부 동기화가 필요하다.
    RetryPolicy policy{std::vector<StatusCode>{408, 429, 503}};

    // 위 vector 식은 prvalue, 이름 있는 policy는 lvalue다. 생성자 안의 std::move 결과는 xvalue이며
    // 실제 버퍼 이전은 vector 이동 생성자가 수행한다. 오늘은 lvalue vector 복사가 없고, 함수가 같은
    // 타입의 class 값을 반환하지 않으므로 RVO/NRVO가 관찰되는 식도 없다. factory를 값 반환으로
    // 추가한다면 임시 prvalue 직접 반환은 보장 복사 생략, 이름 있는 local 반환은 선택적 NRVO다.

    const bool timeout{policy.should_retry(408)};
    const bool bad_request{policy.should_retry(400)};
    const bool unavailable{policy.should_retry(503)};

    // [첫 호출 계약: std::cout operator<< 연쇄]
    // (1) 수신은 유효한 std::ostream lvalue std::cout이다.
    // (2) bool 멤버 삽입과 char 비멤버 삽입 overload가 차례로 선택되어 ostream&를 반환한다.
    // (3) bool lvalue 세 개와 char prvalue를 읽고 소유권을 옮기지 않는다.
    // (4) 반환 참조는 다음 삽입에 쓰고 마지막 것은 버린다.
    // (5) 기본 숫자 bool 형식으로 "1 0 1\n"을 기록하고 policy는 바뀌지 않는다.
    // (6) 표준은 이 형식 출력 연쇄 전체에 하나의 이식 가능한 점근 복잡도를 정하지 않으며 locale,
    //     버퍼 할당과 장치 I/O 비용은 구현별이다. I/O 실패는 상태 비트 또는 설정 시 예외로
    //     보고된다. 무효화는 없고 여러 thread의 한 레코드 원자성을 보장하지 않는다.
    std::cout << timeout << ' ' << bad_request << ' ' << unavailable << '\n';

    // 기계 관점에서 검색은 연속 int load, 비교와 조건 분기로 구현될 수 있다. virtual 함수가 없어
    // 가상 간접 호출은 요구되지 않는다. 실제 명령, inline 여부, 분기 제거·벡터화는 CPU, ABI,
    // 컴파일러, 표준 라이브러리와 최적화 옵션에 따라 달라 특정 어셈블리로 단정하지 않는다.
    return 0;
}
