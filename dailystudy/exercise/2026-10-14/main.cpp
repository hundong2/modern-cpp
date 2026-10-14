// <cstdint>는 wire code 폭을 명확히 고정하는 std::uint16_t를 선언한다.
#include <cstdint>
// <iostream>은 표준 출력 객체 std::cout과 삽입 연산자를 선언한다.
#include <iostream>
// <string>은 추적 번호 문자를 직접 소유하는 std::string을 선언한다.
#include <string>
// <utility>는 C++23 std::to_underlying과 값 범주 변환 std::move를 선언한다.
#include <utility>
// <vector>는 같은 타입 원소를 연속 저장소에 소유하는 std::vector를 선언한다.
#include <vector>

// enum class는 정수와 암시적으로 섞이지 않는 강한 열거형이다. 외부 프로토콜 값을 명시해
// 선언 순서를 바꾸어도 저장 형식이 우연히 바뀌지 않게 한다.
enum class DeliveryState : std::uint16_t {
    queued = 10U,
    in_transit = 20U,
    delivered = 30U,
};

// struct의 멤버는 기본적으로 public이다. 계층 사이에서 값만 운반하는 DTO에 알맞다.
struct Delivery {
    // [첫 생성 계약: std::string 기본 생성자]
    // (1) 수신 tracking_id는 Delivery 안에서 아직 생성되지 않은 정확한 string 멤버다.
    // (2) 선택 생성자는 basic_string()이며 문자·traits·allocator의 기본 template 인자를 쓴다.
    // (3) 명시 인자는 없고 {}는 값 초기화다. 외부 문자 저장소를 빌리거나 소유권을 받지 않는다.
    // (4) 생성자는 반환값이 없고 유효한 빈 문자열을 완성한다.
    // (5) tracking_id의 길이는 0이고 state와 외부 객체는 바뀌지 않는다.
    // (6) 기본 allocator 특수화에서 상수 시간·noexcept이며 보통 할당하지 않는다. 기존 관찰자는
    //     없고 수명은 Delivery와 같으며, 같은 객체의 동시 변경은 별도 동기화가 필요하다.
    std::string tracking_id{};

    // 중괄호 초기화는 미초기화 값을 막고 enum의 기저 값 0을 만든다. 오늘 열거자에는 0이 없으므로
    // 이 기본 상태를 도메인 유효값으로 사용하지 않고 aggregate 생성 때 실제 상태를 항상 지정한다.
    DeliveryState state{};
};

// 외부 저장/API 계층이 이해하는 숫자 표현이다. 문자열도 스스로 소유하므로 원본보다 오래 살 수 있다.
struct WireDelivery {
    std::string tracking_id{};
    std::uint16_t state_code{};
};

// 강한 enum을 숫자로 낮추는 일은 도메인 전체가 아니라 이 adapter 경계 한곳에만 둔다.
[[nodiscard]] constexpr std::uint16_t encode_state(const DeliveryState state) noexcept {
    // [첫 호출 계약: std::to_underlying(state)]
    // (1) 멤버 수신 객체가 없는 자유 함수이며 state는 유효한 DeliveryState 값인 const lvalue 식이다.
    // (2) 선택 특수화는 to_underlying<DeliveryState>(DeliveryState)이고 반환형은 uint16_t다.
    // (3) 유일한 인자 state는 값으로 복사된다. enum의 저장·수명·소유권을 빌리거나 옮기지 않는다.
    // (4) 같은 기저 정수 값을 가진 uint16_t prvalue를 반환하며 return 문의 결과로 사용한다.
    // (5) 호출 뒤 state와 모든 객체 상태는 그대로다. 반환값은 독립된 기본 타입 값이다.
    // (6) O(1), 무할당, noexcept, 참조·반복자 무효화 없음이다. enum 이외 타입이면 컴파일되지
    //     않으며 런타임 오류·UB가 없다. 공유 상태가 없어 독립 호출 사이 스레드 동기화도 필요 없다.
    return std::to_underlying(state);
}

// class의 멤버는 기본적으로 private다. 도메인 owner와 직렬화 절차를 캡슐화한다.
class DeliveryBatchEncoder {
public:
    // using은 긴 class-template 특수화에 도메인 이름을 붙일 뿐 새 타입을 만드는 선언은 아니다.
    // template 인자 T는 각각 Delivery와 WireDelivery다.
    using Source = std::vector<Delivery>;
    using Snapshot = std::vector<WireDelivery>;

    // explicit은 Source 하나가 Encoder로 뜻밖에 암시 변환되는 것을 막는다. 값 매개변수는
    // lvalue 호출에는 복사, xvalue 호출에는 이동을 허용하는 소유권 경계다.
    // [첫 호출 계약: std::move(deliveries)와 vector 이동 생성]
    // (1) 수신 deliveries_는 아직 생성 전이고 deliveries는 유효한 Source lvalue다.
    // (2) move는 T=Source&인 std::move<Source&>(Source&) 특수화로 Source&&를 반환하며,
    //     이어 vector(vector&&)가 선택된다.
    // (3) deliveries는 이름 있는 lvalue, move 결과는 같은 객체를 가리키는 xvalue다. 저장소 소유권을
    //     새 멤버로 넘길 수 있지만 move 자체는 객체를 수정하지 않는다.
    // (4) move의 반환 참조는 멤버 초기화에 사용되고 vector 생성자는 반환값 없이 deliveries_를 완성한다.
    // (5) deliveries_가 원소 저장소를 소유하며 매개변수는 유효하지만 값이 미지정된 이동 후 상태다.
    // (6) 기본 allocator가 같은 vector 이동은 O(1), 무할당, noexcept다. 기존 원소 관찰자는 새 owner의
    //     원소를 계속 가리키며 owner 파괴 때 수명이 끝난다. 같은 객체의 동시 이동/접근은 안전하지 않다.
    explicit DeliveryBatchEncoder(Source deliveries) noexcept
        : deliveries_{std::move(deliveries)} {}

    // const 멤버 함수는 encoder의 원본을 바꾸지 않는다. 반환 vector는 깊게 소유하는 경계 snapshot이다.
    [[nodiscard]] Snapshot encode() const {
        // [첫 생성 계약: std::vector<WireDelivery>()]
        // (1) 수신 result는 아직 생성 전이다.
        // (2) 선택 overload는 vector()이며 T=WireDelivery, allocator는 기본 template 인자다.
        // (3) 명시 인자가 없고 외부 저장소를 빌리거나 소유권을 받지 않는다.
        // (4) 생성자는 반환값 없이 유효한 빈 Snapshot을 완성한다.
        // (5) result.size()==0이고 deliveries_는 그대로다.
        // (6) O(1), 보통 무할당이며 기본 allocator에서는 noexcept다. 관찰자는 아직 없고 지역 객체라
        //     공유 데이터 경쟁도 없다. allocator 생성 계약이 다른 사용자 allocator에는 달라질 수 있다.
        Snapshot result{};

        // [첫 호출 계약: vector::size()와 reserve(count)]
        // (1) size 수신 deliveries_는 완성된 const Source이고, reserve 수신 result는 빈 Snapshot이다.
        // (2) size_type size() const noexcept와 void reserve(size_type count)가 선택된다.
        // (3) size에는 인자가 없다. 반환 size_type prvalue가 count 값 인자가 되며 소유권 이동은 없다.
        // (4) size 반환값은 reserve 인자로 사용되고 reserve는 void라 반환값이 없다.
        // (5) 성공하면 result 크기는 0인 채 capacity가 적어도 count이고 deliveries_는 변하지 않는다.
        // (6) size는 O(1)·무할당·noexcept다. reserve는 기존 원소 수에 선형이고 할당 실패나 길이 오류가
        //     가능하다. 재할당 시 result 관찰자는 모두 무효지만 아직 없으며 동시 접근은 외부 동기화 대상이다.
        result.reserve(deliveries_.size());

        // [첫 숨은 호출 계약: const vector range-for]
        // (1) 수신 deliveries_는 유효한 const Source lvalue이며 완성된 원소를 소유한다.
        // (2) const begin/end, const_iterator 역참조·전위 증가, 그리고 != 비교 또는 C++20 이후
        //     operator==에서 rewrite된 비교가 선택된다. iterator 구현에 따라 비교는 비멤버일 수 있다.
        // (3) begin/end에는 데이터 인자가 없다. 비교의 두 iterator lvalue, 역참조·증가의 현재 iterator
        //     lvalue를 비소유로 쓰며 delivery는 역참조 결과에 const Delivery&로 바인딩된다.
        // (4) begin/end는 const_iterator 값, 비교는 bool, 역참조는 const Delivery&를 반환해 사용하고
        //     전위 증가의 const_iterator& 반환은 버린다.
        // (5) 반복자만 전진하고 원본 vector와 원소는 바뀌지 않는다.
        // (6) 각 연산 O(1), 전체 O(n), 무할당이다. 순회 중 구조 변경·owner 파괴 뒤 역참조는 UB이며
        //     읽기와 동시에 같은 원소를 쓰면 데이터 경쟁이다. 현재 함수는 const 읽기만 한다.
        for (const Delivery& delivery : deliveries_) {
            // [첫 호출 계약: string 복사·이동 생성과 vector::push_back(WireDelivery&&)]
            // (1) 수신 result는 충분한 capacity의 Snapshot이고 delivery.tracking_id는 유효한 const string lvalue다.
            // (2) basic_string(const basic_string&), push_back(value_type&&), 그리고 WireDelivery의 암시적
            //     이동 생성 안에서 basic_string(basic_string&&) noexcept가 선택된다.
            // (3) WireDelivery{...}는 prvalue이고 tracking_id는 깊게 복사된다. state enum은 encode_state가
            //     기본 정수 값으로 바꾸며 원본 문자열·원소 소유권은 유지된다.
            // (4) 두 string 생성자와 push_back은 반환값이 없고 새 DTO가 마지막 원소로 저장된다.
            // (5) 성공하면 result 크기가 1 증가하고 새 원소가 자체 문자열과 숫자 code를 소유한다.
            // (6) reserve 범위 안의 push는 상각/실제 O(1), 문자열 복사는 문자 수에 선형이고 이동은 O(1)·
            //     무할당·noexcept다. 문자열 복사/할당 실패 시 result는 유지되고 재할당은 없다. 기존 원소 관찰자는
            //     유지되지만 과거 end iterator는 삽입 뒤 무효다. 같은 result 동시 변경은 안전하지 않다.
            result.push_back(WireDelivery{
                .tracking_id = delivery.tracking_id,
                .state_code = encode_state(delivery.state),
            });
        }

        // result는 이름 있는 lvalue라 NRVO 후보다. 적용되지 않아도 반환 문맥의 암시적 이동이 가능하다.
        return result;
    }

private:
    // 실제 Delivery와 문자열 수명은 이 vector가 소유한다.
    Source deliveries_;
};

int main() {
    // [첫 생성 계약: vector initializer_list 생성과 string 리터럴 생성]
    // (1) 수신 seed와 각 Delivery의 tracking_id string은 아직 생성 전이다.
    // (2) vector(initializer_list<Delivery>), basic_string(const char*), 그리고 목록의 const Delivery를
    //     저장소에 옮기는 과정에서 basic_string(const basic_string&) overload가 선택된다.
    // (3) 세 문자열 리터럴은 정적 수명의 const char 배열 lvalue이고 null 종료 포인터로 변환된다.
    //     initializer_list의 const 원소들은 vector 저장소로 복사되며 외부 소유권을 빌리지 않는다.
    // (4) 세 생성자는 반환값 없이 문자열까지 소유하는 세 원소 Source를 완성한다.
    // (5) seed는 작성 순서를 보존하고 리터럴 저장소와 독립된 문자를 소유한다.
    // (6) 원소·문자 수에 선형이고 할당 실패가 가능하다. 실패 시 완성된 부분을 정리하며 아직 외부
    //     iterator는 없다. 생성 중 객체는 한 스레드에서만 접근한다.
    DeliveryBatchEncoder::Source seed{
        {"PKG-101", DeliveryState::queued},
        {"PKG-102", DeliveryState::in_transit},
        {"PKG-103", DeliveryState::delivered},
    };

    // seed는 lvalue이고 move(seed)는 같은 객체를 가리키는 xvalue다. 먼저 Source 값 매개변수 deliveries가
    // seed에서 이동 생성되고, 생성자 안 std::move(deliveries)의 두 번째 vector 이동이 deliveries_를 만든다.
    // 즉 추상 기계에는 두 소유권 경계가 있다. 첫 이동 뒤 seed는 유효하지만 값이 미지정이고, 두 번째
    // 이동 뒤 매개변수도 값이 미지정된 채 생성자 종료 때 파괴된다. 호출 뒤에는 encoder 멤버가 소유한다.
    DeliveryBatchEncoder encoder{std::move(seed)};

    // encode가 반환한 vector prvalue가 encoded를 직접 초기화한다. 함수 내부 NRVO와 별개로 반환
    // prvalue에서 결과 객체로의 초기화는 C++17 이후 보장 복사 생략의 대상이다.
    const DeliveryBatchEncoder::Snapshot encoded{encoder.encode()};

    // [첫 생성 계약: std::vector<WireDelivery> 복사 생성]
    // (1) 수신 copied는 생성 전이고 encoded는 세 원소를 가진 const Snapshot lvalue다.
    // (2) 선택 overload는 vector(const vector& other)다.
    // (3) other는 const lvalue이고 WireDelivery 및 그 string을 깊게 복사한다. 소유권 공유가 아니다.
    // (4) 생성자는 반환값 없이 독립 저장소를 가진 copied를 완성한다.
    // (5) 두 vector 값은 같지만 한쪽 수명 종료·변경이 다른 쪽 문자열 수명에 영향을 주지 않는다.
    // (6) 원소·문자 수에 선형이고 할당 실패 시 원본을 보존한다. 새 저장소 때문에 encoded 관찰자는
    //     무효화되지 않으며 서로 다른 객체라도 동일 외부 자원을 공유한다면 별도 동기화가 필요하다.
    const DeliveryBatchEncoder::Snapshot copied{encoded};

    // [첫 호출 계약: vector::empty()]
    // (1) 수신 copied는 세 원소를 소유하는 유효한 const Snapshot lvalue다.
    // (2) bool empty() const noexcept overload가 선택된다.
    // (3) 데이터 인자가 없고 원소 저장소나 소유권을 빌려 두지 않는다.
    // (4) size()==0 여부인 bool prvalue를 반환해 if 조건으로 사용한다.
    // (5) copied의 크기·capacity·원소와 관찰자는 그대로다.
    // (6) O(1), 무할당·무효화·무예외다. 동시 읽기는 가능하다. 일반 vector는 서로 다른 원소의 동시
    //     수정을 허용하지만 구조 변경 또는 같은 원소 읽기/쓰기와 겹치면 동기화가 필요하다.
    if (copied.empty()) {
        return 1;
    }

    // [첫 호출 계약: const vector::operator[](0)]
    // (1) 수신 copied는 empty 검사를 통과한 유효한 const Snapshot lvalue다.
    // (2) const_reference operator[](size_type pos) const overload가 선택된다.
    // (3) int prvalue 0이 size_type 값 0으로 변환되며 0<size()가 증명됐다. 소유권 이동은 없다.
    // (4) 첫 원소의 const WireDelivery&를 반환하고 그 주소를 first에 저장한다.
    // (5) copied와 인자는 그대로이고 first만 해당 원소를 비소유 관찰한다.
    // (6) O(1)·무할당이지만 표준 시그니처는 noexcept를 명시하지 않는다. pos>=size()면 UB이며,
    //     반환 참조/포인터는 원소 제거·재할당·owner 파괴 뒤 무효다. copied에는 동시 쓰기가 없다.
    // 일반 포인터 타입은 null·재바인딩이 가능하지만, 선언의 오른쪽 const 때문에 first 자체는 재바인딩할
    // 수 없다. 여기서는 살아 있는 vector 원소 주소를 가리키며 copied보다 오래 보관하면 안 된다.
    const WireDelivery* const first{&copied[0]};

    // [첫 호출 계약: string::empty()]
    // (1) 수신 first->tracking_id는 살아 있는 첫 원소 안의 유효한 const string lvalue다.
    // (2) bool basic_string::empty() const noexcept overload가 선택된다.
    // (3) 데이터 인자가 없고 문자 저장소를 빌려 반환하거나 소유권을 옮기지 않는다.
    // (4) 문자 수가 0인지 나타내는 bool prvalue를 반환해 if 조건으로 사용한다.
    // (5) 문자열의 문자·크기·capacity와 모든 관찰자는 그대로다.
    // (6) O(1), 무할당·무효화·무예외다. owner 수명 안에서 유효하며 같은 문자열의 동시 쓰기는 없다.
    if (first->tracking_id.empty()) {
        return 2;
    }

    // [첫 숨은 호출 계약: copied의 두 번째 const vector range-for]
    // (1) 수신 copied는 유효한 const Snapshot lvalue다.
    // (2) const begin/end, const_iterator 역참조·전위 증가, != 또는 operator== rewrite 비교가 선택된다.
    //     iterator 구현에 따라 비교는 두 피연산자를 받는 비멤버일 수 있다.
    // (3) begin/end에는 데이터 인자가 없다. 비교에는 현재/끝 iterator lvalue 둘, 역참조·증가에는 현재
    //     iterator lvalue가 쓰이고 row는 결과 원소에 const WireDelivery&로 바인딩된다.
    // (4) begin/end의 iterator 값, 비교의 bool, 역참조의 const WireDelivery&를 사용하고 증가 반환은 버린다.
    // (5) iterator만 전진하며 copied와 원소는 그대로다.
    // (6) 전체 O(n), 무할당이다. 끝 반복자 역참조·수명 종료 뒤 접근은 UB이고 동시 쓰기는 데이터 경쟁이다.
    for (const WireDelivery& row : copied) {
        // [첫 호출 계약: std::cout 삽입 연산자 체인]
        // (1) 최초 수신 cout은 프로그램 수명의 출력 가능한 ostream lvalue다.
        // (2) string/char 비멤버 삽입과 이 w64devkit 구현에서 uint16_t인 unsigned short 정수 멤버
        //     삽입 overload가 선택된다.
        // (3) tracking_id는 const lvalue로 빌려 읽고 ':'/'\n'은 char prvalue, state_code는 값으로 읽는다.
        // (4) 각 연산은 같은 ostream&를 반환해 다음 <<의 수신으로 쓰며 마지막 반환은 버린다.
        // (5) 문자가 순서대로 기록되고 실패하면 상태 비트가 바뀔 수 있으나 row는 그대로다.
        // (6) 복잡도는 형식화와 문자 수에 좌우되고 표준은 공통 점근 상한을 두지 않는다. 예외 mask에
        //     따라 I/O 예외가 가능하며 부분 출력은 롤백되지 않는다. 레코드 단위 동시 출력은 보장하지 않는다.
        std::cout << row.tracking_id << ':' << row.state_code << '\n';
    }

    // 기계 관점에서는 enum 정수 load, 비교/분기, 문자열 문자 load와 stream buffer store가 일어날 수 있다.
    // 실제 인라이닝·간접 호출·명령 선택은 CPU, ABI, 컴파일러, 라이브러리와 최적화 옵션에 따라 달라진다.
    return 0;
}
