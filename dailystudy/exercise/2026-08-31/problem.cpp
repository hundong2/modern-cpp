#include <cassert>      // 학습용 후조건을 assert로 검사한다.
#include <format>       // std::format으로 타입 안전한 사용자 메시지를 만든다.
#include <iostream>     // std::cout 표준 출력 객체를 선언한다.
#include <string>       // 결과 문자를 소유하는 std::string을 선언한다.
#include <string_view>  // 소유하지 않는 문자열 입력 std::string_view를 선언한다.
#include <utility>      // std::move 값 범주 변환을 선언한다.

// 값만 운반하는 struct는 기본 public이다. designated initializer로 각 필드의 뜻을 이름으로 드러낼 수 있다.
struct StorageSnapshot {
    std::string_view volume_name{};  // 리터럴을 빌려 보므로 snapshot이 문자를 소유하지 않는다.
    int used_gib{};                  // 사용량을 담는 기본 정수이며 {}는 0 초기화다.
    int capacity_gib{};              // 전체 용량을 담는 기본 정수이며 0보다 커야 한다.
};

// class의 기본 private 접근으로 접두어 문자열과 표현 규칙을 캡슐화한다.
class StoragePresenter {
public:
    // 생성자에는 반환형이 없다. explicit은 string에서 presenter로의 뜻밖의 암시 변환을 금지한다.
    explicit StoragePresenter(std::string prefix)
        // 값 매개변수를 xvalue로 바꿔 prefix_가 버퍼 소유권을 이동받게 한다.
        : prefix_{std::move(prefix)} {}

    // snapshot은 const lvalue 참조로 빌려 읽고 presenter도 const라 멤버 상태를 바꾸지 않는다.
    [[nodiscard]] std::string present(const StorageSnapshot& snapshot) const {
        assert(snapshot.capacity_gib > 0);
        assert(snapshot.used_gib >= 0 && snapshot.used_gib <= snapshot.capacity_gib);

        // 정수 나눗셈 전에 100을 곱한다. 현재 제약은 작은 양수라 int 오버플로가 없다는 전제다.
        const int percent{snapshot.used_gib * 100 / snapshot.capacity_gib};

        // format(format_string<Args...>, Args&&...)에 리터럴과 prefix_ const string lvalue,
        // volume_name string_view lvalue, 세 int lvalue를 순서대로 준다. 인자는 모두 읽기만 하고 유지된다.
        // 반환 string이 결과 메모리를 소유하며 결과 길이 O(L) 시간·공간과 할당 가능성이 있다.
        return std::format("{} volume={} usage={}/{}GiB ({}%)",
                           prefix_, snapshot.volume_name, snapshot.used_gib,
                           snapshot.capacity_gib, percent);
    }

private:
    std::string prefix_;  // presenter가 소유하는 private 멤버라 객체 수명 동안 유효하다.
};

int main() {
    // explicit 생성자는 직접 초기화로 호출한다. string 임시의 버퍼가 presenter로 이동될 수 있다.
    const StoragePresenter presenter{std::string{"ALERT"}};
    // designated initializer는 선언 순서대로 이름과 값을 대응한다. string_view는 정적 리터럴을 본다.
    const StorageSnapshot snapshot{.volume_name = "data", .used_gib = 72, .capacity_gib = 80};

    // present가 반환한 string prvalue가 message를 직접 초기화해 결과 문자 수명을 main 끝까지 소유한다.
    const std::string message{presenter.present(snapshot)};
    // string operator==는 두 문자 시퀀스를 읽어 같은지 bool로 반환한다. 두 문자열은 유지되고 길이에 선형이다.
    // assert는 그 반환값이 false면 진단 후 종료할 수 있으며 NDEBUG에서는 비교 식도 제거된다.
    assert(message == "ALERT volume=data usage=72/80GiB (90%)");

    // 삽입 연산자는 cout 버퍼를 바꾸고 같은 ostream&를 반환한다. message는 읽기만 하며 그대로 유지된다.
    std::cout << message << '\n';
    return 0;
}
