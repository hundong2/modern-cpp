# cpp20

`CPP20` 폴더의 예제로 바로 이동할 수 있는 빠른 메뉴입니다.

## 빠른 이동

- [Priority Queue Code](./priority_queue.cpp). 
    - [Priority Queue](#using-priority-queue). 
- [CPP20 메인 README](../../CPP20/README.md)
- [실행 스크립트 (exec.sh)](../../CPP20/exec.sh)
- [빌드 배치 (__BUILD_BATFILE)](../../CPP20/__BUILD_BATFILE/)
- [Windows 배치 (c++20.bat)](../../CPP20/c++20.bat)

## 주제별 메뉴

- [ALGORITHM](../../CPP20/ALGORITHM/)
- [ATTRIBUTES](../../CPP20/ATTRIBUTES/)
- [COMPARISON](../../CPP20/COMPARISON/)
- [CONCEPT](../../CPP20/CONCEPT/)
- [CONSTEXPR](../../CPP20/CONSTEXPR/)
- [CONTAINER](../../CPP20/CONTAINER/)
- [COROUTINE](../../CPP20/COROUTINE/)
- [DESIGNATED_INITIALIZATION](../../CPP20/DESIGNATED_INITIALIZATION/)
- [EXPLICIT_BOOL](../../CPP20/EXPLICIT_BOOL/)
- [FEATURE_TEST_MACRO](../../CPP20/FEATURE_TEST_MACRO/)
- [LAMBDA](../../CPP20/LAMBDA/)
- [MODULE](../../CPP20/MODULE/)
- [RANGE](../../CPP20/RANGE/)
- [RANGES_IMPL](../../CPP20/RANGES_IMPL/)
- [RANGE_FOR_WITH_INITIALIZER](../../CPP20/RANGE_FOR_WITH_INITIALIZER/)
- [SPAN](../../CPP20/SPAN/)
- [USING_ENUM](../../CPP20/USING_ENUM/)
- [UTILITY](../../CPP20/UTILITY/)

## Using Priority Queue

### std::priority_queue definition

```cpp
template<
    class T,
    class Container = std::vector<T>,
    class Compare = std::less<typename Container::value_type>
> class priority_queue;
```

```
"A priority queue is a container adaptor 
that provides constant time lookup of the 
largest   (by default) element, at the expense 
of logarithmic insertion and extraction."  
(우선순위 큐는 로그 시간의 삽입/추출 비용을 대가로, 
기본적으로 '가장 큰' 요소를 상수 시간(O(1))에 조회할 
수 있게 해주는 컨테이너 어댑터입니다.)
```


- T (ListNode*): 큐에 저장될 데이터의 타입입니다. 여기서는 노드의 포인터입니다.
- Container (vector<ListNode*>): 내부적으로 힙(Heap) 트리를 구현할 기반 컨테이너입니다. 인덱스로 빠르게 접근 (Random Access)해야 하므로 배열 기반인 std::vector가 표준입니다.
- Compare (Compare 구조체): 정렬 기준입니다.


