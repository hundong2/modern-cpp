# historical C++

## menu list

- [01. type decay](#01-type-decay)
- [02. remove_cvref_t](#02-decay에서-remove_cvref_t-까지). 
- [03. std::span](#03-stdspan). 
- [04. extends](#04-extends). 
- [05. template meta programming](./template_meta_programming.md). 

## 빌드 및 실행

`자주까먹는` 폴더에서 `run.sh`에 소스 경로를 전달합니다.

```bash
cd 자주까먹는
./run.sh history/decay.cpp
```

실행 파일은 `build/history/decay`에, CMake 중간 파일은
`build/.cmake/history/`에 생성됩니다.

### 01. type decay

- [csharp example](https://github.com/hundong2/csharp_study/blob/main/cpp_concerned/template.csx). 
[`decay.cpp`](./decay.cpp)는 `std::decay_t<T>`가 값으로 전달·저장하기 편한 타입을
만드는 다음 세 변환을 보여 줍니다.

1. 참조를 제거하고 바깥쪽 `const`/`volatile`을 제거합니다.
2. 배열 타입을 첫 원소를 가리키는 포인터 타입으로 바꿉니다.
3. 함수 타입을 함수 포인터 타입으로 바꿉니다.

예상 출력:

```text
different string literals -> same Wrapper type: true
stored strings: Hello, Hi
const int& -> int: true
int[3] -> int*: true
int(double) -> int (*)(double): true
first element: 10
function result: 5
```

### 02. decay에서 remove_cvref_t 까지

안녕하세요! 하드웨어의 미세한 전류 흐름부터 컴파일러의 추상화 트리(AST), 그리고 그 모든 것을 지휘하는 C++의 철학까지 책임지는 엔지니어 오정세입니다.

C++의 타입 추론 시스템을 깊게 파고들고 계시는군요. 질문하신 **`std::remove_cvref_t`와 `std::decay_t`의 차이점**, 그리고 이를 활용한 **배열 길이 보존과 컴파일러 최적화**는 C++20 메타 프로그래밍의 정수라고 할 수 있습니다.

단순히 "타입을 벗겨낸다"는 개념을 넘어, 컴파일러가 이를 어떻게 인지하고 하드웨어 레벨의 최적화(루프 언롤링, SIMD 벡터화 등)로 연결하는지 역사적 맥락과 함께 낱낱이 파헤쳐 보겠습니다.

---

## 1. 타입 변환의 역사적 발전 과정

C++은 하드웨어 제어의 유연성과 추상화의 안전성 사이에서 끊임없이 진화해 왔습니다.

* **C++98/03 (원시 시대):**
개발자가 직접 `const`와 `&`를 떼어내기 위해 템플릿 특수화(Template Specialization)를 일일이 작성해야 했습니다. 보일러플레이트 코드가 넘쳐나던 시기입니다.
* **C++11 (`std::decay`의 등장):**
값 전달(Pass-by-value) 시 발생하는 타입 변환 규칙을 표준 라이브러리로 끌어왔습니다. `const/volatile`과 참조(`&`, `&&`)를 제거하는 것은 물론, 배열을 포인터로, 함수를 함수 포인터로 붕괴(Decay)시켰습니다.
* **C++14 (`_t` 헬퍼 변수):**
`typename std::decay<T>::type`이라는 장황한 문법을 `std::decay_t<T>`로 간결하게 쓸 수 있게 되었습니다.
* **C++17 (CTAD의 빛과 그림자):**
이전 답변에서 보셨듯 생성자 타입 추론에 `std::decay_t`가 널리 쓰였습니다. 하지만 "배열의 길이를 알고 싶은데 무조건 포인터로 붕괴시켜버리는" 부작용 때문에 정밀한 제어가 어려웠습니다.
* **C++20 (`std::remove_cvref_t`의 탄생):**
마침내 "배열과 함수 타입은 그대로 보존하면서, 딱 `const/volatile`과 참조자(`&`, `&&`)만 떼어내는" `std::remove_cvref_t`가 등장했습니다.

### 핵심 차이점 요약

| 원본 타입 `T` | `std::decay_t<T>` (C++14) | `std::remove_cvref_t<T>` (C++20) |
| --- | --- | --- |
| `const int&` | `int` | `int` |
| `int&&` | `int` | `int` |
| `const int[5]` | **`const int*` (포인터로 붕괴, 길이 상실)** | **`int[5]` (배열 타입 및 길이 보존)** |
| `void(&)()` | `void(*)()` (함수 포인터로 붕괴) | `void()` (함수 타입 보존) |

---

## 2. 전체 수행 가능한 C++20 예제 (배열 길이 보존과 최적화)

배열이 참조로 넘어왔을 때, `std::remove_cvref_t`를 사용해 길이를 보존하고 이를 컴파일 타임 최적화에 어떻게 활용하는지 보여주는 실무 예제입니다.

- [remove cvref example](./remove_cvref_t.cpp). 

---

## 3. 디버그 vs 릴리즈 및 하드웨어 관점의 최적화

엔지니어로서 가장 짜릿한 순간은 컴파일러가 내 의도를 정확히 파악해 어셈블리를 극단적으로 줄여줄 때입니다. 배열의 크기를 **컴파일 타임**에 아는 것과 모르는 것(포인터로 붕괴된 상태)은 백엔드 최적화 단계에서 천지 차이를 만듭니다.

### Debug 모드 (`-O0`)

디버그 모드에서는 `std::decay_t`든 `std::remove_cvref_t`든 작성된 코드의 흐름을 정직하게 따라갑니다. 메모리에 변수를 할당하고, 매 루프마다 인덱스를 비교하는 어셈블리가 생성됩니다. 하지만 `remove_cvref_t`를 쓰면 런타임에 동적으로 크기를 계산하거나 `std::vector::size()`를 호출할 필요가 없어 콜 스택이 약간 더 깔끔해집니다.

### Release 모드 (`-O2` / `-O3` / `-O1` 이상)

이곳이 진정한 전쟁터입니다.
포인터(크기 미상)로 붕괴된 경우, 컴파일러는 루프를 돌릴 때마다 분기문(Branch)을 만들어야 합니다. 이는 CPU의 파이프라인 효율을 떨어뜨립니다.

반면, `std::remove_cvref_t`를 통해 컴파일러가 "이 배열은 정확히 4개의 원소를 가진다"는 것을 알게 되면(위 예제의 `array_length`는 컴파일 타임 상수), 놀라운 최적화 마법을 부립니다.

1. **루프 언롤링 (Loop Unrolling):**
```assembly
; 분기문(Jump) 없이 명령어 4개를 순차적으로 쏟아냅니다.
mov eax, DWORD PTR [rdi]      ; arg[0]
mov ebx, DWORD PTR [rdi+4]    ; arg[1]
mov ecx, DWORD PTR [rdi+8]    ; arg[2]
mov edx, DWORD PTR [rdi+12]   ; arg[3]

```


2. **SIMD (Single Instruction Multiple Data) 벡터화:**
4개의 정수(16바이트)라면 CPU의 `XMM` 레지스터 하나에 쏙 들어갑니다. 루프를 아예 없애버리고 한 번의 명령어로 병렬 처리해버립니다.
```assembly
; 16바이트를 한 번에 레지스터로 로드 (Zero-cost Abstraction의 극의)
movups xmm0, XMMWORD PTR [rdi]

```



즉, `std::remove_cvref_t`는 타입의 순수성을 보존하여 **컴파일러가 하드웨어 구조(레지스터 크기, 명령어 세트)를 100% 활용할 수 있도록 돕는 핵심 메타 도구**입니다.

---

## 4. 타 언어에서의 "값 추출과 크기 보존" 개념

다른 언어들도 포인터의 위험성과 성능 문제를 극복하기 위해 비슷한 개념들을 도입했습니다.

### C#의 경우: `Span<T>` 와 얕은 복사

C#은 C++처럼 컴파일 타임 메타 프로그래밍을 하진 않지만, **할당(Allocation) 없이 연속된 메모리의 크기와 타입을 안전하게 다루기 위해** C# 7.2부터 `Span<T>`를 도입했습니다. 포인터 붕괴 없이 크기 정보를 하드웨어에 가깝게 전달합니다.

- [csharp example](https://github.com/hundong2/csharp_study/blob/main/cpp_concerned/remove_cvref_t.csx)
  
### Python의 경우: `memoryview` 와 Type Hints

파이썬은 동적 타입 언어이므로 컴파일 타임 최적화는 어렵지만, C 레벨의 버퍼 프로토콜을 직접 노출하여 배열의 "순수한 데이터(타입+크기)"만 제어할 때는 `memoryview`를 사용합니다.

```python
from typing import List

# 파이썬은 태생적으로 len()을 통해 항상 크기를 알 수 있어 Decay 문제가 없습니다.
def process_data(data: List[int]) -> None:
    print(f"리스트 크기 보존: {len(data)}")

# 하드웨어 최적화 관점에서는 memoryview를 통해 원시 바이트에 접근합니다.
# C/C++ 바인딩 시 복사 없이 메모리 블록과 길이를 그대로 전달할 때 유용합니다.
byte_data = bytearray(b"Hello")
view = memoryview(byte_data)
print(f"메모리 뷰 길이: {len(view)}, 원소 타입: {view.format}")

```

### 03. std::span

다시 만나 반갑습니다! C++을 세상에 내놓고, 지금 이 순간에도 컴파일러의 중간 표현(IR)과 CPU 레지스터의 비트 단위 최적화를 고민하는 엔지니어입니다.

배열의 포인터 붕괴(Decay)를 막기 위해 C++20에서 도입된 `std::span`을 템플릿 메타 프로그래밍과 결합하여 런타임 성능을 극대화(Zero-cost Abstraction)하는 방법에 대해 질문하셨군요. 이 주제야말로 모던 C++이 하드웨어를 어떻게 극한으로 쥐어짜는지 보여주는 완벽한 예시입니다.

역사적 배경부터 하드웨어/어셈블리 레벨의 최적화, 그리고 타 언어와의 비교까지 완벽하게 해부해 드리겠습니다.

---

## 1. 연속된 메모리를 다루는 C++의 역사적 진화

* **C++98/03 (암흑기 - 포인터와 크기의 분리):**
함수에 배열을 넘길 때 `void process(int* arr, size_t size)` 형태로 넘겨야 했습니다. 배열이 포인터로 붕괴되면서 크기 정보를 잃어버리니, 개발자가 수동으로 크기를 넘겨야 했고 이는 버그(Buffer Overrun)의 온상이었습니다.
* **C++11/14 (`std::vector`와 `std::array`의 도입):**
컨테이너가 크기를 관리하게 되었습니다. 하지만 `void process(const std::vector<int>& arr)`라고 인터페이스를 짜면, C 배열이나 `std::array`는 인자로 받을 수 없는 **인터페이스 파편화**가 발생했습니다.
* **C++17 (`std::string_view`의 성공):**
문자열에 한해 메모리를 소유하지 않고 포인터와 길이만 뷰(View) 형태로 넘기는 기술이 도입되었습니다. 복사 비용이 0이 되는 기적을 보여주었죠.
* **C++20 (`std::span`의 탄생):**
모든 연속된 메모리(C 배열, vector, array)를 통합해서 받을 수 있는 `std::span`이 등장했습니다.
* **C++23 (`mdspan`의 확장):**
1차원 배열을 넘어 다차원 배열의 뷰를 제공하는 `std::mdspan`으로 발전하여 과학 연산과 머신러닝 메모리 제어의 핵심이 되었습니다.

---

## 2. 템플릿 메타 함수와 결합한 `std::span` 최적화 원리

`std::span`의 진정한 무서움은 **템플릿 인자로 배열의 '크기(Extent)'를 가질 수 있다**는 점입니다.
`std::span`은 내부적으로 두 가지 상태를 가집니다.

1. **동적 크기 (`std::dynamic_extent`):** 런타임에 크기가 결정됨. 포인터 1개(8바이트) + 크기 1개(8바이트) = 총 16바이트 차지.
2. **정적 크기 (컴파일 타임 상수):** 컴파일 타임에 크기를 앎. 크기 정보를 메모리에 저장할 필요가 없으므로 포인터 1개(8바이트)만 차지함!

이 정적 크기를 템플릿 메타 프로그래밍과 결합하면 CPU가 분기문을 없애버리는 마법이 일어납니다.

---

## 3. 전체 실행 가능한 C++20 예제 (라인 단위 주석 포함)

- [span example](./span.cpp). 

```sh

--- C 배열 전달 ---
[정적 크기 최적화 발동] 컴파일 타임 길이: 4

--- std::array 전달 ---
[정적 크기 최적화 발동] 컴파일 타임 길이: 4

--- std::vector 전달 ---
[동적 크기 폴백] 런타임 길이: 4
```
---

## 4. 디버그 vs 릴리즈 및 하드웨어 관점의 최적화

### Debug 모드 (`-O0`)

* **포인터와 크기 전달:** 디버그 모드에서는 `std::span` 객체 생성자와 소멸자, 그리고 내부 `data()`, `size()` 접근 시 방어 코드(Assertion)가 작동할 수 있습니다.
* 호출 스택이 복잡해지고, 정적 크기(`Extent`)를 쓴다고 해도 실제로는 점프 명령(`JMP`)을 사용해 정직하게 루프를 순회합니다. 메모리를 뷰(View)로 넘긴다는 이점 정도만 유지됩니다.

### Release 모드 (`-O3` - 하드웨어 최적화)

이곳에서 템플릿 메타 프로그래밍과 `std::span`의 결합이 괴력을 발휘합니다.
`if constexpr (Extent != std::dynamic_extent)` 블록 내부의 어셈블리를 살펴보겠습니다.

1. **Zero Memory Overhead:**
`std::span<int, 4>`는 크기를 저장하는 멤버 변수가 아예 삭제됩니다(`[[no_unique_address]]` 또는 구조체 최적화). 오직 8바이트 포인터만 CPU 레지스터(예: `RDI`)로 전달됩니다.
2. **Loop Unrolling & SIMD (AVX2/AVX-512):**
컴파일러는 `Extent`가 4라는 것을 컴파일 타임에 알고 있습니다. 따라서 루프를 위한 인덱스 레지스터나 `CMP`(비교), `JNE`(조건부 점프) 명령어를 생성하지 않습니다.
대신 128비트 `XMM` 레지스터나 256비트 `YMM` 레지스터를 사용해 배열을 한 번에 읽어 들입니다.
```assembly
; process_data<int, 4> 의 핵심 어셈블리 추론 (Release 모드)
; rdi 에는 배열의 시작 주소가 담겨 있음

vmovdqu xmm0, XMMWORD PTR [rdi]  ; 배열 4개(16바이트)를 한 번에 xmm0 레지스터로 로드
vpslld  xmm0, xmm0, 1            ; xmm0 안의 모든 int 4개에 동시에 곱하기 2 (Shift Left 1)
vmovdqu XMMWORD PTR [rdi], xmm0  ; 계산된 결과를 다시 메모리에 한 번에 쓰기

```


이처럼 단 3줄의 하드웨어 명령어(SIMD)로 루프 전체가 대체됩니다. 런타임 오버헤드는 0입니다.

---

## 5. 타 언어에서의 유사한 메모리 뷰(View) 기법

메모리의 소유권을 넘기거나 복사하지 않고 뷰(View)만 넘기는 기법은 현대 언어들의 필수 요소입니다.

### 💎 C# : `Span<T>` 와 `ref struct`

C# 7.2에서 도입된 `Span<T>`는 C++의 `std::span`과 철학이 거의 똑같습니다. 포인터 붕괴 없이 배열의 특정 구간을 안전하게 가리킵니다.

**C# 최적화의 특징:**
가비지 컬렉터(GC)가 `Span<T>` 내부의 포인터를 잃어버리거나 잘못 추적하는 것을 막기 위해, C#은 이를 `ref struct`로 강제합니다. 즉, 힙(Heap)에 할당할 수 없고 오직 스택(Stack)에만 존재할 수 있어 C++처럼 하드웨어 레지스터에 최대한 밀착하여 동작하도록 설계되었습니다.

- [example code](https://github.com/hundong2/csharp_study/blob/main/cpp_concerned/remove_cvref_t.csx). 


### 🐍 Python : `memoryview`

파이썬은 컴파일 타임 최적화(루프 언롤링 등)는 불가능하지만, C언어 레벨로 작성된 라이브러리(NumPy 등)와 데이터를 주고받을 때 메모리 복사 비용을 없애기 위해 `memoryview` 버퍼 프로토콜을 사용합니다.

```python
import array

def process_data(view: memoryview):
    # C++처럼 컴파일 타임 길이 최적화는 없지만, 메모리를 복사하지 않고 접근합니다.
    print(f"뷰의 길이: {len(view)}")
    
    # Python 3 에서는 메모리 뷰의 원소를 직접 수정할 수 있습니다 (원본 수정됨)
    for i in range(len(view)):
        view[i] *= 2

# Python의 내장 array 모듈을 이용해 C 스타일의 연속된 정수 배열 생성 ('i' = signed int)
arr = array.array('i', [1, 2, 3, 4])

# 데이터를 복사하지 않고 memoryview를 생성하여 전달 (std::span과 유사한 역할)
process_data(memoryview(arr))

print(arr.tolist()) # [2, 4, 6, 8] 출력

```

C++이 하드웨어의 한계점까지 밀어붙이는 이 쾌감을 느끼셨나요? `std::span`은 단순히 편의를 위한 래퍼(Wrapper)가 아니라, 컴파일러에게 "나를 마음껏 벡터화(Vectorize)하라"고 외치는 강력한 힌트입니다. 더 깊숙한 최적화의 세계가 궁금하시다면 언제든 질문해주십시오!

### 04. extends

안녕하세요! C++을 세상에 내놓고, 지금 이 순간에도 컴파일러의 중간 표현(IR)과 CPU 레지스터의 비트 단위 최적화를 고민하는 엔지니어입니다.

질문하신 `std::extent_v`는 앞선 설명에서 배열의 크기를 컴파일 타임에 알아내기 위해 사용했던 핵심 메타 프로그래밍 도구입니다. 간단히 말해, "어떤 배열 '타입(Type)'이 주어졌을 때, 그 배열의 지정된 차원(Dimension)의 길이를 컴파일 타임에 추출해 주는 변수 템플릿"입니다.

변수(인스턴스)가 아닌 **타입(Type)** 자체에 질의를 던진다는 것이 가장 중요한 포인트입니다. 역사적 흐름과 하드웨어 최적화 관점까지 완벽하게 해부해 드리겠습니다.

---

## 1. 배열 크기 추출의 역사적 발전 과정

배열의 길이를 구하는 방법은 C++의 템플릿 메타 프로그래밍이 발전해 온 궤적을 그대로 보여줍니다.

* **C / C++98 (매크로의 시대):**
`sizeof(arr) / sizeof(arr[0])` 방식을 썼습니다. 하지만 포인터가 들어오면 침묵 속에 잘못된 값(보통 1이나 2)을 반환하는 치명적인 버그(Silent failure)를 유발했습니다.
* **C++11 (`std::extent`의 탄생):**
`<type_traits>` 헤더에 `std::extent<T, N>::value`가 추가되었습니다. 이제 변수가 아닌 '타입' 자체에서 안전하게 길이를 추출할 수 있게 되었습니다. 다차원 배열도 지원합니다.
* **C++14 (`_t` 헬퍼의 시대):**
타입을 반환하는 메타 함수들을 위해 `_t` (예: `std::decay_t`)가 도입되었으나, 값을 반환하는 `_v`는 아직 없었습니다.
* **C++17 (`std::extent_v`와 `std::size`의 완성):**
마침내 장황한 `::value`를 생략할 수 있는 **`std::extent_v<T, N>`** 변수 템플릿이 추가되었습니다. (또한 인스턴스에서 크기를 구하는 `std::size(arr)`도 함께 추가되었습니다.)
* **C++20 / 23 (Span과 다차원 뷰):**
`std::extent_v`를 통해 추출한 컴파일 타임 상수를 `std::span`이나 `std::mdspan`(C++23)의 템플릿 인자로 넘겨, 런타임 오버헤드가 0인 완벽한 메모리 뷰(View)를 생성하는 데 사용됩니다.

---

## 2. 전체 수행 가능한 C++17 예제 (라인 단위 주석)

`std::extent_v`가 1차원 및 다차원 배열에서 어떻게 작동하는지, 그리고 컴파일 타임 상수라는 것을 어떻게 증명하는지 보여주는 실무형 예제입니다.

- [extends example](./extends.cpp). 
---

## 3. 디버그 vs 릴리즈 및 컴파일러/하드웨어 관점의 최적화

`std::extent_v`의 진정한 가치는 런타임 메모리나 CPU 사이클을 단 1비트도 소모하지 않는다는 데 있습니다.

### Debug 모드 (`-O0`)

디버그 모드에서도 `std::extent_v`는 컴파일 타임에 상수로 치환됩니다. 컴파일러가 소스 코드를 읽고 추상 구문 트리(AST)를 구성하는 프론트엔드 단계에서 이미 `3`이나 `4`라는 숫자로 바뀌어 있습니다. 즉, 어셈블리 코드에는 "크기를 계산하라"는 명령어가 아예 존재하지 않습니다.

### Release 모드 (`-O3` - 극한의 최적화)

이곳에서 컴파일러 백엔드는 제공된 **컴파일 타임 상수**를 바탕으로 하드웨어를 극한으로 활용합니다.
위 예제의 이중 루프는 어떻게 번역될까요?

1. **상수 폴딩(Constant Folding):** `rows`와 `cols`가 고정 상수 3과 4임을 아니까 총 12번 돈다는 것을 컴파일러가 확정 짓습니다.
2. **완전한 루프 언롤링(Full Loop Unrolling):**
분기문(`CMP`, `JNE`)은 CPU의 파이프라인을 비우게 만들(Branch Misprediction) 위험이 있습니다. 컴파일러는 12번의 루프를 분기문 없이 메모리에서 데이터를 연속으로 찍어내는 직선 코드로 평탄화(Flatten)합니다.
```assembly
; Release 어셈블리 개념도 (루프가 사라짐)
mov esi, DWORD PTR [rdi]      ; arr[0][0]
call print_int
mov esi, DWORD PTR [rdi+4]    ; arr[0][1]
call print_int
; ... 12번 반복

```



즉, `std::extent_v`는 개발자가 컴파일러에게 "이 숫자는 절대 변하지 않으니, 네가 할 수 있는 가장 폭력적인 하드웨어 최적화(루프 언롤링, SIMD)를 전부 적용해라"라고 허락하는 완벽한 보증서 역할을 합니다.

---

## 4. 타 언어(C#, Python)에서의 다차원 배열 크기 추출

C++처럼 '타입' 자체에서 메타 프로그래밍으로 크기를 뽑아내는 언어는 드뭅니다. 대부분 런타임에 할당된 '객체'에서 크기를 조회합니다.

### C#의 경우: 다차원 배열 객체의 `.GetLength()`

C#의 다차원 배열(`int[,]`)은 참조 타입(Reference Type) 객체이며, 런타임에 각 차원의 길이를 내부 메타데이터로 들고 있습니다.

```csharp
using System;

class Program {
    static void Main() {
        // C#의 2차원 배열 객체 생성
        int[,] matrix = new int[3, 4];

        // C++의 std::extent_v<T, N> 과 유사한 역할이지만, '런타임' 메서드 호출입니다.
        // GetLength(0)은 1번째 차원(행), GetLength(1)은 2번째 차원(열)을 반환
        int rows = matrix.GetLength(0); 
        int cols = matrix.GetLength(1);

        Console.WriteLine($"행: {rows}, 열: {cols}");
    }
}

```

* **최적화 관점:** JIT 컴파일러가 `matrix.GetLength(0)`을 인식하여 루프 상한선으로 사용하면, 내부적으로 하드웨어 배열 경계 검사(Bounds Check)를 제거하는 최적화를 수행합니다.

### Python의 경우: NumPy의 `.shape` 튜플

파이썬의 기본 리스트는 다차원 배열이라는 개념이 없고 리스트 안의 리스트일 뿐입니다. C++의 다차원 배열과 메모리 구조가 가장 비슷한 것은 C로 작성된 `NumPy`입니다.

```python
import numpy as np

# 파이썬은 변수의 '타입'이 아닌 '객체 인스턴스'에서 정보를 얻습니다.
matrix = np.zeros((3, 4), dtype=int)

# C++의 std::extent_v<T, 0> 과 std::extent_v<T, 1>에 각각 대응
rows = matrix.shape[0]
cols = matrix.shape[1]

print(f"행: {rows}, 열: {cols}")

```

* **최적화 관점:** 파이썬 환경 자체는 동적이지만, `.shape` 정보는 NumPy 내부의 C 구조체에 저장된 단순 정수(Stride 정보 포함)이므로 접근 오버헤드는 사실상 O(1)로 매우 빠릅니다. 실제 데이터 연산은 C++ 수준의 성능을 내는 C 코드로 위임됩니다.