// <iostream>은 표준 출력 객체 std::cout과 삽입 연산자 operator<<를 선언한다.
#include <iostream>
// <utility>는 이름 있는 객체를 xvalue 식으로 바꾸는 std::move를 선언한다.
#include <utility>

// Rollback은 실패 시 원래 상태를 복구하는 호출 가능 객체의 타입 매개변수다.
template <class Rollback>
class ScopeRollback {
private: // class는 접근 지정자를 생략해도 기본 private이며 구현 상태를 외부에서 숨긴다.
    Rollback rollback_; // 콜백을 값으로 소유해 생성자 인자의 수명이 끝난 뒤에도 소멸 때 실행할 수 있다.
    bool committed_{false}; // bool을 중괄호로 false 초기화해 복구가 아직 활성임을 나타낸다.

public:
    // 생성자는 반환형이 없고 Rollback 값 매개변수를 받는다. explicit은 ScopeRollback 변수가 콜백에서
    // 암시적으로 만들어지는 것을 막으며 ScopeRollback{callback} 같은 직접 초기화는 허용한다.
    explicit ScopeRollback(Rollback rollback)
        // std::move<Rollback>(rollback)의 실제 인자는 이름 있는 lvalue rollback 하나다. 반환형은 Rollback&&이고
        // 결과는 xvalue다. 함수 자체는 소유권을 옮기지 않지만 멤버의 이동 생성자가 콜백 상태를 rollback_으로
        // 옮길 수 있다. 원본 매개변수는 유효하지만 값은 미지정이며 함수가 끝날 때 파괴된다. O(1)인 작은 람다지만
        // 일반 Rollback의 이동 비용·예외는 그 타입에 달려 있다. 대표 문서: ../standard-library/io-parsing-and-utilities.md
        : rollback_{std::move(rollback)} {}

    // 같은 복구를 두 객체가 실행하면 상태가 두 번 되돌아가므로 복사를 명시적으로 금지한다.
    ScopeRollback(const ScopeRollback&) = delete;
    ScopeRollback& operator=(const ScopeRollback&) = delete;

    // 소멸자는 반환형이 없고 자동 객체의 스코프를 벗어날 때 호출된다.
    ~ScopeRollback() noexcept {
        if (!committed_) { // !는 bool을 반전하며 정상 커밋되지 않은 모든 반환 경로에서만 복구한다.
            rollback_(); // 소유 람다의 operator()를 인자 없이 호출한다. 예외를 던지면 noexcept 소멸자라 종료되므로 콜백은 noexcept여야 한다.
        }
    }

    void commit() noexcept { // void 반환값은 없으며 호출 뒤 복구만 비활성화하고 콜백 객체는 계속 소유한다.
        committed_ = true; // 저장 한 번으로 상태를 바꾸며 할당·참조 무효화·예외가 없다.
    }
};

// 클래스 템플릿 인자 추론 가이드는 생성자 인자의 타입으로 ScopeRollback<Rollback>을 고르게 한다.
template <class Rollback>
ScopeRollback(Rollback) -> ScopeRollback<Rollback>;

// struct는 기본 public이므로 단순 데이터 전달 객체의 두 필드를 직접 관찰할 수 있다.
struct StockRecord {
    int available{}; // int{}는 0 값 초기화이며 판매 가능한 재고를 소유한다.
    int reserved{}; // 예약된 재고 수도 독립된 int 값으로 소유한다.
};

class ReservationService {
private:
    StockRecord& stock_; // 비const lvalue 참조는 재고를 소유하지 않고 빌리며 서비스보다 stock이 오래 살아야 한다.

public:
    // 생성자에는 반환형이 없다. explicit은 StockRecord에서 서비스로의 의도치 않은 암시 변환을 막는다.
    explicit ReservationService(StockRecord& stock) noexcept
        : stock_{stock} {} // 멤버 초기화 목록이 호출자의 lvalue stock에 참조를 바인딩하며 복사·이동하지 않는다.

    [[nodiscard]] bool reserve(int units, bool persistence_succeeds) {
        if (units <= 0 || units > stock_.available) { // ||는 왼쪽이 참이면 오른쪽을 평가하지 않는 단락 조건이다.
            return false; // bool prvalue false가 호출자 결과를 직접 초기화할 수 있다.
        }

        stock_.available -= units; // 읽기-뺄셈-저장으로 가용 재고를 줄인다.
        stock_.reserved += units; // 같은 수량을 예약 재고에 더해 총 재고 불변식을 유지한다.

        // 람다 prvalue는 this 포인터와 units 값을 소유한다. this는 비소유 포인터라 guard보다 서비스가 오래 살아야 하며,
        // units 값 캡처는 호출이 끝나도 안전하다. 직접 초기화와 추론 가이드가 정확한 람다 타입의 guard를 만든다.
        ScopeRollback rollback{[this, units]() noexcept {
            stock_.available += units; // 실패 경로에서 앞선 감소를 보상한다.
            stock_.reserved -= units; // 실패 경로에서 앞선 증가도 보상해 원래 상태를 복원한다.
        }};

        if (!persistence_succeeds) { // 외부 DB 저장 실패를 흉내 내는 조건 분기다.
            return false; // 반환 전에 rollback의 소멸자가 실행되어 두 멤버를 되돌린다.
        }

        rollback.commit(); // 성공 경로에서 복구를 비활성화한다. 수신 ScopeRollback의 콜백 소유권은 유지된다.
        return true; // bool prvalue true를 반환한 뒤 guard는 소멸하지만 콜백을 실행하지 않는다.
    }
};

int main() {
    StockRecord stock{10, 0}; // aggregate 직접 초기화로 두 public int 멤버를 순서대로 설정한다.
    ReservationService service{stock}; // explicit 생성자를 직접 호출하고 stock lvalue를 참조로 빌린다.

    const bool first_result{service.reserve(3, true)}; // 반환 bool prvalue로 const 객체를 직접 초기화한다.
    // operator<<(ostream&, value)는 std::cout lvalue와 각 값/문자를 순서대로 읽고 같은 ostream&를 반환해 연쇄한다.
    // first_result와 stock은 유지되고 cout의 문자·상태만 바뀐다. 반환 참조는 다음 <<에 쓰고 마지막에는 버린다.
    // 출력 문자 수에 선형이며 버퍼 작업이 있을 수 있고, 실패는 기본적으로 스트림 상태 비트에 기록된다.
    std::cout << stock.available << ' ' << stock.reserved << ' ' << first_result << '\n';

    const bool second_result{service.reserve(4, false)}; // 저장 실패라 스코프 종료 시 자동 롤백된다.
    // 두 번째 스트림 연쇄도 값 소유권을 옮기지 않고 실패 후 원상 복구된 상태를 관찰한다.
    std::cout << stock.available << ' ' << stock.reserved << ' ' << second_result << '\n';

    return (stock.available == 7 && stock.reserved == 3 && !second_result) ? 0 : 1; // ?:는 검증 결과에 따라 종료 코드를 고른다.
}
