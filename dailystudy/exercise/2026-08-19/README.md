# 2026-08-19 Modern C++ 학습 자료

## 오늘의 목표

- `std::bit_cast`, `std::endian`, `std::byteswap`으로 도메인 객체와 와이어 바이트 표현의 경계를 분리한다.
- 원시 정수를 `explicit` 강한 타입으로 감싸 단위와 의미가 다른 값을 컴파일 단계에서 구분한다.
- 고정 폭 정수와 네트워크 바이트 순서가 필요한 직렬화 코드를 실무 관점에서 작성하고 한계를 설명한다.
- BOJ 1786으로 대회 문자열 문제의 필수 도구인 KMP 접두 함수, 실패 이동, 겹치는 일치 불변식을 증명한다.

## 생성 파일

- [`main.cpp`](main.cpp): `PacketCodec` 경계와 강한 `MessageKind`를 사용한 8바이트 헤더 왕복 예제
- [`problem.cpp`](problem.cpp): 직접 다시 구현해 볼 `AuditRecord` 직렬화·역직렬화 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): [BOJ 1786 찾기](https://www.acmicpc.net/problem/1786) 제출 가능한 완전한 KMP 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, 높은 경고 수준, CTest 여섯 개 등록
- [`run_icpc_test.cmake`](run_icpc_test.cmake): ICPC 표준 입력·출력 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·호출 계약·KMP 이해 검증
- [`../algorithm/knuth-morris-pratt.md`](../algorithm/knuth-morris-pratt.md): KMP와 접두 함수 대표 문서

## Modern C++와 실무 아키텍처

네트워크·파일·장치 경계는 바이트 순서와 정확한 필드 폭을 계약으로 가진다. 응용 코드 전체에 시프트와 캐스트를 흩뿌리면 호스트 바이트 순서가 새어 나오고 필드 순서를 바꾸기 어렵다. 오늘의 `PacketCodec`은 `PacketHeader`라는 도메인 값과 `EncodedHeader`라는 바이트 값 사이의 변환을 한 경계에 모은다. 이 방식은 전송 계층을 교체하거나 버전 필드를 추가할 때 영향 범위를 줄인다.

`std::bit_cast<To>(from)`은 같은 크기의 trivially copyable 타입 사이에서 객체 표현의 비트를 새 값으로 복사한다. 포인터를 억지로 재해석해 잘못된 별칭·정렬·수명 규칙을 만드는 방식이 아니다. 하지만 엔디언을 자동 변환하지도, 구조체 패딩을 제거하지도, 유효하지 않은 대상 표현을 정당화하지도 않는다. 그래서 오늘 코드는 32비트 단어별로 `byteswap`을 적용한 뒤 각각 `array<byte, 4>`로 바꾸고, 구조체 전체를 통째로 캐스팅하지 않는다.

`MessageKind`와 `Sequence`는 `class`라 기본 접근이 `private`이고 원시 값을 내부에 감춘다. 생성자는 반환형이 없으며 멤버 초기화 목록에서 값을 초기화한다. `explicit`은 `MessageKind kind = 7U;` 같은 암시 변환을 막고 `MessageKind{7U}` 직접 초기화는 허용한다. `PacketHeader`와 `AuditRecord`는 단순 데이터 묶음이라 기본 `public`인 `struct`로 두었다.

이 예제에서 이름 있는 `original`, `wire`, `decoded`는 lvalue이고 `PacketCodec::encode(original)`의 값 반환 결과는 prvalue다. prvalue는 목적 `EncodedHeader`를 직접 초기화할 수 있어 불필요한 복사를 피한다. 함수의 `const&`는 호출자 객체에 바인딩되는 비소유 참조이므로 호출 동안 원본 수명이 유지되어야 한다. `std::array`가 바이트를 값으로 소유하므로 반환 뒤에도 원본 참조와 독립적이다. 독점 자원을 넘기지 않아 xvalue나 이동은 필요 없지만, 이름 있는 반환 지역은 NRVO 또는 이동 대상이 될 수 있다.

바이트 변환은 메모리 load/store나 바이트 재배열로, KMP는 문자 load·비교·조건 분기로 구현될 수 있다. 정확한 명령과 호출 인라이닝 여부는 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 1786 찾기](https://www.acmicpc.net/problem/1786), Baekjoon Online Judge
- 핵심 알고리즘: [Knuth–Morris–Pratt 문자열 검색](../algorithm/knuth-morris-pratt.md)
- 복잡도: 접두 함수 `O(|P|)`, 검색 `O(|T|)`, 전체 `O(|T|+|P|)`; 작업 공간 `O(|P|)`, 정답 저장 포함 `O(|P|+K)`
- 대회 필수 지식: 불일치 때 텍스트 위치를 되돌리지 않고 `prefix[matched-1]`로 패턴의 다음 후보 경계를 찾는다. 완전 일치 뒤에도 같은 실패 이동을 적용해야 `AAAAA`에서 `AAA`가 시작하는 1, 2, 3을 모두 찾는다. 문자열 검색, 주기·테두리, 접두사-접미사 문제의 기반이 되는 우승권 필수 패턴이다.

## 오늘 사용한 표준 라이브러리

공통 계약은 [`../standard-library/README.md`](../standard-library/README.md), 오늘 날짜 색인은 [`../standard-library/by-date.md`](../standard-library/by-date.md)에서 찾는다.

| 심볼·호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할과 호출 계약 요점 | 대표 문서 |
|---|---|---|---|---|
| `std::array<std::byte, N>` | `<array>` | 클래스 템플릿 | 크기와 저장소를 값 안에 고정하며 복사된 결과가 바이트를 독립 소유한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `array::size()` | `<array>` | 멤버 함수 | 인자 없이 고정 원소 수를 반환하고 배열을 바꾸지 않으며 `O(1)`이다. | [컨테이너](../standard-library/containers-and-views.md) |
| `array::operator[](index)` | `<array>` | 멤버 연산자 | 범위 검사 없이 원소 참조를 반환하므로 `index<N`을 호출자가 보장한다. 재할당은 없다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::bit_cast<To>(from)` | `<bit>` | 함수 템플릿 | 같은 크기의 trivially copyable 객체 표현을 독립된 `To` 값으로 복사하며 원본을 바꾸지 않는다. | [비트·바이트](../standard-library/bit-and-byte-utilities.md) |
| `std::byteswap(value)` | `<bit>` | 함수 템플릿 | 정수 값의 바이트 순서를 뒤집은 같은 타입 값을 `O(1)`에 반환하고 인자를 유지한다. | [비트·바이트](../standard-library/bit-and-byte-utilities.md) |
| `std::endian::native/little/big` | `<bit>` | 열거형·열거자 | 컴파일 시간 호스트 바이트 순서를 판별하며 혼합 엔디언은 `static_assert`로 거부한다. | [비트·바이트](../standard-library/bit-and-byte-utilities.md) |
| `std::uint32_t`, `std::byte`, `std::size_t` | `<cstdint>`, `<cstddef>` | 타입 별칭·열거형 | 프로토콜 필드 폭, 원시 바이트 의미, 안전한 배열 인덱스를 각각 표현한다. | [비트·바이트](../standard-library/bit-and-byte-utilities.md) |
| `std::string`, `string::size/empty/operator[]` | `<string>` | 클래스·멤버 함수·연산자 | 공백 포함 입력을 소유하고 KMP가 길이·문자를 읽는다. `[]` 범위는 알고리즘 불변식이 보장한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::vector<int>(count, 0)` | `<vector>` | 클래스 템플릿·생성자 | 접두 함수 `count`개를 0으로 초기화해 소유하며 선형 시간·공간과 할당이 필요하다. | [컨테이너](../standard-library/containers-and-views.md) |
| `vector::reserve(count)` | `<vector>` | 멤버 함수 | 크기는 유지하고 최소 용량만 확보하며 재할당되면 기존 관찰자가 무효화된다. | [컨테이너](../standard-library/containers-and-views.md) |
| `vector::push_back(value)` | `<vector>` | 멤버 함수 | 일치 위치를 복사해 끝에 넣고 크기를 1 늘린다. 일반적으로 상각 `O(1)`이다. | [컨테이너](../standard-library/containers-and-views.md) |
| `vector::size/operator[]/begin/end` | `<vector>` | 멤버 함수·연산자·반복자 | 원소 수와 접두 함수 값을 읽고 정답을 순회한다. 검색 중 vector 크기를 바꾸지 않아 반복자가 유지된다. | [컨테이너·용어](../standard-library/terms.md) |
| `std::getline(std::cin, text)` | `<string>`, `<iostream>` | 함수 템플릿 | 입력 스트림과 수정할 문자열 참조를 받아 개행 전까지 저장하고 `istream&`를 반환한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio(false)`, `cin.tie(nullptr)` | `<iostream>` | 정적 함수·멤버 함수 | 전역 동기화와 입력 전 자동 flush 연결을 바꾸고 이전 상태를 반환하지만 여기서는 버린다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::cout << value` | `<iostream>` | 객체·연산자 | 출력 버퍼와 상태를 바꾸고 같은 `ostream&`를 반환해 연쇄하며 최종 참조는 버린다. | [입출력](../standard-library/io-parsing-and-utilities.md) |

## 빌드와 검증

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-08-19 -B build/2026-08-19 -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build/2026-08-19
ctest --test-dir build/2026-08-19 --output-on-failure
./build/2026-08-19/daily_main.exe
./build/2026-08-19/daily_problem.exe
powershell -ExecutionPolicy Bypass -File dailystudy/exercise/tools/audit-standard-library-docs.ps1 -Scope all
```

CTest는 두 Modern C++ 왕복 예제와 BOJ 공개 예제, 겹치는 일치, 공백, 불일치 경계를 검증한다. 완료 전 모든 C++ 파일을 다시 읽어 기본 타입·초기화·함수·조건·반복·접근 지정자·값 범주·수명·표준 호출의 각 인자와 반환·상태 변화·복잡도·무효화·오류 계약, KMP 링크를 대조한다.

## 직접 해보기

1. `MessageKind kind = 7U;`가 컴파일되지 않는 이유와 `MessageKind{7U}`가 허용되는 이유를 설명한다.
2. 구조체 전체를 바로 `bit_cast`할 때 패딩과 엔디언 때문에 와이어 포맷이 불안정해질 수 있는 이유를 말한다.
3. `PacketHeader`에 16비트 버전 필드를 추가하고 명시적 바이트 오프셋과 CTest를 보강한다.
4. `AAAAA`/`AAA`에서 `matched`와 실패 이동을 문자별로 적어 겹치는 세 일치를 증명한다.
5. KMP의 접두 함수로 문자열의 최소 주기를 구하고 `abababab`에서 결과를 검증한다.
