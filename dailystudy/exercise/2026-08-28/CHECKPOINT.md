# 2026-08-28 이해 검증

자료를 닫고 먼저 답한 뒤 코드와 공용 문서를 다시 확인한다.

## C++ 기본 문법과 객체 모델

- [ ] `struct AuditEvent`의 멤버가 기본 public이고 `class ConcurrentAuditLog`의 멤버가 기본 private인 차이를 설명할 수 있다.
- [ ] 생성자에 반환형이 없는 이유, `explicit`이 `std::ostream& → ConcurrentAuditLog` 암시 변환을 막는 이유, `ConcurrentAuditLog log{sink}` 직접 초기화가 올바른 이유를 말할 수 있다.
- [ ] `std::ostream& output_`이 포트를 소유하지 않는다는 뜻과 `sink`, `log`, `workers`의 안전한 파괴 순서를 설명할 수 있다.
- [ ] `void write(const AuditEvent& event) const`에서 반환형, 매개변수의 복사 여부, 참조 바인딩, 멤버 함수의 `const`가 각각 뜻하는 바를 말할 수 있다.
- [ ] `AuditEvent{...}`와 lambda 식의 prvalue, `event`와 `owned`의 lvalue, `std::move(event)`의 xvalue를 실제 소유 문자열 버퍼 이동과 연결할 수 있다.
- [ ] 이동된 `event`가 유효하지만 값은 미지정이라는 계약과 이후 허용되는 파괴·대입을 설명할 수 있다.
- [ ] `return AuditSummary{...}`에서 객체 수명과 복사 생략이 어떻게 연결되는지 말할 수 있다.
- [ ] 소스의 로드·저장·비교·분기·함수 호출이 특정 어셈블리 명령과 일대일 대응하지 않는 이유를 CPU·ABI·컴파일러·최적화 관점에서 말할 수 있다.

## 표준 라이브러리 호출 계약

- [ ] `std::osyncstream record{output_}`에서 수신 객체 유무, 실제 인자 식의 타입·값 범주·소유권, 생성 후 상태, wrapped stream 수명, 할당 예외를 설명할 수 있다.
- [ ] `record << "service=" << event.service << ...`에서 각 입력의 타입, 각 호출의 `ostream&` 계열 반환값과 연쇄 사용, 호출 직후 record/output_ 상태 차이를 설명할 수 있다.
- [ ] `record` 소멸 시 emit이 보장하는 “레코드가 섞이지 않음”과 보장하지 않는 “스레드 간 레코드 순서”를 구분할 수 있다.
- [ ] `sink.str()`의 수신 객체 정확한 타입과 호출 전 상태, 데이터 값 인자 없음, `std::string` 반환 소유권, sink 호출 후 상태, 시간·공간·할당 계약을 설명할 수 있다.
- [ ] `text.find("service=api code=200")`의 수신 객체·포인터 인자·`size_type` 반환·`npos` 의미·상태 변화 없음·복잡도를 설명할 수 있다.
- [ ] `workers.reserve(events.size())`의 두 실제 호출 각각에서 수신 객체, 인자/반환값, size/capacity 변화, 할당과 포인터·참조 무효화를 설명할 수 있다.
- [ ] `workers.emplace_back(lambda)`가 선택하는 원소 생성, lambda의 참조/이동 캡처 소유권, 반환 `jthread&`의 무시, size/capacity와 예외 상태를 설명할 수 있다.
- [ ] `worker.joinable()`과 `worker.join()`의 인자·반환·전제조건·호출 뒤 joinable 상태·동기화·오류 계약을 각각 설명할 수 있다.
- [ ] `std::sort(points.begin(), points.end(), comparator)`에서 두 반복자와 비교자 인자, void 반환, strict weak ordering 전제, 순서/참조 의미 변화, 복잡도를 설명할 수 있다.
- [ ] `std::min(best, candidate)`와 `numeric_limits<Distance>::max()`의 반환형·참조/값 사용·상태 변화·복잡도를 설명할 수 있다.
- [ ] `std::cin >> point_count`, `sync_with_stdio(false)`, `cin.tie(nullptr)`, `std::cout << answer`의 입력·출력·반환값 사용 여부와 오류 상태를 설명할 수 있다.

## 동시 출력과 아키텍처

- [ ] 일반 `ostream` 자체에 여러 조각을 동시에 쓰는 것과 각 스레드가 같은 최종 stream buffer를 감싼 `osyncstream`을 쓰는 차이를 말할 수 있다.
- [ ] RAII 지역 변수 `record`의 수명 끝을 한 로그 레코드의 commit 경계로 사용한 이유를 설명할 수 있다.
- [ ] `ostream&` 포트 주입으로 운영 sink와 테스트 `ostringstream`를 교체할 수 있는 의존성 역전을 설명할 수 있다.
- [ ] worker 합류 전에 `ostringstream`를 직접 읽으면 안 되는 이유와 join이 만드는 happens-before 관계를 말할 수 있다.
- [ ] 출력량이 매우 많을 때 레코드별 동기화 대신 배치/비동기 큐를 검토해야 하는 비용 근거를 말할 수 있다.

## 최근접 점 분할 정복

- [ ] 완전 탐색 `O(N²)`이 `N=100,000`에서 부적합한 이유를 비교 횟수로 계산할 수 있다.
- [ ] 최초 x정렬, 두 반쪽 재귀, y정렬 병합, 중앙 띠 검사 단계를 순서대로 설명할 수 있다.
- [ ] `closest_pair`가 반환할 때 `[begin,end)`가 y정렬이라는 불변식을 기저·귀납 단계로 증명할 수 있다.
- [ ] `middle_x`를 재귀 호출 전에 저장해야 하는 이유를 재귀 뒤 points 순서와 연결할 수 있다.
- [ ] 좌우 최솟값을 `d`라 할 때 x차 제곱이 `d` 이상인 점을 띠에서 제외해도 되는 이유를 설명할 수 있다.
- [ ] 띠를 y순으로 보며 y차 제곱이 `d` 이상이면 안쪽 반복을 끝내도 되는 이유를 설명할 수 있다.
- [ ] 평면 패킹 논증으로 띠의 각 점당 비교 후보가 상수 개이고 한 재귀 레벨이 `O(N)`임을 설명할 수 있다.
- [ ] 점화식 `T(N)=2T(N/2)+O(N)`에서 `O(N log N)`을 유도하고 공유 scratch의 공간 `O(N)`을 설명할 수 있다.
- [ ] 중복 좌표, 중앙선을 가로지르는 최적 쌍, 모든 y 동일, 좌표 경계가 각각 어떤 버그를 잡는지 말할 수 있다.

## 실기 검증

- [ ] `problem.cpp`를 보지 않고 출력 포트를 주입받는 `MetricWriter`와 두 worker 검증을 다시 작성한다.
- [ ] `record.emit()`을 명시 호출하는 버전을 만들고 호출 뒤 버퍼 상태와 소멸 시 추가 emit 가능성을 문서에서 확인한다.
- [ ] 최근접 점 풀이를 작은 무작위 점 집합 1,000개 이상에 대해 완전 탐색 답과 대조한다.
- [ ] CMake 빌드, 두 학습 실행 파일, CTest 7개, 표준 라이브러리 전체 감사를 모두 통과시킨다.
