#include <iostream>     // 실행 결과를 터미널에 출력하는 std::cout을 제공합니다.
#include <type_traits>  // std::decay_t와 std::is_same_v 같은 컴파일 시간 타입 도구를 제공합니다.
#include <utility>      // 소유 값을 멤버로 이동하는 std::move를 제공합니다.

template <typename T>  // Wrapper가 실제로 보관할 값 타입 T를 선언합니다.
class Wrapper {        // 서로 다른 T마다 별개의 Wrapper<T> 클래스가 컴파일 중에 만들어집니다.
public:                 // 호출자가 사용할 생성자와 학습용 멤버를 공개합니다.
    T value;            // decay가 끝난 T 타입의 값을 Wrapper 객체가 직접 소유합니다.

    explicit Wrapper(T input)        // 값으로 받아 배열·참조가 아닌 저장 가능한 T 객체를 생성합니다.
        : value{std::move(input)} {  // input이 가진 자원을 value 멤버로 이동해 불필요한 깊은 복사를 줄입니다.
    }  // 생성자 실행을 마치면 input은 파괴되고 value가 저장 값을 계속 소유합니다.
};  // Wrapper 클래스 정의를 닫습니다.

template <typename T>                        // 초기화 인수에서 원래 타입 T를 추론합니다.
Wrapper(T&&) -> Wrapper<std::decay_t<T>>;    // 참조·const를 정리하고 배열/함수를 포인터로 바꾼 타입을 저장 타입으로 선택합니다.

int calculate(double value) {  // 함수 타입이 decay될 때 함수 포인터가 되는지 확인할 일반 함수입니다.
    return static_cast<int>(value * 2.0);  // double 계산 결과를 명시적으로 int로 변환해 반환합니다.
}  // calculate 함수의 코드 범위를 닫습니다.

int main() {  // 운영체제가 프로그램을 시작할 때 호출하는 진입점입니다.
    std::cout << std::boolalpha;  // 이후 bool 값을 1/0 대신 true/false 문자열로 출력합니다.

    Wrapper first{"Hello"};  // const char[6]을 받아 deduction guide가 Wrapper<const char*>를 선택합니다.
    Wrapper second{"Hi"};    // 길이가 다른 const char[3]도 동일한 Wrapper<const char*>가 됩니다.

    static_assert(std::is_same_v<decltype(first), decltype(second)>);  // 두 객체 타입이 같은지 컴파일 중에 검증합니다.
    std::cout << "different string literals -> same Wrapper type: "   // 문자열 배열 길이가 저장 타입에서 사라짐을 설명합니다.
              << std::is_same_v<decltype(first), decltype(second)> << '\n';  // 타입이 같으므로 true를 출력합니다.
    std::cout << "stored strings: " << first.value << ", " << second.value << '\n';  // 저장한 리터럴 주소를 통해 문자열을 출력합니다.

    const int number{42};          // const int 객체 number를 현재 스택 프레임 안에 만듭니다.
    const int& reference{number};  // 새 int를 복사하지 않고 number를 읽는 const 참조를 만듭니다.

    using ReferenceType = decltype(reference);             // ReferenceType은 원래 선언 타입인 const int&입니다.
    using DecayedReference = std::decay_t<ReferenceType>;   // 참조와 바깥 const를 제거해 int를 얻습니다.

    static_assert(std::is_same_v<DecayedReference, int>);  // 예상 타입이 int인지 컴파일 중에 검사합니다.
    std::cout << "const int& -> int: "                    // 첫 번째 기본 변환의 설명을 출력합니다.
              << std::is_same_v<DecayedReference, int> << '\n';  // 컴파일 시간 비교 결과 true를 출력합니다.

    int numbers[3]{10, 20, 30};                 // 세 int가 연속으로 들어 있는 고정 길이 배열을 만듭니다.
    using ArrayType = decltype(numbers);         // ArrayType은 배열 크기까지 포함한 int[3]입니다.
    using DecayedArray = std::decay_t<ArrayType>;  // 배열 타입을 첫 원소를 가리키는 int*로 변환합니다.

    static_assert(std::is_same_v<DecayedArray, int*>);  // 배열의 decay 결과가 int*인지 컴파일 중에 확인합니다.
    std::cout << "int[3] -> int*: "                    // 두 번째 기본 변환의 설명을 출력합니다.
              << std::is_same_v<DecayedArray, int*> << '\n';  // 배열에서 포인터로 변환된 결과 true를 출력합니다.

    using FunctionType = decltype(calculate);            // FunctionType은 함수 자체 타입 int(double)입니다.
    using DecayedFunction = std::decay_t<FunctionType>;  // 함수 타입을 함수 주소를 담는 int (*)(double)로 변환합니다.

    static_assert(std::is_same_v<DecayedFunction, int (*)(double)>);  // 함수 포인터 변환 결과를 컴파일 중에 검증합니다.
    std::cout << "int(double) -> int (*)(double): "                  // 세 번째 기본 변환의 설명을 출력합니다.
              << std::is_same_v<DecayedFunction, int (*)(double)> << '\n';  // 함수 포인터 타입이면 true를 출력합니다.

    DecayedArray first_element{numbers};       // 배열 이름이 첫 원소 주소로 변환되어 int* 변수에 저장됩니다.
    DecayedFunction function_pointer{calculate};  // 함수 이름이 코드 주소를 가진 함수 포인터로 변환됩니다.

    std::cout << "first element: " << *first_element << '\n';  // 포인터를 역참조해 배열 첫 값 10을 읽습니다.
    std::cout << "function result: " << function_pointer(2.5) << '\n';  // 포인터를 통해 calculate를 호출해 5를 출력합니다.

    return 0;  // 모든 자동 객체의 수명을 끝내고 운영체제에 정상 종료를 알립니다.
}  // main 함수의 코드 범위를 닫습니다.
