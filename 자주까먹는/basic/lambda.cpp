#include <iostream>

// [] is lambda capture list
//[=] captures all local variables by value
//[&] captures all local variables by reference
//[x] captures only the variable x by value
//[&x] captures only the variable x by reference

void lambda_capture_example();
void lambda_capture_by_working_example();
void reference_example();
void copy_example();
int main() {
    auto add = [](int a, int b) { return a + b;};
    std::cout << "lambda result: " << add(3, 4) << std::endl;
    std::cout << "lambda capture example: " << std::endl;
    lambda_capture_example();
    lambda_capture_by_working_example();
    reference_example();
    copy_example();

}

// this example error occurs if you try to capture 
// int x = 10;
//auto func = []() { cout << x; }; // error: x is not captured correctly
//auto func = []() { cout << "Hello"; }; // not error 

// Example showing capturing by value and by reference
// Demonstrates how lambda functions can capture variables by value and by reference
// you can change how variables are captured by modifying the capture list
void lambda_capture_example() {
    int x = 10;
    auto func_by_value = [x]() { std::cout << x << std::endl; }; // captures x by value
    auto func_by_reference = [&x]() { x++; std::cout << x << std::endl; }; // captures x by reference
    func_by_value();
    func_by_reference();
}

void lambda_capture_by_value_example() {
    int x = 10;
    auto func_by_value = [x]() { std::cout << x << std::endl; }; // captures x by value
    func_by_value();
}

void lambda_capture_const_variable_example() {
    int x = 10;
    auto func_by_value = [=]() { 
            std::cout << x << std::endl; 
            //x = 20; x is captured by value, so modifying it here will not affect the original x
        }; // captures x by value
    //modify x inside the lambda will not affect the original x because it is captured by value
    func_by_value();
    
}

void lambda_capture_by_working_example() {
    int total = 0;
    int factor = 10;
    auto calc = [factor, &total](int val) {
        total += val * factor;
    }; // captures factor by value and total by reference
    calc(5); // example usage of the lambda function
    std::cout << "total after calculation: " << total << std::endl;
}

//Example showing capturing the this pointer in a lambda function within a class
//this is an example of capturing the this pointer in a lambda function within a class
class ReferenceExample {
    int value;
public:
    ReferenceExample(int v) : value(v) {}
    void printValue() {
        auto func = [this]() { std::cout << value << std::endl; }; // captures this pointer to access member variable
        func();
    }
};
void reference_example() {
    ReferenceExample example(42);
    example.printValue();
}

// Example showing capturing a copy of the this pointer in a lambda function within a class
//*this captures a copy of the this pointer in the lambda function */
//safe way to capture a copy of the this pointer in the lambda function
//safe multithreading because the lambda captures a copy of the this pointer
class CopyExample {
    int value;
public:
    CopyExample(int v) : value(v) {}
    void printValue() {
        auto func = [*this]() {  std::cout << value << std::endl; }; // captures a copy of the member variable
        func();
    }
};

void copy_example() {
    CopyExample example(42);
    example.printValue();
}