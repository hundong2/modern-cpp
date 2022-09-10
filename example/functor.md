# function object 

## 1. Function Object?

- operator()를 오버로드한 개체.
- Functor 라고도 한다. 

### Example Functor

```c++
struct Functor
{
    void operator() {}
}

Functor a;
a();
```

