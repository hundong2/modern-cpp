/**
this example optional reference 
*/

#include <functional>
#include <iostream>
#include <optional>
#include <utility>

class A {
 public:
  int data;
};

int main() {
      A a;
      a.data = 5;

      // maybe_a 는 a 의 복사복이 아닌 a 객체 자체의 레퍼런스를 보관하게 된다.
      std::optional<std::reference_wrapper<A>> maybe_a = std::ref(a);

      maybe_a->get().data = 3;
    

      // 실제로 a 객체의 data 가 바뀐 것을 알 수 있다.
      std::cout << "a.data : " << a.data << std::endl;
}