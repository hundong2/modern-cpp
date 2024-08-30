# this page for that books named C++ for professional.

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
