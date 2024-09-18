# this page for that books named C++ for professional.

## Main Reference ( book information )

[book information - Code/Exercises code reference](https://www.wiley.com/en-kr/Professional+C%2B%2B%2C+5th+Edition-p-9781119695547)  


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

## cmake exmaple

[example cmake information](https://gist.github.com/luncliff/6e2d4eb7ca29a0afd5b592f72b80cb5c)  

## Example String 

- `#include <cstring>` : c type calculator  
- [C type string array](./ExampleString.cpp)  
- `strcpy_s()`, `strcat_s()` secure C Library, it defined `ISO/IEC TR 24731`  

### string literal 

- string literal is saved in memory ( readonly area )  
- compiler using same literal coding in the same memory area. 
- `literal pooling` : using literal that name of `hello` called 500 numbers, but it saved just one memory area.  

- don't predict result. 

```c++
char *ptr { "hello" };
ptr[1] = 'a';
```

- using const literal   

```c++
const char* ptr { "hello" };
prt[1] = 'a' //occur error!!! 
```

### raw string literal 

- Raw string literals in C++ allow you to include characters that would otherwise need to be escaped, such as quotes or backslashes. They are particularly useful for including multi-line strings or strings with special characters.  

```c++
const char* str { "Hello "world"!"}; //occur error 

const char* str { "Hello \"world\"!" }; //is OK

const char* str { R"(Hello "World"!)"}; //is OK
```

- escape sequence ( `\n` )

```c++
const char* str { "Line 1\nLine 2" };

smae 

const char* str { R"(Line 1
Line 2)"};
```

```c++
const char* str { R"(Is the following a tab character? \t)" };
//Is the following a tab character? \t
```

### string example 

[ExampleString.cpp exampleString3()](./ExampleString.cpp)  


### three-way comparison operator 

[ExampleString.cpp exampleString4() ](./ExampleString.cpp)  

### string calculator 

- `substr(pos, len)` : substring return pos ~ pos + len 
- `find(str)` : substring position return, if not then `string::npos` returns.
- `replace(pos, len, str)` : replace string from pos to pos + len.  
- `starts_with(str)/end_with(str)` : if matching start/ end substring with str parameter, then returns true values.  

[ExampleString.cpp exampleString5()](./ExampleString.cpp)   

### string literal 

```c++
auto string1 { "Hello World" }; //string1 type is const char* 
auto string2 { "Hello World"s }; //string2 type is std::string -> using namespace std::literals::string_literals
```

#### inline namespace 

```c++
using namespace std::string_literals;
using namespace std::literals::string_literals;
```

```c++
namespace std {
  inline namespace literals {
    inline namespace string_literals {
      //...
    }
  }
}
```

#### std::vector with CTAD ( Class Template Argument deduction, after c++17 )  

```c++
vector names { "John", "Sam", "Joe" }; //vector<const char*>

vector names { "John"s, "Sam"s, "Joe"s }; //vector<std::string>
```

#### changing numeric to string 

```c++
string to_string(T val);
```

- example long double to string 

```c++
long double d { 3.14L };
string s { to_string(d) };
```

#### change string to numeric  

- first space string ignore 
- if change fail then `invalid_argument` exception handling.  

```c++
int stoi(const string& str, size_t *idx=0, int base=10);
long stol(const string& str, size_t *idx=0, int base=10);
unsigned long stoul(const string& str, size_t *idx=0, int base=10);
long long stoll(const string& str, size_t *idx=0, int base=10);
unsigned long long stoull(const string& str, size_t *idx=0, int base=10);
float stof(const string& str, size_t *idx=10);
double stod(const string& str, size_t *idx=10);
long double stold(const string& str, size_t *idx=10);
```

[ExampleString.cpp - exampleString6()](./ExampleString.cpp)  

#### raw level numeric convert to string 

[ExampleString.cpp - exampleString7()](./ExampleString.cpp)  

```c++
to_chars_result to_chars(char* first, char* last, IntegerT value, int base=10;
struct to_chars_result{
  char* ptr;
  errc ec;
};
```

`ec == errc::value_too_large`  

#### example structure binding 

[exampleString.cpp - exampleString8()](./ExampleString.cpp)  

- floating point example

```c++
to_chars_result to_chars(char* first, char* last, FloatT value);
to_chars_result to_chars(char* first, char* last, FloatT value, chars_format format);
to_chars_result to_chars(char* first, char* last, FloatT value, chars_format format, int precision);
```

```c++
enum class chars_format {
  scientific,//style : (-)d.ddde +- dd
  fixed, //style : (-)ddd.ddd
  hex, //style : (-)h.hhhp +- d
  general = fixed | scientific //next 
}
```

### example convert string to numeric format 

[ExampleString.cpp ExampleConvertToString](./ExampleString.cpp)  

```c++
from_chars_result from_chars(const char* first, const char* last, IntegerT& value, int base=10);
from_chars_result from_chars(const char* first, const char* last, FloatT& value, chars_format format = chars_format::genral);
```

```c++
struct from_chars_result {
  const char* ptr;
  errc ec;
}
```

- convert fail : `ptr` is `first`, `errc` is `errc::result_out_of_range`.
- convert success : `ptr` is `last`.
- space words not ingnore.  

## string_view ( after C++17 )  

```c++
std::string_view extractExtension(std::string_view filename)
```
[string_view example extractExtension function](./ExampleString.cpp)  

### note

```
R은 C++에서 "raw string literal"을 나타내는 접두사입니다. 
Raw string literal은 문자열 내에서 이스케이프 시퀀스(예: \n, \t, \\)를 무시하고, 
문자열 그대로를 포함할 수 있게 해줍니다. 이는 특히 파일 경로, 정규 표현식, HTML 코드 등에서 유용합니다.
```

- example string_view result ([ExampleStringView()](./ExampleString.cpp))

```sh
Example string view ExampleStringView
C++ string: .txt
C string: .txt
Literal: .txt
```

### strint_view concatenate 

[ExampleStringView2() function example](./ExampleString.cpp)  

- error situation

```c++
std::string str { "Hello" };
std::string_view sv { " world" };
auto result { str + sv };
```

- but, it's ok 

```c++
auto result { str + sv.data() };
```

```c++
auto result2 { str };
result2.append(sv.data(), sv.size());
```

- string return function is reutrn just `const string&` `string` types.
- class member data just defined `std::string`, ( don't define `const string&` or `string_view`)  

### string_view temporary string 

- never save temporary string in std::string_view.   

### string_view literal 

```c++
auto sv { "My string_view"sv };
```

- it is used with namespace below

```c++
using namespace std::literals::string_view_literals;
using namespace std::string_view_literals;
using namespace std::literals;
using namespace std;
```

`std::string_view` 는 C++17에서 도입된 클래스 템플릿으로, 문자열 데이터를 소유하지 않고 읽기 전용으로 참조할 수 있게 해줍니다.  
이는 문자열 데이터를 복사하지 않고도 효율적으로 처리할 수 있게 해주며, 성능 향상에 기여할 수 있습니다.

`std::string_view`의 주요 사용 사례  
- 함수 인수로 사용: 문자열 데이터를 복사하지 않고 함수에 전달할 수 있습니다.  
- 부분 문자열 참조: 문자열의 일부를 참조할 때 유용합니다.  
- 리터럴 문자열 처리: 문자열 리터럴을 효율적으로 처리할 수 있습니다.  

### new feature C++20, string format 

- `#include <format>`
- `std::format`

```c++
auto s1 { std::format("Read {} bytes from {}", n, "file1.txt")};
auto s2 { std::format("Read {0} bytes from {1}", n, "file1.txt")};
```

don't mix `{}` and `{N}`. 

```c++
auto s2 { std::format("Read {} bytes from {0}", n, "file1.txt")};
```

### format specifiers

```c++
[[fill]align][sign][#][0][width][.precision][type]
```
- dynamic width example  
  - [string formatter - ExampleStringFormat1() ](./ExampleString.cpp)  

```sh
|   42|
|     42|
```

#### [fill]align

- fill character and align method 
- `<` : left align
- `>` : right align
- `^` : center align  

### formatter custom type

- [formatter custom type example = ExmampleKeyValue.hpp](./ExampleKeyValue.hpp)  

## 7.1.1 Memory 

```c++
int i { 7 }; //i is automatic variable, it's saved in stack area. 
```

### using new keyword

- using `new` keyword, it's saved in free store.  

```c++
int *ptr { nullptr };
ptr = new int;
```

or 

```c++
int *ptr { new int };
```

## using new and delete

```c++
void leaky()
{
  new int; //memory leak
}
```

- using delete for memory free 

```c++
int* ptr { new int };
delete ptr;
ptr = nullptr;
```

- malloc 
  - didn't have to call initialize constructor. 
- new 
  - having to call initialize constructor. 
- free()
  - didn't have to call initialize destructor.
- delete 
  - having to call destructor.

- in case of insufficient memory.  
  - using `new` is throw exception. 
  - but, you can use to return `nullptr` below example. 

```c++
int* ptr { new(nothrow) int };
```

### initialize array

```C++
int myArray[5] { 1,2,3,4,5 };//1,2,3,4,5
int myArray[5] { 1,2}; //1,2,0,0,0
int myArray[5] { 0 }; //0,0,0,0,0
int myArray[5] {}; //0,0,0,0,0
int myArray[] {1,2,3,4,5};//1,2,3,4,5
```

```c++
int* myArrayPtr { new int[] { 1, 2, 3, 4, 5} };
delete[] myArrayPtr;
myArrayPtr = nullptr;
```

### Example: multi dimension

[ExampleArray344()](./ExampleMain.cpp)  

- Using `xDimension * yDimension * elementSize` is not efficient.
- A better approach is to allocate `x * yDimension + y`.

## Pointer ( page 346 ) 

- `*` pointer arithmetic.  
- `&` dereference.  
 
### array = pointer 

- [Example code - ExamplePage349() ](./ExamplePointer.hpp)

- it's same means example below. 

```c++
void doubleInts(int* theArray, size_t inSize);
void doubleInts(int theArray[], size_t inSize);
void doubleInts(int theArray[2], size_t inSize);
```

- using parameter just Array Size N 

```c++
void doubleIntsStack(int (&theArray)[4]);
```

or using template

```c++
template<size_t N>
void doubleIntStack(int (&theArray)[N])
{
  for(size_t i { 0 }; i < N; i++) { theArray[i] *= 2; }
}
```

### Example Memory leak check ( window )

```c++
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
  #ifndef DBG_NEW
    #define DBG_NEW new ( _NORMAL_BLOCK, __FILE__, __LINE__ )
    #define new DBG_NEW
  #endif
#endif //_DEBUG
```

- main function 

```c++
_CtrSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
```

### Example Memory leack check ( linux ) 

Valgrind는 메모리 누수와 메모리 사용 오류를 탐지하는 데 유용한 도구입니다. 

```sh
sudo apt-get install valgrind #Debian/Ubuntu
sudo yum install valgrind # CentOS/RHEL
sudo dnf install valgrind # Fedora
```

- execute valgrind 

```sh
valgrind --leak-check=full ./exampleCode 
```

- example result 

```sh
==4421== 
==4421== HEAP SUMMARY:
==4421==     in use at exit: 6 bytes in 3 blocks
==4421==   total heap usage: 54 allocs, 51 frees, 75,966 bytes allocated
==4421==
==4421== 1 bytes in 1 blocks are definitely lost in loss record 1 of 3
==4421==    at 0x4849013: operator new(unsigned long) (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==4421==    by 0x10F057: main (in /mnt/d/workspace/modern-cpp/ProC++/build/ExampleMain)
==4421==
==4421== 1 bytes in 1 blocks are definitely lost in loss record 2 of 3
==4421==    at 0x4849013: operator new(unsigned long) (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==4421==    by 0x10F122: main (in /mnt/d/workspace/modern-cpp/ProC++/build/ExampleMain)
==4421==
==4421== 4 bytes in 1 blocks are definitely lost in loss record 3 of 3
==4421==    at 0x4849013: operator new(unsigned long) (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==4421==    by 0x123986: ExampleReference::exampleReference3() (in /mnt/d/workspace/modern-cpp/ProC++/build/ExampleMain)
==4421==    by 0x10F0E8: main (in /mnt/d/workspace/modern-cpp/ProC++/build/ExampleMain)
==4421==
==4421== LEAK SUMMARY:
==4421==    definitely lost: 6 bytes in 3 blocks
==4421==    indirectly lost: 0 bytes in 0 blocks
==4421==      possibly lost: 0 bytes in 0 blocks
==4421==    still reachable: 0 bytes in 0 blocks
==4421==         suppressed: 0 bytes in 0 blocks
==4421==
==4421== For lists of detected and suppressed errors, rerun with: -s
==4421== ERROR SUMMARY: 3 errors from 3 contexts (suppressed: 0 from 0)
```

## std::make_shared_for_overwrite()

```C++
std::make_shared() value initialises the object(s) it creates, which might be an unnecessary step if you intend to assign values over them later.

std::make_shared_for_overwrite() default initialises the object(s) it creates.

The difference only matters for (sub-)objects of fundamental types, where there is no initialiser.

std::make_shared<int[1000][1000]>() will allocate and zero a million ints std::make_shared_for_overwrite<int[1000][1000]>() will allocate a million ints
```

### using shared_ptr() return 

```c++
class Foo : public enable_shared_from_this<Foo>
{
  public:
    shared_ptr<Foo> getPointer()
    {
      return shared_from_this(); 
      //return shared_ptr<Foo>(this); //occur duplicate delete 
    }
};
int main()
{
  auto ptr1 { make_shared<Foo>() };
  auto ptr2 { ptr1->getPointer() };
}
```

## Construct of class ( ctor )

- [ExampleChap8()](./ExampleMain.cpp)  

- default construct
  - it't not contained any parameter making construct.  
  - it called `0-argument construct` ( zero )  
 
 - uniform initailization 
   - using `{}`

- class uniform initialization ( making stack object )
  
```c++
SpreadsheetCell myCell { };//called default ctor 
```

- class initialization ( making free store object )

```c++
auto smartCellp { make_unique<SpreadsheetCell>() }; //recommand 

//not recommand 
SpreadsheetCell* myCellp { new SpreadsheetCell {} };
//SpreadsheetCell* myCellp { new SpreadsheetCell };
//SpreadsheetCell* myCellp { new SpreadsheetCell() };
delete myCellp; 
myCellp = nullptr;
```

### explicitly defaulted constructor 

```c++
class SpreadsheetCell
{
  public:
    SpreadsheetCell() = default;//making default constructor by compiler 
}
```

### explicitly deleted constructor 

- it's used a class composed only of static methods.

```c++
class MyClass 
{
  public:
    MyClass() = delete;
}
```

### constructor initializer ( ctor-initializer or member initializer list )

```c++
SpreadsheetCell::SpreadsheetCell(double initialValue)
  : m_value { initializeValue }
{ 
}
```

### initializer class 

- "In C++, data members are initialized in the order they appear in the class definition, not in the order they appear in the constructor initializer list."
- [InitializeClass()](./ExampleMain.cpp)  
- [Class Example](./chap8/FooExample.hpp)  


#### copy constructor 

```c++
class SpreadsheetCell
{
  public:
    SpreadsheetCell(const SpreadsheetCell& src);
}

SpreadsheetCell::SpreadsheetCell(const SpreadsheetCell& src) : m_value { src.m_value }
{
  //any code ....
}
```

- If a copy constructor is not explicitly defined, the C++ compiler will generate a copy constructor that initializes the new object with the same data members as the original object.

```c++
classname::classname(const classname& src)
  : m1 { src.m1 }, m2 { src.m2 }, m3 { src.m3 }, ... mn { src.mn }
{ 
}
```

- Explicitly defaulted copy constructor and explicitly deleted copy constructor.  

```c++
SpreadsheetCell(const SpreadsheetCell& src) = default;
SpreadsheetCell(const SpreadsheetCell& src) = delete; //no more don't copy construct 
```
 