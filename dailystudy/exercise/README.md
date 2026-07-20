# Modern C++ 날짜별 연습 안내

이 폴더는 하루에 하나의 작은 프로그램을 빌드·실행하면서 문법, 객체 수명, 설계 경계를 함께 익히는 과정입니다. 날짜 폴더 안에서 `README.md → main.cpp → problem.cpp → CHECKPOINT.md` 순서로 학습하세요.

| 날짜 | 핵심 주제 | C++ 표준 |
|---|---|---:|
| [2026-07-14](./2026-07-14/README.md) | 입력 파싱, `span`, `string_view`, `expected`, RAII, concept | C++23 |
| [2026-07-15](./2026-07-15/README.md) | 명령 라우팅, 오류 모델, RAII, concept | C++23 |
| [2026-07-16](./2026-07-16/README.md) | `variant`, 방문자, 명령·이벤트 아키텍처 | C++20 |
| [2026-07-17](./2026-07-17/README.md) | 저장소 인터페이스, 다형성, `optional`, 복사·이동 | C++20 |
| [2026-07-18](./2026-07-18/README.md) | 강한 타입, `enum class`, `expected`, 검증 서비스 | C++23 |
| [2026-07-19](./2026-07-19/README.md) | RAII, `unique_ptr`, 소유권, 의존성 주입 | C++20 |
| [2026-07-20](./2026-07-20/README.md) | `variant` 상태 모델, `visit`, `if constexpr` | C++17 |

## 공통 학습 순서

1. README의 아키텍처와 예상 출력을 읽습니다.
2. `main.cpp`의 한글 주석을 따라 객체의 생성·복사·이동·소멸을 표시합니다.
3. 실행 전에 출력과 assert 결과를 종이에 예측합니다.
4. 로컬 w64devkit으로 구성·빌드하고 CTest를 실행합니다.
5. `problem.cpp`를 일부 가린 뒤 직접 다시 작성합니다.
6. CHECKPOINT를 자료 없이 풀고, 실기 변경까지 통과해야 다음 날짜로 넘어갑니다.

## 전체 검증

저장소 루트의 PowerShell에서 날짜별로 아래 명령을 실행합니다.

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
$dates = Get-ChildItem dailystudy/exercise -Directory
foreach ($date in $dates) {
    cmake -S $date.FullName -B "$($date.FullName)/build" -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
    cmake --build "$($date.FullName)/build"
    ctest --test-dir "$($date.FullName)/build" --output-on-failure
}
```

`build/`는 생성 파일이므로 Git에 커밋하지 않습니다.

## 값 범주를 읽는 최소 기준

- **lvalue**: 이름과 식별 가능한 저장 위치가 있는 식입니다. 이름 있는 변수는 타입이 `T&&`여도 식으로 사용하면 lvalue입니다.
- **prvalue**: 값을 계산하거나 새 객체를 초기화하는 순수한 값입니다. `T{...}`와 대부분의 값 반환 함수 호출이 해당합니다.
- **xvalue**: 수명이 끝나기 전 자원을 이전할 수 있는 객체를 나타냅니다. `std::move(object)` 결과가 대표적입니다.
- `std::move`는 실제 데이터를 옮기지 않고 식을 xvalue로 변환합니다. 실제 이동 여부는 선택된 생성자·대입 연산자가 결정합니다.
- 어셈블리는 소스 코드와 일대일 대응하지 않습니다. CPU, ABI, 컴파일러, 디버그/릴리스와 최적화 옵션에 따라 load·store·분기·호출이 사라지거나 합쳐질 수 있습니다.
