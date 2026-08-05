# 초보자 관점 재검토와 막힘 해결표

이 문서는 본문/코드를 완성한 뒤 “C++을 처음 보는 사람이 어느 지점에서 문장을 읽지 못하는가?”를 기준으로 다시 검토해 보완한 안내다. 모르는 코드를 만났을 때 문법을 전부 외우는 대신 아래 순서대로 의미를 복원한다.

## 가장 먼저 막히는 12가지

| 보이는 것 | 초보자가 하기 쉬운 오해 | 정확한 읽기 | 더 읽기 |
|---|---|---|---|
| `#include <x>` | 파일 내용을 실행한다 | 선언을 현재 번역 단위에 포함하도록 [전처리](wiki/build-pipeline.md)한다 | [빌드 파이프라인](wiki/build-pipeline.md) |
| `std::` | 특별한 연산자다 | 표준 라이브러리 이름이 들어 있는 namespace 범위 지정이다 | [C++ reference의 namespace](https://en.cppreference.com/w/cpp/language/namespace) |
| `T value` | 변수 이름만 중요하다 | `T`가 저장 크기·연산·수명 규칙을 정하고 `value`는 객체 이름이다 | [메모리와 수명](wiki/memory.md) |
| `T*` | 무조건 heap 객체다 | `T` 주소를 담는 타입일 뿐 소유권/저장 위치를 말하지 않는다 | [RAII](wiki/raii.md) |
| `T&` | 주소 연산이다 | 선언에서는 별명(reference), 식의 unary `&x`는 주소 얻기다 | [값 범주](wiki/value-categories.md) |
| `*p` | 포인터 선언이다 | 선언의 `T* p`와 식의 `*p`(가리킨 객체 접근)는 문맥이 다르다 | [값 범주](wiki/value-categories.md) |
| `auto` | 동적 타입이다 | 컴파일 때 초기화 식에서 정적 타입을 추론한다 | [타입 추론](https://en.cppreference.com/w/cpp/language/auto) |
| `{}` | 모두 같은 블록이다 | 스코프, 함수 본문, aggregate 초기화, initializer list 등 문맥에 따라 다르다 | [초기화](https://en.cppreference.com/w/cpp/language/initialization) |
| `const` | 값이 영원히 불변이다 | 해당 이름/경로를 통한 수정을 제한한다. 다른 alias가 바꿀 수도 있다 | [const](https://en.cppreference.com/w/cpp/language/cv) |
| `std::move` | 즉시 메모리를 이동한다 | [xvalue cast](wiki/value-categories.md)일 뿐 실제 연산은 선택된 함수가 한다 | [값 범주](wiki/value-categories.md) |
| `read()` | 메시지 하나를 읽는다 | 현재 준비된 TCP byte 일부를 읽는다 | [TCP 스트림](wiki/tcp-stream.md) |
| `shared_ptr` | thread-safe 객체다 | 제어 블록 수명만 안전하게 공유하며 객체 데이터는 별도 동기화가 필요하다 | [RAII](wiki/raii.md), [동시성](wiki/concurrency.md) |

## 컴파일 오류와 실행 오류를 분리한다

1. **configure 오류**: CMake가 compiler/library/generator를 못 찾는다. 소스 문법을 보기 전 도구chain 경로를 고친다.
2. **compile 오류**: 한 `.cpp`를 번역하는 중 타입/문법/선언이 맞지 않는다. 첫 오류부터 읽는다.
3. **link 오류**: 선언은 봤지만 함수 정의/library symbol을 결합하지 못한다. target에 source/library가 포함됐는지 본다.
4. **runtime 오류**: 실행 후 crash, hang, 잘못된 값이 생긴다. 수명, 범위, 시스템 호출 반환값, data race를 본다.
5. **logic 오류**: 프로그램은 정상 종료하지만 요구사항과 다르다. 입력→상태 변화→출력 불변식을 테스트한다.

## 이름 하나를 읽는 7단계

예를 들어 `std::map<int, std::shared_ptr<Session>> registry;`를 만나면 다음처럼 푼다.

1. 가장 바깥 타입은 `std::map<K,V>`다.
2. key `K`는 `int`, value `V`는 `std::shared_ptr<Session>`이다.
3. 변수 이름은 `registry`다.
4. map은 value인 shared pointer 객체들을 직접 소유한다.
5. 각 shared pointer는 별도 제어 블록을 통해 Session을 공동 소유한다.
6. `registry.erase(key)`는 map 원소를 파괴해 강한 참조 수를 하나 줄인다.
7. 다른 강한 참조가 없다면 그 순간 Session이 파괴된다.

이 방식은 중첩 template, callback, coroutine 반환 타입도 **바깥에서 안쪽으로** 읽게 해 준다.

## 동기·비동기·blocking·병렬은 서로 다른 축이다

- **동기(synchronous)**: 호출과 완료 관찰의 순서가 직접 결합되어 있다.
- **비동기(asynchronous)**: 시작과 완료 통지가 분리되어 callback/future/coroutine 등으로 나중에 온다.
- **blocking**: 현재 OS thread가 진행 조건을 기다리며 잠든다.
- **non-blocking**: 지금 처리할 수 있는 만큼만 하고 즉시 결과/`EAGAIN`을 돌려준다.
- **동시성(concurrency)**: 여러 작업의 진행 기간이 겹친다.
- **병렬성(parallelism)**: 실제 같은 순간에 여러 CPU 실행 자원을 쓴다.

비동기 코드는 single thread에서도 가능하고, 여러 thread를 써도 blocking I/O일 수 있다. [이벤트 루프](wiki/event-loop.md)와 [프로세스/스레드](wiki/process-thread.md)를 함께 본다.

## 네트워크 코드를 읽을 때 추가하는 질문

- 이 fd/socket의 소유자는 누구이며 모든 실패 경로에서 닫히는가?
- TCP 수신 byte가 0개, 1개, 헤더 일부, 여러 frame일 때 모두 동작하는가?
- peer가 길이를 거짓말하면 최대 메모리는 얼마인가?
- `send`가 일부만 성공하면 나머지는 누가 보관하는가?
- 느린 client/DB 때문에 queue가 계속 커질 때 상한과 거부 정책은 무엇인가?
- callback 완료 전에 session이 파괴될 수 있는가?

## 동시성 코드를 읽을 때 추가하는 질문

- 어떤 필드를 어느 mutex가 보호하는가?
- lock을 잡은 채 사용자 callback, I/O, 긴 작업을 호출하는가?
- 종료 flag와 queue를 같은 동기화 규칙으로 보는가?
- object 파괴와 worker/callback 완료 사이 happens-before가 있는가?
- atomic 하나로 여러 필드의 불변식까지 보호된다고 착각하지 않았는가?

## 원문보다 보강한 난제

- `detach()`를 단순 수명 분리 해법으로 쓰지 않고 join/stop 책임을 설명했다.
- `shared_ptr` 참조 수와 객체 내부 thread safety를 분리했다.
- TCP 부분 read/write, framing 상한, backpressure를 추가했다.
- `epoll = O(1)`, 이동 생성자 = OS zero-copy 같은 과도한 단순화를 정정했다.
- thread pool에 종료 순서, queue 상한, 예외 관찰 문제를 추가했다.
- pool custom deleter가 죽은 pool을 가리키는 수명 오류를 안전한 shared state 모형으로 바꿨다.
- lock-free stack을 그대로 배포할 수 없도록 ABA와 memory reclamation 경고를 코드 구조에 반영했다.
- 단순 lerp 대신 시간 단위에 더 일관된 지수 보정을 추가했다.

## 최종 자가 점검

코드를 보며 다음 문장을 말로 완성할 수 있으면 다음 단계로 넘어간다.

> 이 객체의 타입은 ___이고 소유자는 ___이며 ___ 시점에 파괴된다. 이 함수에는 ___ 방식으로 전달되어 복사/이동 비용은 ___이다. 실패하면 ___ 경로로 가며 자원은 ___가 정리한다. 여러 thread가 접근할 때 ___가 ___ 불변식을 보호한다. 네트워크 byte는 ___ 규칙으로 frame이 되고 크기 상한은 ___이다.

빈칸을 채울 수 없으면 해당 줄의 위키 링크로 돌아가 작은 예제를 실행한다. “모든 C++ 코드를 한눈에 이해”하는 능력은 문법 암기보다 이 질문을 반복해서 적용하는 데서 생긴다.
