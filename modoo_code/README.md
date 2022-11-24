# c++ code example 


## 1. shared_ptr

## 2. explicit & mutable 

### 2.1 explicit

[example Source](example4_6.cpp)

- explicit : 암시적 형변환 

### 2.2 mutable

[example Source](example4_6_mutable.cpp)

- const function 에서도 값을 바꿀 수 있다. 

- 'data_' is not mutable

```bash
example4_6_mutable.cpp: In member function 'void A::DoSomething(int) const':
example4_6_mutable.cpp:9:11: error: assignment of member 'A::data_' in read-only object
    9 |     data_ = x;  // 불가능!
      |     ~~~~~~^~~
Capacity : 3
```

- mutable을 사용하는 이유 
- Data를 반복적, 자주 접근하는 경우 DB에 계속 접근하기 보다, caching을 한 뒤 빠르게 처리함.
- 이때 const function에서 따로 cache에 저장이 필요, mutable 사용

- cache update 불가 but, cache variable 이 mutable 로 선언되어질 경우 가능!!!
```c++
class Server {
  // .... (생략) ....

  Cache cache; // 캐쉬!

  // 이 함수는 데이터베이스에서 user_id 에 해당하는 유저 정보를 읽어서 반환한다.
  User GetUserInfo(const int user_id) const {
    // 1. 캐쉬에서 user_id 를 검색
    Data user_data = cache.find(user_id);

    // 2. 하지만 캐쉬에 데이터가 없다면 데이터베이스에 요청
    if (!user_data) {
      user_data = Database.find(user_id);

      // 그 후 캐쉬에 user_data 등록
      cache.update(user_id, user_data); // <-- 불가능 ( mutable 가능. ) -> mutable Cache cache;
    }

    // 3. 리턴된 정보로 User 객체 생성
    return User(user_data);
  }
};
```

## 3. 연산자 오버로딩

- 기본 형태 
`(return type) operator (received parameter of operator )`

- 연산자 함수
`bool operator==(MyString& str)` . 

- 내부적 처리 방법 
`str1 == str2` 는 `str1.operator==(str2)` 로 내부적으로 처리 된다.  

[overloading example](overloading.cpp)  

### 3.1 friend 

```
자기 자신을 리턴하는 이항 연산자는 멤버함수로, 아닌 애들은 외부 함수로
```

[overloading example with friend](overloading2.cpp)  


### 3.2 In/Out operator overloading 

`std::cout << a;` == `std::cout.operator<<(a);`

```c++
std::ostream& operator<<(std::ostream& os, const Complex& c)
{
    os << "(" << c.real << " , " << c.img << ")" ;
    return os;
}
```
- 위의 경우 c 의 real, img에 접근이 불가능!!!
- Complex 객체에 friend를 사용

```c++
friend ostream& operator<<(ostream& os, const Complex& c);
```

#### example 

[overloading3](overloading3.cpp) . 

### 3.4 첨자 연산자 []

- 원형

```c++
char& operator[](const int index);
```

- exmaple 
```c++
str[10] = 'c';

char& MyString::operator[](const int index){ return string_content[index]; }
```

- example : [overloading4.cpp](overloading4.cpp) . 

### 3.5 Wrapper class ( 타입 변환 연산자 )

- 정의 
```
operator (변환 하고자 하는 타입) ()
example)
operator int()
```

- return type을 쓰면 안된다. 
- int의 경우 `operator int() { return data; }` 로 사용. 

### 3.6 전위/후위 연산자(++, --)

- 전위 연산자 : 자기 자신을 리턴

```c++
A& operator++()
{
    return *this;
}
```

- 후위 연산자 : 연산 전에 객체를 반환.

```c++
A operator++(int)
{
    A temp(A);
    
    //A++수행
    ...
    return temp;
}
```

- 후위 증감 연산의 경우 추가적으로 복사 생성자를 호출. 전위 증감 연산보다 느리다. 
- 정리  
  
```
연산자 오버로딩에 대해 다루면서 몇 가지 중요한 포인트 들만 따로 정리해보자면;
두 개의 동등한 객체 사이에서의 이항 연산자는 멤버 함수가 아닌 외부 함수로 오버로딩 하는 것이 좋습니다. 
(예를 들어 Complex 의 operator+(const Complex&, const Complex&) const 와 같이 말입니다.)
두 개의 객체 사이의 이항 연산자 이지만 한 객체만 값이 바뀐다던지 등의 동등하지 않는 이항 연산자는 멤버 함수로 오버로딩 하는 것이 좋습니다. (예를 들어서 operator+= 는 이항 연산자 이지만 operator+=(const Complex&) 가 낫다)
단항 연산자는 멤버 함수로 오버로딩 하는 것이 좋습니다 (예를 들어 operator++ 의 경우 멤버 함수로 오버로딩 합니다)
일부 연산자들은 반드시 멤버 함수로만 오버로딩 해야 합니다
```

- example : [overloading5.cpp](overloading5.cpp) . 


## 4. type casting

### 4.1 casting의 종류

- `static_cast` : 우리가 흔히 생각하는, 언어적 차원에서 지원하는 일반적인 타입 변환.
- `const_cast` : 객체의 상수성(const) 를 없애는 타입 변환. 쉽게 말해 `const int`가 `int`로 바뀐다. 
- `dynamic_cast` : 파생 클래스 사이에서의 다운 캐스팅.
- `reinterpret_cast` : 위험을 감수하고 하는 캐스팅. 서로 관련이 없는 포인터들 사이의 캐스팅. 

```
(원하는 캐스팅의 종류)<바꾸려는 타입>(what??)
```

- example 
- C 

```C
(int)(float_variable)
```

- C++

```C++
static_cast<int>(float_variable);
```

## 5. array class making

[modoo code](https://modoocode.com/204) . 



