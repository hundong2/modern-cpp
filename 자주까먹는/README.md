# 자주 까먹는 C++ 문법과 기술

## 실행

- [실행 스크립트](./run.sh). 

```sh
./run.sh type/xxx.cpp 
```

## 추가자료

- [디버깅 raw level](./Environment.md). 
- [CPP 확장자 규칙](./ExecuteLibraryArchitecture.md). 
- [ABI 규칙](./ABI_Architecture.md). 
- [type관련](./type/README.md). 
- [project sample template](./template/README.md). 

자주 잊는 C++ 문법, 동작 원리, 실무 패턴을 다시 찾아보기 쉽게 모아 두는
학습 자료의 최상위 목차입니다. 새 자료를 추가할 때는 주제별 하위 폴더를 만들고,
아래 목차에도 해당 폴더의 `README.md` 링크와 핵심 학습 내용을 함께 추가합니다.

## 처음 방문했다면

처음부터 하드웨어 용어를 모두 이해할 필요는 없습니다. 다음 순서로 읽습니다.

```mermaid
flowchart TD
    A["1. 공통 용어집<br/>낯선 단어의 뜻 익히기"] --> B["2. RAII<br/>객체 수명과 자원 관리"]
    B --> C["3. 런타임 기반 6가지<br/>이동·포인터·스레드·다형성·코루틴"]
    C --> D["4. example.cpp 실행<br/>출력을 먼저 관찰"]
    D --> E["5. exercise.cpp 수정<br/>직접 예측하고 검증"]
    E --> F["6. 어셈블리 관찰<br/>마지막 단계에서 선택"]
```

- 먼저 [초보자 공통 용어집](./GLOSSARY.md)을 열어 둡니다.
- 글에서 굵게 표시된 핵심 문장과 다이어그램만 먼저 봅니다.
- 코드는 한 번에 이해하려 하지 말고 출력 결과를 예상한 뒤 실행합니다.
- 어셈블리와 ABI 설명은 앞부분을 이해한 다음 읽어도 됩니다.

## 이 폴더가 연결하는 컴퓨터 계층

```mermaid
flowchart TB
    A["내가 작성한 C++ 코드"] --> B["컴파일러가 타입 검사·최적화"]
    B --> C["기계어와 실행 파일"]
    C --> D["운영체제가 프로세스·스레드·파일 관리"]
    D --> E["CPU가 레지스터·캐시·메모리로 실행"]
    E --> F["화면 출력과 프로그램 결과"]
```

`API`, `ABI`, `CPU`, `OS` 같은 축약어는
[공통 용어집](./GLOSSARY.md)에서 전체 이름, 한국어 뜻, 비유를 함께 확인할 수 있습니다.

## `type`, `cpp23`, `history` 예제 빌드 및 실행

[`run.sh`](./run.sh)는 [`type`](./type/), [`cpp23`](./cpp23/),
[`history`](./history/) 폴더의 `.cpp` 파일 하나를 선택하여 C++23으로 빌드하고
곧바로 실행합니다. 먼저 이 README가 있는 폴더로 이동합니다.

```bash
cd 자주까먹는
```

디렉터리와 파일 이름을 두 인수로 전달할 수 있습니다.

```bash
./run.sh type dynamic_cast.cpp
```

또는 소스 경로를 하나의 인수로 전달해도 됩니다.

```bash
./run.sh type/dynamic_cast.cpp
./run.sh cpp23/main.cpp
./run.sh cpp23/expected.cpp
./run.sh history/decay.cpp
```

예를 들어 `dynamic_cast.cpp`를 실행하면 다음 결과가 출력됩니다.

```text
멍멍
야옹
```

각 `.cpp` 파일은 독립 실행 파일로 빌드되므로 자체 `main()` 함수가 있어야 합니다.
실행 파일은 `build/<폴더>/`에 생성되고 CMake 중간 파일은
`build/.cmake/<폴더>/`에 분리됩니다. 새 `.cpp` 파일을 해당 폴더에 추가하면 각
폴더의 `CMakeLists.txt`가 파일 이름과 같은 실행 타깃을 자동으로 만듭니다.

## 자료 목차

### [RAII: 스코프와 객체 수명으로 자원 관리](./raii-resource-lifetime/README.md)

- 생성자·소멸자, 예외 스택 해제, 역순 파괴, 커스텀 deleter와 범위 잠금을 하드웨어·ABI 관점까지 설명합니다.
- 핵심 표준: C++17
- [C#/Python/Rust 비교](./raii-resource-lifetime/compare.md)
- 예제 및 실습 코드
  - [`example.cpp`](./raii-resource-lifetime/example.cpp): FILE 핸들, 예외 경로, lock_guard 통합 예제
  - [`exercise.cpp`](./raii-resource-lifetime/exercise.cpp): 이동 가능한 Lease guard와 조기 반환 실습
- 빌드 구성
  - [`CMakeLists.txt`](./raii-resource-lifetime/CMakeLists.txt): C++17 대상과 경고 옵션

### [모던 C++ 런타임 기반 6가지](./modern-cpp-runtime-foundations/README.md)

- 방어적 클래스 설계부터 이동, RAII, 원자 연산, 다형성, OS 이벤트 I/O와 코루틴까지
  컴파일러·메모리·하드웨어 관점으로 연결합니다.
- 핵심 표준: 통합 예제와 실습은 C++17, 코루틴 수명 예제는 C++20
- [C#/Python 비교](./modern-cpp-runtime-foundations/compare.md)
- 예제 및 실습 코드
  - [`example.cpp`](./modern-cpp-runtime-foundations/example.cpp): 여섯 개념을 요청 처리 서비스로 통합한 C++17 예제
  - [`coroutine_example.cpp`](./modern-cpp-runtime-foundations/coroutine_example.cpp): 안전한 프레임 수명을 갖는 C++20 수동 코루틴 예제
  - [`exercise.cpp`](./modern-cpp-runtime-foundations/exercise.cpp): 이동 상태, 메모리 순서, 다형성 선택 실습
- 빌드 구성
  - [`CMakeLists.txt`](./modern-cpp-runtime-foundations/CMakeLists.txt): C++17/C++20 대상과 경고 옵션

### [Modern C++23 실무 프로젝트 템플릿](./template/README.md)

- 계층형 클래스 구조, 의존성 주입, `vector`/`list`/`map`, ranges 정렬,
  `unique_ptr`/`shared_ptr`, `const`/`constexpr`, `expected`를 하나의 실행 프로젝트로 연결합니다.
- 핵심 표준: C++23
- [C#/Python 비교](./template/compare.md)
- [템플릿 타입 추론 가이드](./template/template-type-deduction.md):
  `T`, `auto`, `decltype`, 전달 참조, CTAD와 concept를 기초부터 단계별로 설명
- 예제 및 실습 코드
  - [`src/main.cpp`](./template/src/main.cpp): 전체 계층과 소유권을 조립하는 실행 예제
  - [`src/exercise.cpp`](./template/src/exercise.cpp): ranges 정렬과 view를 직접 작성하는 실습
  - [`src/boost_example.cpp`](./template/src/boost_example.cpp): 선택적으로 빌드하는 Boost.Container 예제
- 공개 헤더와 빌드 구성
  - [`include/modern_cpp`](./template/include/modern_cpp/): domain/application/infrastructure 공개 계약
  - [`CMakeLists.txt`](./template/CMakeLists.txt): C++23 정적 라이브러리, 실행 파일, 선택적 Boost target
  - [`Makefile`](./template/Makefile): `make build`, `make run`, `make exercise`, `make boost-run` 단축 명령

### [공통 용어와 축약어 사전](./GLOSSARY.md)

- RAII, ABI, CRTP, CAS, SSO, IOCP, GIL, FFI 등 문서에 등장하는 축약어를 풀이합니다.
- 스택, 힙, 객체 수명, 소유권, 레지스터, 캐시처럼 자주 쓰는 기초 용어를 비유와 함께 설명합니다.

<!--
새 자료는 다음 형식으로 이 주석 바로 위에 추가합니다.

### [주제 이름](./주제-폴더/README.md)

- 한 문장으로 설명한 학습 목표
- 핵심 C++ 표준 또는 문법: C++17 이상
- [C#/Python 비교](./주제-폴더/compare.md)
- 예제 코드
  - [`example.cpp`](./주제-폴더/example.cpp): 예제가 보여 주는 핵심 내용
- 실습 코드
  - [`exercise.cpp`](./주제-폴더/exercise.cpp): 직접 확인할 동작과 과제
-->

## 자료 추가 규칙

이 폴더에서 사이트 주소나 학습할 내용을 제공하면 [AGENTS.md](./AGENTS.md)의
지침에 따라 주제 폴더와 학습 자료를 생성합니다. 각 주제의 진입점은 해당 폴더의
`README.md`이며, 언어별 차이는 `compare.md`에서 확인할 수 있어야 합니다.
