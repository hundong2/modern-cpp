# C++23

- [01. expected](#1-expected). 
- [02. monadic](#02-monadic). 

## 빌드 및 실행

`자주까먹는` 폴더에서 실행할 `.cpp` 경로를 `run.sh`에 전달합니다. 각 파일에는
독립적인 `main()` 함수가 있어야 합니다.

```bash
cd 자주까먹는
./run.sh cpp23/main.cpp
./run.sh cpp23/expected.cpp
```

새 `.cpp` 파일을 이 폴더에 추가하면 CMake가 파일 이름과 같은 실행 타깃을 자동으로
만듭니다. 예를 들어 `optional.cpp`는 `optional` 타깃과
`build/cpp23/optional` 실행 파일을 만듭니다.

## 1. expected

C++23에 도입된 `std::expected`는 기존 `try-catch` 예외 처리의 가장 큰 단점(스택 언와인딩으로 인한 심각한 런타임 성능 저하)을 해결하기 위해 등장한 혁신적인 도구입니다.

이것은 다른 최신 언어들(예: Rust의 `Result` 타입)에서 크게 성공한 "성공한 값(Value) 아니면 에러(Error) 중 하나를 무조건 반환한다"는 개념을 C++에 가져온 것입니다.

에러가 발생할 때 무거운 예외를 던지는(`throw`) 대신, 단순히 값으로 에러를 반환하기 때문에 런타임 오버헤드가 사실상 0에 가깝습니다.

---

## 💡 `std::expected` 핵심 원리

* **`std::expected<T, E>`**: 함수가 정상적으로 끝나면 `T` 타입을 반환하고, 실패하면 `E` 타입(보통 에러 코드)을 반환하겠다는 명시적인 선언입니다.
* **성공 시**: 그냥 `T` 값을 `return` 하면 됩니다.
* **실패 시**: 에러 값 `E`를 `std::unexpected(E)`로 감싸서 반환합니다.

## 🚀 완전한 실행 예제: 오버헤드 없는 런타임 에러 처리

이전의 네트워크 패킷 라우팅 예제를 `try-catch` 대신 `std::expected`를 사용하는 구조 사용

- [expected example code](./expected.cpp). 

## 🏆 실무에서 `std::expected`가 `try-catch`보다 우아한 이유 3가지

1. **압도적인 성능 향상 (Zero-overhead):** C++의 `throw`는 호출 스택을 거슬러 올라가는 복잡한 작업(Stack Unwinding)을 동반하여 성능을 심각하게 떨어뜨립니다. `std::expected`는 내부적으로 단순히 `union`(공용체)과 `bool` 플래그를 사용하는 가벼운 객체이므로 런타임 비용이 일반 함수 반환과 동일합니다.
2. **명시적인 인터페이스:** `void route_message()`라고만 적혀 있으면 이 함수가 무슨 예외를 던질지 문서를 뒤져보거나 코드를 뜯어봐야 합니다. 하지만 `std::expected<std::string, RouteError>`를 보면 **"아, 이 함수는 실패할 수 있고 실패하면 `RouteError`가 나오는구나"** 하고 단번에 파악할 수 있습니다.
3. **제어 흐름의 단절 방지:** 예외(Exception)는 정상적인 코드 흐름을 갑자기 끊어버리는 마치 `goto` 문과 같습니다. `std::expected`를 사용하면 에러 처리도 일반적인 값의 처리처럼 순차적이고 논리적인 흐름(Control Flow)을 유지할 수 있습니다. 


## 02 .monadic 

C++23의 `std::expected`가 제공하는 모나드(Monadic) 연산인 `and_then`과 `transform`을 사용하면, 에러 처리를 위한 지루한 `if`문 도배를 없애고 **데이터가 파이프라인을 타고 흐르듯(Railway Oriented Programming)** 우아하게 코드를 작성할 수 있습니다.

핵심 원리는 다음과 같습니다.

* **`and_then`**: 다음 작업도 **실패할 가능성이 있을 때** (즉, 반환형이 또 `std::expected`일 때) 사용합니다.
* **`transform`**: 다음 작업은 **무조건 성공할 때** (즉, 반환형이 일반 값일 때) 사용합니다.

파이프라인 중간에 에러가 발생하면, 이후의 연산들은 자동으로 실행을 건너뛰고 에러 값을 최종 목적지까지 전달합니다.

---

### 🚀 완전한 실행 예제: 유저 데이터 처리 파이프라인

문자열로 된 '유저 ID'를 입력받아, 숫자로 변환하고, 데이터베이스에서 유저를 조회한 뒤, 환영 메시지를 만드는 3단계 과정을 체이닝으로 구현한 예제

- [monadic code example](./monadic.cpp). 

모나드 연산을 사용하면 에러가 발생하는 순간 파이프라인의 나머지 함수(`fetch_user_from_db`, `create_greeting`)는 호출되지 않고 즉시 통과(Bypass)됩니다. 따라서 개발자는 "성공했을 때의 논리적 흐름"에만 온전히 집중하여 코드를 설계할 수 있습니다.