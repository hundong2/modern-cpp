# RAII와 스마트 포인터

RAII(Resource Acquisition Is Initialization, 자원 획득은 초기화)는 자원 수명을 객체 수명에 묶는 C++ 설계법이다. 생성자가 유효한 자원을 확보하고 소멸자가 해제하므로 조기 반환과 예외에서도 정리된다.

- `std::unique_ptr<T>`: 단독 소유. 기본 선택이며 복사 불가, 이동 가능.
- `std::shared_ptr<T>`: 제어 블록의 강한 참조 수로 공동 소유. 원자적 참조 수 갱신 비용과 순환 참조 위험이 있다.
- `std::weak_ptr<T>`: 소유하지 않는 관찰자. `lock()`으로 잠시 강한 참조를 얻는다.

`shared_ptr` 객체 자체의 참조 수 조작이 thread-safe라는 말은 가리키는 `T`의 멤버 접근도 thread-safe라는 뜻이 아니다.
