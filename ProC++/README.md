# this page for that books named C++ for professional.

## current ubuntu version check 

```sh
lsb_release -a
```

## update g++ version

```sh
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt install -y g++-13
```

- symbolic link setting 

```sh
sudo ln -fs /usr/bin/g++-13 /usr/bin/g++
```

## cmake 

```sh
cd build 
cmake ..
make
```

## docker build 

```sh
docker build -t my_cpp_project . 
```

## docker run

```sh
docker run --rm my_cpp_project
```

## docker exec 

```sh
 docker run -it my_cpp_project /bin/bash
```

## stduy project 

```sh
./exec.sh <c++ file name> 
```

### page 97 ( array example )

[Example Array](./ExampleArray.cpp)  

### page 98 ( array example )

[using std::array](./ExampleArray2.cpp)  

## Example std::optional 

- [optional example code](./ExampleOptional.cpp)  
- result stdout  

```sh
has value() : myoptional true
has value() : myoptional2 false
myOptional.value() = 42 
*myOptional = 42 
myOptional2.value_or(100) = 100
```

## Example Initialize List 

- `std::initializer_list`
- it's type safety.

- [Example code for initializer list](./ExampleInitList.cpp)  
  
```sh
a = 15, b = 55
```
## Designated Initializer 

- [Example code header](./ExampleDesignated.h)  
- [Example Execute code](./ExampleInit.cpp)  

## Pointer

```c++
int* myIntegerPointer { nullptr };
myIntegerPointer = new int; 
```

- if you want access to value, using dereference 

```c++
*myIntegerPointer = 8;
```

- refresh memory of pointer 

```c++
delete myIntegerPointer;
myIntegerPointer = nullptr;
```

- using reference ( & ) for initialize pointer

```c++
int i { 8 };
int* myIntegerPointer { &i };
```

- struct pointer 

```c++
Employee* anEmployee { getEmployee() };
std::cout << (*anEmployee).salary << std::cout;
```

```c++
Employee* anEmployee { getEmployee() };
std::cout << anEmployee->salary << std::endl;
```

- using short-circuiting logic for pointer 

```c++
bool isValidSalary { ( anEmployee && anEmployee->salary > 0)};
```

```c++
bool isValidSalary { (anEmployee != nullptr && anEmployee->salary > 0 )};
```
## Example Initialize array using Pointer 

[example code](./ExampleAllocateArray.cpp)  

## const 

- abbreviation of const : `constant`

```c++
const int* ip;
ip = new int[10];
ip[4] = 5; // compile error

int* const ip;
ip = new int[10];
ip[4] = 5; //compile error

int* const ip { nullptr };
ip = new int[10]; //compile error 
ip[4] = 5; //error nullptr dereferecne
```

- using initialize with pointer

```c++
int* const ip { new int[10] };
ip[4] = 5;
```

- make all pointed-to values const 

```c++
int const* const ip { nullptr };
or 
const int* const ip { nullptr };

```

### constexpr keyword

```c++
const int getArraySize() { return 32; }
int main()
{
    int myArray[getArraySize()]; //error 
}
```

- `constexpr` keyword is compile time setting 

[constexpr](./ExampleConst.cpp)  


## Example Reference &

[Example code](./ExampleReference.cpp)  

- you didn't to use this context 

```c++
int& &
or
int&*
```

## Example casting

`const_cast()`, `static_cast()`, `reinterpret_cast()`, `dynamic_cast()`, `std::bit_cast() (C++20)`  
[Example casting](./ExampleCasting.cpp)  

- `const_cast()`
  - const attribute delete method, const attribute adding method.   
  - it's contained `<utility>` library. 
    - change reference parameter to const reference.  
 
 ```sh
 as_const(obj) 
 =
 const_cast<const T&>(str)
 ```

 [Example cast](./ExampleCasting.cpp)  
 [Reference of as_const](https://en.cppreference.com/w/cpp/utility/as_const)  

## type alias

- Type alias is a way to provide a new name for an existing type.
- It can make code more readable and easier to maintain.
- In C++, you can create a type alias using the `using` keyword or the `typedef` keyword.

### Using `using` keyword

```cpp
using Integer = int;
using String = std::string;
using string = basic_string<char>
```

## type inference

- Type inference is a feature in C++ that allows the compiler to automatically deduce the type of a variable from its initializer.
- This can make code more concise and easier to read.
- The `auto` keyword is used for type inference.

### Example of type inference using `auto`

```cpp
auto x = 10; // x is deduced to be of type int
auto y = 3.14; // y is deduced to be of type double
auto str = "Hello, World!"; // str is deduced to be of type const char*
```

