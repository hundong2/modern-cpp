# 자주쓰는 Modern C++ 실무 문법 커리큘럼

참고 사이트:

- 모두의 코드: <https://modoocode.com/>
- 씹어먹는 C++ 강좌 계획: <https://modoocode.com/135>
- 구글에서는 C++ 을 어떻게 쓰는가?: <https://modoocode.com/335>

`modoocode`는 C++의 기초 문법, 클래스, 템플릿, STL, 문자열, 이동 의미론,
스마트 포인터, 동시성까지 단계적으로 다룹니다. 이 폴더는 그 흐름을 바탕으로
실무에서 자주 보이는 코드 패턴을 메모리 관점으로 다시 정리한 예제 모음입니다.

## 빌드

현재 저장소의 Windows 환경에서는 portable GCC가 `tools/w64devkit`에 설치되어 있습니다.
한글 경로에서는 `MinGW Makefiles`보다 `Ninja` generator가 더 안정적으로 동작합니다.

```powershell
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S 자주쓰는 -B 자주쓰는/build -G Ninja "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build 자주쓰는/build
```

검증 기준:

- `g++ (GCC) 16.1.0`
- `Ninja` generator

## 파일 리스트와 학습 순서

- [01_range_for_and_references.cpp](./01_range_for_and_references.cpp)
  - `for (const auto& value : values)`를 왜 자주 쓰는지 설명합니다.
  - 값 복사, 참조, `const` 참조, 수정 가능한 참조의 차이를 봅니다.

- [02_parameter_passing.cpp](./02_parameter_passing.cpp)
  - 함수 인자를 값, `const&`, `string_view`, 포인터로 받을 때의 기준을 정리합니다.
  - 출력 파라미터보다 반환값을 선호하는 이유와 RVO 관점을 다룹니다.

- [03_vector_memory_reserve.cpp](./03_vector_memory_reserve.cpp)
  - `std::vector`의 capacity, 재할당, `reserve`, `emplace_back`을 다룹니다.
  - 반복자/포인터 무효화가 왜 실무 버그로 이어지는지 설명합니다.

- [04_string_view_span.cpp](./04_string_view_span.cpp)
  - 문자열과 배열을 복사하지 않고 읽는 `std::string_view`, `std::span`을 다룹니다.
  - 뷰 타입의 수명 문제를 메모리 관점에서 설명합니다.

- [05_move_semantics.cpp](./05_move_semantics.cpp)
  - 이동 생성자, 이동 대입, `std::move`가 실제로 무엇을 의미하는지 봅니다.
  - 큰 버퍼를 가진 객체가 복사보다 이동이 유리한 이유를 설명합니다.

- [06_smart_pointers_raii.cpp](./06_smart_pointers_raii.cpp)
  - `unique_ptr`, `shared_ptr`, `weak_ptr`, RAII를 실무 기준으로 정리합니다.
  - 소유권을 코드로 표현하는 방법을 다룹니다.

- [07_algorithms_lambdas.cpp](./07_algorithms_lambdas.cpp)
  - `find_if`, `sort`, `erase_if`, 람다 캡처를 이용한 STL 알고리즘 사용법입니다.
  - 수동 루프보다 알고리즘을 쓰면 의도가 어떻게 명확해지는지 봅니다.

- [08_templates_policy.cpp](./08_templates_policy.cpp)
  - 템플릿을 단순 문법이 아니라 정책 주입(policy)으로 쓰는 실무 패턴입니다.
  - 런타임 다형성과 컴파일 타임 다형성의 비용 차이를 설명합니다.

- [09_perfect_forwarding_factory.cpp](./09_perfect_forwarding_factory.cpp)
  - `T&&`, forwarding reference, `std::forward`를 팩토리 함수 예제로 설명합니다.
  - 불필요한 복사를 줄이는 템플릿 작성법을 다룹니다.

- [10_concepts_type_traits.cpp](./10_concepts_type_traits.cpp)
  - C++20 Concepts와 type traits로 템플릿 오류를 읽기 쉽게 만드는 방법입니다.
  - “아무 타입이나 받는 템플릿”을 “조건을 만족하는 타입만 받는 템플릿”으로 바꿉니다.

- [11_containers_lookup.cpp](./11_containers_lookup.cpp)
  - `vector`, `map`, `unordered_map`의 선택 기준과 조회 비용을 다룹니다.
  - 메모리 지역성, 정렬 필요 여부, 해시 비용을 함께 설명합니다.

- [12_status_result_no_exception.cpp](./12_status_result_no_exception.cpp)
  - 예외 대신 상태 객체와 결과 타입을 반환하는 실무 패턴입니다.
  - 서버/대규모 코드베이스에서 오류를 값으로 다루는 방식을 설명합니다.

- [13_pointer_shared_ptr_memory.cpp](./13_pointer_shared_ptr_memory.cpp)
  - raw pointer, reference, `unique_ptr`, `shared_ptr`, `weak_ptr`의 역할을 비교합니다.
  - 참조 카운트, 제어 블록, 순환 참조, dangling pointer를 메모리 관점에서 설명합니다.

- [14_preprocessor_macros.cpp](./14_preprocessor_macros.cpp)
  - 오픈소스에서 자주 보이는 `#define`, `#`, `##`, variadic macro, X-macro를 설명합니다.
  - 매크로가 타입 검사 없이 치환된다는 점과 안전하게 감싸는 실무 패턴을 다룹니다.

## 실무에서 자주 쓰는 판단 기준

- 큰 객체를 읽기만 하면 `const T&` 또는 뷰 타입을 우선 고려합니다.
- 작은 정수, enum, 포인터, iterator는 값으로 받아도 보통 충분히 저렴합니다.
- 소유권이 있으면 `unique_ptr`, 공유 소유가 정말 필요하면 `shared_ptr`를 사용합니다.
- `shared_ptr`는 “여러 곳에서 접근한다”가 아니라 “여러 곳이 공동 소유한다”는 뜻입니다.
  단순 관찰은 raw pointer, reference, `weak_ptr`를 먼저 고려합니다.
- `std::vector`에 많이 넣을 개수를 알면 `reserve`를 먼저 호출합니다.
- 직접 루프보다 `algorithm`이 의도를 더 잘 드러내면 알고리즘을 사용합니다.
- 템플릿은 “여러 타입에 대해 같은 코드를 재사용”하거나 “정책을 컴파일 타임에 바꾸기”
  위해 사용합니다. 단순히 멋있어 보이게 만들기 위해 쓰지 않습니다.
- 성능 최적화는 복사 횟수, 할당 횟수, 캐시 지역성, 동기화 비용을 먼저 봅니다.
- 매크로는 컴파일 전에 텍스트 치환됩니다. 타입 안전성이 필요하면 `constexpr`,
  `inline function`, `template`, `enum class`를 먼저 고려하고, 조건부 컴파일이나
  코드 생성 패턴처럼 전처리기가 필요한 경우에만 제한적으로 사용합니다.

