# 2026-06-30 Daily Study: Sliding Window + Hash Map

## 문제

알파벳 소문자로 이루어진 문자열 `s`와 정수 `k`가 주어진다. 문자열 `s`에서 **최대 `k`개의 서로 다른 문자**를 포함하는 가장 긴 연속 부분 문자열의 길이를 구한다.

예시:

```text
입력: s = "eceba", k = 2
출력: 3
설명: 조건을 만족하는 가장 긴 부분 문자열은 "ece"이다.

입력: s = "aa", k = 1
출력: 2
설명: 조건을 만족하는 가장 긴 부분 문자열은 "aa"이다.
```

제한 사항:

- `1 <= s.length <= 50,000`
- `0 <= k <= 50`
- 목표 시간 복잡도: `O(N)`

## 핵심 아이디어

모든 부분 문자열을 이중 루프로 검사하면 `O(N^2)`가 된다. 입력 길이가 최대 50,000이므로 이 방식은 적합하지 않다.

이 문제는 현재 검사 중인 구간을 하나의 윈도우로 보고, 오른쪽 끝(`right`)은 계속 확장하고 조건을 위반하면 왼쪽 끝(`left`)을 줄이는 **슬라이딩 윈도우**로 풀 수 있다.

윈도우 안의 문자별 개수는 `std::unordered_map<char, int>`에 저장한다. 맵의 크기(`char_counts.size()`)가 현재 윈도우 안에 존재하는 서로 다른 문자 수가 된다.

## 알고리즘

1. `right` 포인터를 오른쪽으로 이동하며 새 문자를 윈도우에 추가한다.
2. `unordered_map`에서 해당 문자의 개수를 증가시킨다.
3. 서로 다른 문자 수가 `k`를 초과하면, 조건을 만족할 때까지 `left` 포인터를 오른쪽으로 이동한다.
4. `left`가 지나간 문자의 개수를 감소시키고, 개수가 `0`이 되면 맵에서 제거한다.
5. 매 순간 유효한 윈도우 길이 `right - left + 1`로 최대 길이를 갱신한다.

## C++23 구현 포인트

이번 예제는 최신 C++ 학습 목적에 맞춰 **C++23** 기준으로 빌드한다.

- `CMakeLists.txt`에서 `CMAKE_CXX_STANDARD 23`을 사용한다.
- 입력 문자열은 복사를 피하기 위해 `std::string_view`로 받는다.
- `unordered_map`의 iterator를 사용해 조회와 삭제를 깔끔하게 처리한다.

핵심 축소 로직:

```cpp
while (static_cast<int>(char_counts.size()) > k) {
    const char left_char = s[left];
    auto it = char_counts.find(left_char);
    --it->second;

    if (it->second == 0) {
        char_counts.erase(it);
    }

    ++left;
}
```

문자 개수가 `0`이 되었을 때 반드시 `erase()`를 호출해야 `char_counts.size()`가 줄어든다. 그래야 “서로 다른 문자 수가 `k`개 이하”라는 조건을 정확히 유지할 수 있다.

## 복잡도

- 시간 복잡도: `O(N)`
  - `right` 포인터는 문자열을 한 번 순회한다.
  - `left` 포인터도 전체 실행 동안 최대 `N`번만 이동한다.
  - `unordered_map`의 삽입, 조회, 삭제는 평균 `O(1)`이다.
- 공간 복잡도: `O(K)`
  - 윈도우가 유효한 상태에서는 최대 `K`종류의 문자를 저장한다.
  - 잠깐 `K + 1`개가 될 수 있으므로 점근적으로는 `O(K)`다.
  - 입력이 알파벳 소문자로 제한되면 최대 26개라서 사실상 `O(1)`로 볼 수도 있다.

## 개발 환경

이 저장소에는 관리자 권한 없이 사용할 수 있는 portable GCC 툴체인인 `w64devkit`을 설치해 두었다.

설치 위치:

```text
D:\workspace\modern-cpp\tools\w64devkit
```

주요 실행 파일:

```text
D:\workspace\modern-cpp\tools\w64devkit\bin\g++.exe
D:\workspace\modern-cpp\tools\w64devkit\bin\cmake.exe
D:\workspace\modern-cpp\tools\w64devkit\bin\mingw32-make.exe
```

현재 확인된 컴파일러 버전:

```text
g++ (GCC) 16.1.0
```

## 환경 설정 방법

### 1. 현재 PowerShell 세션에서만 사용

PowerShell을 열고 저장소 루트에서 다음 명령을 실행한다.

```powershell
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
```

확인:

```powershell
g++ --version
cmake --version
mingw32-make --version
```

이 방식은 현재 열린 PowerShell 창에서만 적용된다. 새 터미널을 열면 다시 실행해야 한다.

### 2. 사용자 PATH에 영구 등록

현재 사용자 계정의 PATH에 `w64devkit`을 추가하려면 다음 명령을 실행한다.

```powershell
[Environment]::SetEnvironmentVariable(
    'Path',
    'D:\workspace\modern-cpp\tools\w64devkit\bin;' + [Environment]::GetEnvironmentVariable('Path', 'User'),
    'User'
)
```

명령 실행 후 새 PowerShell 창을 열고 다음으로 확인한다.

```powershell
g++ --version
cmake --version
mingw32-make --version
```

중복 등록을 피하고 싶다면 현재 사용자 PATH를 먼저 확인한다.

```powershell
[Environment]::GetEnvironmentVariable('Path', 'User')
```

### 3. 이 프로젝트에서만 명시적으로 사용

전역 PATH를 건드리지 않고 이 예제만 빌드하려면 CMake 구성 시 컴파일러를 직접 지정한다.

```powershell
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
```

## 빌드 및 실행

이 폴더로 이동한다.

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-06-30
```

CMake로 빌드한다.

```powershell
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
```

실행:

```powershell
.\build\sliding_window_k_distinct.exe
```

직접 입력을 전달할 수도 있다.

```powershell
.\build\sliding_window_k_distinct.exe eceba 2
```

예상 출력:

```text
3
```

## CMake 없이 직접 빌드

`g++`가 PATH에 잡혀 있다면 CMake 없이 직접 빌드할 수 있다.

```powershell
g++ -std=c++23 -Wall -Wextra -pedantic main.cpp -o sliding_window_k_distinct.exe
.\sliding_window_k_distinct.exe
```

