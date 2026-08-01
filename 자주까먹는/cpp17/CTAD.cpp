#include <iostream>  // 표준 입출력 스트림을 사용하기 위한 헤더 포함
#include <string>    // std::string 클래스를 사용하기 위한 헤더 포함
#include <typeinfo>  // typeid 연산자를 사용해 런타임에 타입 정보를 얻기 위함 (디버깅/확인용)

// 1. 템플릿 클래스 정의
template <typename T>
class DataContainer {
private:
    T data; // 템플릿 타입 T를 가지는 멤버 변수 선언

public:
    // 생성자: 넘어온 인자 v를 멤버 변수 data에 복사하여 초기화
    // (기초 복습: 멤버 초기화 리스트를 사용하는 것이 기본 생성 후 대입하는 것보다 빠름)
    DataContainer(T v) : data(v) {
        std::cout << "DataContainer 생성됨. 타입: " << typeid(T).name() << "\n";
    }

    // 데이터를 출력하는 단순한 멤버 함수
    void print() const {
        std::cout << "데이터: " << data << "\n";
    }
};

// 2. 사용자 정의 추론 가이드 (CTAD) 작성 - C++17 이상
// 문자열 리터럴(const char*)이 생성자에 들어오면, T를 std::string으로 강제 추론하라는 지시어입니다.
// 이 한 줄이 없으면 "const char*" 포인터 타입으로 추론되어 메모리 수명(Lifetime) 문제가 발생할 수 있습니다.
DataContainer(const char*) -> DataContainer<std::string>;

int main() {
    // 3. C++17 CTAD 기본 사용 (타입 명시 생략)
    // 컴파일러가 정수 '42'를 보고 DataContainer<int>로 자동 추론합니다.
    DataContainer intContainer(42); 
    intContainer.print();

    // 4. 추론 가이드가 적용된 CTAD 사용
    // "Hello C++"는 const char* 타입의 문자열 리터럴입니다.
    // 우리가 작성한 추론 가이드에 의해 DataContainer<std::string>으로 인스턴스화 됩니다.
    DataContainer stringContainer("Hello C++"); 
    stringContainer.print();

    return 0; // 프로그램 정상 종료
}