# 실무 프로젝트 구조: C++23, C#, Python 비교

## 한 줄 요약

C++은 소유권과 값/참조 비용을 타입과 코드로 직접 표현하고, C#과 Python은 가비지
컬렉션이 객체 메모리를 관리하지만 파일·소켓·잠금 같은 자원 수명은 세 언어 모두
명시적인 구조가 필요합니다.

## 1. 컴파일과 실행 모델

| 항목 | C++23 | C# | Python |
|---|---|---|---|
| 일반적인 변환 | 번역 단위 → 목적 파일 → 네이티브 실행 파일 | CIL 바이트코드 → JIT/AOT 네이티브 코드 | 바이트코드 → 인터프리터 실행 |
| 제네릭 | 템플릿 인스턴스화, concept로 제약 | 런타임 타입 정보를 유지하는 generics | 동적 타입, type hint와 protocol |
| 메모리 관리 | RAII와 값 수명, 명시적 소유권 | 추적식 GC + `IDisposable` | 참조 계수/GC 구현 + context manager |
| ABI 영향 | 컴파일러, 표준 라이브러리, 구조체 배치 영향이 큼 | 런타임과 interop 경계에서 중요 | C 확장/FFI 경계에서 중요 |

C++ 헤더의 템플릿 정의는 사용하는 번역 단위마다 인스턴스화될 수 있습니다. C#과
Python 사용자가 C++에 처음 올 때 `#include`를 모듈 import와 같은 저비용 이름 조회로
오해하면 빌드 시간과 ODR 문제를 놓치기 쉽습니다.

## 2. 값, 참조와 객체 수명

### C++23

```cpp
auto task = std::make_unique<Task>(/*...*/);  // 단독 소유
const Task& view = *task;                     // 필수 비소유 관찰
Task* maybe = task.get();                     // 선택적 비소유 관찰
```

`task`의 스코프가 끝나면 Task가 결정적인 시점에 파괴됩니다. `view`와 `maybe`는
Task 수명을 늘리지 않으므로 그 뒤에 사용하면 정의되지 않은 동작입니다.

### C#

```csharp
Task task = new Task();
Task alias = task;
```

두 변수는 같은 관리 객체를 참조하고 GC가 도달 가능성을 기준으로 메모리를 회수합니다.
그러나 파일이나 소켓은 `using`과 `IDisposable`로 결정적인 해제를 표현합니다.

### Python

```python
task = Task()
alias = task
```

두 이름은 같은 객체를 가리킵니다. CPython의 참조 계수로 빠르게 해제되는 경우가 많지만
언어 차원에서 특정 시점의 소멸자 실행에 의존하지 말고 파일은 `with` 문을 사용합니다.

### 흔한 실수

- C#/Python 사용자가 C++ 객체를 값으로 전달하면서 깊은 복사 비용을 예상하지 못함
- C++ 사용자가 C#의 `using`이나 Python의 `with` 없이 GC가 외부 자원까지 즉시
  해제할 것이라 기대함
- C++의 reference를 수명을 늘리는 관리 참조라고 오해함

## 3. `unique_ptr`와 `shared_ptr`

| 의도 | C++ | C# | Python |
|---|---|---|---|
| 독점 소유 | `std::unique_ptr<T>` | 일반 참조 + 설계 규약 | 일반 참조 + 설계 규약 |
| 공유 소유 | `std::shared_ptr<T>` | 관리 참조가 기본 | 객체 참조가 기본 |
| 비소유 관찰 | `T&`, `T*`, `std::span`, `string_view` | 일반 참조/`Span<T>` | 일반 객체 참조, view 타입 |
| 약한 참조 | `std::weak_ptr<T>` | `WeakReference<T>` | `weakref.ref` |

C#과 Python에서는 독점 소유가 타입으로 강제되지 않는 경우가 많습니다. C++에서 모든
것을 `shared_ptr`로 바꾸면 익숙해 보일 수 있지만 참조 계수 비용, 순환 소유와 불명확한
파괴 시점을 가져옵니다. 실제 공유가 아니면 `unique_ptr` 또는 값 멤버가 기본입니다.

## 4. 클래스 인터페이스와 다형성

### C++23

```cpp
class Repository {
public:
    virtual ~Repository() = default;
    virtual void save(Task task) = 0;
};
```

가상 호출은 일반적으로 객체의 vptr를 통해 함수 주소를 찾습니다. 기반 포인터로
파괴한다면 가상 소멸자가 필요합니다. 템플릿/concept를 사용하면 정적 다형성을 선택할
수 있지만 구현이 헤더에 노출되고 타입별 코드가 생성될 수 있습니다.

### C#

```csharp
public interface IRepository
{
    void Save(Task task);
}
```

interface와 virtual dispatch가 런타임에서 자연스럽게 지원됩니다. 값 형식과 boxing,
generic 제약에 따라 비용 구조가 달라집니다.

### Python

```python
from typing import Protocol

class Repository(Protocol):
    def save(self, task: Task) -> None: ...
```

duck typing 또는 `Protocol`로 구조적 계약을 표현합니다. type hint는 일반적으로
런타임 강제가 아니라 정적 분석 도구를 위한 정보입니다.

## 5. 컨테이너

| 목적 | C++ | C# | Python |
|---|---|---|---|
| 연속 동적 배열 | `std::vector<T>` | `List<T>` | `list` |
| 연결 리스트 | `std::list<T>` | `LinkedList<T>` | 표준 내장 직접 대응 없음 |
| 키 정렬 map | `std::map<K,V>` | `SortedDictionary<K,V>` | 일반 `dict`는 삽입 순서, 정렬 아님 |
| 해시 map | `std::unordered_map<K,V>` | `Dictionary<K,V>` | `dict` |

Python의 `list`는 이름과 달리 일반적으로 동적 배열 계열이며 C++ `std::list` 같은
연결 리스트가 아닙니다. C# `List<T>`도 C++ `vector`에 더 가깝습니다.

C++ 컨테이너는 원소를 값으로 소유합니다. `vector<unique_ptr<Task>>`는 포인터 객체를
연속 저장하고 각 Task는 별도 힙 객체에 있습니다. C#과 Python의 일반 객체 컨테이너는
객체 참조를 저장하므로 이 차이가 캐시 지역성과 복사 의미에 영향을 줍니다.

## 6. 정렬과 함수 전달

### C++23

```cpp
std::ranges::sort(tasks, {}, &TaskSnapshot::title);
```

projection으로 비교 전에 제목 멤버를 선택합니다. 정렬 가능한 임의 접근 범위인지
concept가 컴파일 단계에 검사합니다.

### C#

```csharp
tasks.Sort((left, right) =>
    string.Compare(left.Title, right.Title, StringComparison.Ordinal));
```

delegate/lambda를 전달하며 LINQ의 `OrderBy`는 일반적으로 새 열거 결과를 구성합니다.

### Python

```python
tasks.sort(key=lambda task: task.title)
```

key 함수의 결과를 기준으로 제자리 정렬합니다. 동적 타입 오류는 해당 경로가 실행될
때 드러날 수 있습니다.

## 7. 오류 전달

### C++23

```cpp
std::expected<TaskId, std::string> create_task(std::string title);
```

성공과 오류가 타입에 보이며 호출자가 상태를 검사합니다. 예외는 계약 위반이나 해당
계층에서 복구하기 어려운 실패에 별도로 사용할 수 있습니다.

### C#

가장 가까운 기본 관용구는 예외, `TryXxx(out value)` 패턴, nullable reference/value,
또는 프로젝트별 `Result<T>`입니다. `Task<T>`는 비동기 작업 타입이지 C++ 예제의
도메인 `Task`나 `expected`와 같은 의미가 아닙니다.

### Python

예외가 일반적이며 `None`, tuple, 사용자 Result 타입도 사용합니다. type hint의
`Optional[T]`는 C++ `optional<T>`와 개념적으로 가깝지만 런타임 표현과 메모리 배치는
다릅니다.

## 8. `const`, 불변성과 `constexpr`

- C++ `const`: 특정 식과 접근 경로를 통한 변경을 제한합니다. `mutable`, 별칭,
  논리적 const 같은 개념 때문에 객체 메모리가 절대 변하지 않는다는 뜻은 아닙니다.
- C# `const`: 주로 컴파일 시간 상수이며, 참조를 다시 대입하지 않는 것은 `readonly`가
  더 가깝습니다. 참조 대상 객체의 깊은 불변성을 자동 보장하지는 않습니다.
- Python: 언어가 강제하는 직접 대응은 없고 naming convention, immutable type,
  `Final` type hint 등을 사용합니다.
- C++ `constexpr`: 상수 평가가 가능한 코드라는 계약이며 호출 문맥에 따라 런타임에도
  실행될 수 있습니다. C# `const`나 Python 모듈 상수 표기와 동일하지 않습니다.

## 9. 예외, 이동과 성능

C++ move는 보통 vector/string의 힙 버퍼 소유권을 넘겨 깊은 복사를 피하지만,
“항상 포인터만 복사한다”는 보장은 없습니다. 타입별 move 구현을 확인해야 하며 이동된
객체는 유효하지만 값이 미지정인 상태일 수 있습니다.

C#과 Python은 일반 객체 변수를 대입할 때 참조를 복사하므로 C++의 값 복사·이동과
비용 모델이 다릅니다. 반대로 C++의 작은 값 타입은 힙 할당 없이 레지스터나 스택에
직접 배치될 수 있어 값 사용이 더 단순하고 빠를 수 있습니다.

## 10. 동시성과 메모리 모델

이 템플릿의 저장소와 `next_id_`는 스레드 안전하지 않습니다.

- C++에서 data race(동기화되지 않은 동시 읽기/쓰기)는 정의되지 않은 동작입니다.
- `shared_ptr`의 서로 다른 복사본이 관리하는 참조 계수 연산은 안전할 수 있지만,
  가리키는 Task 객체의 멤버 접근까지 자동으로 보호하지는 않습니다.
- C#의 `lock`, concurrent collection과 .NET memory model을 별도로 고려해야 합니다.
- Python도 인터프리터 잠금 구현에 의존해 애플리케이션 불변식이 자동으로 원자적이
  되지는 않습니다. 구현과 C 확장, 비동기 경계에 따라 조건이 달라집니다.

저장소를 동시 사용하려면 단일 멤버가 아니라 “중복 검사 → map 삽입 → list 삽입” 같은
복합 불변식을 하나의 mutex 임계 구역으로 보호해야 합니다.

## 11. FFI와 ABI 주의점

C++ 클래스, `std::string`, `std::vector`, 가상 함수 테이블을 C#, Python에 직접 노출하면
컴파일러와 표준 라이브러리 ABI 차이에 묶입니다. 보통 다음처럼 안정된 C 경계를 둡니다.

```cpp
extern "C" int task_create(const char* title, std::uint64_t* out_id);
```

- 소유권과 해제 함수를 문서화합니다.
- 예외가 C ABI 경계를 넘어가지 않게 내부에서 잡아 오류 코드로 변환합니다.
- 문자열 인코딩, 버퍼 길이와 null 가능성을 명시합니다.
- C#은 P/Invoke, Python은 `ctypes`, CFFI 또는 확장 모듈을 사용합니다.

이 비교의 핵심은 어느 언어가 더 낫다는 것이 아니라, 같은 단어처럼 보이는 기능도
수명·메모리 배치·오류 시점이 다르다는 점을 코드 리뷰에서 놓치지 않는 것입니다.
