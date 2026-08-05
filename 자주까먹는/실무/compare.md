# C++ / C# / Python 사고방식 비교

| 주제 | C++ | C# | Python | C++ 초보자의 주의점 |
|---|---|---|---|---|
| 실행 | 네이티브 컴파일·링크 | IL/JIT·런타임 | 인터프리터/바이트코드 | ABI와 링크 오류가 별도 단계에서 발생한다. |
| 객체 수명 | 스코프 기반 결정적 파괴, [RAII](wiki/raii.md) | GC + `IDisposable` | 참조 계수/GC + context manager | 포인터 도달 가능성과 소유권은 같은 말이 아니다. |
| 값/참조 | 값이 기본, `&`/`*` 명시 | class reference, struct value | 이름이 객체를 참조 | C++ 대입은 실제 복사/이동을 호출할 수 있다. |
| generic | compile-time template | runtime type system generic | duck typing/type hint | template 오류는 인스턴스화 문맥에서 길게 나온다. |
| 예외 | RAII로 정리, 예외 비활성 빌드도 존재 | `using`/`finally` | `with`/`finally` | 소멸자는 예외를 던지지 않게 한다. |
| 동시성 | C++ memory model, data race=UB | CLR memory model | 구현에 따라 GIL | `shared_ptr`가 객체 내부 race를 막지 않는다. |
| async | callback/future/coroutine/library executor | `Task`, `async/await` | event loop, coroutine | coroutine이 곧 새 thread는 아니다. |

Python/C#에서 “나중에 GC가 치운다”는 감각을 C++ raw resource에 옮기면 leak/use-after-free가 생긴다. 반대로 모든 것을 `shared_ptr`로 바꾸면 순환 참조와 원자 참조 수 비용이 생긴다. 먼저 단독 소유자와 관찰자를 정하고 타입으로 표현한다.
