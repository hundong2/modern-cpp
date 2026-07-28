# cppreference를 읽기 위한 C++ 기초

이 폴더의 목표는 C++ 기능을 외우는 것이 아니라, cppreference에 나오는 선언·표·용어를
혼자 해석할 수 있게 되는 것입니다. C++17/20의 새 기능 목록부터 읽지 말고 아래 순서를
따라가세요.

## 가장 먼저 할 일

1. [학습 순서](docs/00-study-plan.md)를 읽고 현재 위치를 확인합니다.
2. [cppreference 페이지 읽는 법](docs/01-how-to-read-cppreference.md)을 읽습니다.
3. 문서와 같은 번호의 `examples/` 코드를 직접 수정하고 실행합니다.
4. 모르는 기호나 용어는 [용어·기호 색인](docs/10-glossary.md)에서 찾습니다.
5. 색인에도 없다면 [모르는 항목을 조사하는 절차](docs/11-unknown-term-protocol.md)를
   사용합니다.

예시로 든 선언은 [함수 선언 해독법](docs/03-declarations-and-signatures.md#완전-해부-bool-is_lock_free-const-volatile-noexcept)과
[volatile·atomic·동시성](docs/07-concurrency-volatile-atomic.md#예시-선언을-다시-읽기)에서 두 번,
서로 다른 관점으로 해부합니다.

```cpp
bool is_lock_free() const volatile noexcept;
```

한 문장으로 먼저 읽으면 다음과 같습니다.

> 인자가 없고 `bool`을 반환하며, `const volatile` 객체에서도 호출할 수 있고,
> 예외를 던지지 않겠다고 선언한 `std::atomic<T>`의 멤버 함수.

## 문서 지도

| 단계 | 문서 | 연결 실습 |
|---:|---|---|
| 0 | [학습 순서](docs/00-study-plan.md) | 전체 |
| 1 | [cppreference 읽는 법](docs/01-how-to-read-cppreference.md) | 페이지 하나 골라 구조 표시 |
| 2 | [핵심 문법](docs/02-core-syntax.md) | `01`, `02` |
| 3 | [선언·call signature 해독](docs/03-declarations-and-signatures.md) | `01`, `03`, `05` |
| 4 | [타입·값·수명](docs/04-types-values-lifetime.md) | `03`, `04`, `10` |
| 5 | [클래스·템플릿·호출 가능 객체](docs/05-classes-templates-callables.md) | `04`, `05` |
| 6 | [표준 라이브러리 어휘](docs/06-standard-library-vocabulary.md) | `06` |
| 7 | [volatile·atomic·동시성](docs/07-concurrency-volatile-atomic.md) | `08` |
| 8 | [C++17 읽기 가이드](docs/08-cpp17-guide.md) | `07` |
| 9 | [C++20 읽기 가이드](docs/09-cpp20-guide.md) | `09` |
| 10 | [용어·기호 색인](docs/10-glossary.md) | 막힐 때마다 |
| 11 | [실전 선언 해독 문제](docs/12-signature-workbook.md) | 전 과정 복습 |

## 빌드

필요 조건은 C++20을 지원하는 컴파일러와 CMake 3.16 이상입니다.

```powershell
cd read-cpp-reference
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

각 파일을 따로 컴파일해도 됩니다.

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic examples/07_cpp17.cpp -o cpp17
g++ -std=c++20 -Wall -Wextra -pedantic examples/09_cpp20.cpp -o cpp20
```

MSVC라면 Developer PowerShell에서 다음처럼 실행합니다.

```powershell
cl /std:c++17 /EHsc /W4 examples\07_cpp17.cpp
cl /std:c++20 /EHsc /W4 examples\09_cpp20.cpp
```

## 권장 학습 방식

- 먼저 실행 결과를 예상해 적고 실행합니다.
- 한 번에 한 줄만 바꿉니다.
- 컴파일 오류는 실패가 아니라 “어떤 규칙이 적용됐는지 알려 주는 설명”으로 봅니다.
- `TRY_COMPILE_ERROR` 주석이 있는 줄은 주석을 풀어 오류를 읽은 뒤 다시 되돌립니다.
- API 페이지를 읽을 때 선언, 요구 조건, 반환값, 복잡도, 예외, 주의사항 순서로
  자기 말로 한 줄씩 바꿔 씁니다.

## 기준 자료

- [cppreference C++17](https://cppreference.com/cpp/17)
- [cppreference C++20](https://cppreference.com/cpp/20)
- [`std::atomic<T>::is_lock_free`](https://cppreference.com/cpp/atomic/atomic/is_lock_free)

cppreference는 ISO 표준 문서 자체가 아니라 실용적인 참고 자료입니다. 정확한 규범 문구가
필요할 때는 페이지의 `Paper(s)`, 결함 보고서(CWG/LWG), ISO 표준 조항으로 거슬러 올라갑니다.
