#include <iostream>

template<typename T> 
struct my_remove_reference { 
    using type = T; 
};

// 2. 특수화 (좌측값 참조 '&'가 있을 때)
template<typename T> 
struct my_remove_reference<T&> { 
    using type = T; 
};

// 3. 특수화 (우측값 참조 '&&'가 있을 때)
template<typename T> 
struct my_remove_reference<T&&> { 
    using type = T; 
};

// 타이핑을 줄이기 위한 별칭 (C++14의 _t 스타일)
template<typename T>
using my_remove_reference_t = typename my_remove_reference<T>::type;
//C++20 미만에서는 typename이 없으면 error 발생 
//typename my_remove_reference<T>::type; // error: missing 'typename' prior to dependent type name 'my_remove_reference<T>::type'


template <typename T> //template typename T 
constexpr my_remove_reference_t<T>&& my_move(T&& arg) noexcept {
    // 1. T에 붙은 &, &&를 모두 뜯어내서 순수한 타입을 알아냅니다.
    // 2. 그 순수한 타입 뒤에 &&를 붙여 완벽한 우측값 참조(rvalue reference)로 만듭니다.
    // 3. 넘어온 arg를 그 타입으로 static_cast(강제 형변환) 합니다.
    return static_cast<my_remove_reference_t<T>&&>(arg);
}
//1. constexpr 컴파일 타임 최적화 
//이 함수는 너무 단순해서 프로그램이 실행 될 때 ( Run-time) 계산할 필요도 없어, 
//컴파일 할 때 미리 결과를 계싼(inline)해서 코드에 박아 넣을 수 있다.

//2. my_remove_reference_t<T>&& 
//T에 &나 &&가 붙어 있으면 싹 다 잘라내고 순수한 타입만 남긴다. 
//이후 리턴 값으로 &&를 붙여서 rvalue reference로 만들어 준다.

//3. noexcept 
//이 함수는 절대 예외를 던지지 않는다.
//최적화에 대한 힌트로 컴파일러에게 알려주는 것이다.


class TestItem {
public:
    std::string name;

    // 기본 생성자
    TestItem(std::string n) : name(n) {
        std::cout << "[생성] " << name << "\n";
    }

    // 복사 생성자 (const T&)
    TestItem(const TestItem& other) : name(other.name + "_복사본") {
        std::cout << "[복사됨] 원본 '" << other.name << "' 보존됨. 비용 발생!\n";
    }

    // 이동 생성자 (T&&)
    TestItem(TestItem&& other) noexcept : name(other.name + "_이동됨") {
        std::cout << "[이동됨] 원본 '" << other.name << "' 에서 자원 훔쳐옴! (Zero-Copy)\n";
        other.name = "빈 껍데기"; // 원본 데이터 탈취 표시
    }
};

int main()
{
    std::cout << "--- 1. 객체 준비 ---\n";
    TestItem item1("전설의검"); // 이름이 있는 lvalue입니다.


    std::cout << "\n--- 2. 일반 대입 (복사 발생) ---\n";
    // lvalue를 그냥 대입하면 복사 생성자가 호출됩니다.
    TestItem item2 = item1; 


    std::cout << "\n--- 3. 우리가 만든 my_move 사용 (이동 발생) ---\n";
    // item1은 lvalue지만, 우리가 만든 my_move를 거치면서 완벽하게 rvalue로 변신합니다.
    // 결과적으로 복사 생성자가 아닌 '이동 생성자'가 성공적으로 호출됩니다!
    TestItem item3 = my_move(item1); 


    std::cout << "\n--- 4. 최종 상태 확인 ---\n";
    std::cout << "item1 (원본): " << item1.name << "\n";
    std::cout << "item2 (사본): " << item2.name << "\n";
    std::cout << "item3 (탈취): " << item3.name << "\n";

    return 0;
    return 0;
}

// --- 1. 객체 준비 ---
// [생성] 전설의검

// --- 2. 일반 대입 (복사 발생) ---
// [복사됨] 원본 '전설의검' 보존됨. 비용 발생!

// --- 3. 우리가 만든 my_move 사용 (이동 발생) ---
// [이동됨] 원본 '전설의검' 에서 자원 훔쳐옴! (Zero-Copy)

// --- 4. 최종 상태 확인 ---
// item1 (원본): 빈 껍데기
// item2 (사본): 전설의검_복사본
// item3 (탈취): 전설의검_이동됨