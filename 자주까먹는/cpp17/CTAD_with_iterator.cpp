#include <iostream>
#include <vector>
#include <iterator>
// C++20: 타입 제약 조건(Concepts)을 사용하기 위한 헤더
#include <concepts> 

// ---------------------------------------------------------
// [템플릿 클래스 정의]
// ---------------------------------------------------------
template <typename T>
class DataBox {
public:
    // 1. 단일 값(value)을 받는 생성자
    explicit DataBox(const T& value) : data_{value} {}

    // 2. [핵심 추가] 두 개의 반복자(begin, end)를 받아 내부 컨테이너를 초기화하는 생성자!
    // C++20 문법: std::input_iterator를 만족하는 타입만 이 생성자를 쓸 수 있도록 강제합니다.
    template <std::input_iterator Iter>
    DataBox(Iter first, Iter last) : data_(first, last) {
        // 내부의 T(여기서는 std::vector<int>)가 반복자를 받는 생성자를 가지고 있어야만 동작합니다.
        // vector는 (begin, end)를 받는 생성자가 있으므로 완벽하게 복사(초기화)됩니다.
    }

    // [기본 문법] const: 이 함수는 멤버 변수(data_)를 절대 수정하지 않음을 보증합니다. (Read-only)
    [[nodiscard]] const T& get() const { return data_; }

private:
    T data_;
};

// ---------------------------------------------------------
// ★ C++20 실무형 추론 가이드 (Deduction Guide) ★
// ---------------------------------------------------------
// C++20부터는 포인터(int*)인지 진짜 반복자인지 구별 없이 값의 타입을 뽑아내기 위해 
// std::iter_value_t를 사용합니다. 당신이 쓴 Iter::value_type의 완벽한 상위 호환입니다.
template <std::input_iterator Iter>
DataBox(Iter first, Iter last) -> DataBox<std::vector<std::iter_value_t<Iter>>>;

int main() {
    // ---------------------------------------------------------
    // 실제 동작 확인
    // ---------------------------------------------------------
    // 배열이 메모리에 할당됩니다.
    int raw_array[] = {1, 2, 3, 4, 5};
    
    // 1. [타입 추론]: std::begin, end는 int* (포인터)를 반환합니다. 
    //    추론 가이드가 이를 낚아채어 DataBox<std::vector<int>> 로 타입을 확정 짓습니다.
    // 2. [객체 생성]: 타입이 확정된 후, 우리가 새로 추가한 '반복자 2개를 받는 생성자'가 호출됩니다.
    //    내부의 vector(data_)가 (begin, end)로 초기화되면서 드디어 {1, 2, 3, 4, 5}가 들어갑니다!
    DataBox box3{std::begin(raw_array), std::end(raw_array)};

    // 데이터가 진짜 vector로 들어갔는지 확인합니다.
    std::cout << "추론된 벡터의 크기: " << box3.get().size() << '\n'; // 출력: 5
    std::cout << "3번째 원소: " << box3.get()[2] << '\n';           // 출력: 3

    return 0;
}