#include <iostream>     // 표준 입출력 스트림
#include <type_traits>  // std::extent_v 등 메타 함수가 정의된 헤더

// 1. 다차원 배열 타입 별칭(Alias) 선언
// 기초: 3행 4열짜리 2차원 정수 배열 '타입'입니다. 변수가 아닙니다!
using MatrixType = int[3][4]; 

// 2. 템플릿 함수: 배열을 참조(&)로 받아 Decay(포인터 붕괴)를 막음
template <typename T>
void print_array_info(T& arr) {
    // 3. std::extent_v 활용
    // T가 어떤 타입이든 첫 번째 차원(인덱스 0, 행)의 길이를 구합니다.
    // 두 번째 템플릿 인자를 생략하면 기본값인 0이 들어갑니다.
    constexpr size_t rows = std::extent_v<T, 0>;
    
    // 두 번째 차원(인덱스 1, 열)의 길이를 구합니다.
    constexpr size_t cols = std::extent_v<T, 1>;

    // 4. 컴파일 타임 검증 (C++11 ~ C++17)
    // constexpr 변수이므로 static_assert를 통해 컴파일 타임에 에러를 잡을 수 있습니다.
    // 이는 런타임 오버헤드가 완벽히 '0'임을 증명합니다.
    static_assert(rows > 0, "배열은 최소 1개 이상의 요소를 가져야 합니다.");

    std::cout << "배열 정보 - 행: " << rows << ", 열: " << cols << "\n";

    // 5. 하드웨어 최적화 친화적인 순회 (Release 모드에서 완전 언롤링 가능)
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            std::cout << arr[i][j] << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    // 6. 실제 데이터 메모리 할당 (스택 영역)
    MatrixType my_matrix = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };

    std::cout << "--- std::extent_v를 이용한 컴파일 타임 크기 추출 ---\n";
    // 7. 배열을 템플릿 함수에 전달. 
    // 컴파일러는 T를 'int[3][4]' 타입으로 완벽하게 추론합니다.
    print_array_info(my_matrix);

    return 0; // 정상 종료
}

