# 2026-08-29 CHECKPOINT

자료를 닫고 실제 코드 식을 가리키며 답한다.

## Modern C++ 문법·아키텍처

1. `struct ConfigUpdate` 멤버가 기본 public이고 `class SingleUpdateMailbox` 멤버가 기본 private인 이유는 무엇인가?
2. `SingleUpdateMailbox() : ready_{0}, slot_{}`에서 생성자에 반환형이 없는 이유와 각 중괄호 초기화 결과를 말하라.
3. `publish(ConfigUpdate update)`의 값 매개변수는 호출자 객체와 어떤 소유 관계인가? prvalue와 lvalue 호출의 복사·이동 차이를 말하라.
4. `std::move(update)` 자체가 문자열 버퍼를 옮기는가? 이 식의 값 범주와 실제 이동이 일어나는 연산을 구분하라.
5. `Lease(Semaphore&)`의 `explicit`이 막는 암시 변환과 올바른 `Lease{slots_}` 직접 초기화를 설명하라.
6. `Semaphore* gate_`는 무엇을 소유하지 않는가? gate가 Lease보다 먼저 파괴되면 왜 위험한가?
7. 복사 삭제, 이동 생성자, 소멸자가 permit 반환을 정확히 한 번으로 만드는 불변식을 증명하라.
8. 반환 `ConfigUpdate`/`Lease` prvalue에서 복사 생략이 가능한 위치와, 복사 생략이 없어도 이동으로 안전한 이유를 말하라.

## 표준 라이브러리 호출 계약

1. `ready_{0}`의 수신 객체 정확한 타입, 입력 `0`의 타입·허용 범위, 생성 뒤 counter, 반환/예외/할당 계약을 설명하라.
2. `ready_.release()`의 숨은 기본 인자, 반환형, 전후 counter, 최대값 전제조건, 깨우기·공정성 보장을 말하라.
3. `ready_.acquire()`의 입력 수, 반환형, 호출 전후 상태, 블로킹 가능성, publish의 payload 쓰기와 생기는 happens-before를 말하라.
4. `slots_{2}`와 `slots_.acquire()`에서 템플릿 인자 `2`, 생성자 인자 `2`, 감소량 `1`을 각각 구분하라.
5. `std::exchange(other.gate_, nullptr)`의 두 인자 식·타입·값 범주, 반환형/저장 위치, 호출 뒤 other 상태를 설명하라.
6. `std::jthread worker{lambda}`의 callable 소유권, 참조 캡처 수명, 생성 실패, `worker.join()`의 반환/후조건/오류/동기화를 말하라.
7. `augmented.push_back('\0')`의 수신 상태, 입력, 반환, 크기·용량 변화, 재할당 시 포인터/참조/반복자 무효화를 설명하라.
8. `group.swap(next_group)`의 두 수신/인자 객체, 반환, 복잡도, 버퍼 소유권과 기존 원소 참조가 따라가는 대상을 말하라.
9. `suffix_order.size()`와 `suffix_order[order]`의 반환형·복잡도·범위 검사·수명 계약 차이를 말하라.
10. `std::cin >> text`와 `std::cout << value`가 반환하는 참조, 바꾸는 객체/상태, 실패 시 동작과 인자 소유 여부를 설명하라.

## 접미 배열·LCP 증명

1. 모든 접미사 문자열을 생성해 정렬하면 최악 시간·공간이 왜 커지는가?
2. doubling 단계 시작 시 `group[i]`가 뜻하는 것과, `(group[i], group[i+half])` 쌍으로 길이 `2*half` 순서를 결정할 수 있는 이유를 증명하라.
3. 두 번째 절반 기준으로 위치를 `-half` 순환 이동한 뒤 첫 그룹만 계수 정렬하면 왜 두 key가 모두 정렬되는가?
4. sentinel `\0`이 소문자보다 작고 유일해야 하는 이유와 출력에서 제거할 위치를 설명하라.
5. 단계당 `O(N+G)`, 단계 수 `O(log N)`을 이용해 `O(N log N)`을 유도하라.
6. Kasai에서 현재 LCP가 `h`이면 다음 시작 위치의 후보를 `h-1`에서 시작해도 되는 이유를 증명하라.
7. Kasai의 while 문자 비교 총합이 `O(N)`인 이유를 height 증가/감소 횟수로 설명하라.
8. `banana`, `aaaa`, `abab`의 suffix array와 LCP를 손으로 계산해 CTest 기대값과 대조하라.

## 초보자 실기 검증

- `Lease` 이동 생성자에서 `std::exchange` 대신 단순 포인터 복사를 하고 어떤 assert/전제조건이 깨지는지 확인한다.
- `release()`/`acquire()` 순서를 바꾸지 않은 채 payload를 두 개 연속 publish할 수 있도록 하려면 어떤 큐·잠금·counter 설계가 더 필요한지 제안한다.
- 길이 1, 모든 문자 동일, 모든 문자 다름, 주기 문자열을 직접 실행한다.
- 길이 1~30의 무작위 문자열에서 풀이 SA/LCP를 실제 `substr` 정렬과 대조한다. 제출 코드는 여전히 `substr`를 만들지 않아야 한다.
- CTest와 전체 표준 라이브러리 감사가 모두 통과해야 완료로 판정한다.
