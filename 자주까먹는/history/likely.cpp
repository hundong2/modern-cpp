#include <iostream>
#include <vector>
#include <optional>
#include <expected>
#include <string>

// ==========================================
// [C++11/14] 매크로 기반의 원시적인 힌트
// ==========================================
// 표준이 없었기에 GCC/Clang의 내장 함수를 매크로로 감싸서 사용했습니다.
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

bool process_cxx11(int value) {
    // !!(x)는 값을 무조건 0 또는 1(bool)로 바꾸는 C 언어 시절의 트릭입니다.
    if (UNLIKELY(value < 0)) { // 이 조건은 거의 발생하지 않음을 컴파일러에 알림
        return false;
    }
    return true; // 99% 확률로 이 코드가 실행됨
}

// ==========================================
// [C++17] if constexpr (컴파일 타임 분기)
// ==========================================
// 런타임 분기 예측 실패 자체를 없애기 위해, 
// 타입이나 템플릿 조건에 따라 아예 런타임 'if'문을 삭제해 버립니다.
template <typename T>
void process_cxx17(T value) {
    // 릴리스 빌드 시, 조건이 안 맞는 블록은 어셈블리에서 완전히 소멸합니다.
    if constexpr (std::is_integral_v<T>) {
        std::cout << "정수 최적화 경로\n";
    } else {
        std::cout << "일반 경로\n";
    }
}

// ==========================================
// [C++20] [[likely]] / [[unlikely]] 표준 어트리뷰트 도입
// ==========================================
// 드디어 언어 표준으로 하드웨어 분기 예측 힌트가 들어왔습니다.
bool process_cxx20(int status_code) {
    // 상태 코드가 200(정상)인 경우가 압도적으로 많다고 가정
    if (status_code == 200) [[likely]] {
        // [컴파일러 최적화] 이 블록의 기계어는 분기 점프(jmp) 명령어 없이 
        // CPU가 바로 읽을 수 있게 일직선상에 배치됩니다.
        return true; 
    } 
    else if (status_code == 404) [[unlikely]] {
        // [컴파일러 최적화] 이 블록은 메모리 저 멀리(Cold section)로 유배됩니다.
        // 덕분에 CPU L1 캐시가 깨끗하게 유지됩니다.
        std::cout << "Not Found 처리\n";
        return false;
    } 
    else {
        return false;
    }
}

// ==========================================
// [C++23] std::expected를 통한 우아한 에러 핸들링
// ==========================================
// 예외(throw) 처리는 CPU 캐시를 엉망으로 만듭니다.
// std::expected는 정상 값(Hot path)과 에러 값(Cold path)을 
// 논리적으로 분리하여 분기 예측을 자연스럽게 돕습니다.
std::expected<int, std::string> process_cxx23(int id) {
    if (id > 0) [[likely]] {
        return id * 100; // 정상 로직
    } else [[unlikely]] {
        return std::unexpected("유효하지 않은 ID"); // 에러 로직
    }
}

int main() {
    // 1. 기초적인 변수 선언 (스택 메모리에 할당)
    int test_val = 200;

    // 2. C++11 방식 호출
    if (process_cxx11(test_val)) {
        std::cout << "C++11: 정상 처리\n";
    }

    // 3. C++17 방식 호출 (컴파일 타임 분기)
    process_cxx17(test_val);

    // 4. C++20 표준 방식 호출
    if (process_cxx20(test_val)) {
        std::cout << "C++20: HTTP 200 정상\n";
    }

    // 5. C++23 모나딕(Monadic) 에러 핸들링
    auto result = process_cxx23(test_val);
    
    // has_value() 내부적으로 최적화되어, 정상 패스를 빠르게 통과합니다.
    if (result.has_value()) { 
        std::cout << "C++23 결과: " << result.value() << "\n";
    } else {
        std::cout << "C++23 에러: " << result.error() << "\n";
    }

    return 0; // 프로그램이 정상적으로 운영체제에 반환값을 넘기고 종료함
}

// == execute: /modern-cpp/자주까먹는/build/history/likely ==

// C++11: 정상 처리
// 정수 최적화 경로
// C++20: HTTP 200 정상
// C++23 결과: 20000