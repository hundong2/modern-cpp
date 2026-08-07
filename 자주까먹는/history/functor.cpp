#include <iostream>
#include <vector>

// [1] Functor (함수 객체) 정의: C++98부터 사용되던 전통적인 방식
struct Parent_Double {
    // 반환타입이 void이고 double 인자를 받는 operator() 연산자 오버로딩
    void operator()(double d) const { 
        // 상태를 변경하지 않으므로 const를 붙이는 것이 최적화 및 안전성에 좋습니다.
        std::cout << "Functor 처리 결과: " << d * 2.0 << "\n"; 
    }
};

// [2] 배열 접근 연산자 오버로딩 예시
class CustomArray {
private:
    std::vector<double> data; // 실제 데이터를 담을 컨테이너
public:
    // 생성자: 초기 리스트를 받아 벡터를 초기화합니다. (C++11 기능)
    CustomArray(std::initializer_list<double> init) : data(init) {}

    // operator[] 오버로딩: 객체를 배열처럼 [index]로 접근할 수 있게 합니다.
    // 반환형이 'double&' (레퍼런스)이므로 값을 읽고 쓸 수 있습니다.
    double& operator[](size_t index) {
        return data[index];
    }
};

int main() {
    // --- [Functor 사용 예시] ---
    
    // 1. 객체를 변수에 할당한 후 호출
    Parent_Double functor_instance; // 메모리에 빈 객체 생성 (실제 크기는 1바이트)
    functor_instance(1.2);          // operator() 호출. 출력: 2.4

    // 2. 임시 객체를 생성하며 즉시 호출 (질문하신 내용의 올바른 문법)
    Parent_Double{}(3.14);          // 출력: 6.28

    // 3. C++11 람다(Lambda)와의 비교
    // 컴파일러는 아래 람다를 보면 내부적으로 'Parent_Double'과 완벽히 동일한 이름 없는 struct를 생성합니다.
    auto lambda_double = [](double d) { 
        std::cout << "Lambda 처리 결과: " << d * 2.0 << "\n"; 
    };
    lambda_double(5.5);             // 출력: 11.0


    // --- [operator[] 사용 예시] ---
    CustomArray arr = { 1.1, 2.2, 3.3 }; // 객체 생성
    arr[1] = 9.9;                        // operator[]가 호출되어 내부 vector의 두 번째 요소가 수정됨
    
    std::cout << "배열 접근 결과: " << arr[1] << "\n"; // 출력: 9.9

    return 0;
}