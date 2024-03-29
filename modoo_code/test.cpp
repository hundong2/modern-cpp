#include <iostream>
#include <memory>
#include <variant>
#include <format>
class A {
 public:
  void a() { std::cout << "I am A" << std::endl; }
};

class B {
 public:
  void b() { std::cout << "I am B" << std::endl; }
};


//monostate : 아무것도 안들어 있거나, A, B
std::variant<std::monostate, A, B> GetDataFromDB(bool is_a) {
  if (is_a) {
    return A();
  }
  return B();
}

int main() {

	std::cout << std::format("{} test", 123) << std::endl;	
	//std::get<B>(v).b(); //'std::bad_variant_access'
}
