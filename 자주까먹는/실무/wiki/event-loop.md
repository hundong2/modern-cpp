# I/O 멀티플렉싱과 Reactor/Proactor

`select`/`poll`/`epoll`은 여러 파일 디스크립터의 준비 상태를 관찰한다. Reactor는 “준비됨” 이벤트를 받고 애플리케이션이 non-blocking I/O를 수행한다. Proactor는 I/O 작업을 제출하고 “완료됨” 결과를 받는 모델이다.

`select`는 관찰 집합 복사와 전체 검사, `FD_SETSIZE` 제한이 있다. Linux `epoll`은 관심 집합을 커널에 유지하고 준비 목록을 반환하지만 모든 연산이 무조건 O(1)인 것은 아니다. 이벤트 수만큼 처리해야 하고 실제 I/O 비용은 남는다. LT(level-triggered)는 상태가 남아 있으면 재통지하고, ET(edge-triggered)는 상태 변화 때 통지하므로 `EAGAIN`까지 drain해야 한다.
