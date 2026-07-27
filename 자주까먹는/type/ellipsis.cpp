#include <iostream>
#include <variant>
#include <string>
#include <vector>


// ==============================================================================
// [기술 1] 매크로에서의 ... (Variadic Macro)
// 전처리기가 코드를 텍스트 단위로 복사+붙여넣기 할 때 사용합니다.
// ==============================================================================
// ... 로 들어온 모든 텍스트를 __VA_ARGS__ 자리에 그대로 쏟아붓습니다.
#define LOG_SYSTEM(Prefix, ...) \
    std::cout << "[System " << Prefix << "] ";\
    modern_print(__VA_ARGS__);

// ==============================================================================
// [기술 2] C++17 Fold Expression (단접기)를 이용한 가변 인자 풀기
// 과거처럼 템플릿 재귀 호출을 하지 않고, 한 줄로 가변 인자를 전개합니다.
// ==============================================================================
// 1. 포장하기 (Pack): typename... Args -> "몇 개가 올지 모르는 타입들을 Args 보따리에 넣어라"
//modern_print(10, 3.14, "C")
//... Args = [int, double, const char*]
//Args... = void modern_print(int, double, const char*)
//(std::cout << ... << args) << std::endl;
// std::cout << 10 << 3.14 << "C" << std::endl;
template<typename... Args>
void modern_print(Args... args)
{
    // 3. 풀기 (Expand): ( ... 연산자 args )
    // args 보따리를 풀면서 그 사이에 << 연산자를 집어넣어 한 줄로 코드를 전개(Fold)합니다.
    // 컴파일러는 이를 (std::cout << arg1 << arg2 << arg3) 처럼 자동 번역합니다.
    (std::cout << ... << args) << std::endl;
}

// ==============================================================================
// [기술 3] std::visit을 위한 overloaded 패턴 (다중 상속 흑마법)
// 여러 개의 람다(Lambda) 함수를 하나의 구조체로 뭉칠 때 사용합니다.
// ==============================================================================
template <typename... Ts> 
struct overloaded : Ts... {       // 보따리를 풀어 Ts 안에 있는 모든 람다 타입을 '다중 상속' 받습니다.
    using Ts::operator()...;      // 보따리를 풀어 상속받은 모든 operator() 함수를 내 것처럼 씁니다.
};
// 템플릿 타입 추론 가이드: 생성자에 들어온 객체들을 보고 Ts 보따리의 타입을 알아서 맞춰라.
template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int main()
{
    std::cout << "... example" << std::endl; 
    LOG_SYSTEM("INIT", "서버 부팅 완료. ", "포트 번호: ", 8080, ", 응답 시간: ", 1.5, "ms");
    LOG_SYSTEM("WARN", "접속자 수 초과: ", 9999, "명");

    // 2. overloaded 흑마법과 std::visit 테스트
    using EntityVariant = std::variant<int, std::string, double>;
    std::vector<EntityVariant> data_stream = { 404, std::string("Hello C++"), 3.14159 };

    std::cout << "\n[데이터 스트림 다형성 처리 시작]\n";
    for (const auto& data : data_stream) {
        // 배열 안에 들어있는 타입이 무엇인지 런타임에 확인하고, 
        // 오버로딩된 람다 함수 중 일치하는 것을 찾아 실행합니다.
        std::visit(overloaded {
            [](int i) { modern_print("정수 처리됨: ", i, " (에러 코드인가?)"); },
            [](const std::string& s) { modern_print("문자열 처리됨: ", s); },
            [](double d) { modern_print("실수 처리됨: ", d, " (좌표 데이터)"); }
        }, data);
    }
}