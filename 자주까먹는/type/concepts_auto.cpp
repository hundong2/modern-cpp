#include <iostream>
#include <concepts>
#include <vector>
#include <ranges>
#include <string>

// 실무에서 자주 쓰이는 C++20 Concepts 활용 예제 모음

// 1. [정수형 데이터] std::integral
// ID, 인덱스, 횟수 등 정수값만 받아야 할 때 사용합니다.
void process_id(std::integral auto id) {
    std::cout << "[정수 처리] 유효한 ID입니다: " << id << '\n';
}

// 2. [문자열 변환 가능] std::convertible_to
// const char*, std::string 등 문자열처럼 쓸 수 있는 타입만 허용합니다.
void log_message(std::convertible_to<std::string_view> auto msg) {
    std::string_view view = msg;
    std::cout << "[로그 기록] " << view << '\n';
}

// 3. [반복 가능한 컨테이너] std::ranges::range
// std::vector, std::list, 배열 등 순회(for문)가 가능한 컬렉션만 허용합니다.
// 실무에서 컨테이너 데이터를 일괄 처리할 때 가장 많이 쓰입니다.
void print_elements(std::ranges::range auto const& container) {
    std::cout << "[컬렉션 출력] 데이터: ";
    for (const auto& item : container) {
        std::cout << item << ' ';
    }
    std::cout << '\n';
}

// 4. [호출 가능한 객체] std::invocable
// 함수 포인터, 람다(Lambda), std::function 등 콜백 함수를 인자로 받을 때 사용합니다.
// 멀티스레딩이나 이벤트 핸들러(Event Handler) 작성 시 필수적입니다.
void run_callback(std::invocable auto task) {
    std::cout << "[콜백 실행] 작업을 시작합니다...\n";
    task(); // 함수 실행
    std::cout << "[콜백 실행] 작업이 완료되었습니다.\n";
}

//compile time 분기 C++17
//old : enable_if 를 이용해 여러개 오버로딩 
//if constexpr 을 사용하면 단 하나의 함수 안에서 타입에 따라 코드를 깔끔하게 분기 가능 
void print_smartly(auto const& value){
    if constexpr ( std::is_pointer_v<std::remove_reference_t<decltype(value)>>) {
        if ( value != nullptr ) {
            std::cout << "direction point value : "<< *value << std::endl;
        } else {
            std::cout << "normal value: " << value << std::endl;
        }
    }
    else {
        std::cout << "no pointer value: " << value << std::endl;
    }
}
//1. decltype(value) : 타입 추론, 괄호 안에 들어간 변수나 표현식의 정확한 타입을 그대로 가져옴
//                  매개 변수로 auto const& value 이므로 만약 int x = 10을 넘겼다면
//                  decltype(value)는 int const& ( 상수 참조형 )가 된다. 
//2. remove_reference_t<...>:  참조 떼어 내기 
//                             타입에 붙어 있는 참조 기호(& 또는 &&)를 제거해 주는 표준 라이브러리 
//                             decltype이 가져온 int const& 에서 &를 떼어내어 순수한 int const만 남김
//                             (포인터인지 검사하려면 &가 붙어 있으면 안되기 때문에 )
//3. std::is_pointer_v<...>: 포인터 판독기 
//                           안에 들어간 최종 타입이 포인터(*)인지 확인하여, 맞으면 true, 틀리면 false를 반환
//                           알맹이 타입이 int* 면 true가 되어 if constexpr안의 코드가 컴파일 되고, 일반 int면
//                           false 가 되어 else 구문이 컵파일 된다. 

int main() {
    // 1. std::integral 테스트
    process_id(1024);         // 정상: int형
    // process_id(3.14);      // 컴파일 에러! (실수는 정수가 아님)

    // 2. std::convertible_to 테스트
    log_message("System OK"); // 정상: const char* (string_view로 변환 가능)
    log_message(std::string("Warning")); // 정상: std::string
    // log_message(100);      // 컴파일 에러! (숫자는 문자열로 자동 변환 불가)

    // 3. std::ranges::range 테스트
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    print_elements(numbers);  // 정상: vector는 범위(range)임

    // 4. std::invocable 테스트
    run_callback([]() { 
        std::cout << "  -> 람다 함수가 성공적으로 호출되었습니다!\n"; 
    });                       // 정상: 람다 함수 전달

    int data = 42;
    int* ptr = &data;
    
    std::cout << "---print_smartly execute result ---\n";
    print_smartly(data);
    print_smartly(ptr);

    return 0;
}

// == execute: /modern-cpp/자주까먹는/build/concepts_auto ==

// [정수 처리] 유효한 ID입니다: 1024
// [로그 기록] System OK
// [로그 기록] Warning
// [컬렉션 출력] 데이터: 1 2 3 4 5 
// [콜백 실행] 작업을 시작합니다...
//   -> 람다 함수가 성공적으로 호출되었습니다!
// [콜백 실행] 작업이 완료되었습니다.
// ---print_smartly execute result ---
// no pointer value: 42
// direction point value : 42