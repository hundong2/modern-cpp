# 현재 코드의 표준 라이브러리 심볼 색인

이 색인은 `dailystudy/exercise/YYYY-MM-DD/*.cpp`에서 추출한 `std::` 이름을 세부 설명으로 연결한다. 새 심볼이 생기면 기존 대표 문서를 보강하고 이 색인에 한 번만 추가한다.

## 알고리즘과 ranges

| 심볼 | 짧은 역할 | 상세 문서 |
|---|---|---|
| `std::accumulate` | 초기값 타입으로 왼쪽 누적 | [알고리즘](algorithms-and-ranges.md) |
| `std::erase_if` | 술어가 참인 컨테이너 원소 삭제 | [알고리즘](algorithms-and-ranges.md) |
| `std::fill` | 반복자 구간에 같은 값 대입 | [알고리즘](algorithms-and-ranges.md) |
| `std::find_if` | 술어가 참인 첫 반복자 반환 | [알고리즘](algorithms-and-ranges.md) |
| `std::iota` | 연속 증가값으로 범위 채우기 | [알고리즘](algorithms-and-ranges.md) |
| `std::max_element` | 최댓값 원소 반복자 반환 | [알고리즘](algorithms-and-ranges.md) |
| `std::min`, `std::max` | 두 값 중 작은/큰 값 선택 | [알고리즘](algorithms-and-ranges.md) |
| `std::sort` | 반복자 범위 제자리 정렬 | [알고리즘](algorithms-and-ranges.md) |
| `std::swap` | 두 객체 값 교환 | [알고리즘](algorithms-and-ranges.md) |
| `std::ranges::count_if` | 범위에서 술어가 참인 개수 | [알고리즘](algorithms-and-ranges.md) |
| `std::ranges::end` | 범위 끝 센티널 획득 | [알고리즘](algorithms-and-ranges.md) |
| `std::ranges::find` | 범위에서 값 탐색 | [알고리즘](algorithms-and-ranges.md) |
| `std::ranges::sort` | 범위·프로젝션 기반 정렬 | [알고리즘](algorithms-and-ranges.md) |
| `std::views::filter` | 조건을 만족하는 원소의 지연 뷰 | [알고리즘](algorithms-and-ranges.md) |
| `std::views::transform` | 변환 결과의 지연 뷰 | [알고리즘](algorithms-and-ranges.md) |
| `std::less`, `std::greater` | 엄격 순서 비교 함수 객체 | [알고리즘](algorithms-and-ranges.md) |

## 컨테이너와 문자열

| 심볼 | 짧은 역할 | 상세 문서 |
|---|---|---|
| `std::array` | 컴파일 시간 고정 크기 연속 배열 | [컨테이너](containers-and-views.md) |
| `std::deque` | 양끝 상수 시간 삽입·삭제 시퀀스 | [컨테이너](containers-and-views.md) |
| `std::map` | 정렬 키 연관 컨테이너 | [컨테이너](containers-and-views.md) |
| `std::unordered_map` | 해시 기반 연관 컨테이너 | [컨테이너](containers-and-views.md) |
| `std::queue` | FIFO 컨테이너 어댑터 | [컨테이너](containers-and-views.md) |
| `std::priority_queue` | 힙 기반 우선순위 어댑터 | [컨테이너](containers-and-views.md) |
| `std::vector` | 동적 연속 배열 | [컨테이너](containers-and-views.md) |
| `std::string` | 소유 문자 시퀀스 | [컨테이너](containers-and-views.md) |
| `std::string::npos` | string 검색 실패 위치 상수 | [컨테이너](containers-and-views.md) |
| `std::string_view` | 비소유 문자 범위 | [컨테이너](containers-and-views.md) |
| `std::string_view::npos` | 검색 실패 위치 상수 | [컨테이너](containers-and-views.md) |
| `std::span` | 비소유 연속 원소 범위 | [컨테이너](containers-and-views.md) |
| `std::mdspan`, `std::dextents` | 다차원 비소유 뷰와 동적 차원 | [컨테이너](containers-and-views.md) |
| `std::pmr::memory_resource` | 실행 시간 할당 자원 인터페이스 | [컨테이너](containers-and-views.md) |
| `std::pmr::monotonic_buffer_resource` | 일괄 해제형 증가 할당 자원 | [컨테이너](containers-and-views.md) |
| `std::pmr::vector` | 다형적 할당자를 쓰는 vector 별칭 | [컨테이너](containers-and-views.md) |

## 비트와 바이트 표현

| 심볼 | 짧은 역할 | 상세 문서 |
|---|---|---|
| `std::bit_cast` | 같은 크기 객체 표현을 새 값으로 복사 | [비트·바이트](bit-and-byte-utilities.md) |
| `std::byteswap` | 정수의 바이트 순서를 반대로 변환 | [비트·바이트](bit-and-byte-utilities.md) |
| `std::endian` | 구현의 바이트 순서를 표현하는 열거형 | [비트·바이트](bit-and-byte-utilities.md) |
| `std::endian::native` | 구현의 스칼라 바이트 순서 | [비트·바이트](bit-and-byte-utilities.md) |
| `std::endian::little`, `std::endian::big` | 알려진 작은/큰 바이트 순서 값 | [비트·바이트](bit-and-byte-utilities.md) |
| `std::int64_t`, `std::uint32_t`, `std::uint64_t`, `std::uintmax_t`, `std::byte`, `std::size_t`, `std::ptrdiff_t` | 고정·최대 폭 정수, 원시 바이트, 크기·차이 타입 | [비트·바이트](bit-and-byte-utilities.md) |

## 소유권과 어휘 타입

| 심볼 | 짧은 역할 | 상세 문서 |
|---|---|---|
| `std::unique_ptr` | 독점 소유 스마트 포인터 | [소유권](ownership-and-vocabulary-types.md) |
| `std::make_unique` | 객체와 unique_ptr 생성 | [소유권](ownership-and-vocabulary-types.md) |
| `std::shared_ptr` | 참조 횟수 기반 공유 소유권 | [소유권](ownership-and-vocabulary-types.md) |
| `std::make_shared` | 객체와 shared_ptr 생성 | [소유권](ownership-and-vocabulary-types.md) |
| `std::weak_ptr` | shared_ptr 객체의 비소유 관찰자 | [소유권](ownership-and-vocabulary-types.md) |
| `std::optional`, `std::nullopt` | 선택적 값과 빈 상태 태그 | [소유권](ownership-and-vocabulary-types.md) |
| `std::expected`, `std::unexpected` | 성공값 또는 오류값 | [소유권](ownership-and-vocabulary-types.md) |
| `std::variant` | 여러 후보 중 하나의 태그된 합 | [소유권](ownership-and-vocabulary-types.md) |
| `std::visit` | variant 활성 대안 방문 | [소유권](ownership-and-vocabulary-types.md) |
| `std::get`, `std::get_if`, `std::holds_alternative` | variant/tuple 대안 접근·검사 | [소유권](ownership-and-vocabulary-types.md) |
| `std::pair`, `std::tuple` | 위치 기반 이종 값 묶음 | [소유권](ownership-and-vocabulary-types.md) |
| `std::apply` | tuple-like 원소를 위치 인자로 펼치는 호출 어댑터 | [소유권](ownership-and-vocabulary-types.md) |
| `std::reference_wrapper`, `std::cref` | 복사 가능한 비소유 참조 래퍼 | [소유권](ownership-and-vocabulary-types.md) |
| `std::function` | 호출 가능 객체 타입 소거 래퍼 | [소유권](ownership-and-vocabulary-types.md) |
| `std::move_only_function` | 이동 전용 호출 가능 객체 타입 소거 래퍼 | [소유권](ownership-and-vocabulary-types.md) |
| `std::source_location`, `std::source_location::current` | 호출 소스 위치 캡처 | [소유권](ownership-and-vocabulary-types.md) |
| `std::strong_ordering` | 강한 삼방향 비교 결과 | [소유권](ownership-and-vocabulary-types.md) |

## 타입 특성과 concept

| 심볼 | 짧은 역할 | 상세 문서 |
|---|---|---|
| `std::is_same_v` | 두 타입의 정확한 동일성 | [소유권](ownership-and-vocabulary-types.md) |
| `std::remove_cvref_t` | 최상위 cv와 참조 제거 | [소유권](ownership-and-vocabulary-types.md) |
| `std::decay_t` | 값 전달형 타입 변환 | [소유권](ownership-and-vocabulary-types.md) |
| `std::same_as` | 같은 타입 concept | [소유권](ownership-and-vocabulary-types.md) |
| `std::integral` | 정수 타입 concept | [소유권](ownership-and-vocabulary-types.md) |
| `std::convertible_to` | 변환 가능 concept | [소유권](ownership-and-vocabulary-types.md) |

## 동시성, 시간, 파일 시스템, 코루틴

| 심볼 | 짧은 역할 | 상세 문서 |
|---|---|---|
| `std::atomic` | 원자 load/store/RMW 상태 | [동시성](concurrency-time-filesystem.md) |
| `std::memory_order_relaxed` | 원자성만 필요한 약한 순서 | [동시성](concurrency-time-filesystem.md) |
| `std::memory_order_release` | 게시 저장 순서 | [동시성](concurrency-time-filesystem.md) |
| `std::memory_order_acquire` | 게시 값 관찰 로드 순서 | [동시성](concurrency-time-filesystem.md) |
| `std::mutex`, `std::scoped_lock`, `std::unique_lock` | 독점 잠금과 RAII 소유자 | [동시성](concurrency-time-filesystem.md) |
| `std::condition_variable` | mutex와 술어를 결합한 대기·깨우기 | [동시성](concurrency-time-filesystem.md) |
| `std::shared_mutex`, `std::shared_lock` | 읽기 공유·쓰기 독점 잠금 | [동시성](concurrency-time-filesystem.md) |
| `std::jthread`, `std::stop_token` | RAII 스레드와 협력적 취소 | [동시성](concurrency-time-filesystem.md) |
| `std::latch` | 일회성 카운트다운 대기·동기화 | [동시성](concurrency-time-filesystem.md) |
| `std::barrier` | 완료 함수를 포함한 반복 단계 동기화 | [동시성](concurrency-time-filesystem.md) |
| `std::counting_semaphore` | counter 기반 동시 permit 상한과 release/acquire 동기화 | [동시성](concurrency-time-filesystem.md) |
| `std::binary_semaphore` | 최대 counter 1인 단일 신호·handoff 별칭 | [동시성](concurrency-time-filesystem.md) |
| `std::this_thread::sleep_for` | 현재 스레드 상대 시간 대기 | [동시성](concurrency-time-filesystem.md) |
| `std::chrono::milliseconds`, `std::chrono::microseconds` | 밀리·마이크로초 duration 별칭 | [동시성](concurrency-time-filesystem.md) |
| `std::chrono::steady_clock`, `std::chrono::time_point` | 단조 시계와 시점 | [동시성](concurrency-time-filesystem.md) |
| `std::chrono::duration_cast`, `std::chrono_literals` | duration 단위 변환과 리터럴 | [동시성](concurrency-time-filesystem.md) |
| `std::filesystem` | 경로와 파일 시스템 namespace | [동시성](concurrency-time-filesystem.md) |
| `std::error_code` | 값 기반 시스템 오류 | [동시성](concurrency-time-filesystem.md) |
| `std::promise`, `std::future` | 한 번 게시하고 한 번 소비하는 비동기 shared-state handle | [동시성](concurrency-time-filesystem.md) |
| `std::coroutine_handle`, `std::suspend_always` | 코루틴 프레임 핸들·항상 중단 awaiter | [동시성](concurrency-time-filesystem.md) |

## 입출력, 파싱, 유틸리티

| 심볼 | 짧은 역할 | 상세 문서 |
|---|---|---|
| `std::format` | 타입 검사 형식 문자열과 인자로 새 소유 문자열 생성 | [입출력](io-parsing-and-utilities.md) |
| `std::bad_alloc` | 동적 할당 요청 실패를 나타내는 예외 타입 | [입출력](io-parsing-and-utilities.md) |
| `std::cin`, `std::cout`, `std::cerr` | 표준 입력·출력·오류 스트림 객체 | [입출력](io-parsing-and-utilities.md) |
| `std::istream` | 추출 연산이 같은 입력 스트림 참조를 이어 반환하는 기반 타입 | [입출력](io-parsing-and-utilities.md) |
| `std::ostream` | 출력 연산이 같은 스트림 참조를 이어 반환하는 기반 타입 | [입출력](io-parsing-and-utilities.md) |
| `std::osyncstream` | 스레드별 출력 조각을 한 덩어리로 emit하는 동기 스트림 | [입출력](io-parsing-and-utilities.md) |
| `std::ostringstream` | 내부 문자열 버퍼를 소유하는 메모리 출력 스트림 | [입출력](io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio` | C/C++ 표준 스트림 동기화 설정 | [입출력](io-parsing-and-utilities.md) |
| `std::fixed`, `std::setprecision` | 부동소수점 출력 형식 조작 | [입출력](io-parsing-and-utilities.md) |
| `std::from_chars`, `std::errc` | 비예외 문자 범위 숫자 파싱 | [입출력](io-parsing-and-utilities.md) |
| `std::getline` | 구분자 전까지 한 줄을 소유 문자열에 입력 | [입출력](io-parsing-and-utilities.md) |
| `std::to_string` | 숫자를 소유 문자열로 변환 | [입출력](io-parsing-and-utilities.md) |
| `std::move`, `std::forward`, `std::exchange` | 값 범주 변환·전달·값 교체 | [입출력](io-parsing-and-utilities.md) |
| `std::numeric_limits` | 기본 타입 수치 한계 | [입출력](io-parsing-and-utilities.md) |
| `std::abs` | 절댓값 계산 | [입출력](io-parsing-and-utilities.md) |
| `std::numbers::pi_v` | 타입별 원주율 상수 | [입출력](io-parsing-and-utilities.md) |
| `std::exit`, `std::terminate` | 스택 정상 복귀가 아닌 프로그램 종료 | [입출력](io-parsing-and-utilities.md) |

## 색인 갱신 규칙

감사 스크립트가 누락 심볼을 보고하면 다음 순서로 처리한다.

1. 기존 세부 문서에서 실질적으로 같은 주제가 있는지 검색한다.
2. 있으면 그 문서의 대표 섹션을 보강하고 이 색인에 링크한다.
3. 없으면 kebab-case 파일 하나를 만들되 비슷한 문서를 중복 생성하지 않는다.
4. 해당 날짜 README 표와 코드 첫 의미 있는 호출 주석을 보강한다.
5. 감사, 컴파일, 실행, CTest를 다시 수행한다.
