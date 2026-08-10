// <iostream>은 표준 출력 객체 std::cout을 제공한다.
#include <iostream>
// <optional>은 값이 있거나 없는 상태를 타입으로 나타내는 std::optional을 제공한다.
#include <optional>
// <string>은 문자를 소유하는 std::string을 제공한다.
#include <string>
// <utility>는 이동을 요청하는 std::move를 제공한다.
#include <utility>
// <vector>는 연속 메모리에 객체를 소유하는 가변 길이 컨테이너를 제공한다.
#include <vector>

// struct의 기본 접근은 public이어서 단순 값 객체에 알맞다.
struct Record {
    int id{};              // int 멤버를 중괄호로 0 초기화한다.
    std::string name{};    // string 멤버가 자신의 문자 메모리를 소유한다.
};

// class의 기본 접근은 private이며, 인터페이스와 구현을 분리하는 추상 기반 클래스이다.
class Repository {
public:
    // 생성자와 소멸자에는 반환형이 없다. 가상 소멸자는 기반 포인터로 파생 객체를 안전하게 파괴한다.
    virtual ~Repository() = default;
    // const int은 값 매개변수라 호출자의 값과 별개이고, 뒤 const는 객체 상태를 바꾸지 않음을 뜻한다.
    [[nodiscard]] virtual std::optional<Record> find(int id) const = 0;
};

// final은 더 파생하지 않을 구현임을 나타낸다.
class RecordStore final : public Repository {
public:
    // explicit은 vector 하나가 RecordStore로 암시적 변환되는 것을 막고 RecordStore{...} 직접 초기화만 허용한다.
    // 생성자 매개변수 records는 값으로 받아 복사 또는 이동하고, 멤버 초기화 목록에서 records_를 만든다.
    explicit RecordStore(std::vector<Record> records) : records_{std::move(records)} {}

    // override는 기반 클래스 가상 함수와 서명이 맞는지 컴파일러가 검사하게 한다.
    [[nodiscard]] std::optional<Record> find(int id) const override {
        // const Record&는 각 원소 lvalue에 복사 없이 바인딩하며 records_보다 오래 보관하지 않는다.
        for (const Record& record : records_) {
            // == 비교가 참이면 조건 분기가 선택된다.
            if (record.id == id) {
                // Record{record}는 lvalue record를 복사한 prvalue이고 optional이 그 복사본을 소유한다.
                return std::optional<Record>{Record{record}};
            }
        }
        // std::nullopt는 유효한 값이 없다는 명시적 상태이다.
        return std::nullopt;
    }

private:
    std::vector<Record> records_{}; // private 멤버가 레코드의 소유권과 수명을 관리한다.
};

int main() {
    // using 별칭은 긴 템플릿 타입 vector<Record>에 짧고 의미 있는 이름을 준다.
    using Records = std::vector<Record>;
    Records records{{1, "Ada"}, {2, "Bjarne"}}; // 중첩 중괄호로 vector와 각 Record를 직접 초기화한다.
    // std::move(records)는 lvalue를 xvalue로 바꾸며, 생성자가 내부 버퍼의 소유권을 이동할 수 있게 한다.
    const RecordStore store{std::move(records)};
    // 함수 반환 prvalue로 result를 직접 초기화하며 C++17의 복사 생략 규칙이 불필요한 임시 복사를 피한다.
    const std::optional<Record> result{store.find(2)};
    // optional의 bool 문맥 변환이 값 존재 여부를 검사하고 ->가 내부 Record를 가리킨다.
    if (result) {
        std::cout << result->name << '\n'; // << 연산자가 문자열과 개행을 출력 스트림에 삽입한다.
    }
    // 실제 호출은 가상 간접 호출이 될 수 있으나 최적화로 직접화될 수도 있으며 CPU·ABI·컴파일러 옵션에 따라 달라진다.
    return result && result->name == "Bjarne" ? 0 : 1; // && 단락 평가와 ?: 조건 연산자로 종료 코드를 고른다.
}
