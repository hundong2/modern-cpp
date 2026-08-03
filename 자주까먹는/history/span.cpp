#include <iostream>
#include <span>       // C++20 std::span을 사용하기 위한 헤더
#include <vector>
#include <array>
#include <type_traits> // 메타 함수 사용을 위한 헤더

// 1. 템플릿 메타 프로그래밍과 span의 결합
// Extent 인자는 기본값으로 std::dynamic_extent를 가집니다.
// C 배열이나 std::array가 들어오면 컴파일러가 자동으로 Extent를 고정된 상수로 추론합니다!
template <typename T, std::size_t Extent = std::dynamic_extent>
void process_data(std::span<T, Extent> data) {
    
    // 2. 컴파일 타임 분기 (if constexpr - C++17)
    // Extent가 동적 크기가 아니라면 (즉, 컴파일 타임에 크기를 안다면)
    if constexpr (Extent != std::dynamic_extent) {
        
        std::cout << "[정적 크기 최적화 발동] 컴파일 타임 길이: " << Extent << "\n";
        
        // 메모리 크기를 나타내는 변수가 없으므로(Zero-cost), 하드웨어는 이를 단순 주소 연산으로 바꿉니다.
        // 이 루프는 Release 모드에서 완전히 언롤링(Unrolling)되거나 SIMD 명령어로 치환됩니다.
        for (std::size_t i = 0; i < Extent; ++i) {
            data[i] *= 2; // 각 원소에 2를 곱함 (기초: data[i]는 내부적으로 *(data.data() + i)와 동일)
        }
        
    } else {
        
        std::cout << "[동적 크기 폴백] 런타임 길이: " << data.size() << "\n";
        
        // 크기를 런타임 변수(data.size())에서 읽어와야 하므로 전통적인 조건 비교 루프를 생성합니다.
        for (auto& item : data) { // 범위 기반 for 루프 (C++11) - 내부적으로 begin(), end() 사용
            item *= 2;
        }
    }
}

int main() {
    // 3. 다양한 연속 메모리 컨테이너 준비
    int c_array[4] = {1, 2, 3, 4};                                // C 스타일 배열
    std::array<int, 4> cpp_array = {5, 6, 7, 8};                  // C++11 array
    std::vector<int> cpp_vector = {9, 10, 11, 12};                // C++98 vector

    // 4. process_data 호출
    // span은 값(Value)으로 전달하는 것이 원칙입니다! 참조(&)로 넘기면 
    // 레지스터에 바로 담길 수 있는 8~16바이트 데이터에 포인터 간접 참조(Indirection) 비용이 추가됩니다.

    std::cout << "--- C 배열 전달 ---\n";
    // 배열의 길이 4를 컴파일러가 추론하여 process_data<int, 4>로 인스턴스화 됨
    process_data(std::span{c_array}); 

    std::cout << "\n--- std::array 전달 ---\n";
    // 역시 컴파일 타임 상수 4를 가지므로 process_data<int, 4> 발동
    process_data(std::span{cpp_array}); 

    std::cout << "\n--- std::vector 전달 ---\n";
    // vector는 힙(Heap) 메모리를 쓰며 크기가 런타임에 변하므로 
    // process_data<int, std::dynamic_extent> 로 인스턴스화 됨
    process_data(std::span{cpp_vector}); 

    return 0; // 프로그램 정상 종료
}
