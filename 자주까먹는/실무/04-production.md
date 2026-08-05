# 4장: 외부 시스템과 계층화

## Day 7: 계층화

네트워크 계층은 연결, byte buffer, framing을 담당하고 프로토콜 계층은 역직렬화/검증, 비즈니스 계층은 게임 규칙, 데이터 계층은 영속화를 담당한다. 계층 분리는 클래스 개수 늘리기가 아니라 **변경 이유와 오류 경계 분리**다.

실습: [day07_layered_server.cpp](examples/day07_layered_server.cpp)

## Day 13: Protobuf

[직렬화](wiki/serialization.md)는 C++ 메모리 레이아웃을 wire format과 분리한다. Protobuf 필드 번호는 wire compatibility 계약이므로 재사용하지 않는다. 새 optional 필드 추가는 구버전이 모르는 필드를 건너뛸 수 있지만 required 의미 변경, 타입 변경, 필드 번호 재사용은 호환성을 깨뜨릴 수 있다.

```bash
protoc --cpp_out=. examples/day13_packet.proto
```

생성 코드는 길이 prefix와 별개다. TCP로 전송하려면 직렬화 결과 크기를 framing하고 상한을 검증한다.

실습: [schema](examples/day13_packet.proto), [표준 C++ 모형](examples/day13_protobuf_model.cpp)

## Day 14: Redis

[Redis](wiki/storage.md)는 캐시/세션의 빠른 조회에 유용하지만 네트워크 실패, timeout, key TTL, cache stampede, stale data를 설계해야 한다. hiredis reply는 `freeReplyObject`로 해제되므로 custom deleter를 가진 `unique_ptr`가 적합하다. 이벤트 루프에서 동기 Redis 호출을 하지 않는다.

실습 모형: [day14_redis_model.cpp](examples/day14_redis_model.cpp)

## Day 15: MySQL

DB 작업은 latency가 길고 connection 수가 제한된다. network thread → bounded DB job queue → connection pool worker → completion queue → owner executor로 복귀하는 경계를 둔다. `std::async`를 요청마다 호출하면 실행 정책/스레드 수를 통제하기 어렵다.

Prepared statement의 placeholder에 값을 bind하고 문자열 연결로 SQL을 만들지 않는다. transaction 경계, timeout, retry 가능한 오류, idempotency를 구분한다.

실습 모형: [day15_mysql_model.cpp](examples/day15_mysql_model.cpp)

## Day 16: 전체 아키텍처

한 패킷의 흐름은 다음과 같다.

```text
socket bytes -> receive buffer -> frame parser -> schema validation
             -> session/authorization -> logic executor -> world state
             -> cache/DB job -> completion -> serialize -> send queue
```

각 화살표는 queue가 될 수 있으며 queue마다 capacity와 owner thread를 정해야 한다. [backpressure](wiki/event-loop.md)가 없으면 느린 DB/클라이언트가 전체 메모리를 소진한다. 단일 서버를 넘으면 gateway, service discovery, sharding, message broker를 도입할 수 있지만 분산 트랜잭션·순서·중복·관찰 가능성 비용이 따라온다.

실습: [day16_packet_pipeline.cpp](examples/day16_packet_pipeline.cpp)

## 외부 라이브러리 실전 전환 체크리스트

- SDK 버전과 ABI가 빌드 환경과 맞는가?
- connection/socket/reply/message의 소유자가 타입으로 드러나는가?
- timeout/cancellation/shutdown 경로가 테스트되는가?
- user input 크기와 인증·권한이 검증되는가?
- password/token을 로그에 남기지 않는가?
- retry가 중복 쓰기를 만들지 않는가?
