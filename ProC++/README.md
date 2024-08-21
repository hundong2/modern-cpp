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
