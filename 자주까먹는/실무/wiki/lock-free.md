# lock-free, CAS, ABA

CAS(compare-and-swap)는 메모리 값이 예상값과 같을 때만 새 값으로 바꾸는 원자 연산이다. lock-free는 시스템 전체가 언젠가 진행함을 뜻하지만 특정 스레드가 굶지 않는 wait-free 보장은 아니다.

Treiber stack의 head 포인터가 A→B→A로 돌아오면 포인터 값만 비교한 CAS가 중간 변경을 놓치는 ABA가 생긴다. 더 심각하게 다른 스레드가 읽는 노드를 즉시 `delete`하면 use-after-free다. tagged pointer, hazard pointer, epoch reclamation 같은 검증된 메모리 회수 전략이 필요하다. 교육용 단순 스택을 운영 코드에 복사하지 않는다.
