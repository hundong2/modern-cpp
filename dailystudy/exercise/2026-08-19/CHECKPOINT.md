# 2026-08-19 CHECKPOINT

자료를 닫고 먼저 답한 뒤 코드를 실행한다. “무엇을 호출한다”가 아니라 입력·출력·상태 변화를 말해야 통과다.

## Modern C++ 문법·아키텍처

1. `class MessageKind`와 `struct PacketHeader`의 기본 접근 권한 차이, `public`/`private`가 바꾸는 접근 범위를 설명한다.
2. `explicit MessageKind(std::uint32_t value)`에는 왜 반환형이 없으며, `value_{value}` 멤버 초기화 목록은 함수 본문 대입과 무엇이 다른가?
3. `MessageKind kind = 7U;`와 `MessageKind kind{7U};` 중 어느 식이 허용되는지, `explicit`이 막는 암시 변환과 연결한다.
4. `original`, `PacketCodec::encode(original)`, `wire`, 이름 있는 지역 반환 객체의 lvalue·prvalue·xvalue 범주를 구분한다.
5. `const PacketHeader&`가 소유권을 갖지 않는 이유, 참조 바인딩 동안 필요한 객체 수명, 반환 `std::array`가 원본과 독립적인 이유를 설명한다.
6. 구조체 전체를 `bit_cast`하는 대신 32비트 필드별로 변환한 이유를 패딩·엔디언·객체 표현 관점에서 설명한다.
7. 바이트 변환이 반드시 특정 CPU 명령 하나가 된다고 단정할 수 없는 이유를 ABI·컴파일러·최적화와 연결한다.

## 표준 라이브러리 호출 계약

8. `std::bit_cast<EncodedWord>(network_value)`에서 템플릿 인자, 실제 입력 식의 타입·값 범주, 크기와 타입 전제조건, 반환형·소유권, 호출 뒤 원본 상태, 복잡도·할당·예외를 설명한다.
9. `std::byteswap(host_value)`에서 수신 객체 유무, 함수 템플릿 인자, 값 매개변수, 반환값 사용, 호출 뒤 인자 상태, 허용·금지 타입을 설명한다.
10. `kind_bytes.size()`와 `result[index]` 각각의 수신 객체 정확한 타입, 인자 수, 반환형, 범위 전제조건, 상태 변화, 복잡도, 참조·포인터 무효화를 비교한다.
11. `std::vector<int> prefix(pattern.size(), 0)`에서 두 생성자 인자를 순서대로 설명하고 원소 수·값·소유권·할당 실패 후 상태를 말한다.
12. `positions.reserve(text.size())`에서 수신 vector의 호출 전 크기·용량, count 인자의 타입·값 범주, void 반환, 호출 후 크기/용량, 재할당 시 무효화, 시간·공간·예외 보장을 설명한다.
13. `positions.push_back(one_based_start)`에서 선택된 `const int&` 오버로드, 인자 복사, void 반환, 크기·용량 변화, 상각 복잡도, 이번 코드에서 재할당이 없는 이유를 설명한다.
14. `std::getline(std::cin, text)`의 두 명시 인자와 기본 구분자, 반환 `istream&` 사용 여부, 입력 위치와 text 상태 변화, 공백 처리, 재할당·수명·오류 계약을 설명한다.
15. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`가 각각 받는 인자, 반환하는 이전 상태, 호출부가 반환을 버리는지, 호출 뒤 보장하는 것과 보장하지 않는 것을 설명한다.
16. `std::cout << positions.size() << '\n'`의 세 표현이 선택하는 연산자, 각 반환 `ostream&`, 최종 반환값 사용 여부, 출력 실패 표현을 설명한다.

## KMP 알고리즘

17. `prefix[i]`의 정확한 정의에서 “진부분 문자열”이 필요한 이유를 예와 함께 설명한다.
18. 불일치 때 `matched = prefix[matched - 1]`로 가도 이미 확인한 텍스트 문자를 다시 읽지 않아도 되는 불변식을 증명한다.
19. 각 텍스트 문자가 앞으로 한 번 진행되고 `matched`의 총 후퇴도 선형임을 이용해 `O(|T|+|P|)`를 증명한다.
20. 완전 일치 뒤 `matched=0` 대신 접두 함수로 이동해야 겹치는 일치를 찾는 이유를 `AAAAA`/`AAA`로 보인다.
21. 패턴이 한 글자, 텍스트와 같은 길이, 공백 포함, 일치 없음인 네 경계를 직접 실행해 기대 출력을 적는다.

## 실기 통과 기준

- `PacketHeader`에 `std::uint32_t version`을 추가해 12바이트로 왕복시키고 양 끝 값 CTest를 추가한다.
- `find_matches`가 위치 vector를 저장하지 않고 발견 즉시 콜백에 전달하는 변형의 소유권과 복잡도를 설계한다.
- `cmake --build`, `ctest --output-on-failure`, 표준 라이브러리 전체 감사가 모두 통과해야 완료다.
