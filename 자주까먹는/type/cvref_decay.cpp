#include <type_traits>

// 테스트를 위한 함수 시그니처
void my_function() {}

int main() {
    // ---------------------------------------------------------
    // 1. 일반적인 변수의 경우 (차이 없음)
    // ---------------------------------------------------------
    using NormalType = const int&;
    
    // 둘 다 순수한 int 타입으로 깔끔하게 벗겨냅니다.
    static_assert(std::is_same_v<std::remove_cvref_t<NormalType>, int>);
    static_assert(std::is_same_v<std::decay_t<NormalType>, int>);

    // ---------------------------------------------------------
    // 2. 배열의 경우 (결정적 차이 발생!)
    // ---------------------------------------------------------
    using ArrayType = int(&)[5]; // 길이가 5인 int 배열의 참조형
    
    // remove_cvref_t: 참조(&)만 떼어내고, '길이가 5인 배열'이라는 정체성을 유지합니다.
    static_assert(std::is_same_v<std::remove_cvref_t<ArrayType>, int[5]>); 
    
    // decay_t: 배열 붕괴(Array Decay)가 발생하여 크기(5)를 잃고 포인터(int*)로 바뀝니다.
    static_assert(std::is_same_v<std::decay_t<ArrayType>, int*>);

    // ---------------------------------------------------------
    // 3. 함수의 경우 (함수 포인터 변환)
    // ---------------------------------------------------------
    using FuncType = void(&)(); // 반환값이 없는 함수의 참조형
    
    // remove_cvref_t: 참조(&)만 떼어내고 '함수 타입(void())' 자체를 반환합니다.
    static_assert(std::is_same_v<std::remove_cvref_t<FuncType>, void()>);
    
    // decay_t: 우리가 흔히 콜백으로 넘길 때 쓰는 '함수 포인터(void(*)())'로 변환시킵니다.
    static_assert(std::is_same_v<std::decay_t<FuncType>, void(*)()>);

    return 0;
}