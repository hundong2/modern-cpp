# 2026-10-21 학습 체크포인트

이 문서는 정답을 외우는 목록이 아니다. 각 문항에 **소리 내어 근거를 설명하고**, 표시된 식을 직접
수정·컴파일·실행해야 통과한다. 답할 때는 타입, 값 범주, 소유자, 유효 수명, 호출 뒤 상태를 함께 말한다.

## 1. C++ 기초 문법 확인

1. `#include <memory>`와 `#include <functional>`이 각각 선언하는 오늘의 핵심 타입을 두 개씩 말하라.
2. `using Callback = std::function<bool(int)>;`가 새 class를 만드는지, 기존 타입에 별칭만 붙이는지 설명하라.
3. `bool`과 `int`의 역할이 무엇이며 `int minimum_code_{};`가 어떤 값으로 초기화되는지 말하라.
4. `EventGate gate{...};` 같은 중괄호 초기화가 괄호 초기화보다 narrowing을 더 엄격히 막는 예를 만들어라.
5. 함수 선언에서 반환형, 함수 이름, 매개변수 목록, 뒤의 `const`를 각각 가리켜라.
6. 값 매개변수 `int minimum_code`와 `const int& minimum_code`의 복사·수명·null 가능성을 비교하라.
7. pointer와 reference의 null 표현 가능성, 재바인딩, 대상 수명 비소유라는 공통점을 설명하라.
8. `struct`와 `class`의 기능은 같지만 기본 멤버/상속 접근이 각각 무엇인지 말하라.
9. `public`, `private`가 호출자에게 허용하는 접근 범위를 설명하라.
10. `explicit EventGate(FactoryKey, int)`가 어떤 copy-list 초기화를 막는지 작은 반례로 보이라.
11. base와 `: minimum_code_{minimum_code}`가 생성자 본문보다 먼저 실행되는 이유와 실제 초기화 순서를 말하라.
12. `std::enable_shared_from_this<EventGate>`의 `EventGate`가 template 인자로 하는 일을 설명하라.
13. `const std::shared_ptr<EventGate> pinned{observer.lock()}`와 `if (!pinned)`의 초기화, 조건, 두 경로를 나눠 읽어라.
14. `return false;`가 제어 흐름과 반환 객체 수명에 미치는 영향을 설명하라.

## 2. `main.cpp` 식을 실제로 읽기

다음 식마다 정적 타입, 값 범주, 소유권, 호출 뒤 상태를 한 줄씩 적는다.

1. `10`
2. `EventGate::create(10)`
3. 이름 있는 `owner`
4. 생성자 안의 이름 있는 매개변수 `minimum_code`
5. `std::move(observer)`
6. `weak_from_this()`의 반환 prvalue
7. 람다 init-capture에 저장된 `observer`
8. `observer.lock()`의 반환 prvalue
9. `const std::shared_ptr<EventGate> pinned{observer.lock()}` 뒤의 `pinned`
10. `pinned->accepts(event_code)`에서 `operator->`로 빌린 대상
11. 이름 있는 `callback`
12. `callback(12)`의 `12`와 반환 `bool`
13. `owner.reset()` 직전과 직후의 강한 참조 수 및 `EventGate` 객체 상태
14. 마지막 owner가 사라진 뒤에도 callback 안의 weak handle이 관찰할 수 있는 것

아래 실행 순서를 객체 그림으로 그린다.

```text
shared owner ──strong──> EventGate
callback ──weak──> control block ──observes──> EventGate
```

1. 첫 callback 실행 중 `lock()`이 만든 지역 `shared_ptr`가 왜 함수가 끝날 때까지 대상을 살리는가?
2. 첫 callback이 끝난 뒤 지역 strong owner가 사라져도 바깥 `owner`가 있으면 왜 대상이 사는가?
3. `owner.reset()` 뒤 두 번째 `lock()`은 어떤 값을 돌려주는가?
4. weak handle이 control block 수명은 연장하지만 `EventGate` 객체 수명은 연장하지 않는다는 말을 설명하라.
5. callback이 `shared_ptr`를 직접 capture했다면 출력과 객체 소멸 시점이 어떻게 달라지는가?
6. callback이 raw `[this]`를 capture했다면 owner 소멸 뒤 호출이 왜 미정의 동작이 될 수 있는가?

## 3. 표준 라이브러리 호출 계약 말하기

각 호출에 대해 다음 여섯 칸을 빠짐없이 채운다.

1. 정확한 수신 객체 타입과 호출 전 상태
2. 선택된 시그니처/오버로드/template 인자
3. 모든 인자 식의 타입·값 범주·소유권·허용값
4. 반환형·값의 의미·실제 사용 여부
5. 호출 뒤 수신 객체와 인자의 상태 변화
6. 전제/후조건, 복잡도, 할당, 무효화, 수명, 오류/예외, thread 보장

설명 대상:

1. `std::make_shared<EventGate>(FactoryKey{}, minimum_code)`
2. `std::move(observer)`
3. `weak_from_this()`
4. `observer.lock()`
5. 람다로부터 `std::function<bool(int)>`을 만드는 생성자
6. `callback(12)`의 `std::function::operator()`
7. `owner.reset()`
8. `std::enable_shared_from_this<EventGate>` base 기본 생성
9. `std::cout << value`
10. `operator<<(std::ostream&, char)`로 출력하는 `'\n'`
11. ICPC 코드의 `std::vector` fill 생성자
12. `cost[employee][task]`의 두 `vector::operator[]`
13. `std::cin >> value`와 실패 뒤 stream 상태
14. `std::ios::sync_with_stdio(false)`와 첫 I/O 이전이라는 사용 조건
15. `std::cin.tie(nullptr)`가 반환하는 이전 연결 포인터와 그 반환값을 버린다는 사실

추가 확인:

- 빈 `std::function`을 호출하는 것과 만료된 `weak_ptr::lock()` 결과를 검사하는 것은 왜 서로 다른 오류 계약인가?
- 서로 다른 `shared_ptr` 객체의 참조 횟수 연산이 안전하다는 사실이 `EventGate`의 `int` 멤버 동시 접근까지 안전하게 만드는가?
- `weak_from_this()`를 생성자 안에서 호출하면 아직 shared owner와 연결되지 않아 어떤 결과가 가능한가?
- `shared_ptr<EventGate>{this}`를 새로 만드는 것이 두 control block과 이중 삭제를 만들 수 있는 이유는 무엇인가?

## 4. 값 범주·참조 바인딩·복사와 이동

1. 이름 있는 `std::weak_ptr<EventGate>&&` 매개변수도 식으로 쓰면 왜 lvalue인가?
2. `std::move`가 control block 관찰 권한을 직접 옮기지 않는다는 말을 생성자 선택과 연결해 설명하라.
3. weak_ptr lvalue를 capture할 때와 `std::move(observer)` xvalue를 init-capture할 때의 복사/이동을 비교하라.
4. `weak_from_this()` 결과 prvalue가 지역 `observer`를 직접 초기화하고, 이어 `std::move(observer)` xvalue가
   callback의 init-capture 멤버를 이동 생성하는 두 단계를 구분해 설명하라.
5. `lock()` 결과 prvalue가 지역 `shared_ptr`를 직접 초기화할 때 불필요한 임시 복사가 왜 필요하지 않은가?
6. factory가 `return std::make_shared<EventGate>(...);`로 돌려주는 prvalue와 이름 있는 local의 NRVO를 구분하라.
7. `const auto pinned`가 pointee까지 `const EventGate`로 만드는지 답하고 이유를 말하라.
8. callback의 복사본을 만들면 weak count와 강한 count가 각각 어떻게 달라지는가?
9. `std::function` 이동 뒤 원본은 어떤 상태일 수 있으며 왜 원본을 바로 호출하면 안 되는가?

## 5. 객체 수명과 실무 아키텍처

1. EventGate가 자신을 직접 소유하지 않으면서 자기 내부에서 안전한 지연 작업을 만드는 이유를 설명하라.
2. `EventGate direct{{}, 1};`과 `EventGate copied{*owner};`가 각각 컴파일에 실패하는 이유를 구분하라.
   private passkey 기본 생성자와 삭제한 `EventGate` 복사·이동 특수 멤버가 factory/shared-owner 불변식을
   어떻게 함께 지키는가?
3. 공개(public)이며 비모호한 `enable_shared_from_this` 기반이 필요한 이유를 조사해 설명하라.
4. scheduler가 callback을 EventGate보다 오래 보관해도 안전한 이유와, 작업이 조용히 취소되는 의미를 말하라.
5. 반드시 실행되어야 하는 작업이라면 weak capture 대신 어떤 명시적 소유/취소 정책이 필요한가?
6. callback이 다른 thread에서 실행되고 EventGate에 가변 상태를 추가할 때 데이터 경쟁을 막을 설계를 제안하라.
7. weak capture는 순환 소유권을 막지만 논리적 순서, 재시도, 중복 실행까지 해결하는가?
8. API가 `bool`만 반환할 때 “owner 만료”와 “도메인 거절”을 구분해야 한다면 어떤 어휘 타입이 적합한가?

## 6. 기계 실행 관점

다음을 특정 어셈블리 명령으로 단정하지 않고 설명한다.

1. `weak_ptr::lock()`이 control block 상태를 읽고 조건부로 strong count를 늘릴 때 필요한 원자적 조정의 목적
2. 빈/non-empty `shared_ptr` 검사에서 가능한 load·compare·conditional branch
3. `std::function::operator()`에서 타입 소거된 호출 대상에 도달하는 간접 호출 가능성
4. `accepts`에서 두 `int` load와 비교가 나타날 수 있지만 최적화가 이를 바꿀 수 있는 이유
5. Hungarian 내부의 연속 `vector` 원소 load/store와 최솟값 비교·조건 분기
6. 컴파일러, 표준 라이브러리 구현, ABI, 최적화 옵션, CPU가 실제 명령·할당·인라인 여부를 바꾸는 이유

## 7. Hungarian 알고리즘 손 추적

다음 비용 행렬에서 최소 완전 배정을 손으로 구한다.

```text
       작업1 작업2 작업3
직원1    8     2     5
직원2    3     7     6
직원3    4     1     9
```

1. 각 직원과 작업이 정확히 하나씩 선택되는 배정을 세 개 이상 열거하라.
2. 1-based sentinel 열 `0`이 실제 작업이 아닌 이유를 설명하라.
3. `p[j]`가 작업 `j`에 현재 매칭된 직원이라는 표현을 그림으로 그려라.
4. 새 직원 `i`를 `p[0]=i`에 놓고 증강을 시작하는 이유를 말하라.
5. `used[j]`가 현재 교대 트리에 포함된 작업 집합을 나타내는 불변식을 설명하라.
6. `minv[j]`가 tree 안 직원들에서 아직 쓰지 않은 작업 `j`로 가는 최소 감소 비용인 이유를 보이라.
7. `way[j]`가 최종 빈 작업에서 역추적할 predecessor 작업을 저장하는 방식을 손으로 따라가라.
8. `delta`만큼 잠재치를 바꾼 뒤 감소 비용이 음수가 되지 않는 이유를 설명하라.
9. 하나 이상의 새 slack이 0이 되어 교대 트리가 전진하는 이유를 말하라.
10. `p[j0]==0`이 되면 augmenting path가 존재한다는 의미를 설명하라.
11. 역추적으로 매칭 크기가 정확히 1 증가하고 기존 직원이 사라지지 않는 이유를 증명하라.
12. 바깥 반복 `i`가 끝날 때 처음 `i`명의 직원이 서로 다른 작업과 매칭된다는 귀납 불변식을 적어라.

## 8. 정확성·복잡도·경계

1. feasible potential과 0 감소 비용 간선만으로 만든 교대 경로가 최적성을 보존하는 근거를 설명하라.
2. 완전 매칭 비용이 dual potential 하한과 같아질 때 최적이라는 사실을 말로 증명하라.
3. 직원 한 명 삽입에서 작업을 최대 `n`번 확장하고 매번 최대 `n`개 slack을 훑어 `O(n²)`인 이유를 적어라.
4. 직원 `n`명 전체가 `O(n³)`이고 비용 행렬까지 포함한 공간이 `O(n²)`인 이유를 적어라.
5. CSES 제약 `n<=200`, 비용 `<=1000`에서 최적 합의 최대값을 계산하라.
6. 구현이 더 큰 문제로 재사용될 때 잠재치·합계를 64-bit로 두는 이유를 설명하라.
7. 음수 비용이 있는 정사각 행렬에도 Hungarian 핵심이 작동하는지, sentinel과 INF 선택은 무엇을 조심해야 하는지 조사하라.
8. 직사각 행렬에서 직원 수와 작업 수 관계에 따라 transpose/dummy 정점을 어떻게 쓰는지 설명하라.
9. 희소 그래프에서 모든 `n²` 비용을 저장하는 Hungarian보다 최소 비용 유량이 나을 수 있는 조건을 말하라.
10. 최대 가중치 배정을 최소화 구현으로 바꾸는 안전한 변환과 overflow 위험을 설명하라.

## 9. 직접 수정해 증명하기

1. owner를 reset하기 전에 callback 복사본을 하나 만들고 두 콜백의 결과와 객체 수명을 예측·실행한다.
2. weak capture를 strong capture로 바꿔 `owner.reset()` 뒤 두 번째 호출 결과가 왜 달라지는지 확인한다.
3. 연습용 복사본에서 passkey 제한을 잠시 푼 stack 객체로 callback을 만들어 결과가 false인지 확인한다.
   raw `[this]` 버전은 실행하지 말고 왜 위험한지 설명한다.
4. `accepts`가 음수 event code를 별도 오류로 구분하도록 바꾸고 반환 의미를 문서화한다.
5. callback 반환을 `std::expected<int, Error>`로 바꿔 만료와 입력 오류를 분리한다.
6. Hungarian 공식 예제의 비용 한 칸을 바꾸어 최적 배정이 어떻게 바뀌는지 손 계산 뒤 테스트한다.
7. `n<=8` 무작위 행렬에서 모든 순열 brute force와 결과 비용을 비교한다.
8. `n=200` 행렬로 실행 시간을 재고 `O(n³)` 예측과 비교한다.
9. 출력된 쌍에서 모든 직원/작업이 정확히 한 번씩 등장하고 합이 첫 줄과 같은지 검사기를 작성한다.

## 최종 통과 기준

- [ ] 모든 C++ 파일을 주석 없이도 다시 구현할 수 있다.
- [ ] 오늘의 모든 표준 호출에 여섯 항목 계약을 말할 수 있다.
- [ ] strong/weak count, 객체 수명, control block 수명을 서로 구분할 수 있다.
- [ ] raw self capture, strong self capture, weak self capture의 실패 모드를 비교할 수 있다.
- [ ] Hungarian의 `p`, `way`, `minv`, `used`, potential 불변식을 코드 없이 설명할 수 있다.
- [ ] 정확성의 primal/dual 직관과 `O(n³)` 시간·`O(n²)` 공간을 증명할 수 있다.
- [ ] CMake 빌드, CTest, 무작위 oracle, 공용 문서 감사를 모두 통과했다.
