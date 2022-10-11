#include <iostream>
#include <memory>
#include <variant>

class A {
 public:
  void a() { std::cout << "I am A" << std::endl; }
};

class B {
 public:
  void b() { std::cout << "I am B" << std::endl; }
};

std::variant<A, B> GetDataFromDB(bool is_a) {
  if (is_a) {
    return A();
  }
  return B();
}

int main() {
  auto v = GetDataFromDB(true);

  std::cout << v.index() << std::endl;
  std::get<A>(v).a();  // 혹은 std::get<0>(v).a()
	
	std::get<B>(v).b(); //'std::bad_variant_access'
}