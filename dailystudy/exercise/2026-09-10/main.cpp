// <iostream>은 결과를 쓰는 std::cout, 기반 타입 std::ostream, 정수·문자 삽입 연산을 선언한다.
#include <iostream>
// <memory>는 독점 소유자 std::unique_ptr와 C++23 출력 포인터 어댑터 std::out_ptr를 선언한다.
#include <memory>
// <utility>는 이름 있는 소유자를 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>

// struct는 기본 접근이 public이다. 이 타입은 C ABI 경계가 채우는 단순 레코드이므로 필드를 공개한다.
// int는 기본 정수 타입이고 {} 기본 멤버 초기화는 값을 빠뜨렸을 때 0으로 만든다.
struct LegacyConnection {
    int endpoint{};
};

// 이 함수들은 `T**` 출력 매개변수를 쓰는 오래된 C API 모양을 재현한다. 실제 프로젝트에서는 이 선언만
// C 헤더에서 받고 구현은 외부 라이브러리에 있을 수 있다. output은 포인터 값을 써 넣을 "슬롯의 주소"다.
extern "C" int legacy_connect(int endpoint, LegacyConnection** output) noexcept {
    // 포인터 비교와 조건 분기다. 잘못된 출력 슬롯이나 도메인상 허용하지 않는 endpoint에는 쓰지 않는다.
    if (output == nullptr || endpoint <= 0) {
        return 1;
    }

    try {
        // new는 LegacyConnection 한 객체의 저장소를 할당하고 {endpoint}로 초기화한다. 성공한 원시 포인터를
        // 호출자가 준 슬롯에 저장한다. 이 소유권은 아래 custom deleter가 정확히 한 번 delete해야 한다.
        *output = new LegacyConnection{endpoint};
    } catch (...) {
        // 메모리 할당 실패가 C 모양의 경계를 넘어 예외로 새지 않게 상태 코드로 바꾼다.
        return 2;
    }
    return 0;
}

extern "C" void legacy_disconnect(LegacyConnection* connection) noexcept {
    // delete는 null이면 아무 일도 하지 않고, non-null이면 객체 수명을 끝낸 뒤 같은 저장소를 해제한다.
    delete connection;
}

// 상태 없는 deleter는 unique_ptr가 소멸/reset될 때 C 해제 함수를 호출하는 정책 객체다.
struct ConnectionCloser {
    // 반환형 void, 매개변수는 소유권을 끝낼 원시 포인터 값이다. unique_ptr가 non-null일 때만 호출한다.
    void operator()(LegacyConnection* connection) const noexcept {
        legacy_disconnect(connection);
    }
};

// using은 새 타입을 만드는 것이 아니라 긴 템플릿 특수화에 별칭을 붙인다. 첫 템플릿 인자는 관리 객체,
// 둘째는 해제 정책이다. 이 포인터는 복사할 수 없고 이동으로만 단독 소유권을 넘긴다.
using ConnectionPtr = std::unique_ptr<LegacyConnection, ConnectionCloser>;

// class는 기본 접근이 private이다. C 포인터와 상태 코드를 감추고, 유효성 및 endpoint 조회만 공개한다.
class Connection {
public:
    // 생성자는 반환형이 없다. explicit은 ConnectionPtr가 뜻하지 않게 Connection으로 암시 변환되는 것을 막는다.
    // 값 매개변수 handle은 소유권을 받는 sink이고, 멤버 초기화 목록은 본문 전에 handle_을 직접 구성한다.
    // [호출 계약: std::move(handle)와 ConnectionPtr 이동 생성]
    // (1) move에는 수신 객체가 없다. handle은 완성된 ConnectionPtr 값 매개변수이자 이름 있는 lvalue이고,
    //     목적 멤버 handle_은 아직 수명이 시작되지 않았다.
    // (2) 선택 함수는 template<class T> remove_reference_t<T>&& move(T&&) noexcept에서 T=ConnectionPtr&이며,
    //     이어 unique_ptr(unique_ptr&&) noexcept 이동 생성자가 선택된다.
    // (3) 유일한 인자 식 handle은 ConnectionPtr lvalue다. 유효한 빈/소유 상태를 모두 허용하며 move는 객체를
    //     빌려 값 범주만 xvalue로 바꾸고, 후속 생성자가 원시 포인터와 deleter 소유권을 넘겨받는다.
    // (4) move는 같은 객체를 가리키는 ConnectionPtr&&를 반환해 handle_ 구성에 즉시 사용한다. 두 생성자는
    //     별도 반환값이 없고, 완성된 Connection이 결과다.
    // (5) handle_이 자원을 단독 소유하고 handle은 빈 유효 상태가 된다. 대상 객체의 주소와 값은 바뀌지 않는다.
    // (6) 두 동작은 O(1), 무할당·noexcept이며 참조/포인터를 새로 무효화하지 않는다. 수명을 연장하는 것은
    //     handle_의 소유권이고 move 자체가 아니다. 같은 소유자를 동시에 조작하지 않아 데이터 경쟁도 없다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
    explicit Connection(ConnectionPtr handle) noexcept
        : handle_{std::move(handle)} {}

    // unique_ptr 멤버 때문에 복사는 금지된다. 이동은 단독 소유권을 새 Connection으로 옮긴다.
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) noexcept = default;
    Connection& operator=(Connection&&) noexcept = default;
    ~Connection() = default;

    [[nodiscard]] static Connection open(int endpoint) {
        // [생성 계약: ConnectionPtr 기본 생성]
        // (1) 목적 handle은 아직 없고 stateless ConnectionCloser는 기본 생성 가능하다.
        // (2) unique_ptr의 constexpr unique_ptr() noexcept 기본 생성자가 선택된다. T=LegacyConnection,
        //     D=ConnectionCloser이며 명시적 데이터 인자는 없다.
        // (3) 인자·외부 소유권 입력은 없다. deleter가 기본 생성 가능해야 한다.
        // (4) 생성자는 반환값이 없고, handle이라는 빈 ConnectionPtr 객체의 수명을 시작한다.
        // (5) handle.get()에 해당하는 저장 포인터는 null이고 아직 관리 객체가 없다.
        // (6) O(1), 무할당·noexcept이며 무효화할 관찰자도 없다. 이후 소멸 시 non-null이면 deleter를 정확히
        //     한 번 부른다. 같은 handle에 대한 동시 접근은 외부 동기화가 필요하나 여기서는 한 흐름만 쓴다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        ConnectionPtr handle{};

        // [호출 계약: std::out_ptr(handle)와 adapter의 LegacyConnection** 변환]
        // (1) 자유 함수라 수신 객체는 없다. handle은 살아 있는 빈 ConnectionPtr lvalue이고 legacy_connect는
        //     호출 동안 쓸 유효한 LegacyConnection** 출력 슬롯을 요구한다.
        // (2) template<class Pointer=void, class Smart, class... Args> auto out_ptr(Smart&, Args&&...)에서
        //     Smart=ConnectionPtr, Args는 빈 팩이고 Pointer는 기본값 void다. 표준의 P 선택 규칙이
        //     ConnectionPtr::pointer인 LegacyConnection*을 고르므로 반환 구체 타입은
        //     std::out_ptr_t<ConnectionPtr, LegacyConnection*>이며 그 Pointer* 변환이 선택된다.
        // (3) 유일한 명시 인자 handle은 non-const lvalue reference로 빌려 주며 소유자를 복사하지 않는다.
        //     변환 결과 LegacyConnection**는 adapter 임시 수명 안에서만 허용되고 저장해서는 안 된다.
        // (4) out_ptr는 adapter prvalue를 반환해 C 함수의 둘째 인자로 즉시 쓴다. legacy_connect는 int 상태를
        //     status에 저장한다. adapter 변환은 내부 null 포인터 슬롯의 주소를 반환한다.
        // (5) adapter 생성 시 handle은 먼저 reset되어 비며, C 함수 성공 시 슬롯에 새 포인터가 기록된다.
        //     전체 표현식 끝에서 adapter가 파괴되며 non-null 포인터를 handle.reset(pointer)로 채택한다.
        //     실패 시 C 계약상 슬롯을 쓰지 않아 handle은 빈 상태다.
        // (6) adapter 자체와 unique_ptr reset은 여기서 O(1)·무할당이다. 실제 C 함수의 new가 O(1) 저장소와
        //     할당 실패를 가질 수 있으나 상태 코드로 바꾼다. out_ptr 생성자는 일반적으로 noexcept로 명세되지
        //     않는다. adapter보다 출력 슬롯을 오래 쓰면 수명 위반이며, 같은 handle 동시 접근은 데이터 경쟁이다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        const int status{legacy_connect(endpoint, std::out_ptr(handle))};

        if (status != 0) {
            // 위 기본 생성 계약과 같은 빈 ConnectionPtr prvalue를 값 매개변수로 넘긴다.
            return Connection{ConnectionPtr{}};
        }

        // handle은 이름 있는 lvalue다. 위 move 계약대로 소유권을 Connection prvalue에 넘긴다. 반환식이 함수
        // 반환 타입과 같은 prvalue이므로 C++17부터 호출자의 결과 객체에 직접 구성되어 Connection 복사/이동이 없다.
        return Connection{std::move(handle)};
    }

    // const 멤버 함수는 handle_ 자체를 바꾸지 않는다.
    [[nodiscard]] bool valid() const noexcept {
        // [호출 계약: ConnectionPtr::operator bool]
        // (1) 수신자는 수명이 유효한 const ConnectionPtr lvalue handle_이며 빈 상태이거나 한 객체를 소유한다.
        // (2) 선택 멤버는 constexpr explicit operator bool() const noexcept이다. 데이터 인자는 없다.
        // (3) 숨은 const this만 빌리고 포인터·deleter 소유권을 옮기지 않는다.
        // (4) 저장 포인터가 null이 아니면 true인 bool prvalue를 반환해 valid()의 결과로 즉시 사용한다.
        // (5) handle_과 관리 객체의 상태·수명은 그대로다.
        // (6) O(1), 무할당·비무효화·noexcept다. 살아 있는 unique_ptr라는 전제만 필요하며 동기화를 제공하지
        //     않는다. 다른 흐름이 같은 handle_을 변경하지 않는 const 관찰만 수행한다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        return static_cast<bool>(handle_);
    }

    [[nodiscard]] int endpoint() const noexcept {
        // operator bool의 계약은 valid()와 같다. 빈 상태를 먼저 분기해 아래 역참조 전제조건을 보장한다.
        if (!handle_) {
            return -1;
        }

        // [호출 계약: ConnectionPtr::operator->]
        // (1) 수신자는 LegacyConnection 한 객체를 소유하는 유효한 const ConnectionPtr lvalue handle_이다.
        // (2) 선택 멤버는 pointer operator->() const noexcept이며 pointer는 LegacyConnection*이다.
        // (3) 데이터 인자는 없고 숨은 this만 빌린다. 바로 앞 검사가 non-null이라는 허용 조건을 보장한다.
        // (4) 비소유 원시 포인터 prvalue를 반환하고 내장 ->가 endpoint int lvalue를 찾아 그 값을 복사한다.
        // (5) 소유권, 포인터, LegacyConnection 값은 바뀌지 않는다.
        // (6) O(1), 무할당·비무효화·noexcept다. 반환 포인터는 handle_의 reset/이동/소멸 또는 대상 해제 전까지만
        //     유효하다. null 결과를 역참조하면 미정의 동작이며, 동시 대상 변경은 별도 동기화가 필요하다.
        // 대표 문서: ../standard-library/ownership-and-vocabulary-types.md
        return handle_->endpoint;
    }

private:
    // private 접근 지정자는 외부가 raw pointer를 release/reset해 클래스 불변식을 깨지 못하게 한다.
    ConnectionPtr handle_;
};

int main() {
    // open(7)과 open(-1)은 Connection prvalue다. 각각 같은 타입의 const 결과 객체에 직접 구성되어 중간
    // Connection 복사/이동이 없다. connected/rejected라는 이름을 가진 뒤의 식은 const lvalue다.
    const Connection connected{Connection::open(7)};
    const Connection rejected{Connection::open(-1)};

    // [호출 계약: std::ostream 정수·bool·문자 삽입 연쇄]
    // (1) 최초 수신자는 출력 가능한 std::cout의 std::ostream lvalue이고 두 Connection은 살아 있다.
    // (2) int와 bool에는 대응하는 ostream 멤버 operator<< overload, 공백·개행에는
    //     operator<<(std::ostream&, char) 비멤버 overload가 선택된다.
    // (3) endpoint()의 int prvalue, char prvalue 두 개, valid()의 bool prvalue를 값으로 읽는다. bool은 기본
    //     형식이라 0/1로 쓰고 어떤 피연산자도 소유권을 넘기지 않는다.
    // (4) 각 삽입은 같은 std::ostream&를 반환해 다음 삽입의 수신자로 사용하며 마지막 반환 참조는 버린다.
    // (5) 성공하면 `7 0\n`이 버퍼에 순서대로 추가되고 Connection 및 관리 객체는 그대로다.
    // (6) 복잡도는 숫자 변환·locale·버퍼·장치에 따라 달라 점근 상한을 고정하지 않는다. 버퍼 준비 중 할당,
    //     실패 상태 비트와 설정된 예외 마스크의 예외가 가능하다. 무효화는 없고 단일 흐름에서만 출력한다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cout << connected.endpoint() << ' ' << rejected.valid() << '\n';

    // 기계 실행 관점에서는 out_ptr 임시 안의 raw pointer 슬롯에 store하고, full-expression 끝에 unique_ptr로
    // 다시 store하며, null/상태 코드 비교와 조건 분기, deleter 호출이 생길 수 있다. unique_ptr 소멸의 deleter는
    // 구체 타입이라 인라인될 수도 있다. 실제 load/store/분기 제거·할당·호출 형태는 CPU, ABI, 표준 라이브러리,
    // 컴파일러와 최적화 옵션에 따라 달라 특정 어셈블리나 가상/직접 호출 형태로 단정하지 않는다.
    return 0;
}
