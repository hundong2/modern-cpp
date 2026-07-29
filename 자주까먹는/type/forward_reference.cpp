#include <iostream>
#include <concepts>

void process_normal(std::string&& val) {
    std::cout << "not forward reference" << std::endl;
}

void process_normal_for_forwarding(auto&& val) {
    std::cout << "forwarding reference" << std::endl;
}

int main(){
    std::string my_str = "value";
    
    process_normal(std::string("Temp"));
    process_normal(std::move(my_str));
    
    //process_normal(my_str);
    
//     main.cpp: In function ‘int main()’:
// main.cpp:14:20: error: cannot bind rvalue reference of type ‘std::string&&’ {aka ‘std::__cxx11::basic_string&&’} to lvalue of type ‘std::string’ {aka ‘std::__cxx11::basic_string’}
//   14 |     process_normal(my_str);
//       |                    ^~~~~~
// main.cpp:4:35: note:   initializing argument 1 of ‘void process_normal(std::string&&)’
//     4 | void process_normal(std::string&& val) {
//       |                     ~~~~~~~~~~~~~~^~~
    process_normal_for_forwarding(std::string("Temp"));
    process_normal_for_forwarding(std::move(my_str));
    
    process_normal_for_forwarding(my_str);

    //output
    // forwarding reference
    // forwarding reference
    // forwarding reference

}
