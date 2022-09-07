# Perfect forwarding 

## 1.rvalue

- 임시 객체
- 이름 없는 객체
- 주소가 없는 객체
- 단일 표현식 이후에도 없어지지 않고 지속되는 객체.

## 2. lvalue

- 이름과 주소를 가지고 있음.  

```c++
int five = 5;
std::string a = std::string("Rvalue");
std::string b = std::tring("R") + std::string("value");
std::string c = a + b;
std::string d = std::move(b);
```

@ move( after C++11 ) lvalue b를 rvalue reference로 변환하는 함수. 

## 3. Perfect factory method

- 가변길이 인자를 받을 수 있어야 한다. 
- lvalue, rvalue 모드 인자로 받을 수 있어야 한다. 
- 생성자에 똑같은 인자를 전달할 수 있어야 한다. 

[reference - a-researcher.tistory.com](https://a-researcher.tistory.com/11)  


