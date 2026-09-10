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
- `forward_range` 위 `filter_view`는 첫 통과 반복자를 cache할 수 있다. 이미 순회를 시작한 view는 술어가 보는 원소를 바꾸거나 기반 `vector`를 재할당한 뒤 재사용하지 말고 새로 만든다. 원본 객체 파괴는 view를 dangling으로 만들며, 재할당은 이미 얻었거나 cache한 반복자를 무효화한다.

## `std::ranges::to` — `<ranges>`의 C++23 범위 변환 함수 템플릿·adaptor closure

`std::ranges::to`는 입력 범위를 지정한 컨테이너 값으로 materialize한다. 오늘 코드는 지연 `filter`/`transform` 파이프라인을 서비스 경계에서 `std::vector` 소유 스냅숏으로 고정해, 반환 결과가 원본 컨테이너와 중간 view 객체의 수명에 매이지 않게 한다.

- **항목 종류·헤더**: `<ranges>`가 C++23 함수 템플릿 overload 집합을 선언한다. 대표 직접 호출은 `template<class C, ranges::input_range R, class... Args> requires (!ranges::view<C>) constexpr C ranges::to(R&& range, Args&&... args)`다. `range | ranges::to<C>(args...)` 형태에서는 인자들을 보관한 range adaptor closure prvalue를 먼저 만들고, 파이프 적용이 같은 변환을 수행한다. 컨테이너 템플릿만 주어 결과 특수화를 추론하는 overload도 있다.
- **대상 타입·선택 조건**: `C`는 cv 한정되지 않은 class/union 타입이어야 하고 view일 수 없다. `R`은 `input_range`를 만족해야 한다. 구현은 제약을 만족하는 범위 직접 생성자, `from_range` 생성자, iterator/sentinel 생성자, 또는 빈 컨테이너를 만든 뒤 `emplace_back`·`push_back`·`emplace`·`insert`하는 경로 중 표준이 정한 우선순위로 가능한 형태를 고른다. 원소가 곧바로 변환되지 않는 중첩 범위는 조건을 만족할 때 재귀적으로 대상 원소 컨테이너로 변환한다.
- **수신 객체·호출 전 상태**: 자유 함수에는 수신 객체가 없다. 오늘 파이프의 왼쪽은 원본을 빌리는 유효한 view 식이고, 그 iterator/sentinel과 캡처한 참조는 materialization 순회가 끝날 때까지 유효해야 한다. 대상 컨테이너 객체는 아직 존재하지 않는다.
- **매개변수·값 범주·소유권**: 직접 형태의 `R&&`는 forwarding reference라 lvalue 범위는 lvalue로, 임시 파이프는 rvalue로 전달한다. 추가 `Args&&...`도 선택된 컨테이너 생성자에 그대로 전달하며 오늘 호출의 팩은 비어 있다. 이 전달만으로 원본 저장소 소유권을 얻는 것은 아니며, 각 원소는 `range_reference_t<R>`의 값 범주와 선택된 생성·삽입 경로에 따라 복사·이동·변환된다.
- **반환형·반환값 사용**: 직접 호출과 파이프 적용은 완성된 `C` 값을 반환한다. 오늘 `C=std::vector<Snapshot>`이며 반환 prvalue는 이름 있는 결과 객체를 직접 초기화하므로 같은 타입의 불필요한 중간 컨테이너 복사/이동이 필요 없다. `ranges::to<C>()` 자체는 `C`가 아니라 closure를 반환한다는 차이를 구분한다.
- **전제조건·사후조건**: 호출 전 범위가 `input_range`의 의미 요구사항을 지키고 선택된 생성/삽입 연산이 유효해야 한다. 성공 뒤 결과 컨테이너에는 순회 순서대로 변환한 원소가 들어 있고 컨테이너 저장소는 결과가 소유한다. 다만 원소 타입 자체가 pointer, view, `reference_wrapper`라면 그 원소가 가리키는 대상까지 새로 소유하는 것은 아니다.
- **복잡도·할당**: 모든 대상 `C`에 공통인 하나의 고정 복잡도는 없고 선택된 생성자·삽입·원소 변환 비용을 따른다. 오늘 파이프는 `sized_range`는 아니지만 `forward_range`라 vector의 from-range 생성 경로가 선택된다. 거리를 구하는 순회와 원소 구성 순회가 가능하므로 filter 술어를 입력 하나당 정확히 한 번만 부른다고 보장할 수 없고, projector와 문자열 복사는 선택된 `M`개를 구성하는 순회에서 실행된다. 입력 `N`, 결과 `M`, 복사 문자 수 `L`에 시간 `O(N+M+L)`, 결과 공간 `O(M+L)`이며 vector 원소 저장소 재할당은 없다. 반면 forward가 아닌 일반 input range가 삽입 경로를 선택하면 결과 vector가 성장하며 재할당할 수 있다.
- **상태 변화·무효화**: 읽기 전용 lvalue 원본 원소는 보통 바뀌지 않지만 여러 순회는 `filter_view` 같은 view의 내부 iterator cache를 갱신할 수 있고, 결과는 독립 저장소를 갖는다. rvalue 원본을 직접 넘기거나 원소 reference가 이동을 선택하게 하면 원본 원소가 moved-from 상태가 될 수 있으므로 선택된 overload를 확인한다. 오늘의 forward-range vector 생성에는 원소 저장소 재할당이 없고, 일반 삽입 경로에서 재할당이 생겨도 호출 밖으로 결과 관찰자가 아직 노출되지는 않는다. 반환 뒤 무효화 규칙은 `C`의 일반 계약을 따른다.
- **수명**: 변환 순회 중에는 원본 범위, iterator/sentinel, 술어·변환 함수가 캡처한 참조가 모두 살아 있어야 한다. 성공한 소유 컨테이너와 값 원소는 원본/view보다 오래 살 수 있다. adaptor closure가 추가 인자를 보관한다면 decay-copy된 객체의 수명과 그 객체 내부의 비소유 참조도 별도로 점검한다.
- **오류·예외 보장**: 대상 생성, 저장소 할당, 원소 복사·이동·변환, 술어/변환 함수와 삽입이 던진 예외는 호출자에게 전파될 수 있다. 결과 객체 초기화는 완료되지 않으며 이미 구성된 결과 원소는 정리되지만, 앞서 이동된 원본 원소나 사용자 함수의 외부 부수 효과까지 rollback하지는 않는다. 크기 한계는 `length_error`, 저장소 부족은 `bad_alloc` 등 선택된 컨테이너 계약을 따른다.
- **컴파일 오류·미정의 동작**: 대상/범위/원소 변환 요구를 만족하지 않으면 적합한 overload가 없어 컴파일에 실패하며 런타임 빈 결과로 표현되지 않는다. 유효하지 않은 iterator 범위, 댕글링 view, 의미 요구를 거짓으로 주장한 사용자 iterator/container, 또는 데이터 경쟁 상태의 원본을 순회하면 계약 위반과 미정의 동작으로 이어질 수 있다.
- **스레드 보장**: 자체 동기화를 제공하지 않는다. 서로 다른 원본을 변환하는 호출은 독립적일 수 있지만, 같은 원본을 한 실행 흐름이 순회하는 동안 다른 흐름이 구조나 같은 원소를 동기화 없이 변경하면 데이터 경쟁 또는 iterator 무효화가 생길 수 있다.

기계 실행 관점에서 오늘 materialization은 거리 계산을 위한 원본 순회, 결과 저장소 확보, 다시 수행하는 술어 비교와 조건 분기, 선택 원소 변환·store로 이어질 수 있다. 이 forward-range vector 생성 중 원소 저장소 재할당은 없다. 실제 명령, inlining, 분기 제거와 벡터화 여부는 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라 특정 어셈블리로 단정하지 않는다.

## `std::ranges::end`

- 범위의 끝 센티널을 사용자 정의 `end`까지 고려해 얻는 customization point object다.
- 반환값은 마지막 원소가 아니라 마지막 다음 위치이며 역참조할 수 없다.
- 템플릿 코드에서 멤버 `end()`와 ADL `end`를 일관되게 찾게 한다.

## `std::ranges::subrange<I, S, K>` — `<ranges>`

`subrange`는 시작 iterator `I`와 끝 sentinel `S`를 값으로 보관해 반열린 범위 `[begin,end)`를 만드는 C++20 class template이자 view다. **iterator/sentinel 객체는 소유하지만 그들이 가리키는 원소와 저장소는 소유하지 않는다.** 컨테이너 전체가 아니라 일부 구간만 표준 range 인터페이스로 넘기되 원소 복사를 피할 때 사용한다.

- 항목 종류·대표 선언: `template<input_or_output_iterator I, sentinel_for<I> S = I, subrange_kind K = ...> class subrange`. `S=I`이면 같은 iterator 타입이 끝 역할도 한다. `K`는 `sized` 또는 `unsized`이며 기본값은 `sized_sentinel_for<S,I>` 만족 여부로 정해진다.
- iterator 쌍 생성자: 대표 형태 `subrange(I first, S last)`는 두 인자를 값으로 이동/복사해 보관한다. **호출 시점부터 `[first,last)`가 유효한 범위, 즉 sentinel `last`가 iterator `first`에서 허용된 반복으로 도달 가능한 경계여야 한다.** 이 전제조건을 어긴 생성 자체가 라이브러리 계약 위반이며 미정의 동작이다. `K==sized`인데 `S`와 `I`의 차이를 바로 구할 수 없는 형태는 별도 size 인자가 필요하다. 생성자는 반환값이 없고 원본 범위·원소를 변경하지 않는다.
- range 생성자: 적합한 `borrowed_range<R>`에서는 `subrange(R&&)` 형태로 `ranges::begin/end`를 얻을 수 있다. 임시 소유 컨테이너처럼 borrowed range가 아닌 입력을 거부하는 overload 제약은 흔한 dangling을 줄이지만, 사용자가 임시 컨테이너 iterator를 직접 꺼내 쌍 생성자에 넣는 잘못까지 막지는 못한다.
- `begin()` / `end()`: 저장한 시작 iterator와 끝 sentinel을 값으로 반환한다. vector iterator처럼 복사 가능한 `I`에서는 수신 subrange와 원소가 바뀌지 않는다. `end()` 결과는 마지막 원소가 아니라 마지막 다음 경계이므로 역참조하면 안 된다.
- `size()`: `K==subrange_kind::sized`일 때만 제공한다. 저장 크기가 있으면 그 값을, `sized_sentinel_for`이면 끝과 시작의 차이를 부호 없는 차이 타입 값으로 반환한다. random-access vector iterator 쌍에서는 `O(1)`이다.
- 복잡도·할당: iterator 복사/이동과 sentinel 차이가 상수 시간인 오늘 타입에서는 생성, 복사, `begin`, `end`, `size` 모두 `O(1)`이며 subrange 자체가 동적 할당을 요구하지 않는다. 사용자 iterator의 연산 비용·예외가 다르면 그 계약을 따른다.
- 수명·무효화: vector에서 얻은 iterator라면 owner 파괴, 재할당, 그리고 erase 위치에 따른 vector 무효화 규칙을 그대로 따른다. `const_iterator`는 그 iterator를 통한 쓰기만 막고 다른 별칭의 vector mutation을 막지 않는다. 생성 뒤 owner 변화로 dangling이 된 subrange의 비교·차이·역참조·순회도 전제조건을 깨며 미정의 동작으로 이어질 수 있다.
- 예외·보장: 표준 컨테이너 iterator처럼 복사·차이 계산이 던지지 않는 타입에서는 오늘 호출도 예외를 내지 않고 원본 상태를 바꾸지 않는다. 일반 사용자 iterator/sentinel은 복사·이동·연산에서 예외를 던질 수 있으므로 generic 코드에서는 그 예외 보장을 전파한다.
- 스레드: subrange는 동기화를 제공하지 않는다. 서로 다른 실행 흐름이 같은 원소를 읽기만 하는 것은 원본 타입의 규칙을 따르지만, 한쪽이 vector 구조나 같은 원소를 쓰는 동안 동기화 없이 순회하면 iterator 무효화 또는 데이터 경쟁이 생길 수 있다.
- 오늘 코드의 역할: `RecordBook::Page{first,last}`는 owner보다 짧게 살아 있는 읽기 전용 페이지를 서비스 함수 안에서 즉시 집계한다. page를 장기 저장하지 않는 구조가 비소유 수명 계약을 API 흐름에 드러낸다.

기계 실행 관점에서 vector의 random-access iterator 두 개는 주소와 비슷한 값으로 최적화될 수 있고 `size()`는 뺄셈이 될 수 있다. 그러나 iterator 표현, load·비교·분기 수와 bounds 관련 최적화는 CPU, ABI, 표준 라이브러리, 컴파일러와 옵션에 따라 달라 특정 어셈블리로 단정하지 않는다.

### 최소 실행 예제

```cpp
#include <iostream>
#include <ranges>
#include <vector>

int main() {
    std::vector<int> values{10, 20, 30, 40};
    using Iterator = std::vector<int>::const_iterator;
    const std::ranges::subrange<Iterator> middle{values.cbegin() + 1, values.cbegin() + 3};

    int sum{};
    for (const int value : middle) {
        sum += value;
    }
    std::cout << middle.size() << ' ' << sum << '\n'; // 2 50
}
```

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
5. `subrange`와 `span`이 각각 표현할 수 있는 sentinel/연속 메모리 조건과 원소 소유권을 비교한다.
6. vector iterator subrange를 만든 뒤 `push_back`이 재할당할 때 생기는 무효화를 설명한다.
