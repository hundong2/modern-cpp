#include <iostream>
#include <string>
#include <tuple>
#include <map>

std::tuple<int, std::string, bool> GetStudent(int id)
{
    if(id == 0)
    {
        return std::make_tuple(30,"man", true);
    }
    else
    {
        return std::make_tuple(28,"woman", false);
    }
}

//struct binding for map
void structBindingCoutEx()
{
    std::map<int, std::string> m = {{3, "hi"}, {5, "hello"}};
    for (auto& [key, val] : m) 
    {
        std::cout << "Key : " << key << " value : " << val << std::endl;
    }
}
int main() 
{
    //normal tuple example
    std::tuple<int, double, std::string> tp;
    tp = std::make_tuple(1, 3.14, "hi");

    std::cout << std::get<0>(tp) << ", " << std::get<1>(tp) << ", "
            << std::get<2>(tp) << std::endl;

    auto student = GetStudent(1);

//  not structed binding 
    int age = std::get<0>(student);
    std::string name = std::get<1>(student);
    bool is_male = std::get<2>(student);

// structed binding using 
auto [age1, name1, is_male1 ] = student;

// not structed binding
    std::cout << "name : " << name << std::endl;
    std::cout << "age : " << age << std::endl;
    std::cout << "male : " << std::boolalpha << is_male << std::endl;

//structed binding example 
   std::cout << "name : " << name1 << std::endl;
    std::cout << "age : " << age1 << std::endl;
    std::cout << "male : " << std::boolalpha << is_male1 << std::endl;

    structBindingCoutEx();

}