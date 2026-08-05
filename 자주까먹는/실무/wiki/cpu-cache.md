# CPU 캐시와 false sharing

CPU는 메모리를 cache line 단위(흔히 64바이트지만 아키텍처 의존)로 가져온다. 서로 다른 스레드가 논리적으로 다른 원자 변수를 갱신해도 같은 cache line에 있으면 코어 사이 소유권 이전이 반복되는 false sharing이 생긴다.

`alignas(std::hardware_destructive_interference_size)`가 의도를 표현하지만 해당 상수의 지원/값은 구현 의존이다. padding은 메모리를 늘리고 모든 워크로드를 빠르게 하지 않으므로 profiler와 benchmark로 확인한다.
