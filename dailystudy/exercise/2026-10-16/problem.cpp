// 직접 해보는 연습: 두 단계 자원 차감이 전부 성공해야만 결과를 남긴다.
// ScopeExit를 사용하면 두 번째 검사에서 일찍 반환해도 첫 번째 차감을 자동으로 되돌린다.

// <iostream>은 검증 결과를 쓰는 std::cout과 스트림 삽입 연산을 선언한다.
#include <iostream>

// 종료 동작의 정확한 closure 타입을 템플릿 인자로 소유하는 최소 RAII 가드다.
template <class ExitFunction>
class ScopeExit {
public:
    // 이 파일의 lambda는 int와 포인터만 담으므로 값 복사도 예외 없이 작다.
    explicit ScopeExit(ExitFunction exit_function)
        : exit_function_{exit_function} {}

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;

    ~ScopeExit() noexcept {
        if (active_) {
            exit_function_();
        }
    }

    void release() noexcept {
        active_ = false;
    }

private:
    ExitFunction exit_function_;
    bool active_{true};
};

// class는 기본 접근이 private이므로 remaining_을 외부가 임의로 깨뜨리지 못한다.
class Quota {
public:
    // explicit은 int가 Quota로 뜻하지 않게 암시 변환되는 것을 막는다.
    // 생성자는 반환형이 없고, 멤버 초기화 목록이 함수 몸체보다 먼저 remaining_을 초기화한다.
    explicit Quota(int capacity) noexcept
        : remaining_{capacity} {}

    // 반환형 bool은 두 차감이 commit되었는지를 값으로 알려 준다.
    // 두 int 매개변수는 기본 정수의 값 복사이므로 호출자의 변수나 소유권을 바꾸지 않는다.
    [[nodiscard]] bool consume_pair(int first, int second) noexcept {
        // snapshot은 이름 있는 int lvalue이고 현재 상태를 값으로 복사해 독립적으로 소유한다.
        const int snapshot{remaining_};

        // [RAII 타입 생성 계약: ScopeExit<rollback-closure>]
        // (1) 목적 객체 rollback은 아직 없고, 수신할 상태인 *this와 snapshot은 살아 있다.
        // (2) CTAD가 lambda의 고유 closure 타입을 ExitFunction으로 정하고 위 생성자를 선택한다.
        // (3) 인자는 [this, snapshot] lambda prvalue다. this는 Quota를 가리키는 비소유 포인터로
        //     복사되고 snapshot은 int 값으로 closure가 소유한다. null this로 멤버 함수를 부르면 안 된다.
        // (4) 생성자는 반환값이 없고 closure를 소유하는 active guard를 만든다.
        // (5) remaining_은 아직 그대로이며 release하지 않고 scope를 나가면 closure가 한 번 실행된다.
        // (6) 이 closure 저장/실행은 상수 시간, 무할당, noexcept이고 참조 무효화가 없다. guard는
        //     복사 불가이며 Quota가 guard보다 오래 살아야 한다. 동일 객체의 동시 호출은 데이터 경쟁이다.
        ScopeExit rollback{[this, snapshot]() noexcept {
            // 기본 int 대입은 snapshot 값을 remaining_에 저장한다. 특정 store 명령으로 단정할 수 없고
            // 컴파일러는 관찰 가능한 결과를 유지하는 범위에서 최적화할 수 있다.
            remaining_ = snapshot;
        }};

        // [RAII 수명 계약: ScopeExit 소멸자]
        // (1) 수신 rollback은 active 또는 release된 ScopeExit lvalue다.
        // (2) 인자 없는 ~ScopeExit() noexcept가 모든 return과 정상 블록 끝에서 자동 호출된다.
        // (3) 명시 인자는 없고 내부 closure와 그 값/포인터 캡처만 사용한다.
        // (4) 반환값은 없고 active일 때 closure의 void 결과도 버린다.
        // (5) active이면 remaining_을 snapshot으로 되돌린 뒤 파괴되고, inactive이면 실행 없이 파괴된다.
        // (6) 상수 시간, 무할당, 예외 없음이다. this가 유효해야 하며 guard가 함수 지역 객체라
        //     그 전제를 만족한다. 외부 동기화 없는 동시 접근 안전성은 제공하지 않는다.

        // ||도 왼쪽부터 단락 평가한다. 음수는 자원을 늘리는 잘못된 차감이므로 거부한다.
        if (first < 0 || second < 0 || first > remaining_) {
            return false;
        }

        // 첫 단계가 성공해도 아직 guard가 active이므로 아래 실패 시 snapshot으로 복구된다.
        remaining_ -= first;
        if (second > remaining_) {
            return false;
        }
        remaining_ -= second;

        // [RAII commit 계약: ScopeExit::release]
        // (1) 수신 rollback의 정확한 타입은 위 closure를 소유한 active ScopeExit lvalue다.
        // (2) void release() noexcept 멤버가 선택된다.
        // (3) 명시 인자는 없고 Quota나 closure의 소유권을 이전하지 않는다.
        // (4) 반환형은 void라 호출 결과를 사용하지 않는다.
        // (5) guard가 inactive가 되어 소멸 시 closure를 실행하지 않고 두 차감을 commit한다.
        // (6) 전제조건 없이 상수 시간, 무할당, 예외 없음, 참조 무효화 없음이다. guard를 다른
        //     thread와 공유하지 않으며 Quota 동시 접근에는 별도 동기화가 필요하다.
        rollback.release();
        return true;
    }

    // 뒤의 const는 관찰 중 Quota 멤버를 바꾸지 않겠다는 타입 계약이다.
    [[nodiscard]] int remaining() const noexcept {
        return remaining_;
    }

private:
    // {} 멤버 초기화는 생성 경로가 바뀌어도 기본값 0을 보장한다.
    int remaining_{};
};

int main() {
    // `Quota quota{10}`은 결과 객체에 `Quota(int)`를 직접 적용하는 direct-list-initialization이다.
    // 별도의 `Quota{10}` 임시를 quota로 옮기는 식이 아니므로, 이 줄을 C++17 보장 복사 생략이나 RVO의
    // 예로 설명하면 안 된다. 중괄호는 narrowing을 검사하고 explicit 생성자도 직접 초기화에서 허용한다.
    Quota quota{10};

    // bool 반환식은 prvalue이고 각 const 객체를 직접 초기화한다.
    const bool committed{quota.consume_pair(3, 4)}; // 10 -> 3, 두 단계 모두 성공한다.
    const bool rolled_back{quota.consume_pair(2, 2)}; // 3 -> 1 뒤 실패하여 다시 3이 된다.

    // 이 파일은 참조 매개변수를 쓰지 않지만, 참조는 기존 객체의 null 불가 별칭이고 포인터는 주소
    // 값이라 null/재지정이 가능하다. lambda의 this는 quota를 소유하지 않는 포인터이므로 quota의
    // 수명을 늘리지 않는다. 여기서는 guard가 멤버 함수 안에서 먼저 파괴되어 dangling이 생기지 않는다.

    // [첫 호출 계약: std::cout의 operator<< 연쇄]
    // (1) 수신/왼쪽 객체는 정상 상태의 std::ostream인 std::cout이며 한 thread에서만 사용한다.
    // (2) 두 bool에는 basic_ostream<char>& basic_ostream<char>::operator<<(bool), remaining()에는
    //     basic_ostream<char>& basic_ostream<char>::operator<<(int)가 선택된다. 각 문자는
    //     template<class CharT,class Traits> basic_ostream<CharT,Traits>& operator<<(
    //     basic_ostream<CharT,Traits>&, CharT)에서 CharT=char, Traits=char_traits<char>로 선택된다.
    //     boolalpha를 켜지 않아 bool 값은 1과 0으로 형식화된다.
    // (3) committed/rolled_back은 bool lvalue, remaining() 결과는 int prvalue, 공백/개행은 char
    //     prvalue다. 값과 스트림 버퍼 소유권은 이전하지 않는다.
    // (4) 각 연산은 같은 ostream&를 반환해 다음 <<에 사용하고 마지막 반환 참조는 버린다.
    // (5) 성공하면 "1 0 3\n"을 순서대로 버퍼링하고 quota는 바뀌지 않는다. 실패 시 상태 비트가 선다.
    // (6) 시간은 출력 문자 수에 비례한다. 내부 버퍼/로캘 할당과 실제 시스템 호출은 구현별이며,
    //     exceptions mask에 따라 ios_base::failure가 가능하다. 참조 무효화는 없고 여러 thread의
    //     원자적 묶음 출력은 보장하지 않는다. 실제 비교/분기/load/store도 CPU와 최적화에 따라 다르다.
    std::cout << committed << ' ' << rolled_back << ' ' << quota.remaining() << '\n';

    return (committed && !rolled_back && quota.remaining() == 3) ? 0 : 1;
}
