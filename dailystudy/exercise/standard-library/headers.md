# 현재 코드에서 사용하는 표준 헤더

`#include`는 해당 선언을 현재 번역 단위에서 사용할 수 있게 한다. 헤더 주석은 “헤더를 제공한다”가 아니라 이 파일에서 실제 사용하는 선언을 구체적으로 말해야 한다.

| 헤더 | 현재 자료에서 사용하는 대표 선언 |
|---|---|
| `<algorithm>` | `sort`, `find_if`, `fill`, `max_element`, ranges 알고리즘 |
| `<array>` | 고정 크기 `std::array` |
| `<atomic>` | `std::atomic`, 메모리 순서 상수 |
| `<barrier>` | C++20 반복 단계 동기화 객체 `std::barrier`와 도착 token |
| `<bit>` | `bit_cast`, `byteswap`, `endian` 객체 표현·바이트 순서 도구 |
| `<cassert>` | 디버그 검증 매크로 `assert` (`NDEBUG`에서 제거 가능) |
| `<charconv>` | 비예외 숫자 파싱 `from_chars` |
| `<chrono>` | duration, time_point, clock, 시간 리터럴 |
| `<compare>` | 삼방향 비교 범주 `strong_ordering` |
| `<concepts>` | `same_as`, `integral`, `convertible_to` |
| `<condition_variable>` | mutex 기반 술어 대기와 `notify_one`/`notify_all` |
| `<coroutine>` | `coroutine_handle`, `suspend_always` |
| `<cstddef>` | `size_t`, `ptrdiff_t`, `byte` |
| `<cstdint>` | `uint64_t` 등 고정 폭 정수 별칭 |
| `<cstdlib>` | 정수 `abs`, `exit` 등 C 호환 유틸리티 |
| `<deque>` | 양끝 큐 `std::deque`와 앞·뒤 삽입/삭제 API |
| `<expected>` | C++23 `expected`, `unexpected` |
| `<filesystem>` | `path`, 디렉터리 반복자와 파일 상태 API |
| `<functional>` | `function`, C++23 `move_only_function`, 비교 함수 객체, 참조 래퍼 |
| `<iomanip>` | `setprecision` 등 스트림 형식 조작자 |
| `<iostream>` | `cin`, `cout`, `cerr` 표준 스트림 |
| `<latch>` | C++20 일회성 카운트다운 동기화 객체 `std::latch` |
| `<limits>` | `numeric_limits` 타입별 수치 한계 |
| `<map>` | 정렬 연관 컨테이너 `map` |
| `<memory>` | 스마트 포인터와 `make_unique`/`make_shared` |
| `<memory_resource>` | `std::pmr` 메모리 자원과 컨테이너 별칭 |
| `<mdspan>` | C++23 다차원 비소유 뷰 `mdspan`, 동적 차원 `dextents`, 레이아웃 정책 |
| `<mutex>` | `mutex`, `scoped_lock`, `unique_lock` |
| `<numbers>` | `pi_v` 등 수학 상수 템플릿 |
| `<numeric>` | `accumulate`, `iota` 수치 알고리즘 |
| `<optional>` | 선택적 값 `optional`, 빈 태그 `nullopt` |
| `<queue>` | FIFO `queue`, 힙 `priority_queue` |
| `<ranges>` | range 개념, views, customization point |
| `<semaphore>` | C++20 계수형·이진 permit 동기화 객체 `counting_semaphore`, `binary_semaphore` |
| `<shared_mutex>` | 읽기-쓰기 잠금과 `shared_lock` |
| `<source_location>` | 호출 위치 값 `source_location` |
| `<span>` | 비소유 연속 범위 `span` |
| `<sstream>` | 문자열 버퍼를 소유하는 입력·출력 스트림과 `ostringstream` |
| `<stop_token>` | 협력적 취소 상태 `stop_token` |
| `<string>` | 소유 문자열 `string`, 변환 보조 함수 |
| `<string_view>` | 비소유 문자 뷰 `string_view` |
| `<syncstream>` | 레코드 단위 동시 출력 `osyncstream`과 동기 버퍼 |
| `<system_error>` | `error_code`, `errc` 오류 값 |
| `<thread>` | `jthread`, `this_thread` 함수 |
| `<tuple>` | 이종 값 묶음 `tuple`과 `get` |
| `<type_traits>` | `is_same_v`, `decay_t`, `remove_cvref_t` |
| `<unordered_map>` | 해시 연관 컨테이너 `unordered_map` |
| `<utility>` | `move`, `forward`, `exchange`, `pair` |
| `<variant>` | 태그된 합 타입 `variant`와 방문 API |
| `<vector>` | 동적 연속 컨테이너 `vector`, `erase_if` 오버로드 |

## `assert` 주의점

`assert(condition)`은 표준 함수가 아니라 `<cassert>`가 제공하는 매크로다. 조건이 거짓이면 진단 후 중단하지만 `NDEBUG`가 정의된 빌드에서는 식 자체가 제거될 수 있다. 프로그램의 필수 입력 검증이나 부수 효과를 `assert` 안에 넣지 않는다.

## 직접 검증

1. 각 C++ 파일에서 직접 사용하는 선언이 어느 헤더에서 왔는지 하나씩 연결한다.
2. 전이 포함에 우연히 기대는 심볼이 없는지 필요한 헤더를 직접 포함했는지 확인한다.
3. `NDEBUG` 빌드에서 사라지면 안 되는 로직이 `assert` 안에 없는지 검색한다.
