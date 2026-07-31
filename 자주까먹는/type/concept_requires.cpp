#include <iostream>
#include <concepts>
#include <string> 

template <typename T>
concept JSONSerializable = requires(T obj) {
    { obj.to_json() } -> std::convertible_to<std::string>;
};
//1. JSONSerializable (사용자 정의 이름)
// c++내장 키워드가 아님. 우리가 JSON으로 만들수 있는 것들 이라는 의미로 직접 지어준
// concept의 이름 
// 변수 이름 짓듯이 마음대로 지을 수 있다. 
//2. to_json() (요구하는 멤버 함수)
//c++표준이 아닌 우리가 직접 정한 규칙, requires 블록 안에 적어둠으로써,
//이 객체(obj)는 반드시 to_json() 이라는 멤버 함수를 가지고 있어야 한다. 라고 컴파일러에 강제 
//3. std::convertible_to<std::string> ( 표준 변환값 검사기)
//c++표준에 포함된 기본 컨셉트중 하나, 왼쪽의 결과값이 오른쪽의 타입(std::string)으로 안전하게 변환될 수 있는지 검사 
//obj.to_json()을 실행했을 떄 나오는 반환 값이 std::string이거나, 최소한 std::string으로 문제없이 바뀔수 있는 타입
//(const char*)이어야 한다는 제약 
template<typename T>
concept HasIdAndName = requires(T obj) {
    { obj.name } -> std::convertible_to<std::string>;
    { obj.id } -> std::convertible_to<int>;
};
template<typename T>
concept HasValueTypeAndIterator = requires {
    typename T::value_type; //T내부에 value_type이라는 티입이 있는가?
    typename T::iterator; //T내부에 iterator이라는 타입이 있는가?
};

void send_to_server(JSONSerializable auto const& data) {
    std::string payload = data.to_json();
    std::cout << "Send to server " << payload << std::endl;
}
void check_information(HasIdAndName auto const& data) {
    std::cout << "[" << data.id << "]" << data.name << std::endl;
}
void check_container(HasValueTypeAndIterator auto const& data) {
    std::cout << "[succes] this type is satisfy normal standard" << std::endl;
}

struct User{
    int id = 1;
    std::string name = "yojo";
    std::string to_json() const { return R"({"user_id": 1})"; }
};
struct Dummy {
    int id = 2;
};
//1. conditional ok 
struct MyVector {
    using value_type = int;
    using iterator = int*;
};
//2.conditional fail 
struct FakeVector {
    int value_type {0};
    int iterator {0};
};
//3. Half vector 
struct HalfVector {
    using value_type = int;
};
int main() {
    User my_user;
    Dummy my_dummy;
    
    std::cout << "---sned_to_server execute result---\n";
    send_to_server(my_user);
    //send_to_server(my_dummy); //error
    check_information(my_user);
    //check_information(my_dummy); //error 
    MyVector my_vector;
    check_container(my_vector);
    //check_container(FakeVector); //error
    //check_container(HalfVector); //error 
}


// ---sned_to_server execute result---
// Send to server {"user_id": 1}
// [1]yojo
// [succes] this type is satisfy normal standard
