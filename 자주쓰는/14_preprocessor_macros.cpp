#include <iostream>
#include <string_view>

// 전처리기(preprocessor)는 C++ 컴파일 전에 소스 코드를 텍스트로 변환합니다.
// #define은 "타입이 있는 함수"가 아니라 "텍스트 치환 규칙"입니다.
// 그래서 강력하지만 타입 검사, 스코프, 디버깅 면에서 위험할 수 있습니다.

#define PROJECT_VERSION_MAJOR 1
#define PROJECT_VERSION_MINOR 4

// # 연산자: 매크로 인자를 문자열 리터럴로 만듭니다.
// STRINGIFY(hello) -> "hello"
#define STRINGIFY_IMPL(value) #value
#define STRINGIFY(value) STRINGIFY_IMPL(value)

// 두 단계 STRINGIFY가 필요한 이유:
// STRINGIFY_IMPL(PROJECT_VERSION_MAJOR)는 "PROJECT_VERSION_MAJOR"가 됩니다.
// STRINGIFY(PROJECT_VERSION_MAJOR)는 먼저 1로 확장한 뒤 "1"이 됩니다.

// ## 연산자: 토큰을 붙여 새 토큰을 만듭니다.
// MAKE_NAME(counter, 1) -> counter1
#define MAKE_NAME(prefix, suffix) prefix##suffix

// 여러 문장을 가진 매크로는 do { ... } while (false)로 감쌉니다.
// 그래야 if/else 안에서 하나의 문장처럼 안전하게 동작합니다.
#define LOG_VALUE(name, value)                                                       \
    do {                                                                            \
        std::cout << "[log] " << (name) << " = " << (value) << '\n';                \
    } while (false)

// variadic macro: 인자 개수가 가변인 매크로입니다.
// C++20의 __VA_OPT__는 추가 인자가 있을 때만 쉼표를 넣게 해 줍니다.
#define TRACE(message, ...)                                                         \
    do {                                                                            \
        std::cout << "[trace] " << (message) __VA_OPT__(<< " | " << __VA_ARGS__)    \
                  << '\n';                                                          \
    } while (false)

// 조건부 컴파일:
// NDEBUG가 정의되면 assert 계열 코드를 빼는 식으로 오픈소스에서 자주 보입니다.
#if defined(NDEBUG)
#define DEBUG_ONLY(statement)                                                       \
    do {                                                                            \
    } while (false)
#else
#define DEBUG_ONLY(statement)                                                       \
    do {                                                                            \
        statement;                                                                  \
    } while (false)
#endif

// 컴파일러/플랫폼 차이를 감추는 매크로도 오픈소스에서 자주 보입니다.
// 동적 라이브러리 export/import, warning 제어, attribute 래핑 등에 사용합니다.
#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
#define FORCE_INLINE inline
#endif

FORCE_INLINE int AddFast(int left, int right) {
    return left + right;
}

// X-macro:
// 같은 목록을 enum, 문자열 변환, switch 등에 중복 작성하지 않기 위한 패턴입니다.
// 큰 오픈소스에서 에러 코드, 메시지 타입, 명령어 목록에 종종 등장합니다.
#define ERROR_CODE_LIST(X)                                                          \
    X(Ok, "ok")                                                                     \
    X(NotFound, "not found")                                                        \
    X(Timeout, "timeout")                                                           \
    X(PermissionDenied, "permission denied")

enum class ErrorCode {
#define DECLARE_ENUM(name, text) name,
    ERROR_CODE_LIST(DECLARE_ENUM)
#undef DECLARE_ENUM
};

std::string_view ToString(ErrorCode code) {
    switch (code) {
#define DECLARE_CASE(name, text)                                                     \
    case ErrorCode::name:                                                            \
        return text;
        ERROR_CODE_LIST(DECLARE_CASE)
#undef DECLARE_CASE
    }

    return "unknown";
}

// 위험한 매크로 예시:
#define BAD_SQUARE(x) x * x

// 안전한 함수 대안:
template <typename T>
constexpr T Square(T value) {
    return value * value;
}

int main() {
    std::cout << "version "
              << STRINGIFY(PROJECT_VERSION_MAJOR) << "."
              << STRINGIFY(PROJECT_VERSION_MINOR) << '\n';

    int MAKE_NAME(counter, 1) = 10;
    LOG_VALUE("counter1", counter1);

    TRACE("start");
    TRACE("with detail", 42);

    DEBUG_ONLY(std::cout << "debug only log\n");

    std::cout << "AddFast=" << AddFast(2, 3) << '\n';
    std::cout << "ErrorCode::Timeout=" << ToString(ErrorCode::Timeout) << '\n';

    std::cout << "BAD_SQUARE(1 + 2)=" << BAD_SQUARE(1 + 2) << '\n';
    // BAD_SQUARE(1 + 2)는 1 + 2 * 1 + 2로 치환되어 5가 됩니다.
    // 함수 템플릿 Square(1 + 2)는 인자를 먼저 계산하므로 9가 됩니다.
    std::cout << "Square(1 + 2)=" << Square(1 + 2) << '\n';

    // 실무 판단:
    // 1. 상수: constexpr 또는 enum class 우선
    // 2. 짧은 계산: inline 함수 또는 template 우선
    // 3. 로그/플랫폼 분기/조건부 컴파일: 매크로가 현실적인 경우가 있음
    // 4. X-macro: 중복 목록을 줄일 때 유용하지만 디버깅이 어려워질 수 있음
}

