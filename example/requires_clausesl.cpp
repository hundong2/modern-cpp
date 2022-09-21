#include <type_traits>

constexpr bool check() 
{
    return true;
}

template<typename T> requires true
void f1(T a) {}

//template<typename> requires false void f2(T a) {}
template<typename T> requires std::is_pointer_v<T>  void f3(T a){}
//template<typename T> requires 1                     void f4(T a){} //반드시 bool 타입이어야 한다. 변환은 허용 되지 않음(int 안됨) 
template<typename T> requires (check())             void f5(T a){} 
template<typename T> requires (sizeof(T) > 4)       void f6(T a){}
int main()
{
    //f1(0);
}