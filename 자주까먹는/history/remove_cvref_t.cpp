#include <iostream>     // 표준 입출력 스트림
#include <type_traits>  // 메타 프로그래밍용 헤더 (std::decay_t, std::remove_cvref_t, std::extent_v)

// 1. 과거 방식 (C++11/14 스타일): std::decay_t 사용
// T&& 보편적 참조로 배열을 받습니다.
template <typename T>
void process_with_decay(T&& arg) {
    // 배열이 들어와도 'const int*' (포인터)로 붕괴(Decay)됩니다.
    using DecayedType = std::decay_t<T>;
    
    if constexpr (std::is_pointer_v<DecayedType>) {
        // unused-parameter 경고를 없애기 위해 arg를 직접 출력에 사용합니다.
        // 포인터로 붕괴되었지만 메모리 주소는 유지되므로 첫 번째 원소에는 접근 가능합니다.
        std::cout << "[Decay] 포인터로 붕괴됨. 첫 원소 값: " << arg[0] << "\n";
    }
}

// 2. 모던 방식 (C++20 스타일): std::remove_cvref_t와 std::extent_v 사용
template <typename T>
void process_with_remove_cvref(T&& arg) {
    // const와 참조(&)만 제거하고 순수 배열 타입(int[4])을 얻어냅니다.
    using CleanType = std::remove_cvref_t<T>;
    
    // 타입이 배열인지 확인합니다.
    if constexpr (std::is_array_v<CleanType>) {
        // [🔥핵심 에러 수정 지점🔥]
        // sizeof 트릭 대신, C++17부터 제공되는 std::extent_v를 사용하여 
        // "타입(CleanType)" 자체에서 배열의 크기를 컴파일 타임 상수로 안전하게 추출합니다.
        constexpr size_t array_length = std::extent_v<CleanType>;
        
        std::cout << "[Remove_cvref] 배열 타입/크기 보존! 길이: " << array_length << "\n";
        std::cout << "데이터 병렬 처리 시작: ";
        
        // unused-parameter 경고 해결 및 하드웨어 최적화를 위한 순회 코드
        for (size_t i = 0; i < array_length; ++i) {
            std::cout << arg[i] << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    // C 스타일 배열 선언. 타입은 'const int[4]' 입니다.
    const int my_data[4] = {10, 20, 30, 40};

    std::cout << "--- 1. std::decay_t 테스트 ---\n";
    process_with_decay(my_data); // my_data가 포인터로 붕괴하여 전달됨

    std::cout << "\n--- 2. std::remove_cvref_t 테스트 ---\n";
    process_with_remove_cvref(my_data); // my_data의 배열 속성(크기 4)이 완벽히 보존됨

    return 0; // 정상 종료
}