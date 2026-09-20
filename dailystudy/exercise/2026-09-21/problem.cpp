#include <functional> // std::bind_back: 마지막 인자를 저장한 구체 호출 래퍼를 만든다.
#include <iostream>   // std::cout: 연속 호출 뒤 상태가 보존됐는지 출력한다.

// 직접 해보기: 두 번째 호출을 지운 뒤 출력이 어떻게 달라지는지 먼저 예측하라.
// struct는 기본 public이다. 요청처럼 단순히 값을 운반하는 타입에 적합하다.
struct ReservationRequest {
    int seats{}; // 기본 정수형 int이며 {}는 0으로 값 초기화한다.
};

// 반환 결과는 참조가 아닌 값이어서 ledger나 bind_back 래퍼보다 오래 안전하게 보관할 수 있다.
struct Receipt {
    int sequence{};
    int seats{};
    int accumulated_seats{};
};

// class는 기본 private이다. 순번과 누적 좌석을 public 함수 하나만 변경하게 한다.
class ReservationLedger {
public:
    // 매개변수 seats는 값 복사다. 반환형 Receipt도 독립 값이다.
    [[nodiscard]] Receipt reserve(int seats) {
        // if는 비교 결과에 따라 분기한다. 0 이하 요청은 상태를 바꾸지 않는 실패 영수증으로 표현한다.
        if (seats <= 0) {
            return Receipt{0, 0, accumulated_seats_};
        }

        const int issued{next_sequence_}; // const 지역값은 초기화 뒤 바꿀 수 없다.
        ++next_sequence_;
        accumulated_seats_ += seats;
        // Receipt{...}는 prvalue이며 C++17 이후 반환 목적 객체에 직접 구성된다.
        return Receipt{issued, seats, accumulated_seats_};
    }

private:
    int next_sequence_{1};    // 멤버 중괄호 초기화로 첫 정상 순번을 1로 정한다.
    int accumulated_seats_{}; // {}는 누적값을 0으로 만든다.
};

// using 별칭은 새 타입이 아니다. 함수 포인터는 함수를 소유하지 않고 정적 수명의 코드를 가리킨다.
using ReserveFunction = Receipt (*)(ReservationRequest, ReservationLedger&);

// ledger의 비-const 참조는 호출자가 소유한 원장을 빌리고 실제 상태 변경을 허용한다.
[[nodiscard]] Receipt reserve_request(ReservationRequest request, ReservationLedger& ledger) {
    return ledger.reserve(request.seats);
}

int main() {
    ReserveFunction reserve_function{&reserve_request};

    // [호출 계약: std::bind_back]
    // (1) 수신 객체 없는 자유 함수다. reserve_function은 유효한 함수 포인터 lvalue이고
    //     ReservationLedger{}는 아직 외부 소유자가 없는 prvalue다.
    // (2) bind_back<F, Args...>(F&&, Args&&...)에서 F=ReserveFunction&,
    //     Args={ReservationLedger}로 추론하고 두 decay 타입을 래퍼 상태로 직접 초기화한다.
    // (3) 함수 포인터는 값 복사되고 임시 원장은 rvalue로 이동 구성된다. 반환 래퍼가 원장을 소유한다.
    // (4) 구현 지정 래퍼 prvalue를 반환하며 reserve_in_order를 직접 초기화한다.
    // (5) 이후 이 non-const lvalue 래퍼를 호출하면 앞쪽 요청 뒤에 저장 원장 lvalue를 붙인다.
    //     원본 전달 임시는 전체 식 끝에 파괴되지만 별도 이동 구성된 상태는 래퍼 수명 동안 유지된다.
    // (6) decay 함수 포인터와 원장은 각 입력에서 구성 가능하고 MoveConstructible이어야 하며 여기서는
    //     만족한다. 저장 초기화 예외는 전파되고 표준은 일반적인 동적 할당/복잡도 상한을 약속하지 않는다.
    //     래퍼는 동기화를 제공하지 않아 여러 스레드의 상태 변경 호출은 데이터 경쟁이다.
    auto reserve_in_order{std::bind_back(reserve_function, ReservationLedger{})};

    // [호출 계약: bind_back 반환 래퍼의 operator()]
    // (1) reserve_in_order는 함수 포인터와 next_sequence=1인 원장을 소유한 non-const lvalue다.
    // (2) 반환 타입의 operator() 선언 형태는 구현 지정이다. 이 non-const lvalue 래퍼는 저장 원장을
    //     ReservationLedger&로 전달한다. 요청 prvalue는 임시를 materialize해 전달 참조에 바인딩되고
    //     래퍼 안에서 ReservationRequest&& xvalue로 forward되어 대상 값 매개변수를 이동 구성한다.
    // (3) 원장 소유권은 래퍼에 남는다. const lvalue/rvalue 래퍼는 각각 const Ledger&/const Ledger&&,
    //     non-const rvalue 래퍼는 Ledger&&를 전달하므로 비-const Ledger&만 요구하는 이 대상에는
    //     non-const lvalue 래퍼 호출만 성립한다.
    // (4) Receipt 값을 반환해 first를 직접 초기화한다.
    // (5) 호출 뒤 저장 원장은 next_sequence=2, accumulated_seats=2이고 임시 요청은 파괴된다.
    // (6) 표준은 래퍼 호출의 일반 복잡도·할당 상한을 약속하지 않는다. 이 대상 함수 자체는 할당하지
    //     않으며 잘못된 인자 조합은 컴파일 오류다. 함수 포인터는 non-null이고 대상 예외는 그대로 전파된다.
    //     래퍼가 파괴되면 저장 원장도 파괴된다.
    const Receipt first{reserve_in_order(ReservationRequest{2})};

    // 같은 lvalue 래퍼를 다시 호출하므로 새 원장이 아니라 앞 호출에서 바뀐 같은 저장 원장을 사용한다.
    const Receipt second{reserve_in_order(ReservationRequest{3})};

    // [호출 계약: ostream 정수·문자열 리터럴·문자 삽입]
    // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 두 Receipt는 살아 있는 const 값이다.
    // (2) operator<<(ostream&, const char*), ostream::operator<<(int),
    //     operator<<(ostream&, char)를 연쇄한다.
    // (3) NUL 종료 리터럴, int lvalue, '\n' char prvalue를 읽기만 하며 소유권 이전은 없다.
    // (4) 매 호출은 같은 ostream&를 반환해 다음 삽입에 쓰고 마지막 참조는 버린다.
    // (5) 출력 버퍼/상태만 변하고 영수증과 래퍼 원장은 변하지 않는다.
    // (6) 표준은 일반 복잡도·할당 상한을 명시하지 않는다. 이 식은 표시된 문자들을 내보내며 실패는
    //     상태 비트 또는 설정된 예외로 보고된다. 같은 stream의 복합 출력을 여러 스레드가 수행한다면
    //     별도 동기화가 필요하다.
    std::cout << "first=" << first.sequence << ',' << first.accumulated_seats << '\n';
    std::cout << "second=" << second.sequence << ',' << second.accumulated_seats << '\n';

    // 기계 관점: 원장 갱신은 정수 load/store, 유효성 검사는 비교·조건 분기가 될 수 있다.
    // 구체 bind_back 래퍼 호출은 인라인될 수 있지만 실제 명령은 CPU·ABI·컴파일러·최적화에 따라 달라진다.
}
