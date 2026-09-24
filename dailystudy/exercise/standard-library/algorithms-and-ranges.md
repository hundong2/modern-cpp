# 알고리즘, ranges, views

표준 알고리즘은 컨테이너 자체보다 반복자·범위와 호출 가능 객체를 받는다. 알고리즘을 호출하기 전에 입력 범위, 반환값, 비교자/술어의 의미, 원소 재배치 여부를 확인한다.

## `std::sort`와 `std::ranges::sort` — `<algorithm>`

- **항목 종류·현재 역할**: `<algorithm>`의 `std::sort`는 반복자 기반 함수 템플릿이고 `std::ranges::sort`는 C++20 ranges 알고리즘 함수 객체(niebloid)다. 2026-09-25 풀이는 오른쪽 절반 부분집합 합을 소유한 `std::vector<long long>`을 오름차순으로 재배치해 뒤의 `equal_range`가 이진 탐색 전제조건을 만족하게 한다.
- **C++23 range overload와 템플릿 인자**: 대표 형태는 `template<random_access_range R, class Comp = ranges::less, class Proj = identity> requires sortable<iterator_t<R>, Comp, Proj> constexpr borrowed_iterator_t<R> sort(R&& range, Comp comp = {}, Proj projection = {});`다. 오늘 호출에서는 `R=std::vector<long long>&`, `Comp=std::ranges::less`, `Proj=std::identity`이며 `iterator_t<R>`는 vector의 mutable random-access iterator다. 고전 `std::sort(first, last, comp)`는 임의 접근 반복자 쌍을 받는 별도 overload다.
- **수신 객체·호출 전 상태**: 함수 객체 호출이라 사용자 데이터 수신 객체는 없다. `right_sums`는 유효한 non-const lvalue vector이고, 모든 원소는 살아 있으며 이동·교환 가능하다. 정렬 전 순서는 임의여도 된다.
- **매개변수·값 범주·소유권**: `R&&`는 forwarding reference이고 lvalue `right_sums`에 바인딩되므로 저장소를 복사하거나 이동하지 않고 호출 동안 빌린다. 생략한 `comp`와 `projection`은 값으로 생성되는 `ranges::less{}`와 `identity{}`다. 사용자 비교자·projection을 넘기면 알고리즘 실행 동안 유효해야 하며 원소의 정렬 관계를 깨는 부작용을 내면 안 된다.
- **반환형·사용 여부**: lvalue vector는 borrowed range이므로 반환형은 `std::vector<long long>::iterator`이고 정렬된 범위의 끝 위치를 가리킨다. 오늘 코드는 반환 iterator를 의도적으로 버린다. 임시 non-borrowed range라면 `borrowed_iterator_t<R>`가 `std::ranges::dangling`일 수 있다.
- **사후 상태·안정성**: 성공하면 projected 원소가 비교자 기준 비내림차순이고 원소들은 입력의 permutation이다. 크기·capacity·저장소 소유권은 유지되지만 위치별 값은 바뀐다. 같은 키의 상대 순서는 보존되지 않으므로 필요하면 `std::stable_sort`를 검토한다.
- **복잡도**: 원소 수를 `N`이라 할 때 `O(N log N)`회의 비교와 projection을 요구한다. 원소 이동·교환 비용은 타입에 따르고, ranges overload의 끝 iterator 계산 비용도 sentinel 성질에 따른다.
- **할당·무효화·수명**: 표준 계약만으로 구현의 보조 저장소 사용이 없다고 단정하지 않는다. vector 자체의 구조 변경이나 재할당은 하지 않으므로 기존 iterator·포인터·참조는 계속 유효하지만, 같은 위치가 정렬 전과 같은 논리 원소를 뜻하지는 않는다. range와 원소는 호출이 끝날 때까지 살아 있어야 한다.
- **전제조건·오류·예외·미정의 동작**: 범위는 random-access range이고 iterator는 permutable해야 하며 `comp(proj(a), proj(b))`가 전체 실행 동안 엄격 약순서를 이뤄야 한다. `<=`처럼 엄격하지 않은 비교자, 댕글링 iterator/range, 이동·교환 요구사항 위반은 전제조건 위반이며 미정의 동작이다. 비교·projection·원소 이동/교환 또는 구현의 자원 확보가 던지면 예외가 전파될 수 있고, 그때 범위는 유효하더라도 원래 순서나 완전 정렬을 보장하지 않는다. 오늘의 `long long` 기본 비교·이동은 던지지 않지만 함수 선언 자체를 `noexcept`로 가정하지 않는다.
- **스레드·기계 실행 관점**: 자체 동기화가 없으므로 같은 vector를 다른 실행 흐름이 동시에 읽거나 쓰는 동안 정렬하면 안 된다. 구현은 비교, load/store, 원소 교환과 조건 분기를 조합할 수 있으나 구체적 정렬 전략·SIMD·명령열·보조 메모리는 CPU, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라진다.

## `std::ranges::equal_range` — `<algorithm>`의 비교 동등 구간 이진 탐색

- **항목 종류·현재 역할**: C++20 ranges 알고리즘 함수 객체(niebloid)다. 2026-09-25 CSES 1628 풀이는 정렬된 오른쪽 절반합 `vector`에서 `target-left_sum`과 비교 동등한 **모든 중복값**의 반열린 구간을 찾아 경우의 수를 더한다. 값 하나의 존재만 확인하는 `binary_search`와 목적이 다르다.
- **C++23 range overload**: 대표 형태는 `template<forward_range R, class T, class Proj = identity, indirect_strict_weak_order<const T*, projected<iterator_t<R>, Proj>> Comp = ranges::less> constexpr borrowed_subrange_t<R> equal_range(R&& range, const T& value, Comp comp = {}, Proj projection = {});`다. C++26에서 `T` 기본 인자가 보강된 선언과 오늘 C++23 호출의 요구사항을 혼동하지 않는다.
- **수신 객체·입력 상태**: ranges 알고리즘 함수 객체의 호출이므로 사용자 데이터 수신 객체는 없다. 오늘 `right_sums`는 유효한 `std::vector<long long>` lvalue이고 기본 비교와 항등 projection에 맞춰 오름차순 정렬돼 있다.
- **매개변수·값 범주·소유권**: `R&&`는 forwarding reference이므로 오늘 lvalue vector의 저장소를 이동하거나 복사하지 않고 호출 동안 빌린다. `needed`는 `const long long& value`에 바인딩되는 lvalue다. 생략한 `comp`와 `projection`은 각각 `ranges::less{}`와 `identity{}` 의미이며 값으로 사용된다.
- **반환형·반환값 사용**: vector lvalue 호출은 두 iterator를 보관한 sized `subrange`를 값으로 반환한다. 시작은 첫 비교 동등 원소, 끝은 마지막 동등 원소 다음이다. 일치가 없으면 두 iterator가 같은 삽입 위치다. 오늘 코드는 반환값을 `matches`에 저장하고 `matches.size()`로 중복 부분집합 수를 얻는다.
- **비교 동등성과 전제조건**: 동등성은 `operator==`가 아니라 `comp(element,value)`와 `comp(value,element)`가 둘 다 거짓인 관계다. 입력은 두 비교 식에 대해 partition돼 있어야 하고 비교자는 엄격 약순서를 만족해야 한다. 같은 비교자·projection으로 전체 vector를 먼저 정렬하면 이 전제를 만족한다.
- **사후 상태**: 알고리즘은 vector의 크기·용량·원소·소유권을 바꾸지 않고 기존 iterator도 무효화하지 않는다. 반환 subrange는 iterator를 소유하지만 원소와 저장소를 소유하지 않는다.
- **복잡도**: 최대 `2*log2(N)+O(1)`회의 비교·projection을 수행한다. vector random-access iterator에서는 위치 이동도 로그 규모다. 일반 forward range에서는 비교 횟수는 로그여도 iterator 증가는 선형일 수 있다.
- **할당·무효화·수명**: 알고리즘 자체는 동적 저장소를 요구하지 않는다. 반환 iterator는 owner 파괴, vector 재할당, 관련 erase 때 무효화된다. 주소가 유지돼도 검색 뒤 원소 값을 바꾸면 반환 구간이 같은 값을 뜻한다는 의미 보장은 사라진다. 임시 non-borrowed range라면 반환형이 usable iterator 구간이 아닌 `dangling`이 될 수 있다.
- **오류·예외·미정의 동작**: 기본 정수 비교·vector iterator에는 별도 오류값이 없다. 일반 comparator, projection, iterator 연산이 던진 예외는 전파된다. partition, 엄격 약순서 또는 iterator/range 수명·유효성 전제조건을 깨고 호출하면 미정의 동작이다.
- **스레드 보장**: 자체 동기화를 제공하지 않는다. 같은 vector를 여러 실행 흐름이 읽기만 하는 것은 원소 타입 규칙을 따르지만, 한쪽이 구조나 원소를 쓰는 동안 검색하면 데이터 경쟁 또는 iterator 무효화가 발생할 수 있다.
- **범위 범주 주의**: `std::generator`는 `input_range`일 뿐 `forward_range`가 아니어서 이 알고리즘의 입력으로 직접 사용할 수 없다. 오늘처럼 정렬 가능한 소유 vector로 materialize한 범위를 사용해야 한다.

### 최소 실행 예제

```cpp
#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

int main() {
    const std::vector<int> sorted{1, 2, 2, 2, 5};
    const auto matches{std::ranges::equal_range(sorted, 2)};
    std::cout << matches.size() << '\n'; // 3
}
```

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

## `std::ranges::fold_left` — C++23 왼쪽 값 축약

- **항목 종류·헤더·현재 역할**: `<algorithm>`이 선언하는 C++23 ranges 알고리즘 함수 객체(niebloid)다. 2026-09-22 코드는 읽기 전용 원장/점검 `vector`를 왼쪽부터 훑어, 입력 수명과 독립된 `BatchSummary`/`AuditReport` 소유 값 하나로 materialize한다. 공유 누산기를 밖에서 변경하는 대신 누산기 소유권을 reducer 호출 사이로 전달하는 functional-core 경계다.
- **대표 형태와 선택된 overload**: 공개 매개변수만 단순화한 범위 overload는 `template<std::ranges::input_range R, class T, class F> constexpr auto fold_left(R&& range, T init, F f);` 형태다. 실제 참여 조건에는 표준이 설명 전용으로 정의한 left-foldable 제약이 붙으며, 그 설명용 이름은 프로그램이 직접 쓸 라이브러리 API가 아니다. iterator/sentinel overload도 있다. 결과 누산기 타입 `U`는 첫 `invoke(f, std::move(init), *first)` 결과를 decay한 타입을 바탕으로 정해지며 단순히 언제나 선언한 `T`라고 가정하면 안 된다. 오늘 reducer는 `U`와 `T`가 같은 보고서 타입이다.
- **수신 객체·입력 상태**: ranges 알고리즘 함수 객체의 호출 연산이므로 사용자 데이터 수신 객체는 없다. 범위는 유효한 `input_range`여야 하고 `[begin,end)`가 순회가 끝날 때까지 유효해야 한다. 빈 범위에는 원소 접근이나 reducer 호출이 없고 초기값에서 결과를 만든다. 오늘의 `const vector&`는 호출 동안 구조와 원소를 바꾸지 않는다.
- **매개변수·값 범주·소유권**: `R&&`는 forwarding reference라 lvalue 범위는 빌리고 rvalue 소유 범위는 해당 값 범주로 받는다. `init`와 `f`는 값으로 전달된다. 개념적으로 각 단계는 현재 누산기를 `std::move(accum)`인 xvalue로 reducer 첫 인자에, 현재 범위 원소를 역참조 결과의 값 범주로 둘째 인자에 넘긴다. 따라서 이동 전용 누산기도 조건을 만족할 수 있지만, 이동 뒤의 이전 누산기를 reducer가 다시 읽으면 안 된다. 함수 객체 내부의 포인터·view·참조는 별도 비소유 수명을 유지한다.
- **반환형·반환값·사후 상태**: 최종 `U` 값을 반환한다. 오늘 호출부는 그 prvalue로 이름 있는 const 보고서를 직접 초기화한다. 성공하면 반환값이 카운트와 문자열을 소유하고 원본 범위는 그대로다. 빈 범위 결과는 초기값을 `U`로 변환한 값이다. reducer가 원본 원소를 비const 참조로 받아 바꾸거나 rvalue 원소를 소비할 수 있는 일반 호출이라면 그 부수 효과는 해당 reducer 계약을 따른다.
- **정확성 불변식**: `k`개 원소를 처리한 누산기는 `f(...f(f(init,e0),e1)...,e[k-1])`와 같은 왼쪽 결합 결과다. `fold_left`는 결합법칙이나 교환법칙을 요구하지 않으며 순서를 임의로 재배열하는 병렬 reduce가 아니다. 문자열 연결, 첫 실패 기록처럼 순서에 민감한 reducer에도 사용할 수 있다.
- **복잡도·할당**: 범위 크기 `N`에 reducer를 정확히 `N`번 적용하고 iterator를 선형으로 전진하므로 알고리즘 오버헤드는 `O(N)`이다. 전체 비용은 reducer의 복사·이동·할당을 더한다. 알고리즘 자체가 별도 동적 저장소를 요구한다는 보장은 없지만 값 누산기의 `string`·`vector` 연산은 할당할 수 있으므로 “항상 allocation-free”라고 단정하지 않는다.
- **반복자·참조 무효화와 수명**: 읽기 전용 순회 자체는 vector의 반복자·참조를 무효화하지 않는다. reducer가 기반 컨테이너를 구조 변경하면 현재 iterator/sentinel이 무효화되어 이후 동작이 미정의가 될 수 있다. 반환값에 원소의 pointer, iterator, `string_view`, `reference_wrapper`를 저장하면 fold가 소유 값으로 반환돼도 가리킨 대상 수명은 연장되지 않는다. 오늘 보고서는 문자열을 깊게 복사해 그 위험을 끊는다.
- **오류·예외 보장**: iterator 연산, 초기값/함수 객체 이동, `invoke`, 누산기 대입·구성이 던진 예외를 호출자에게 전파할 수 있다. 이미 실행한 reducer의 외부 부수 효과와 이미 이동된 원본은 자동 rollback되지 않는다. 오늘 입력은 const이고 reducer의 외부 부수 효과가 없어서 실패해도 원장/점검 목록은 그대로다. string 할당 실패는 `bad_alloc`, 크기 한계는 선택된 문자열 연산의 `length_error`로 나타날 수 있다.
- **전제조건·컴파일 오류·미정의 동작**: 범위와 reducer가 표준의 left-foldable 개념/의미 요구를 만족하지 않으면 적합한 overload가 없어 컴파일되지 않는다. 댕글링 범위, 무효 반복자, 누산기 이동 뒤 잘못된 관찰, reducer가 순회 중 같은 vector를 재할당하는 행위, 부호 있는 정수 오버플로처럼 언어 전제조건을 깨는 연산은 미정의 동작으로 이어질 수 있다. 오늘 원장 예제는 모든 중간 합이 `long long`에 표현 가능하다는 도메인 전제를 둔다.
- **스레드 보장**: 자체 동기화를 제공하지 않는다. 서로 다른 소유 범위를 각각 접는 호출은 독립적으로 실행할 수 있다. 같은 범위를 읽기만 하는 호출은 원소/함수 객체의 규칙을 따르지만, 다른 실행 흐름이 같은 구조나 원소를 동기화 없이 쓰면 데이터 경쟁 또는 iterator 무효화가 발생한다.

기계 실행 관점에서는 범위 반복마다 iterator 전진, 끝 비교, 원소 load, reducer 호출과 누산기 store가 생길 수 있다. 구체 reducer와 타입이 보이면 호출이 인라인되고 불필요한 이동·분기가 사라질 수 있지만, 실제 명령·벡터화·복사 생략·메모리 배치는 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라진다.

### 최소 실행 예제

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    const std::vector<int> values{3, 1, 4};
    const int decimal{std::ranges::fold_left(
        values,
        0,
        [](int accumulated, int value) { return accumulated * 10 + value; })};
    std::cout << decimal << '\n'; // 314: 왼쪽 결합이라 순서가 보존된다.
}
```

## `std::swap`과 `std::ranges::swap`

- 두 객체의 값을 교환한다. 사용자 타입은 이동 생성·이동 대입 또는 사용자 정의 `swap`을 사용할 수 있다.
- 일반 템플릿 코드에서는 `using std::swap; swap(a,b);`로 ADL 사용자 정의 교환을 허용하는 관용구가 있다.
- Union-Find에서 랭크가 큰 루트를 왼쪽에 두는 등 불변식을 단순화할 때 사용한다.

## `std::generator<Ref, Val, Allocator>` — `<generator>`의 C++23 동기 lazy input range

- **항목 종류·현재 역할**: coroutine이 `co_yield`에 도달할 때마다 원소 하나를 생산하는 이동 전용 class template이자 `view`·`input_range`다. 2026-09-25의 `std::generator<const MetricSample&>`와 `std::generator<const StockItem&>`는 별도 필터 결과 컨테이너를 먼저 만들지 않고 frame-owned 배치의 원소를 읽기 전용 참조로 지연 노출한다.
- **대표 선언**: `template<class Ref, class Val = void, class Allocator = void> class generator;`, 삭제된 복사 생성자, `generator(generator&&) noexcept`, `iterator begin()`, `default_sentinel_t end() const noexcept`가 핵심이다. `Ref=const MetricSample&`, `Val=void`이면 설명용 `value`는 `MetricSample`, iterator의 `reference`와 공개 `yielded`는 참조 축약 뒤 모두 `const MetricSample&`다.
- **생성·소유권**: coroutine 함수 호출은 coroutine state를 소유하는 generator prvalue를 반환하고 initial suspend 때문에 본문은 아직 실행하지 않는다. generator는 **frame**을 소유하지만 외부 컨테이너 원소를 yield했다면 그 원소를 자동 소유하지 않는다. 오늘 코드는 owner를 coroutine 값 매개변수로 이동해 frame 안에 두므로 원소 수명을 generator에 묶는다.
- **숨은 coroutine protocol**: 호출 시 구현이 필요하면 `promise_type::operator new(size_t)`로 frame 저장소를 확보하고 promise와 매개변수 사본을 만든다. `get_return_object() noexcept`가 generator owner를 만들고 `initial_suspend() const noexcept`가 본문 전 실행을 멈춘다. 정상 본문 끝은 `return_void() const noexcept` 뒤 `final_suspend() noexcept`로 owner가 파괴할 때까지 frame을 유지한다. 본문 예외는 `unhandled_exception()` 경로로 처리돼 재개 호출자에게 전달된다. generator 소멸자는 보유 handle이 있으면 frame을 destroy하고 대응 delete가 저장소를 해제하므로 iterator·frame 지역·참조 수명도 끝난다.
- **첫 `begin()` 계약**: 수신 generator는 유효하고 initial suspend 지점을 가리켜야 한다. `begin()`은 active coroutine stack에 handle을 등록한 뒤 coroutine을 첫 `co_yield` 또는 종료까지 재개하고 같은 coroutine을 가리키는 iterator 값을 반환한다. `begin()`은 `noexcept`가 아니며 active-stack bookkeeping과 최초 재개가 실패할 수 있다. 같은 generator에 `begin()`을 두 번 호출하는 것은 미정의 동작이다. 비const 멤버라 const generator를 일반 range-for로 순회할 수도 없다.
- **`end`, 역참조, 증가와 비교**: `end()`는 상태를 바꾸지 않고 default sentinel 값을 반환한다. 유효한 yield 지점에서 iterator `operator*`는 현재 `reference`, 오늘은 `const T&`를 반환한다. `operator++`는 다음 yield 또는 종료까지 coroutine을 재개하고 iterator&를 반환한다. sentinel 비교는 coroutine 완료 여부를 bool로 돌려준다.
- **`co_yield`/`yield_value`**: 참조형 specialization에서 `co_yield sample`은 promise의 `yield_value(const T&)`에 현재 원소 주소를 연결하고 `suspend_always`로 중단한다. T를 복사하지 않지만 원소 owner가 안정적으로 살아 있어야 한다. 값형 specialization은 선택된 `yield_value` overload에 따라 별도 임시 저장 수명이 개입할 수 있으므로 참조형 규칙을 무조건 일반화하지 않는다.
- **단일 통과**: generator는 `forward_range`가 아니라 `input_range`다. iterator나 현재 참조를 복제해 독립적으로 여러 번 순회한다는 가정을 할 수 없다. 멀티패스·정렬·이진 탐색이 필요하면 원소를 소유 컨테이너로 materialize한다.
- **수명·무효화**: generator 파괴는 frame과 frame 지역/값 매개변수를 파괴해 iterator와 그 객체를 가리키는 참조를 무효화한다. 외부 vector 원소를 yield했다면 vector 파괴·재할당·관련 erase도 참조를 댕글링시킨다. 현재 참조를 iterator 증가 뒤 보관하려면 yield 대상 주소·수명이 계속 안정적임을 별도로 증명해야 한다. generator 이동은 frame 소유권을 목적 객체로 넘기며 기존 iterator는 목적 generator 쪽 coroutine을 계속 가리킨다.
- **복잡도·할당**: coroutine state 저장소가 필요할 수 있어 생성이 항상 allocation-free라고 단정하지 않는다. 첫 `begin()`의 active-stack bookkeeping도 자원을 확보할 수 있다. `begin()`과 각 증가는 다음 yield까지 사용자 코드를 실행하므로 고정 `O(1)`이 아니다. 오늘처럼 N개 원소를 한 번 검사하고 원소당 상수 작업이면 전체가 `O(N)`이며 generator 자체는 N개짜리 결과 컨테이너를 만들지 않는다.
- **오류·예외**: 생성은 frame 할당에 실패할 수 있고, `begin()`은 active-stack bookkeeping 또는 최초 resume에서 실패할 수 있으며, iterator 증가는 재개된 본문의 예외를 전파할 수 있다. 참조형 `yield_value` 자체는 복사를 요구하지 않는다. 두 번째 begin, 종료 iterator 역참조·증가, generator나 owner 파괴 뒤 iterator·참조 사용은 미정의 동작이다.
- **스레드 보장**: 스레드를 만들거나 동기화를 제공하지 않는 동기식 범위다. 같은 generator를 동시에 resume·이동·파괴하면 안 되며 yield 원소의 동시 읽기·쓰기도 원본 타입의 동기화 규칙을 따른다.
- **설계 경계**: 외부 owner를 빌리는 generator라면 owner가 순회보다 오래 산다는 API 보장이 필요하다. 오늘은 owner를 frame에 값으로 넣고, owner 관찰자는 `items() const &`만 허용하며 `items() const && = delete`로 임시에서 참조를 꺼내는 별도 경로도 막는다.

기계 실행 관점에서 frame 생성은 상태 저장소 확보, `begin`/증가는 저장된 재개 지점 load와 간접 제어 이동, `co_yield`는 현재 주소와 중단 상태 store를 포함할 수 있다. 실제 할당 제거, 프레임 배치, 분기와 인라인 여부는 CPU·ABI·표준 라이브러리·컴파일러·최적화 옵션에 따라 달라 특정 명령으로 단정하지 않는다.

### 최소 실행 예제

```cpp
#include <generator>
#include <iostream>

std::generator<int> countdown(int value) {
    while (value > 0) {
        co_yield value--;
    }
}

int main() {
    for (const int value : countdown(3)) {
        std::cout << value << ' ';
    }
}
```

## `std::views::filter`와 `std::views::transform` — `<ranges>`

- `filter(predicate)`는 술어가 참인 원소만 지연 노출한다.
- `transform(function)`은 원소를 함수 결과로 지연 투영한다.
- 파이프 `source | views::filter(...) | views::transform(...)`는 보통 중간 컨테이너를 즉시 만들지 않는다.
- 뷰는 원본을 소유하지 않을 수 있고 술어/변환 객체를 보관한다. 원본과 캡처한 참조의 수명이 뷰보다 길어야 한다.
- 실제 함수 호출은 순회할 때 발생하므로 뷰를 만들기만 해서는 부수 효과가 실행되지 않을 수 있다.
- 같은 뷰를 여러 번 순회할 수 있는지는 기반 범위와 뷰 종류의 범주에 따라 다르다.
- `forward_range` 위 `filter_view`는 첫 통과 반복자를 cache할 수 있다. 이미 순회를 시작한 view는 술어가 보는 원소를 바꾸거나 기반 `vector`를 재할당한 뒤 재사용하지 말고 새로 만든다. 원본 객체 파괴는 view를 dangling으로 만들며, 재할당은 이미 얻었거나 cache한 반복자를 무효화한다.

## `std::views::zip`과 `std::ranges::zip_view` — `<ranges>`의 C++23 lockstep view

`zip`은 여러 범위를 같은 위치끼리 묶어 하나의 행처럼 지연 노출한다. 2026-09-23 코드는 이름·현재값·기준값 세 열을 각각 소유하는 **길이가 같다고 검증된 columnar storage**를 유지하고, 서비스 경계에서는 복사된 행 컨테이너 대신 `zip_view`를 잠깐 빌려 row projection으로 순회한다. 각 열을 따로 저장하는 구조와 행 단위 처리 코드를 연결하되, `zip` 자체가 열 길이 불변식이나 소유권을 대신 관리한다고 오해하면 안 된다.

- **항목 종류·헤더·현재 역할**: `std::views::zip`은 `<ranges>`가 선언하는 C++23 range adaptor 객체이며 평범한 사용자 정의 자유 함수가 아니다. 호출 결과의 공개 타입은 `std::ranges::zip_view<Views...>` class template이다. 오늘 `LatencyTable::rows() const &`와 `StockTable::rows() const &`는 소유 `vector` 열을 빌리는 작은 view 값을 반환하고, `&&`와 `const &&` overload를 모두 삭제해 곧 파괴될 임시 table에서 비소유 view를 꺼내지 못하게 한다.
- **대표 형태·템플릿 인자**: 설명을 위해 단순화한 호출 형태는 `template<std::ranges::viewable_range... Rs> constexpr auto std::views::zip(Rs&&... ranges);`다. 각 `Rs`는 호출식에서 추론되고 결과는 의미상 `std::ranges::zip_view<std::views::all_t<Rs>...>`다. 공개 class template의 대표 제약은 `template<std::ranges::input_range... Views> requires (std::ranges::view<Views> && ...) && (sizeof...(Views) > 0) class zip_view;`다. 인자 없는 `views::zip()`은 빈 `tuple` 원소의 `empty_view`를 반환하는 별도 경우이고, 오늘 코드는 세 범위를 넘기는 overload를 사용한다.
- **수신 객체·호출 전 상태**: 함수처럼 보이는 adaptor 객체의 호출이므로 열 데이터를 담은 사용자 수신 객체는 없다. 각 인자는 유효한 `viewable_range`이고 `views::all_t<Rs>`가 `input_range`를 만족해야 한다. 오늘 세 `vector`는 살아 있는 `LatencyTable` 또는 `StockTable`의 lvalue 멤버이고, 생성 경계에서 세 열의 `size()`가 같은지 검사한 뒤 호출한다. `zip`은 이 동등성을 검사하지 않는다.
- **입력 식·값 범주·소유권**: 오늘 service/item 이름 열, 관측/재고 열, 기준 열은 모두 const vector lvalue 식으로 전달된다. 각 `views::all_t<const vector<T>&>`는 보통 해당 컨테이너를 가리키는 `ref_view`가 되므로 결과는 열 원소를 복사하거나 소유하지 않는다. 일반적으로 movable한 rvalue 비-view 범위는 `views::all`의 `owning_view`에 이동되어 결과가 그 범위를 소유할 수 있지만, 이것을 lvalue 호출에도 적용되는 규칙으로 일반화하면 안 된다. view가 보관한 사용자 정의 view 객체 내부의 포인터·참조 수명도 별도로 점검한다.
- **반환형·반환값 사용·사후 상태**: 오늘의 세 lvalue vector 호출은 개념상 세 `ref_view`를 템플릿 인자로 가진 `zip_view` prvalue를 반환한다. 호출부는 이를 `rows()`에서 값으로 반환하고 range-for가 즉시 소비한다. 작은 view 객체의 값 반환은 원소 복사가 아니며, prvalue 직접 초기화에서는 불필요한 같은 타입 중간 객체가 필요 없다. 생성 직후 원본 열의 크기·용량·원소는 바뀌지 않고 iterator도 무효화되지 않는다.
- **최단 범위 종료와 `size()`**: 순회는 **어느 한 기반 범위라도 끝에 도달하면** 종료한다. 따라서 길이 5와 3을 zip하면 오류나 예외 없이 3행만 보인다. 이것은 정의된 동작이지만, 열 정렬이 필수인 도메인에서는 뒤 두 원소를 조용히 잃는 논리 버그다. 모든 기반 view가 `sized_range`일 때 대표 형태 `constexpr auto size() requires (std::ranges::sized_range<Views> && ...);`가 제공되며, 인자 없이 각 크기의 최솟값을 공통의 unsigned-like 크기 타입으로 반환한다. 오늘은 생성 시 길이를 같게 검증했으므로 반환값이 모든 열의 길이와 같다. `size()`는 불변식을 새로 검증하지 않는다.
- **`begin`/`end`와 iterator 진행 계약**: `begin()`은 각 기반 view의 시작 iterator를 tuple로 보관한 zip iterator를 만들고, `end()`는 기반 범주의 성질에 맞는 iterator 또는 sentinel을 만든다. 역참조 가능한 위치에서 `operator*`는 각 현재 iterator를 한 번씩 역참조하고, `operator++`는 모든 현재 iterator를 한 칸씩 전진시킨다. 끝 비교는 어느 구성 iterator라도 해당 끝에 닿으면 순회를 끝내는 최단 범위 의미를 구현한다. iterator concept와 지원 연산은 기반 범위 중 가장 약한 능력보다 강해지지 않으며, legacy `iterator_category`는 별도로 input category가 될 수 있다. 따라서 하나가 input range뿐이면 전체를 임의 접근 범위라고 가정할 수 없다.
- **참조 tuple과 구조적 바인딩**: iterator 역참조 결과는 `std::tuple<std::ranges::range_reference_t<Views>...>` 형태의 prvalue다. 일반 mutable `vector<string>`과 `vector<int>`에서는 `tuple<string&, int&>`이므로 `for (auto&& [name, quantity] : rows)`의 두 이름은 원본 원소를 빌리고 `quantity` 대입은 수량 열을 바꾼다. 오늘 코드는 `const LatencyTable&`/`const StockTable&`의 열을 zip하므로 구조적 바인딩이 `const` 원소 참조를 빌리고, 결과의 문자열만 별도 소유 객체로 깊게 복사한다. 일반 범위의 reference 타입은 `vector<bool>` 같은 proxy일 수도 있으므로 항상 실제 `T&`라고 단정하지 않는다. 또한 `const zip_view` 자체가 가리키는 원소까지 자동으로 const로 만들지는 않는다. 읽기 전용 행이 필요하면 오늘처럼 const owner의 열을 zip한다.
- **전제조건·후조건과 아키텍처 불변식**: 순회 동안 모든 기반 범위와 그 iterator/sentinel이 유효해야 한다. 성공적으로 view를 만드는 것만으로는 원소를 읽거나 사용자 동작을 실행하지 않는다. 행을 순회해 읽기만 하면 원본은 그대로이고, non-const reference 원소에 대입하면 바로 해당 열이 변경된다. 오늘 batch는 생성·교체 시에만 열 길이를 함께 바꾸고 외부에는 구조 변경 API를 노출하지 않아 equal-length 불변식을 유지한다. 한 열에만 `push_back`하는 API를 추가한다면 호출 뒤 불변식을 다시 세우기 전에는 기존 행 view를 사용하지 않는다.
- **복잡도·할당**: 기반 범위 수를 `K`, 최단 길이를 `N`이라 하면 view 생성, `begin`, `end`, `size`, 한 번의 역참조와 전진은 각각 기반 연산을 `K`개 조합해 `O(K)`이고, 전체 순회는 `O(KN)`이다. 오늘처럼 `K=3`이 타입에 고정돼 있으면 원소 수에 대해서는 생성·행당 오버헤드가 상수이고 전체는 `O(N)`이다. `zip_view` 자체는 행 저장소를 만들거나 원소를 복사하지 않으며 동적 할당을 요구하지 않는다. 단, 사용자 정의 view의 복사·이동·`begin` 연산 또는 루프 본문이 수행하는 문자열/컨테이너 연산은 별도로 할당할 수 있다.
- **반복자·참조 무효화**: 오늘 `ref_view` 기반 zip 객체는 vector 객체를 계속 가리키므로 vector 재할당만으로 그 작은 view 객체 안의 owner 주소가 바뀌지는 않는다. 그러나 재할당 전에 얻은 zip iterator와 역참조 결과의 원소 참조는 각 vector의 무효화 규칙에 따라 댕글링된다. `erase`, `insert`, `push_back`, 이동 대입처럼 기반 iterator를 무효화할 수 있는 연산을 진행 중인 순회와 섞지 않는다. 구조 변경 뒤 새 `begin()`을 얻더라도 한 열만 길이가 달라졌다면 zip은 새 최단 길이에서 조용히 끝나므로 도메인 불변식은 별도로 복구해야 한다.
- **수명·dangling·borrowed range**: lvalue 컨테이너로 만든 오늘 view는 `LatencyTable`/`StockTable`의 수명을 연장하지 않는다. owner 파괴 뒤 view, iterator, 참조 tuple 또는 구조적 바인딩 이름을 사용하면 미정의 동작이다. 특히 임시 table의 `rows()` 결과를 반환하거나 저장하지 않도록 non-const/const rvalue overload를 모두 삭제한다. `zip_view`는 모든 기반 view가 `borrowed_range`일 때만 borrowed range가 되지만, borrowed라는 표지는 원본 저장소를 살려 주는 소유권이 아니라 view 임시가 사라져도 iterator가 별도 owner를 계속 가리킬 수 있다는 뜻이다.
- **오류·예외·컴파일 실패·미정의 동작**: 입력이 필요한 range/view 제약을 만족하지 않으면 적합한 호출이 없어 컴파일에 실패하며 런타임 오류값을 반환하지 않는다. `views::all` 변환, 기반 view 복사·이동, `begin`/`end`, iterator 연산과 원소 연산이 던지는 예외는 일반적으로 전파될 수 있다. 오늘의 vector lvalue wrapper 생성은 행 저장소를 할당하거나 원소 연산을 하지 않지만 공개 호출 전체를 근거 없이 항상 `noexcept`라고 단정하지 않는다. 길이 불일치는 예외나 UB가 아니라 최단 길이 결과다. 반면 끝 iterator 역참조, owner 파괴 또는 iterator 무효화 뒤 접근, 의미 요구를 위반한 사용자 range, 데이터 경쟁은 미정의 동작으로 이어질 수 있다. 루프 본문 중 예외가 나면 앞서 변경한 원소를 `zip`이 rollback하지 않는다.
- **스레드 보장**: `zip_view`는 잠금이나 원자성을 제공하지 않는다. 서로 독립된 owner의 view는 각 owner 규칙에 따라 독립적으로 쓸 수 있고, 같은 owner를 여러 실행 흐름이 읽기만 하는 경우도 원소 타입의 const-read 규칙을 따른다. 한 실행 흐름이 열 구조나 같은 원소를 쓰는 동안 다른 흐름이 동기화 없이 순회·접근하면 iterator 무효화 또는 데이터 경쟁이 생길 수 있다. 여러 열을 한 행처럼 보인다고 해서 열 사이의 원자적 snapshot이 생기는 것도 아니다.

기계 실행 관점에서 zip iterator는 여러 기반 iterator를 묶은 상태로 구현될 수 있고, 한 행마다 끝 비교, 각 열의 원소 load, 루프 본문의 비교·조건 분기와 필요한 store가 생길 수 있다. `zip`은 columnar storage를 row-major 메모리로 재배치하거나 SIMD gather를 보장하지 않는다. 구체 타입과 반복 횟수가 보이면 컴파일러가 tuple/adaptor 층을 인라인하고 비교를 단순화할 수 있지만, 실제 명령, 메모리 접근 순서, 벡터화와 복사 생략은 CPU, ABI, 표준 라이브러리, 컴파일러 및 최적화 옵션에 따라 달라진다.

### 최소 실행 예제

```cpp
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

int main() {
    std::vector<std::string> names{"cache", "api"};
    std::vector<int> quantities{7, 11};

    // 실제 타입에서는 이 검사를 생성 경계에 두어 이후 모든 rows() 호출의 불변식으로 만든다.
    if (names.size() != quantities.size()) {
        return 1;
    }

    // 두 lvalue vector는 소유권을 넘기지 않고 ref_view로 감싸진다.
    auto rows{std::views::zip(names, quantities)};
    std::cout << rows.size() << '\n';

    // 역참조 결과는 tuple<string&, int&>이므로 quantity 대입은 원본 열을 바꾼다.
    for (auto&& [name, quantity] : rows) {
        ++quantity;
        std::cout << name << ':' << quantity << '\n';
    }
}
```

### 흔한 실수와 점검 질문

1. 길이가 다른 두 열을 zip한 뒤 예외가 날 것이라 기대한다. 실제 종료 길이와 누락되는 원소를 계산해 본다.
2. `const auto rows = views::zip(mutable_vector, ...)`만으로 원소가 읽기 전용이 된다고 생각한다. const owner를 zip하는 설계와 차이를 설명한다.
3. `auto&& [name, quantity]`가 행 값을 복사한다고 생각한다. 역참조 tuple의 각 원소 타입과 대입의 실제 대상을 적는다.
4. 임시 owner의 멤버에서 반환한 zip view를 다음 문장에서 사용한다. 어느 전체 표현식 끝에 owner가 파괴되고 무엇이 dangling이 되는지 추적한다.
5. 순회 중 한 vector에 `push_back`해 재할당을 일으킨다. zip 객체 자체, 이미 얻은 iterator, 구조적 바인딩 참조를 나누어 유효성을 판단한다.
6. 기반 범위 하나가 input range이고 다른 하나가 random-access range일 때 zip 결과가 제공할 수 있는 iterator 능력을 설명한다.
7. `zip_view::size()`가 equal-length 검증 함수가 아닌 이유와 오늘 생성 경계에서 별도 검사가 필요한 이유를 말한다.

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
7. 길이가 4와 2인 두 vector를 `views::zip`했을 때 `size()`와 순회 횟수를 말하고, 이것이 오류가 아닌 이유를 설명한다.
8. mutable lvalue vector 두 개의 zip iterator를 역참조한 tuple과 `auto&&` 구조적 바인딩이 각 원소를 소유하는지 빌리는지 설명한다.
