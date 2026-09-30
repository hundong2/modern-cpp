# 알고리즘, ranges, views

표준 알고리즘은 컨테이너 자체보다 반복자·범위와 호출 가능 객체를 받는다. 알고리즘을 호출하기 전에 입력 범위, 반환값, 비교자/술어의 의미, 원소 재배치 여부를 확인한다.

## `std::sort`와 `std::ranges::sort` — `<algorithm>`

- **항목 종류·현재 역할**: `<algorithm>`의 `std::sort`는 반복자 기반 함수 템플릿이고 `std::ranges::sort`는 C++20 ranges 알고리즘 함수 객체(niebloid)다. 2026-09-25 풀이는 ranges overload로 오른쪽 절반 부분집합 합을 정렬하고, 2026-09-30 CSES 1144 풀이는 고전 두-반복자 overload로 `vector<int>` 급여 좌표를 정렬해 뒤의 `unique`와 이진 탐색 전제조건을 만든다.
- **C++23 overload와 템플릿 인자**: 고전 형태는 `template<class RandomIt> constexpr void sort(RandomIt first, RandomIt last);`와 `template<class RandomIt, class Compare> constexpr void sort(RandomIt first, RandomIt last, Compare comp);`다. 2026-09-30의 `sort(sort_first,sort_last)`는 `RandomIt=std::vector<int>::iterator`인 첫 형태이고, C++20 이후 생략된 비교는 `std::less{}` 의미를 쓴다. ranges 형태는 `template<random_access_range R, class Comp = ranges::less, class Proj = identity> requires sortable<iterator_t<R>, Comp, Proj> constexpr borrowed_iterator_t<R> sort(R&& range, Comp comp = {}, Proj projection = {});`다. 2026-09-25 호출에서는 `R=std::vector<long long>&`, `Comp=std::ranges::less`, `Proj=std::identity`다.
- **수신 객체·호출 전 상태**: 고전 자유 함수와 ranges 함수 객체 모두 사용자 데이터 수신 객체는 없다. 오늘의 `[sort_first,sort_last)`는 같은 살아 있는 mutable `vector<int>`의 유효한 random-access 범위이고, ranges 예제의 `right_sums`도 유효한 non-const lvalue vector다. 원소는 살아 있고 이동·교환 가능하며 정렬 전 순서는 임의여도 된다.
- **매개변수·값 범주·소유권**: 고전 호출의 두 iterator lvalue는 값 복사되어 같은 저장소의 반열린 범위를 비소유로 가리킨다. ranges 호출의 `R&&`는 forwarding reference로 lvalue `right_sums`에 바인딩된다. 어느 쪽도 vector 저장소를 복사·이동하지 않는다. 생략된 비교는 각각 `std::less{}`와 `std::ranges::less{}`이고 ranges projection은 `identity{}`다. 사용자 비교자·projection은 실행 동안 유효해야 하며 정렬 관계를 깨는 부작용을 내면 안 된다.
- **반환형·사용 여부**: 고전 `std::sort`의 반환형은 `void`라 2026-09-30 코드는 재배치 효과만 쓴다. ranges 호출은 lvalue vector가 borrowed range이므로 `std::vector<long long>::iterator`를 반환하고 2026-09-25 코드는 그 끝 iterator를 버린다. 임시 non-borrowed range라면 `borrowed_iterator_t<R>`가 `std::ranges::dangling`일 수 있다.
- **사후 상태·안정성**: 성공하면 projected 원소가 비교자 기준 비내림차순이고 원소들은 입력의 permutation이다. 크기·capacity·저장소 소유권은 유지되지만 위치별 값은 바뀐다. 같은 키의 상대 순서는 보존되지 않으므로 필요하면 `std::stable_sort`를 검토한다.
- **복잡도**: 원소 수를 `N`이라 할 때 `O(N log N)`회의 비교와 projection을 요구한다. 원소 이동·교환 비용은 타입에 따르고, ranges overload의 끝 iterator 계산 비용도 sentinel 성질에 따른다.
- **할당·무효화·수명**: 표준 계약만으로 구현의 보조 저장소 사용이 없다고 단정하지 않는다. vector 자체의 구조 변경이나 재할당은 하지 않으므로 기존 iterator·포인터·참조는 계속 유효하지만, 같은 위치가 정렬 전과 같은 논리 원소를 뜻하지는 않는다. range와 원소는 호출이 끝날 때까지 살아 있어야 한다.
- **전제조건·오류·예외·미정의 동작**: 범위는 random-access range이고 iterator는 permutable해야 하며 `comp(proj(a), proj(b))`가 전체 실행 동안 엄격 약순서를 이뤄야 한다. `<=`처럼 엄격하지 않은 비교자, 댕글링 iterator/range, 이동·교환 요구사항 위반은 전제조건 위반이며 미정의 동작이다. 비교·projection·원소 이동/교환 또는 구현의 자원 확보가 던지면 예외가 전파될 수 있고, 그때 범위는 유효하더라도 원래 순서나 완전 정렬을 보장하지 않는다. 오늘의 `long long` 기본 비교·이동은 던지지 않지만 함수 선언 자체를 `noexcept`로 가정하지 않는다.
- **스레드·기계 실행 관점**: 자체 동기화가 없으므로 같은 vector를 다른 실행 흐름이 동시에 읽거나 쓰는 동안 정렬하면 안 된다. 구현은 비교, load/store, 원소 교환과 조건 분기를 조합할 수 있으나 구체적 정렬 전략·SIMD·명령열·보조 메모리는 CPU, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라진다.

## `std::unique`와 `std::ranges::unique` — `<algorithm>`의 인접 중복 압축

- **항목 종류·현재 역할**: `<algorithm>`의 `std::unique`는 제자리 변경 함수 템플릿이고 `std::ranges::unique`는 C++20 ranges 알고리즘 함수 객체(niebloid)다. 둘 다 **연속해서 인접한** 동등 원소만 한 대표 원소로 압축한다. 2026-09-30 CSES 1144 풀이는 먼저 급여 좌표를 정렬해 같은 값이 인접한다는 불변식을 만든 뒤 고전 `std::unique`가 돌려준 논리 끝을 `vector::erase`에 넘겨 실제 크기까지 줄인다.
- **C++23 고전 overload의 정확한 형태**: 순차 형태는 `template<class ForwardIt> constexpr ForwardIt unique(ForwardIt first, ForwardIt last);`와 `template<class ForwardIt, class BinaryPredicate> constexpr ForwardIt unique(ForwardIt first, ForwardIt last, BinaryPredicate pred);`다. 실행 정책 형태는 앞에 `template<class ExecutionPolicy, class ForwardIt>` 또는 `template<class ExecutionPolicy, class ForwardIt, class BinaryPredicate>`가 붙고 첫 함수 인자로 `ExecutionPolicy&& policy`를 받으며, `remove_cvref_t<ExecutionPolicy>`가 표준 실행 정책으로 인식될 때만 overload 후보가 된다. 오늘 호출은 정책·술어가 없는 순차 형태에서 `ForwardIt=std::vector<int>::iterator`이고, 생략한 술어 의미는 C++20 이후 `std::equal_to{}`다.
- **C++23 ranges overload와 제약**: iterator/sentinel 형태는 `template<permutable I, sentinel_for<I> S, class Proj = identity, indirect_equivalence_relation<projected<I, Proj>> C = ranges::equal_to> constexpr subrange<I> ranges::unique(I first, S last, C comp = {}, Proj proj = {});`다. range 형태는 `template<forward_range R, class Proj = identity, indirect_equivalence_relation<projected<iterator_t<R>, Proj>> C = ranges::equal_to> requires permutable<iterator_t<R>> constexpr borrowed_subrange_t<R> ranges::unique(R&& range, C comp = {}, Proj proj = {});`다. C++23 ranges에는 실행 정책 overload가 없으며, `permutable`은 forward iterator·간접 이동 저장·교환 요구를 묶는다.
- **수신 객체·호출 전 상태**: 자유 함수 또는 함수 객체 호출이므로 사용자 데이터 수신 객체는 없다. 오늘 `[sort_first,sort_last)`는 같은 살아 있는 mutable `std::vector<int>`의 유효한 random-access 범위이며 직전 `sort`로 오름차순이다. `unique` 자체는 정렬을 요구하지 않지만, 정렬되지 않은 `1,2,1`에서는 세 값 모두 서로 인접한 중복이 아니므로 그대로 남는다. 빈 범위도 유효하다.
- **매개변수·값 범주·소유권**: 두 iterator는 lvalue에서 값 복사되어 범위를 비소유로 가리킨다. 사용자 `pred`와 ranges의 `comp`·`proj`는 값으로 전달되어 알고리즘이 복사할 수 있다. 알고리즘은 살아남을 원소를 앞쪽 원소 슬롯에 이동 대입할 수 있지만 vector 버퍼나 컨테이너 소유권을 옮기지는 않는다. 오늘 `int` 이동 대입은 값 복사와 같은 효과이고 별도 자원을 소유하지 않는다.
- **동등 관계·안정성 불변식**: 술어는 반사성·대칭성·추이성을 갖는 동등 관계여야 하며 ranges overload는 projection 결과에 대한 `indirect_equivalence_relation`을 요구한다. 각 인접 run에서 첫 원소가 대표로 남고 살아남은 대표들의 상대 순서는 보존된다. `<=`, “차이가 10 이하”처럼 동등 관계가 아닌 술어를 쓰거나 술어가 비교 중 원소를 변경하면 계약을 깨뜨릴 수 있다. 전역 중복 제거가 목적이면 오늘처럼 먼저 같은 기준으로 정렬하거나 별도 집합 자료구조를 사용한다.
- **반환형·반환값 사용**: 고전 overload는 결과 논리 범위 `[first,j)`의 끝 `ForwardIt j`를 반환한다. ranges overload는 제거 대상 꼬리 `[j,last)`를 나타내는 `subrange<I>{j,last}` 또는 range 값 범주에 따른 `borrowed_subrange_t<R>`를 반환한다. 오늘 코드는 고전 반환값을 `unique_end`에 저장해 `coordinates.erase(unique_end,sort_last)`의 시작으로 사용한다. 빈 범위면 `j==last`, 중복이 없으면 역시 원래 `last`다.
- **사후 상태와 erase-remove 계열 관용구**: 성공하면 `[first,j)`에는 각 연속 동등 run의 첫 대표만 원래 순서대로 있고, `[j,last)` 원소는 여전히 살아 있어 파괴·대입할 수 있지만 값은 **유효하지만 미지정 상태**다. vector의 물리적 `size()`와 `capacity()`는 `unique`만으로 바뀌지 않는다. 오늘 뒤이은 범위 `erase`가 꼬리 객체를 파괴하고서야 실제 크기가 줄어든다. 따라서 반환 iterator를 버리고 원래 `end()`까지 출력하면 논리적으로 제거한 꼬리까지 관찰하는 버그다.
- **복잡도**: `N=distance(first,last)`일 때 빈 범위는 술어 호출이 없고, 비어 있지 않으면 정확히 `N-1`회 동등 비교한다. ranges에서는 projection 호출이 그 두 배 이하다. 원소 이동 대입과 iterator 진행도 선형이므로 전체 시간은 `O(N)`이다. 오늘 정렬 단계 `O(N log N)` 뒤의 압축 자체는 선형이다.
- **할당·반복자 무효화·수명**: 별도 결과 컨테이너를 만들지 않고 같은 원소 저장소 안에서 압축하므로 오늘의 vector/int 순차 호출에는 동적 할당이 필요 없다. 사용자 술어·projection 내부 동작까지 무할당이라고 일반화하지 않는다. `unique` 자체는 vector 구조를 변경하지 않아 iterator·포인터·참조의 주소 유효성을 없애지 않지만, 가리키는 위치의 **값**은 이동 대입으로 달라질 수 있다. 뒤이은 `erase`는 지운 첫 위치 이후의 iterator·참조와 기존 `end()`를 무효화한다. 범위와 호출 객체가 참조하는 외부 상태는 호출 완료까지 살아 있어야 한다.
- **오류·예외 보장**: 순차 overload에서 술어·projection, iterator 연산 또는 원소 이동 대입이 던지면 예외가 호출자에게 전파되고 이미 앞쪽으로 옮긴 원소를 원래 배열로 rollback하지 않는다. 오늘 `int`의 기본 동등 비교와 대입은 던지지 않지만 템플릿 전체를 모든 타입에 대해 `noexcept`로 간주하지 않는다. 표준 실행 정책 overload에서 사용자 함수가 던질 때의 종료/전파 규칙은 해당 실행 정책 계약을 따르며, 오늘은 실행 정책을 쓰지 않는다.
- **미정의 동작·컴파일 실패**: `[first,last)`가 유효하지 않거나 반복자가 writable/MoveAssignable·permutable 요구를 충족하지 않거나, 고전 술어가 동등 관계가 아니거나, 이동 대입과 비교의 의미 요구를 위반하면 전제조건 위반 또는 미정의 동작으로 이어질 수 있다. ranges의 문법적 concept를 만족하지 않으면 적합한 overload가 없어 컴파일되지 않는다. `unique_end` 이후의 유효하지만 미지정 값을 읽는 것 자체와 곧바로 `erase`로 파괴하는 것은 구분해야 하며, 미지정 값을 애플리케이션 의미에 의존해 사용하는 것이 잘못이다.
- **스레드·기계 실행 관점**: 원소를 제자리 이동 대입하므로 같은 범위에 대한 동시 읽기나 쓰기를 자체적으로 동기화하지 않는다. 독립 범위는 별도로 처리할 수 있지만 같은 vector 원소를 다른 실행 흐름이 동기화 없이 접근하면 데이터 경쟁이다. 순차 구현은 인접 원소 load·동등 비교·조건 분기와 살아남은 원소의 앞쪽 store로 나타날 수 있다. 실행 정책이 병렬 작업을 허용하더라도 결과 의미와 사용자 술어의 무경쟁 요구는 유지되며, 실제 병렬화·벡터화·명령열은 구현, CPU와 최적화 옵션에 따라 달라진다.

### 최소 실행 예제

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    std::vector<int> values{1, 1, 2, 2, 2, 4};
    const auto logical_end{std::unique(values.begin(), values.end())};
    values.erase(logical_end, values.end());
    for (const int value : values) {
        std::cout << value << ' '; // 1 2 4
    }
}
```

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

## `std::lower_bound`와 `std::upper_bound` — `<algorithm>`의 이진 탐색 경계

- **항목 종류·현재 역할**: 둘 다 `<algorithm>`의 비수정 함수 템플릿이다. `std::lower_bound`는 검색값보다 작지 않은 첫 위치, `std::upper_bound`는 검색값보다 큰 첫 위치를 돌려준다. 2026-09-30 CSES 1144 풀이는 정렬·중복 제거한 `std::vector<int>` 좌표에서 전자로 `value` 미만 좌표 수와 정확한 급여 순위를, 후자로 `value` 이하 좌표 수를 얻어 Fenwick tree의 닫힌 구간 질의를 구성한다. 중복 제거는 이 풀이의 좌표 압축 불변식일 뿐 두 알고리즘 자체의 요구사항은 아니다.
- **C++23 고전 overload의 정확한 형태**: `lower_bound`는 `template<class ForwardIt, class T> constexpr ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value);`와 `template<class ForwardIt, class T, class Compare> constexpr ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T& value, Compare comp);`를, `upper_bound`도 같은 template 매개변수의 두 형태를 제공한다. C++23에서는 `T`에 반복자 값 타입 기본 인자가 없으며 그것은 C++26 변경이다. 반복자는 LegacyForwardIterator 요구를 만족해야 한다. 비교자 없는 overload의 `comp` 의미는 `std::less{}`다. 오늘 두 호출은 `ForwardIt=std::vector<int>::const_iterator`, `T=int`인 3인자 overload다.
- **C++23 ranges overload와 제약**: iterator/sentinel 형태는 각각 `template<forward_iterator I, sentinel_for<I> S, class T, class Proj = identity, indirect_strict_weak_order<const T*, projected<I, Proj>> Comp = ranges::less> constexpr I ranges::lower_bound(I first, S last, const T& value, Comp comp = {}, Proj proj = {});`이며 `upper_bound`도 같은 형태다. range 형태는 `template<forward_range R, class T, class Proj = identity, indirect_strict_weak_order<const T*, projected<iterator_t<R>, Proj>> Comp = ranges::less> constexpr borrowed_iterator_t<R> ranges::lower_bound(R&& range, const T& value, Comp comp = {}, Proj proj = {});`이고 `upper_bound`도 동일한 제약과 반환 틀을 쓴다. 오늘 코드는 projection 없는 고전 overload이므로 ranges의 `borrowed_iterator_t`와 임시 non-borrowed range의 `dangling` 규칙을 직접 사용하지 않는다.
- **수신 객체·호출 전 상태**: 자유 함수라 수신 객체는 없다. 오늘 `[first,last)`는 같은, 살아 있는 `const std::vector<int>`에서 얻은 유효한 반열린 random-access iterator 범위이고 오름차순 정렬돼 있다. 빈 범위도 유효하며 곧바로 `last`를 반환한다. 검색 중 기반 vector의 구조와 비교에 쓰이는 원소 값을 바꾸지 않는다.
- **매개변수·값 범주·소유권**: `first`와 `last`는 `const_iterator` lvalue에서 값 복사되어 위치만 비소유 관찰하고, `value`는 살아 있는 `const int` lvalue에 `const T&`로 바인딩된다. 인자나 vector 저장소의 소유권은 이동하지 않는다. 비교자 overload의 `comp`는 값으로 받아 알고리즘이 복사할 수 있고, ranges의 `proj`도 값 매개변수다. 그 내부에 포인터·참조가 있으면 호출이 끝날 때까지 대상이 살아 있어야 한다.
- **두 partition 전제와 경계 의미**: `lower_bound`는 모든 원소 `e`가 `bool(invoke(comp, e, value))`인 앞부분과 거짓인 뒷부분으로 partition돼 있어야 하고, 그 참인 앞부분의 바로 다음 위치를 반환한다. `upper_bound`는 모든 `e`가 `!bool(invoke(comp, value, e))`인 앞부분과 거짓인 뒷부분으로 partition돼 있어야 하며 역시 앞부분 다음을 반환한다. ranges에서는 `e` 대신 `invoke(proj,e)`를 비교한다. 기본 정수 오름차순에서는 각각 첫 `e >= value`, 첫 `e > value`이고, 전역 정렬은 이 전제를 세우는 흔한 충분조건이지만 특정 검색값에 대한 partition만으로도 계약은 충족된다.
- **반환형·사용·사후 상태**: 고전 overload는 입력과 같은 `ForwardIt` 값을 반환한다. 경계가 범위 밖 오른쪽이면 `last`이며 이는 정상 결과라 역참조하지 않는다. 오늘 코드는 `found-first`를 `Index`로 바꿔 `lower_bound` 결과를 `value` 미만 좌표 개수, `upper_bound` 결과를 `value` 이하 좌표 개수로 사용한다. 호출 뒤 vector, 원소, 검색값, 크기와 capacity는 그대로이고 반복자도 이 호출 때문에 무효화되지 않는다.
- **복잡도**: 길이를 `N`이라 하면 각 호출의 비교 횟수는 최대 `log2(N)+O(1)`이고 ranges overload는 projection도 같은 횟수 이하다. 다만 forward iterator에서는 iterator 증가가 `O(N)`일 수 있다. 오늘 vector의 random-access iterator는 중간 위치로 상수 시간 이동하므로 비교와 위치 이동을 합쳐 `O(log N)`이다.
- **할당·무효화·수명**: 알고리즘은 결과 컨테이너나 원소 저장소를 만들지 않으며 오늘의 정수·vector 반복자 호출에는 동적 할당이 없다. 일반 사용자 비교자·projection의 복사나 호출 내부 동작까지 allocation-free라고 보장하지는 않는다. 반환 iterator는 owner 파괴, vector 재할당과 해당 위치를 포함하는 `erase` 등에 의해 무효화될 수 있다. 호출 뒤 원소 값을 바꿔 partition을 깨면 기존 iterator 주소가 살아 있어도 다음 이진 탐색의 전제는 사라진다.
- **오류·예외·미정의 동작**: 별도 오류값이나 예외 변환은 없다. 사용자 비교자·projection 또는 사용자 반복자 연산이 던지면 그대로 전파될 수 있다. 오늘의 `int` 기본 비교와 정상 vector iterator 연산은 던지지 않지만 함수 템플릿 자체를 무조건 `noexcept`로 보지 않는다. `[first,last)`가 유효하지 않거나 두 반복자가 다른 범위에서 왔거나 해당 검색 식에 대해 partition돼 있지 않거나 비교 관계의 의미 요구를 깨면 라이브러리 전제조건 위반으로 동작이 정의되지 않는다. 반환값이 `last`인지 검사하지 않고 역참조하는 것도 미정의 동작이다.
- **스레드·기계 실행 관점**: 자체 동기화가 없다. 같은 불변 범위를 여러 실행 흐름이 읽는 것은 원소 타입의 동시 읽기 규칙을 따르지만, 검색 중 다른 실행 흐름이 같은 vector의 구조나 비교 대상 원소를 동기화 없이 쓰면 데이터 경쟁 또는 iterator 무효화가 생긴다. vector에서는 반복적으로 중간 iterator를 계산하고 원소를 load해 비교한 뒤 왼쪽/오른쪽 반으로 조건 분기할 수 있으나 실제 분기 제거·명령열·prefetch는 CPU, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라진다.

### 최소 실행 예제

```cpp
#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    const std::vector<int> sorted{1, 2, 2, 5};
    const auto lower{std::lower_bound(sorted.begin(), sorted.end(), 2)};
    const auto upper{std::upper_bound(sorted.begin(), sorted.end(), 2)};
    std::cout << (lower - sorted.begin()) << ' ' << (upper - sorted.begin()) << '\n'; // 1 3
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

## `std::views::enumerate`와 `std::ranges::enumerate_view` — `<ranges>`의 C++23 위치 결합 view

- **항목 종류·헤더·현재 역할**: `<ranges>`가 C++23 range adaptor 객체 `std::views::enumerate`와 class template `std::ranges::enumerate_view<V>`를 선언한다. 2026-10-01 `ReleasePlan`과 `ReviewChecklist`는 살아 있는 const owner의 vector에 0-based 위치를 붙여 순회한 뒤, 외부 경계에는 1-based 번호와 문자열을 소유하는 별도 vector를 만든다.
- **공개 class 제약**: 형태는 `template<ranges::view V> requires range-with-movable-references<V> class ranges::enumerate_view;`다. 여기서 표준의 설명 전용 `range-with-movable-references<R>`는 `input_range<R>`이면서 `range_reference_t<R>`와 `range_rvalue_reference_t<R>`가 각각 `move_constructible`일 것을 요구한다. 이 설명 전용 이름을 사용자 코드에서 공개 concept처럼 직접 사용할 수는 없다. 생성자는 `constexpr explicit enumerate_view(V base);`이고 deduction guide는 `template<class R> enumerate_view(R&&) -> enumerate_view<views::all_t<R>>;` 형태다.
- **adaptor 호출의 선택 규정**: `views::enumerate`의 공개 객체 타입과 구체 `operator()` 선언은 구현 세부다. 표준 계약은 식 `E`에 대해 `views::enumerate(E)`가 `enumerate_view<views::all_t<decltype((E))>>(E)`와 expression-equivalent라고 정한다. 오늘의 `E`는 `const std::vector<T>` lvalue이므로 `views::all_t<decltype((E))>`는 보통 `ranges::ref_view<const std::vector<T>>`이고 결과는 owner를 빌린다.
- **수신 객체·매개변수·값 범주·소유권**: 함수처럼 호출되는 adaptor 객체 외에 사용자 데이터 수신 객체는 없다. 유일한 range 식은 살아 있는 const lvalue이며 저장소·원소 소유권을 넘기지 않는다. 일반 rvalue viewable range는 `views::all`이 `owning_view`로 소유할 수 있으므로 모든 enumerate 결과가 비소유라고 일반화하지 않는다. 오늘처럼 lvalue 컨테이너를 넘긴 결과는 owner 수명과 iterator 유효성에 종속된다.
- **반환과 index 타입**: 호출은 작은 지연 view prvalue를 반환하며 구성만으로 원소를 순회하지 않는다. iterator 역참조의 첫 성분은 0에서 시작해 증가하는 `range_difference_t<V>` 값이고, 둘째 성분은 `range_reference_t<V>`다. 첫 성분은 일반적으로 signed 차이 타입이지 `size_t`라고 단정할 수 없다. 오늘은 비음수 index임을 근거로 명시 변환한 뒤 표시용 1을 더한다.
- **tuple-like 역참조·구조적 바인딩**: 역참조 결과는 index 값과 기반 원소 참조를 tuple-like하게 보이는 prvalue다. `for (auto&& [index, element] : view)`의 바깥 참조는 그 반복의 tuple-like 결과를 붙잡고, `element`는 기반 원소를 계속 빌린다. 결과를 저장한다고 원소가 복사되거나 수명이 연장되지 않는다. 기반이 proxy reference를 내는 범위라면 둘째 성분도 실제 `T&`가 아닐 수 있다.
- **순서·파이프 조합**: enumerate는 자신이 받은 범위의 순서에 번호를 붙인다. `source | views::filter(pred) | views::enumerate`는 필터를 통과한 원소를 0부터 다시 번호 매기고, `source | views::enumerate | views::filter(...)`는 원본 위치를 보존한 index를 필터와 함께 관찰한다. 어느 의미가 도메인 요구인지 먼저 정해야 한다.
- **사후 상태·복잡도·할당**: lvalue vector 기반 view 구성은 O(1), 원소 복사·이동·동적 할당 없음이다. iterator 증가·역참조·비교는 기반 iterator의 해당 연산에 상수 작업을 더하고, 전체 한 번 순회는 O(n)이다. `size()`가 제공되는 기반에서는 enumerate view도 같은 원소 수를 상수 시간에 보고할 수 있다. 구성과 순회는 기반 원소·size·capacity를 바꾸지 않는다.
- **무효화·수명**: ref-view 기반 enumerate, iterator, tuple-like 결과의 원소 참조는 owner 파괴 뒤 댕글링한다. vector 재할당은 모든 iterator/reference를 무효화하고, erase/insert는 위치에 따른 vector 무효화 규칙을 그대로 적용한다. view 객체의 const 여부만으로 mutable 기반 원소가 const가 되지 않으므로 읽기 전용 관찰이 필요하면 오늘처럼 const owner lvalue를 넘긴다.
- **오류·예외·미정의 동작·스레드**: 필요한 view/input-range 및 차이 타입 요구를 만족하지 않으면 적합한 호출이 없어 컴파일 실패한다. `views::all` 변환, 기반 `begin/end`, iterator 연산과 원소 접근이 던지는 예외는 전파될 수 있어 모든 일반 호출을 무조건 `noexcept`라고 단정하지 않는다. end iterator 역참조, owner 파괴나 무효화 뒤 접근, 의미 요구를 깨는 사용자 range는 UB로 이어질 수 있다. 같은 기반을 읽기만 하는 별도 순회는 가능하지만 한 스레드의 구조 변경·원소 쓰기와 다른 스레드의 읽기는 자동 동기화되지 않는다.

### 최소 실행 예제

```cpp
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

int main() {
    const std::vector<std::string> names{"compile", "test", "deploy"};
    for (auto&& [index, name] : std::views::enumerate(names)) {
        std::cout << (index + 1) << ':' << name << '\n';
    }
}
```

### 흔한 실수와 오늘의 연결

1. index를 언제나 `size_t`라고 가정해 signed/unsigned 변환 경고와 음수 sentinel 확장을 놓친다.
2. 구조적 바인딩의 element가 소유 사본이라고 생각해 owner보다 오래 보관한다.
3. `filter | enumerate`가 원본 위치를 보존한다고 오해한다.
4. 임시 domain owner의 lvalue 멤버를 enumerate한 view를 반환해 owner 파괴 직후 댕글링시킨다.
5. 순회 중 기반 vector에 push/erase해 현재 iterator와 element 참조를 무효화한다.

[`../2026-10-01/main.cpp`](../2026-10-01/main.cpp)는 `indexed_steps() const &`와 삭제한 `const &&` overload로 owner 수명 의도를 API에 드러내고, `make_snapshot()`에서만 문자열을 깊게 복사한다. [`../2026-10-01/problem.cpp`](../2026-10-01/problem.cpp)는 validation 결과에 원래 1-based 위치를 붙이는 같은 패턴을 연습한다.

## `std::views::filter`와 `std::views::transform` — `<ranges>`

- `filter(predicate)`는 술어가 참인 원소만 지연 노출한다.
- `transform(function)`은 원소를 함수 결과로 지연 투영한다.
- 파이프 `source | views::filter(...) | views::transform(...)`는 보통 중간 컨테이너를 즉시 만들지 않는다.
- 뷰는 원본을 소유하지 않을 수 있고 술어/변환 객체를 보관한다. 원본과 캡처한 참조의 수명이 뷰보다 길어야 한다.
- 실제 함수 호출은 순회할 때 발생하므로 뷰를 만들기만 해서는 부수 효과가 실행되지 않을 수 있다.
- 같은 뷰를 여러 번 순회할 수 있는지는 기반 범위와 뷰 종류의 범주에 따라 다르다.
- `forward_range` 위 `filter_view`는 첫 통과 반복자를 cache할 수 있다. 이미 순회를 시작한 view는 술어가 보는 원소를 바꾸거나 기반 `vector`를 재할당한 뒤 재사용하지 말고 새로 만든다. 원본 객체 파괴는 view를 dangling으로 만들며, 재할당은 이미 얻었거나 cache한 반복자를 무효화한다.

## `std::views::chunk_by`와 `std::ranges::chunk_by_view` — `<ranges>`의 C++23 인접 구간 view

`chunk_by`는 기반 범위의 **서로 인접한 두 원소**에 술어를 적용하고 거짓이 되는 경계에서 범위를 나눈다. 같은 키를 전역으로 모으거나 정렬하지 않는다. 예를 들어 키가 `A, B, A`이고 “키가 같다”가 술어면 세 구간이며, 떨어진 두 `A`는 합쳐지지 않는다.

- **항목 종류·헤더·오늘 역할**: `<ranges>`가 C++23 range adaptor object `std::views::chunk_by`와 view class template `std::ranges::chunk_by_view<V, Pred>`를 선언한다. 2026-09-30 `PayrollSnapshot`은 이미 부서 키 순서로 놓인 급여 레코드의 연속 부서 run을, `HealthTimeline`은 연속한 동일 건강 상태 run을 중간 컨테이너 없이 묶는다. 이 view는 입력 정렬이나 그룹 키 불변식을 검사하지 않으므로 owner의 생성·갱신 경계가 순서 불변식을 책임진다.
- **C++23 class template의 정확한 제약**: 공개 형태는 `template<ranges::forward_range V, indirect_binary_predicate<ranges::iterator_t<V>, ranges::iterator_t<V>> Pred> requires ranges::view<V> && is_object_v<Pred> class ranges::chunk_by_view;`다. 기반 `V`는 단일 통과 `input_range`가 아니라 적어도 `forward_range`인 view여야 하고, `Pred`는 참조나 함수 타입이 아닌 객체 타입이며 두 간접 원소 조합에 대해 `indirect_binary_predicate`를 만족해야 한다. 생성자는 `constexpr explicit chunk_by_view(V base, Pred pred);`, deduction guide는 `template<class R, class Pred> chunk_by_view(R&&, Pred) -> chunk_by_view<views::all_t<R>, Pred>;`다.
- **adaptor 호출의 정확한 규정**: `views::chunk_by`의 공개 타입과 `operator()` 함수 템플릿 시그니처는 표준이 지정하지 않는 구현 세부사항이다. 표준 계약은 두 식 `E`, `F`에 대해 `views::chunk_by(E, F)`가 `chunk_by_view(E, F)`와 expression-equivalent라고 정한다. 따라서 존재하지 않는 표준화된 `views::chunk_by(R&&, Pred&&)` 선언을 API 선언처럼 의존하지 않는다. `E | views::chunk_by(F)`는 range adaptor closure 규칙으로 같은 두 인자 적용을 표현하며, 결과 class의 위 제약과 `views::all_t<R>` 구성이 유효할 때만 호출이 성립한다.
- **수신 객체·인자 값 범주·소유권**: adaptor 객체가 함수처럼 호출되므로 사용자 데이터 수신 객체는 없다. 오늘 `E`는 살아 있는 `const std::vector<T>` 멤버 lvalue이고 `views::all_t<const vector<T>&>`는 이를 비소유로 가리키는 `ref_view<const vector<T>>`가 된다. `F`는 캡처 없는 lambda의 const lvalue지만 deduction guide의 값 매개변수 `Pred`가 cv/ref를 제거한 lambda 객체 타입을 추론하고 view가 그 사본을 값으로 보관한다. 기반 원소나 vector 저장소는 복사·이동되지 않는다. 일반 rvalue viewable range는 `views::all` 결과가 소유할 수도 있으므로 모든 `chunk_by_view`가 비소유라고 일반화하지 않는다.
- **반환형·lazy 상태 변화**: 오늘 직접 호출은 개념상 `chunk_by_view<ref_view<const Records>, Lambda>` 같은 작은 prvalue를 반환한다. 생성자는 `base_`와 `pred_`를 각각 이동해 보관하지만 원소를 순회하거나 술어를 호출하거나 구간을 materialize하지 않는다. 이름 있는 view를 처음 순회할 때 경계 검색이 실행된다. owner의 vector 크기·capacity·원소는 생성과 읽기 전용 순회로 바뀌지 않는다.
- **인접 술어 의미와 불변식**: 각 인접 쌍 `(previous,current)`에서 `bool(invoke(pred, previous, current))`가 참인 동안 같은 subrange가 계속되고, 처음 거짓인 쌍의 `current`부터 다음 subrange가 시작한다. 비어 있지 않은 입력의 각 결과 subrange는 비어 있지 않으며 모든 subrange를 이어 붙이면 원래 순서와 원소를 정확히 한 번씩 얻는다. 술어가 대칭·추이적인 동치 관계일 필요는 없으므로 `less_equal`은 비감소 run을 만들 수 있다. 반대로 “모든 같은 값을 모은다”는 전역 group-by 의미는 제공하지 않는다.
- **`begin`/`end`의 const 제한과 첫 경계 cache**: 공개 순회 함수는 `constexpr iterator begin();`와 `constexpr auto end();`뿐이고 const overload가 없다. 내부 경계 cache와 술어를 사용하므로 `const chunk_by_view` 자체는 range가 아니며, 기반이 const 원소 범위인 것과 view 객체의 const 여부를 구분해야 한다. 첫 `begin()`은 기반 시작과 첫 거짓 인접 쌍 뒤 위치를 찾고 그 첫 경계를 view 내부에 cache한다. 이후 `begin()`은 range 개념의 amortized constant-time 요구를 위해 cached 결과를 재사용한다. `common_range<V>`이면 `end()`는 끝/끝 iterator를, 아니면 `default_sentinel`을 반환한다.
- **바깥 iterator와 안쪽 subrange**: 바깥 iterator의 `value_type`은 `ranges::subrange<ranges::iterator_t<V>>`이고 역참조는 현재 구간의 `[current,next)` iterator 쌍을 값으로 반환한다. 기반 `V`가 `bidirectional_range`면 `iterator_concept`가 `bidirectional_iterator_tag`이고 `operator--`가 제공되며, 그 밖에는 `forward_iterator_tag`다. legacy `iterator_category`는 어느 경우에도 `input_iterator_tag`다. vector가 random-access여도 바깥 chunk iterator는 random-access가 되지 않는다. 안쪽 subrange의 능력과 원소 참조의 const 여부는 기반 iterator를 따른다.
- **복잡도**: `ref_view`와 작은 술어를 보관하는 오늘의 view 생성은 원소 수에 무관한 `O(1)`이다. 첫 `begin()`은 첫 구간 길이에 선형일 수 있고 이후 같은 view의 `begin()`은 cached 경계 덕분에 amortized `O(1)`이다. 바깥 `operator++`는 다음 경계까지 인접 쌍을 조사하므로 한 번의 비용은 다음 구간 길이에 선형일 수 있지만, 처음부터 끝까지 한 번 전진 순회하면 각 인접 경계를 한 번씩 검사해 전체 `O(N)`이다. 양방향 `operator--`도 이전 경계를 찾는 구간 길이에 비례한다. 반복해서 전체 순회하면 첫 경계 외의 경계는 다시 계산될 수 있다.
- **할당·materialization**: `chunk_by_view`와 각 `subrange`는 별도 원소 배열을 만들지 않으며, 오늘의 vector lvalue·캡처 없는 lambda 조합은 view/순회 자체에 동적 할당이 필요 없다. 다만 사용자 정의 기반 view나 술어의 복사·이동·호출이 자체적으로 할당할 가능성까지 표준이 없애 주지는 않는다. 결과 구간을 `vector` 등으로 materialize하는 후속 코드는 그 컨테이너 계약에 따라 별도 할당한다.
- **무효화·cache·수명**: 오늘 결과는 `ref_view`라 `PayrollSnapshot`/`HealthTimeline` owner의 수명을 연장하지 않는다. owner 파괴 뒤 view·바깥 iterator·subrange·원소 참조는 사용할 수 없다. vector 재할당, 관련 `erase`·`insert`는 이미 얻은 iterator뿐 아니라 view가 cache한 첫 경계도 무효화할 수 있으므로 구조 변경 뒤 같은 view를 재사용하지 않고 새로 만든다. iterator가 물리적으로 유효한 채 predicate가 보는 원소 값만 바뀌어도 cached 첫 경계와 이후 그룹 의미가 낡을 수 있다. 바깥 iterator는 부모 view의 주소와 술어에 의존하므로 view 파괴 뒤에는 기반 owner가 살아 있어도 댕글링이다. `chunk_by_view`에는 기반의 borrowed 성질을 그대로 전달하는 `enable_borrowed_range` 특수화가 없다.
- **전제조건·후조건**: 기반과 술어는 view 생성 및 모든 순회 동안 template concept의 문법뿐 아니라 의미 요구사항도 지켜야 한다. `begin()`과 경계 탐색에는 술어 보관함이 값을 가진다는 전제조건이 있으므로 그러한 보장이 없는 이동 후 view를 무턱대고 순회하지 않는다. 성공한 완전 순회 뒤 입력 범위는 그대로이고, 각 반환 subrange의 내부 인접 쌍에는 술어가 참이며 두 연속 subrange 사이 경계 쌍에는 거짓이다.
- **오류·예외·미정의 동작**: 런타임 오류값은 없다. 기반 view와 술어의 복사·이동, `begin`/`end`, 반복자 연산과 술어 호출이 던지는 예외는 전파될 수 있고 이미 실행된 사용자 부수 효과를 view가 rollback하지 않는다. 오늘 술어는 `noexcept`이고 읽기만 하지만 adaptor 전체를 모든 타입에 대해 무조건 `noexcept`라고 가정하지 않는다. 끝 바깥 iterator 역참조, 댕글링/무효 iterator 사용, 비어 있는 술어 상태에서 전제조건을 어긴 `begin`, concept의 의미 요구를 거짓으로 만족시킨 타입, 술어가 관찰 중인 같은 객체의 데이터 경쟁은 미정의 동작으로 이어질 수 있다.
- **스레드 보장**: 자체 동기화를 제공하지 않으며 첫 `begin()`은 논리적으로 읽기처럼 보여도 view 내부 cache를 갱신할 수 있다. 따라서 같은 view 객체를 여러 실행 흐름이 동기화 없이 처음 순회하지 않는다. 살아 있는 같은 불변 owner에서 실행 흐름마다 별도 view를 만들어 읽는 것은 원소와 술어 타입의 동시 읽기 규칙을 따르지만, owner 구조나 같은 원소에 동시 쓰기가 있으면 외부 동기화가 필요하다.

기계 실행 관점에서는 경계 검색이 인접 원소 load, 술어 비교, 조건 분기와 iterator 증가로 나타날 수 있다. view·subrange 층과 캡처 없는 lambda는 구체 타입이 보여 인라인될 가능성이 높고 가상 간접 호출을 요구하지 않지만, 실제 분기 예측·벡터화·명령열·cache 접근은 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라진다.

### 최소 실행 예제

```cpp
#include <iostream>
#include <ranges>
#include <vector>

int main() {
    std::vector<int> values{1, 1, 2, 3, 3, 1};
    for (auto run : std::views::chunk_by(values, [](int left, int right) {
             return left == right;
         })) {
        std::cout << '[';
        for (const int value : run) {
            std::cout << value << ' ';
        }
        std::cout << ']';
    }
    // [1 1 ][2 ][3 3 ][1 ]: 떨어진 두 값 1은 합쳐지지 않는다.
}
```

### 흔한 실수와 점검 질문

1. `A, B, A`를 같은 키 두 그룹으로 만들 것이라 기대한다. 인접 쌍별 술어 결과와 실제 세 subrange를 적는다.
2. `const auto groups = views::chunk_by(...)`를 range-for에 넣는다. 왜 기반 원소의 const와 view 객체의 const가 다르고 `begin() const`가 없는지 설명한다.
3. 첫 `begin()` 뒤 기반 vector를 재할당하거나 키 값을 바꾼다. 바깥 iterator와 cached 첫 경계가 각각 왜 재사용 불가능한지 판단한다.
4. vector 기반이므로 바깥 iterator도 random-access라고 생각한다. 표준의 `iterator_concept`와 legacy `iterator_category`를 각각 말한다.
5. owner보다 view를 오래 보관한다. `ref_view`, 부모 포인터를 가진 바깥 iterator, 안쪽 subrange의 수명 의존성을 나눠 추적한다.

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

## `std::views::slide`와 `std::ranges::slide_view` — `<ranges>`의 C++23 겹치는 창 view

`slide`는 기반 범위의 연속한 `N`개 원소를 빌리는 창을 한 칸씩 겹쳐 노출한다. 길이 `M`인 범위와 창 너비 `N`에서 `M >= N`이면 창은 `M-N+1`개, `M < N`이면 0개다. 2026-09-28의 `ServiceLatencyHistory`와 `VibrationTrace`는 vector를 직접 소유하고 const lvalue에서만 view를 반환해, 복사 없는 분석과 owner 수명 규칙을 한 API에 묶는다.

- **헤더·항목 종류**: `<ranges>`가 range adaptor object `std::views::slide`와 class template `std::ranges::slide_view<V>`를 선언한다. 표준 초안의 대표 형태는 `template<forward_range V> requires view<V> class slide_view;`이며 `views::slide(E, N)`은 `slide_view(views::all(E), N)`과 같은 의미의 호출이다.
- **관련 alias와 기반 view**: `std::ranges::ref_view<R>`는 lvalue `R`을 포인터와 같은 비소유 상태로 감싸는 view다. `std::ranges::range_difference_t<R>`는 iterator 차이·창 너비에 쓰는 signed 정수형, `std::ranges::range_reference_t<R>`는 `*ranges::begin(r)`의 결과 타입을 나타내는 alias template다. 셋은 새 원소 저장소를 만들지 않는다. 오늘 별칭은 각각 `const vector<int>` owner를 빌리는 기반, 양수 너비 타입, 한 창 iterator 역참조 결과 타입을 정확히 표현한다. ref_view와 그 파생 iterator/reference는 원본 수명을 연장하지 않는다.
- **수신 객체·선택 호출**: 일반 자유 함수처럼 보이지만 `views::slide`라는 adaptor 객체의 함수 호출 연산자다. 오늘 첫 인자는 살아 있는 owner의 `const std::vector<T>` lvalue라 `views::all_t`가 대개 `ref_view<const vector<T>>`가 되고, 두 번째 인자는 양수 창 너비다. 반환 공개 타입은 해당 `ref_view`를 템플릿 인자로 가진 `slide_view` 값이다.
- **매개변수·값 범주·소유권**: 기반 vector lvalue는 빌려 전달되며 원소나 저장소를 복사·이동하지 않는다. 너비는 기반 범위의 `range_difference_t`로 변환돼 view 안에 값으로 저장된다. 생성 전제조건은 `N > 0`이다. 0 또는 음수는 빈 결과 요청이 아니라 전제조건 위반이므로 오늘 owner API가 먼저 양수를 요구한다.
- **반환값·사후 상태**: 호출은 작은 view prvalue를 반환하고 호출부의 range-for가 즉시 소비한다. owner의 크기·용량·원소는 바뀌지 않고 관찰자도 무효화되지 않는다. 각 바깥 iterator 역참조 결과는 현재 위치부터 `N`개를 나타내는 `views::counted` 계열의 작은 내부 view 값이며 원소를 소유하지 않는다.
- **`begin`·`end`·역참조·증가**: `begin()`/`end()`는 첫/끝 창 위치를 표현한다. 바깥 iterator `operator*()`는 길이 `N`인 내부 view를 만들고, `operator++()`는 창 시작과 구현이 보관하는 끝 위치를 한 칸 전진시킨다. 내부 range-for는 그 창 view의 `begin`/`end`, 역참조·증가·비교로 기반 원소를 읽는다. 기반이 vector처럼 random-access+sized이면 view는 별도 iterator cache 없이 시작 위치와 너비 산술로 구현될 수 있지만 이는 관찰 가능한 저장소 복사를 뜻하지 않는다.
- **`size()`**: 기반과 const 기반이 `sized_range`이면 `size()`가 제공된다. 인자 없이 `max(M-N+1,0)`에 해당하는 unsigned-like 값을 반환하고 상태를 바꾸지 않는다. vector 기반에서는 상수 시간·무할당이다. 창이 없다는 사실은 오류가 아니지만 너비 0 전제조건 위반과 구분한다.
- **범위 능력·복잡도**: 기반은 적어도 `forward_range`여야 한다. 결과 iterator 능력은 기반에 따라 forward/bidirectional/random-access까지 보존될 수 있다. view 생성은 원소 수와 무관한 상수 작업이고, vector 기반의 창 이동·역참조도 상수 시간이다. 모든 `W`개 창에서 `N`개 원소를 각각 합산하면 겹침에도 불구하고 전체 분석은 `O(WN)`이며 slide 자체가 누적합을 계산해 주지 않는다. 별도 동적 할당은 요구하지 않는다.
- **수명·무효화**: lvalue vector로 만든 오늘 view는 owner를 소유하거나 수명을 연장하지 않는다. owner 파괴 뒤 view·바깥/안쪽 iterator·원소 참조는 모두 댕글링한다. vector 재할당은 얻어 둔 iterator와 창 view가 가리키는 원소 관찰자를 무효화하며, 순회 중 `push_back`·`erase` 같은 구조 변경을 섞지 않는다. rvalue owner의 멤버에서 비소유 view가 새어나오지 않도록 오늘 `windows() const &&`와 `frames() const &&`를 삭제한다.
- **오류·예외·UB**: forward/viewable-range 제약을 만족하지 않으면 컴파일에 실패한다. 너비가 양수가 아니면 생성자 전제조건 위반이다. 끝 iterator나 빈 내부 창을 잘못 역참조하거나, owner 파괴·무효화 뒤 접근하거나, 같은 저장소와 데이터 경쟁하면 미정의 동작으로 이어질 수 있다. 기반 view/iterator 연산이 던지는 예외는 전파될 수 있지만 vector lvalue wrapper 생성 자체는 원소를 복사·할당하지 않는다.
- **스레드 보장**: slide_view는 snapshot, 잠금, 원자성을 만들지 않는다. 같은 불변 owner를 여러 실행 흐름이 읽는 것은 원소 타입의 동시 const-read 조건을 따르지만, 한 실행 흐름이 owner 구조나 같은 원소를 수정하는 동안 다른 흐름이 순회하려면 외부 동기화가 필요하다.

기계 실행 관점에서 vector 기반 창 순회는 시작·끝 iterator load, 인덱스/포인터 증가, 원소 load, 합산과 조건 비교·분기를 만들 수 있다. 겹친 원소를 다시 읽는 코드를 컴파일러가 자동 누적합으로 바꾼다고 보장할 수 없고, bounds 증명·인라인·벡터화·load 재사용은 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라진다.

### 최소 실행 예제

```cpp
#include <iostream>
#include <ranges>
#include <vector>

int main() {
    std::vector<int> values{2, 4, 3, 5};
    for (const auto window : std::views::slide(values, 3)) {
        int sum{};
        for (const int value : window) {
            sum += value;
        }
        std::cout << sum << ' ';
    }
}
```

### 흔한 실수와 점검 질문

1. 너비 0을 “빈 창”으로 생각한다. 생성 전제조건과 `M < N`의 정상 빈 결과를 구분한다.
2. 반환 view가 원소를 복사해 소유한다고 생각한다. owner 파괴·vector 재할당 뒤 어떤 객체가 댕글링하는지 적는다.
3. 임시 owner의 멤버에서 반환한 view를 저장한다. ref-qualified accessor가 이 호출을 어떻게 막는지 설명한다.
4. 모든 창 합이 자동 `O(M)`이라고 생각한다. 단순 중첩 순회 `O((M-N+1)N)`와 rolling-sum 대안을 비교한다.
5. 순회 중 기반 vector를 변경한다. 바깥 iterator, 내부 창 view, 원소 참조의 무효화를 각각 판단한다.

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

## 비교 함수 객체 `std::less`, `std::greater`, `std::equal_to` — `<functional>`

- `<functional>`이 선언하는 비교 함수 객체 class template다. `less<T>{}(a,b)`는 `a<b`, `greater<T>{}(a,b)`는 `a>b`, `equal_to<T>{}(a,b)`는 `a==b` 의미이며 기본 객체는 비교 대상을 소유하지 않는 무상태 값이다.
- 대표 호출은 `constexpr bool std::less<T>::operator()(const T& lhs, const T& rhs) const`와 `constexpr bool std::equal_to<T>::operator()(const T& lhs, const T& rhs) const`다. 두 const lvalue를 빌려 bool을 반환하고 객체·피연산자·반복자를 바꾸거나 저장소를 할당하지 않는다. 시간·예외 명세는 각각 선택된 `<`와 `==` 식을 따른다. 오늘의 int 비교는 `O(1)`이고 던지지 않으며, `Entry::operator<`도 값과 인덱스의 정수 비교만 해 `O(1)`·`noexcept`다.
- 2026-09-28의 `std::multiset<Entry>`는 기본 `std::less<Entry>`를 보관해 `(value,index)` 엄격 약순서를 정한다. 비교자 기본 생성·소멸은 `O(1)`·무할당·비투척이고 컨테이너 원소 수명을 소유하지 않는다. multiset이 살아 있는 동안 비교자가 유효해야 하며, 같은 원소들에 일관된 엄격 약순서를 제공하지 않으면 연관 컨테이너의 의미 요구를 깨뜨린다.
- 2026-09-30의 두-반복자 `std::sort`는 C++20 이후 생략된 순서 비교에 `std::less{}` 의미를, `std::unique`는 생략된 동등 비교에 `std::equal_to{}` 의미를 쓴다. 전자는 엄격 약순서를, 후자는 동등 관계를 이뤄야 한다. 비교 객체와 피연산자의 수명은 호출까지 유지되어야 하고 같은 원소에 대한 동시 쓰기는 데이터 경쟁이며, 사용자 정의 비교가 던지면 알고리즘으로 전파될 수 있다.
- `std::priority_queue<T,Container,std::greater<T>>`는 작은 값이 `top`이 되는 최소 힙을 만든다. 투명 비교자인 `std::less<>`는 서로 비교 가능한 다른 타입을 받아 불필요한 키 임시 생성을 줄일 수 있다.

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
