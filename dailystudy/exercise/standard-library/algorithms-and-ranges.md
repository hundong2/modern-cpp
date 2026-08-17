# 알고리즘, ranges, views

표준 알고리즘은 컨테이너 자체보다 반복자·범위와 호출 가능 객체를 받는다. 알고리즘을 호출하기 전에 입력 범위, 반환값, 비교자/술어의 의미, 원소 재배치 여부를 확인한다.

## `std::sort`와 `std::ranges::sort` — `<algorithm>`

- 원소를 제자리에서 오름차순으로 재배치한다. 평균·최악 비교 복잡도는 `O(N log N)` 요구사항을 따른다.
- 같은 키 원소의 상대 순서는 보존하지 않는다. 필요하면 `std::stable_sort`를 검토한다.
- 반복자·참조 자체가 가리키는 위치의 값이 바뀌므로 정렬 전 인덱스 의미를 보관할 때 주의한다.
- `std::sort(first,last,comp)`는 임의 접근 반복자 쌍을 받는다.
- `std::ranges::sort(range,comp,projection)`은 범위 자체와 선택적 프로젝션을 받으며 끝 반복자를 반환한다.
- 비교자는 엄격 약순서를 만족해야 한다. `<=`를 비교자로 쓰면 요구사항을 깨뜨릴 수 있다.

## `std::find_if`, `std::ranges::find` — `<algorithm>`

- `find_if(first,last,predicate)`는 술어가 처음 참인 원소의 반복자를 반환하고 없으면 `last`를 반환한다.
- `ranges::find(range,value)`는 값과 같은 첫 원소를 찾고 범위 끝 반복자를 반환한다.
- 선형 탐색이라 최악 `O(N)`이다. 반환 반복자를 역참조하기 전에 끝과 비교한다.
- 컨테이너를 바꾸지 않지만 반환 반복자의 수명은 원본 범위와 무효화 규칙에 묶인다.

## `std::max_element`, `std::min`, `std::max`

- `max_element(first,last)`는 최댓값 원소의 반복자를 반환한다. 빈 범위면 `last`다. 최악 `N-1`회 비교한다.
- `min(a,b)`와 `max(a,b)`는 선택된 인자에 대한 `const T&`를 반환할 수 있다. 임시 인자를 넘긴 뒤 반환 참조를 오래 보관하지 않는다.
- `initializer_list` 오버로드는 값을 반환할 수 있어 오버로드별 반환형을 확인한다.
- 매크로 `min/max`와 충돌하는 플랫폼 헤더가 있을 수 있으므로 괄호 또는 매크로 설정을 확인한다.

## `std::accumulate` — `<numeric>`

- `accumulate(first,last,init)`는 `init`에서 시작해 왼쪽부터 누적한 값을 반환한다.
- 결과 타입은 원소 타입이 아니라 `init` 타입의 영향을 받는다. 큰 합인데 `0`을 넘기면 `int`로 누적될 수 있으므로 `0LL` 또는 명시 타입을 쓴다.
- 정확히 `N`회에 가까운 누적 연산을 수행해 `O(N)`이다.
- 부동소수점 덧셈은 결합법칙이 성립하지 않아 순서에 따라 반올림 결과가 달라질 수 있다.

## `std::iota` — `<numeric>`

- `[first,last)`에 시작값부터 `++value`한 연속 값을 써 넣는다.
- 반환값은 없고 범위 원소를 변경한다. 시간 복잡도는 `O(N)`이다.
- Union-Find 부모 배열을 `0,1,2,...`로 초기화하거나 인덱스 순열을 만들 때 유용하다.

## `std::fill` — `<algorithm>`

- `[first,last)`의 모든 원소에 값을 대입한다. `O(N)`이며 반환값은 없다.
- 객체별 대입 연산이 호출되므로 바이트 패턴을 채우는 `memset`과 의미가 다르다.
- `vector<bool>`처럼 프록시 참조를 쓰는 컨테이너에서도 반복자 계약을 통해 동작한다.

## `std::erase_if` — `<vector>`, `<string>`, 연관 컨테이너별 헤더

- C++20에서 컨테이너와 술어를 받아 조건에 맞는 원소를 지우고 제거한 개수를 반환한다.
- `vector`에서는 뒤 원소 이동 때문에 `O(N)`이고 삭제 위치 이후 반복자·참조가 무효가 된다.
- erase-remove 관용구를 한 호출로 표현해 의도를 분명하게 한다.

## `std::ranges::count_if` — `<algorithm>`

- 범위와 술어를 받아 참인 원소 수를 반복자 차이 타입으로 반환한다.
- 범위를 한 번 순회해 `O(N)`이다. 술어 호출 횟수와 부수 효과에 의존하는 코드는 피한다.
- 반환형이 `int`가 아닐 수 있으므로 큰 범위나 signed/unsigned 비교를 고려한다.

## `std::swap`과 `std::ranges::swap`

- 두 객체의 값을 교환한다. 사용자 타입은 이동 생성·이동 대입 또는 사용자 정의 `swap`을 사용할 수 있다.
- 일반 템플릿 코드에서는 `using std::swap; swap(a,b);`로 ADL 사용자 정의 교환을 허용하는 관용구가 있다.
- Union-Find에서 랭크가 큰 루트를 왼쪽에 두는 등 불변식을 단순화할 때 사용한다.

## `std::views::filter`와 `std::views::transform` — `<ranges>`

- `filter(predicate)`는 술어가 참인 원소만 지연 노출한다.
- `transform(function)`은 원소를 함수 결과로 지연 투영한다.
- 파이프 `source | views::filter(...) | views::transform(...)`는 보통 중간 컨테이너를 즉시 만들지 않는다.
- 뷰는 원본을 소유하지 않을 수 있고 술어/변환 객체를 보관한다. 원본과 캡처한 참조의 수명이 뷰보다 길어야 한다.
- 실제 함수 호출은 순회할 때 발생하므로 뷰를 만들기만 해서는 부수 효과가 실행되지 않을 수 있다.
- 같은 뷰를 여러 번 순회할 수 있는지는 기반 범위와 뷰 종류의 범주에 따라 다르다.

## `std::ranges::end`

- 범위의 끝 센티널을 사용자 정의 `end`까지 고려해 얻는 customization point object다.
- 반환값은 마지막 원소가 아니라 마지막 다음 위치이며 역참조할 수 없다.
- 템플릿 코드에서 멤버 `end()`와 ADL `end`를 일관되게 찾게 한다.

## 비교 함수 객체 `std::less`, `std::greater` — `<functional>`

- 두 값을 비교하는 함수 객체다. `less<T>{}(a,b)`는 보통 `a<b`, `greater<T>{}(a,b)`는 `a>b` 의미다.
- `std::priority_queue<T,Container,std::greater<T>>`는 작은 값이 `top`이 되는 최소 힙을 만든다.
- 투명 비교자인 `std::less<>`는 서로 비교 가능한 다른 타입을 받아 불필요한 키 임시 생성을 줄일 수 있다.

## 최소 예제

```cpp
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    std::vector<int> values{4, 1, 3, 2};
    std::ranges::sort(values); // 제자리 오름차순 [1,2,3,4]
    const int sum{std::accumulate(values.begin(), values.end(), 0)};
    const auto even_count{std::ranges::count_if(values, [](int value) {
        return value % 2 == 0;
    })};
    std::cout << sum << ' ' << even_count << '\n'; // 10 2
}
```

## 직접 검증

1. `accumulate`의 초기값을 `0`과 `0LL`로 줄 때 반환형과 오버플로 가능성을 비교한다.
2. `sort` 비교자로 `left <= right`를 쓰면 엄격 약순서의 어떤 규칙을 깨는지 설명한다.
3. `find_if`가 반환한 반복자를 `vector::push_back` 뒤에도 보관할 수 있는 조건을 말한다.
4. `views::filter`를 만든 뒤 원본 `vector`를 파괴하는 최소 댕글링 예를 작성한다.
