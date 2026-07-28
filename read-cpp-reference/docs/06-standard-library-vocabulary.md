# 06. 표준 라이브러리 문서를 읽는 어휘

## 헤더, namespace, 이름

```cpp
#include <vector>
std::vector<int> numbers;
```

헤더는 선언을 프로그램에 보이게 하고 `std`는 표준 라이브러리 namespace입니다.
헤더를 직접/간접 include했는지에 의존하지 말고 사용하는 이름을 규정한 헤더를 직접
include하세요. 전역 `using namespace std;`는 이름 충돌을 키우므로 특히 헤더에서 피합니다.

## 컨테이너

주요 분류:

- sequence: `array`, `vector`, `deque`, `list`, `forward_list`
- ordered associative: `set`, `map`, `multiset`, `multimap`
- unordered associative: `unordered_set`, `unordered_map`, ...
- adapters: `stack`, `queue`, `priority_queue`
- text: `string`, 비소유 `string_view`

API를 고를 때 확인할 것:

1. 요소 순서와 중복 허용
2. 탐색/삽입/삭제 복잡도
3. 연속 메모리 필요 여부
4. iterator/reference invalidation
5. 예외 안전성
6. allocator

## iterator와 range

iterator는 위치를 나타내고 `*it`로 요소에 접근하며 `++it`로 이동하는 일반화된 포인터입니다.
전통 API는 `[first, last)` 반열린 구간을 씁니다. `last` 자체는 포함하지 않습니다.

전통 category는 기능이 누적됩니다.

```text
input / output
    └─ forward
         └─ bidirectional
              └─ random access
                   └─ contiguous
```

알고리즘이 요구하는 최소 category를 만족해야 합니다. `std::sort`는 random access iterator가
필요하므로 `std::list`에는 쓸 수 없고 `list.sort()`를 씁니다.

C++20 ranges에서는 range/iterator/sentinel/concept가 분리되고 view는 대개 lazy하며
비소유입니다. 원본의 수명과 변경을 확인합니다.

## 알고리즘 문서

```cpp
auto it = std::find(begin, end, value);
```

- 입력 범위: `[begin, end)`
- 결과: 찾은 iterator 또는 못 찾으면 `end`
- precondition: 범위가 유효해야 함
- complexity: 최대 `N`번 비교
- invalidation: 보통 알고리즘보다 전달한 컨테이너/연산이 결정

`std::remove`는 컨테이너에서 물리적으로 지우지 않고 남길 원소를 앞으로 옮긴 뒤 새 논리적
끝을 반환합니다. C++17에서는 `erase-remove` idiom, C++20에서는 많은 컨테이너에
`std::erase`/`std::erase_if`가 있습니다.

## 복잡도 표기

- constant `O(1)`: 입력 크기와 무관한 상한
- logarithmic `O(log N)`: 균형 트리 탐색 등
- linear `O(N)`: 모든 요소 한 번 정도
- linearithmic `O(N log N)`: 비교 정렬 등
- amortized constant: 여러 연산 평균으로 상수; 한 번은 비쌀 수 있음

“at most N applications”, “amortized”, “average case” 같은 정확한 단서를 함께 읽습니다.

## named requirements와 concepts

자주 보이는 이름:

- `DefaultConstructible`, `MoveConstructible`, `CopyAssignable`
- `Swappable`, `EqualityComparable`, `LessThanComparable`
- `Callable`, `Predicate`, `BinaryPredicate`, `Compare`
- `Allocator`
- `LegacyInputIterator` 등 iterator 요구 조건
- C++20 `std::regular`, `std::invocable`, `std::ranges::range`

이 요구는 함수 모양뿐 아니라 의미론적 법칙도 포함할 수 있습니다. 위반이 컴파일 오류인지,
undefined behavior인지, overload 후보 제외인지 페이지의 wording을 확인합니다.

## 정책, customization, allocator

- execution policy: 순차/병렬/벡터화 실행 가능성을 지정
- allocator: 컨테이너 저장 공간 획득/해제 정책
- comparator/hash/equality: 정렬/버킷/동치 정책
- deleter: 스마트 포인터가 자원을 해제하는 방식

정책 객체는 타입의 일부가 되어 복사/이동/예외 명세에 영향을 줄 수 있습니다.

## 오류 표현 방식

- 예외: `throw`, `try/catch`, `std::exception`
- 오류 코드: `std::error_code`
- 빈 값: `std::optional`
- 여러 대안: `std::variant`
- iterator/sentinel: `end()`가 “없음” 표시
- bool/status enum

각 API가 실패를 어떤 채널로 전달하는지 확인합니다. 예를 들어 filesystem API는 예외를
던지는 overload와 `std::error_code&`를 받는 overload가 함께 있는 경우가 많습니다.

## thread safety 문구

표준 컨테이너의 서로 다른 객체를 별도 스레드에서 접근하는 것과 같은 객체를 동시에
수정하는 것은 다릅니다. const 멤버라고 해서 모든 내부 구현과 모든 조합이 자동으로
thread-safe인 것은 아닙니다. 라이브러리의 thread safety 보장과 해당 API의 data race
문구를 확인하세요.

## feature-test macro

```cpp
#include <version>

#if defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L
// 요구하는 ranges 기능 사용
#endif
```

언어 기능은 `__cpp_*`, 라이브러리 기능은 `__cpp_lib_*` 형태가 일반적입니다. 값은 기능의
개정 수준을 나타내므로 정의 여부만이 아니라 최소값을 검사할 수 있습니다.

## 구현 정의와 미지정

- implementation-defined: 구현이 선택하고 문서화해야 함
- unspecified: 허용된 여러 값 중 하나지만 어느 것인지 문서화 의무 없음
- undefined: 아무 요구도 없음
- ill-formed: 프로그램이 문법/의미 규칙에 맞지 않음
- IFNDR: ill-formed, no diagnostic required

이 구분은 “출력이 왜 다르지?”를 판단하는 핵심입니다.

다음: [`volatile`, `atomic`, 동시성](07-concurrency-volatile-atomic.md)
