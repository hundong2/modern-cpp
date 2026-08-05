# 1장: 모던 C++ 언어와 자원 관리

## Day 1: 템플릿·STL·스마트 포인터

[템플릿](https://en.cppreference.com/w/cpp/language/templates)은 타입을 매개변수로 받아 같은 알고리즘을 여러 타입에 적용한다. `PacketBox<T>`의 `T`는 컴파일 때 실제 타입으로 치환되며, Java/C#의 런타임 generic과 달리 각 인스턴스화가 별도 코드를 만들 수 있다.

[STL 컨테이너](https://en.cppreference.com/w/cpp/container)는 소유하는 원소의 수명과 반복자 무효화 규칙을 가진다.

- `std::vector`: 연속 메모리, 빠른 순회, 뒤 삽입 amortized O(1). 재할당 시 포인터/참조/반복자 무효화.
- `std::list`: 노드 기반 양방향 연결 리스트. 임의 접근이 없고 캐시 지역성이 나쁘다. “삽입이 O(1)”도 삽입 위치를 이미 알고 있을 때만 맞다.
- `std::map`: 키 정렬을 유지하며 검색/삽입 O(log N). 흔히 red-black tree지만 표준은 구현 자료구조를 강제하지 않는다.
- `std::unordered_map`: 평균 O(1) 검색이지만 해시/재해시/최악 O(N)을 고려한다.

[스마트 포인터](wiki/raii.md)는 소유권을 타입으로 표현한다. 세션 registry가 세션을 공동 소유해야만 `shared_ptr`를 쓴다. 단일 manager가 소유하고 나머지가 관찰한다면 `unique_ptr`와 비소유 핸들이 더 단순하다. `use_count()`는 학습/진단용이지 동기화 판단에 쓰지 않는다.

실습: [day01_templates_smart_ptr.cpp](examples/day01_templates_smart_ptr.cpp)

관찰 질문: 내부 블록을 나갈 때 지역 `shared_ptr`는 사라지는데 세션 객체가 남는 이유는? `registry.clear()` 뒤에는 누가 마지막 소유자였는가?

## 특별편: 매크로와 값 범주

함수형 매크로는 타입 검사가 없고 인자를 여러 번 평가할 수 있다. `SQUARE(i++)`는 수정 사이의 순서 규칙 때문에 과거 표준에서는 undefined behavior였고 C++17에서도 의도와 다르게 두 번 증가한다. 부작용 있는 식을 매크로 인자로 넣지 말고 `constexpr` 함수/템플릿을 쓴다.

`inline`은 [빌드/ODR 의미](wiki/build-pipeline.md)와 최적화 힌트의 역사를 구분해야 한다. 컴파일러는 `inline`이 있어도 호출을 남길 수 있고 없어도 인라이닝할 수 있다.

`const`는 그 경로로 값을 바꾸지 않겠다는 타입 속성이다. `constexpr` 변수는 컴파일 타임 상수 표현식이어야 한다. `constexpr` 함수는 상수 인자로 호출되면 상수 평가될 수 있지만, 런타임 인자로 호출하면 일반 함수처럼 실행될 수 있다. “반드시 컴파일 때 계산”은 상수 평가가 요구되는 문맥에서만 보장된다.

[lvalue/rvalue와 이동](wiki/value-categories.md)을 배울 때 세 가지를 분리한다.

1. 식의 값 범주가 어떤 오버로드를 선택하는가.
2. 선택된 이동 생성자가 어떤 자원을 이전하는가.
3. 이동 후 원본이 만족하는 불변식은 무엇인가.

raw `new/delete`로 Rule of Five를 직접 구현하기 전에 `std::string`·`std::vector`·`unique_ptr` 자체의 올바른 이동을 조합하는 Rule of Zero를 우선한다.

실습: [day01b_value_move.cpp](examples/day01b_value_move.cpp), [day01c_inline_constexpr.cpp](examples/day01c_inline_constexpr.cpp)

## 실무 체크리스트

- 함수 인자: 작은 값은 값, 읽기만 하는 큰 객체는 `const&`, 소유권 이전은 값 또는 `unique_ptr`로 표현했는가?
- 반환: 지역 객체를 값으로 반환하고 [copy elision](https://en.cppreference.com/w/cpp/language/copy_elision)에 맡겼는가? 지역 반환값에 습관적으로 `std::move`를 붙여 NRVO를 방해하지 않았는가?
- 클래스: 직접 소멸자를 쓰는 순간 복사/이동 정책(Rule of Five)을 모두 검토했는가?
- 컨테이너: 복잡도만 보지 말고 캐시 지역성과 반복자 무효화를 확인했는가?

## 연습

1. Day 1 registry를 `std::map<int, std::unique_ptr<Session>>`로 바꾸고 관찰자는 `Session*`를 잠시만 사용하게 하라.
2. `std::list<std::shared_ptr<Session>> waiting_room`을 추가하고 참조 수 변화를 예상하라.
3. 이동 생성자의 `noexcept`를 제거한 타입을 `vector`가 재배치할 때 복사/이동 중 무엇을 선택하는지 로그로 관찰하라.
