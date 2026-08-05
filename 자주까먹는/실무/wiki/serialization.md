# 직렬화·엔디언·정렬

직렬화는 객체의 의미를 프로세스/언어/버전 경계를 넘는 바이트 규약으로 바꾸는 일이다. C++ 구조체의 padding, ABI, 포인터 값, CPU endian이 다를 수 있어 구조체 메모리를 그대로 전송하면 이식성과 보안 문제가 생긴다.

네트워크 바이트 순서는 관례적으로 big-endian이다. `htons`/`ntohl` 등으로 정수 필드를 변환한다. `#pragma pack(1)`은 padding을 줄일 수 있지만 비표준 구현 지시이며 misaligned access 비용/장애가 생길 수 있다. Protobuf 같은 스키마 기반 형식을 우선 고려한다.
