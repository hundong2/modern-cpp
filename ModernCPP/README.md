# ModernCPP - C++20 학습 프로젝트

C++20의 최신 기능과 Boost 라이브러리를 활용한 네트워크 프로그래밍 학습 프로젝트입니다.

## 📋 목차

- [소개](#소개)
- [환경 설정](#환경-설정)
- [프로젝트 구조](#프로젝트-구조)
- [빌드 방법](#빌드-방법)
- [실행 방법](#실행-방법)
- [학습 가이드](#학습-가이드)
- [새 코드 추가하기](#새-코드-추가하기)
- [라이브러리 추가하기](#라이브러리-추가하기)
- [문제 해결](#문제-해결)

**📖 상세 가이드:**
- [BUILD.md](BUILD.md) - 빌드 시스템 상세 설명 및 확장 방법
- [QUICKSTART.md](QUICKSTART.md) - 5분 빠른 시작
- [INSTALL.md](INSTALL.md) - 플랫폼별 설치 가이드

---

## 소개

이 프로젝트는 C++20의 최신 기능들을 학습하고, Boost.Asio를 사용한 비동기 네트워크 프로그래밍을 익히기 위한 교육용 프로젝트입니다.

### 학습할 수 있는 내용

- ✅ C++20 코루틴 (Coroutines)
- ✅ C++20 개념 (Concepts)
- ✅ C++20 범위 라이브러리 (Ranges)
- ✅ Boost.Asio를 사용한 네트워크 프로그래밍
- ✅ 비동기 프로그래밍 패턴
- ✅ 서버-클라이언트 통신
- ✅ 모던 C++ 코딩 패턴

---

## 환경 설정

### 필수 요구사항

- **운영체제**: macOS, Linux, Windows (WSL)
- **컴파일러**: g++ 10 이상 (C++20 지원 필수)
- **빌드 도구**: make
- **라이브러리**: Boost 1.75 이상

### macOS 설치 (Homebrew)

```bash
# Homebrew가 없다면 먼저 설치
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 필수 패키지 설치
brew install gcc        # g++ 컴파일러
brew install boost      # Boost 라이브러리
brew install make       # Make 빌드 도구

# 설치 확인
g++ --version           # g++ 버전 확인 (10 이상이어야 함)
brew list boost         # Boost 설치 확인
```

### Linux (Ubuntu/Debian) 설치

```bash
# 패키지 업데이트
sudo apt update

# g++ 컴파일러 설치
sudo apt install -y g++-10 g++

# Boost 라이브러리 설치
sudo apt install -y libboost-all-dev

# 심볼릭 링크 설정 (g++-10을 g++로)
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100

# 설치 확인
g++ --version
```

### 프로젝트 클론 및 초기화

```bash
# 프로젝트 디렉토리로 이동
cd /path/to/modern-cpp/ModernCPP

# 디렉토리 구조 확인
ls -la
```

---

## 프로젝트 구조

```
ModernCPP/
├── Makefile                 # 빌드 설정 파일
├── README.md               # 이 파일
├── include/                # 헤더 파일
│   ├── logger.hpp         # 로깅 유틸리티
│   └── message.hpp        # 메시지 구조체
├── src/
│   ├── server/            # 서버 소스 코드
│   │   └── main.cpp       # 서버 메인 프로그램
│   ├── client/            # 클라이언트 소스 코드
│   │   └── main.cpp       # 클라이언트 메인 프로그램
│   ├── common/            # 공통 유틸리티 (추가 가능)
│   └── examples/          # C++20 기능 예제들
│       ├── 01_concepts.cpp        # Concepts 예제
│       ├── 02_ranges.cpp          # Ranges 예제
│       ├── 03_coroutine.cpp       # Coroutine 예제
│       └── 04_cpp20_features.cpp  # 기타 C++20 기능
├── build/                 # 빌드 결과물 (자동 생성)
└── docs/                  # 추가 문서
```

### 디렉토리 역할

- **include/**: 공통으로 사용되는 헤더 파일
- **src/server/**: 서버 프로그램 소스 코드
- **src/client/**: 클라이언트 프로그램 소스 코드
- **src/common/**: 서버와 클라이언트가 공통으로 사용하는 코드
- **src/examples/**: C++20 기능을 학습하기 위한 독립 예제들
- **build/**: 컴파일된 실행 파일과 오브젝트 파일 (자동 생성)

---

## 빌드 방법

### 기본 명령어

```bash
# 도움말 보기 (모든 명령어 확인)
make help

# 모든 것 빌드 (서버 + 클라이언트 + 예제)
make

# 또는
make all
```

### 개별 빌드

```bash
# 서버만 빌드
make server

# 클라이언트만 빌드
make client

# 예제들만 빌드
make examples
```

### 빌드 정리

```bash
# 빌드 파일 모두 삭제
make clean

# 다시 처음부터 빌드
make clean && make
```

### 빌드 결과 확인

```bash
# 빌드된 파일 확인
ls -l build/

# 예제 실행 파일들 확인
ls -l build/examples/
```

---

## 실행 방법

### 서버 실행

```bash
# 방법 1: make를 통한 실행
make run-server

# 방법 2: 직접 실행
./build/server
```

**출력 예시:**
```
[2024-02-06 14:00:00] [INFO] === ModernCPP 서버 (C++20 + Boost.Asio) ===
[2024-02-06 14:00:00] [INFO] 서버 시작: 포트 8080
[2024-02-06 14:00:00] [INFO] 서버가 포트 8080에서 대기 중...
```

### 클라이언트 실행

**서버를 먼저 실행한 후**, 새 터미널에서:

```bash
# 방법 1: make를 통한 실행
make run-client

# 방법 2: 직접 실행
./build/client

# 방법 3: 호스트와 포트 지정
./build/client 127.0.0.1 8080
```

**출력 예시:**
```
[2024-02-06 14:00:10] [INFO] === ModernCPP 클라이언트 (C++20 + Boost.Asio) ===
[2024-02-06 14:00:10] [INFO] 서버에 연결됨!
[2024-02-06 14:00:10] [INFO] 전송: Hello, Server!
[2024-02-06 14:00:10] [INFO] 응답: Echo: Hello, Server!
```

### 예제 실행

```bash
# 모든 예제 빌드
make examples

# 개별 예제 실행
./build/examples/01_concepts
./build/examples/02_ranges
./build/examples/03_coroutine
./build/examples/04_cpp20_features
```

---

## 학습 가이드

### 초보자를 위한 순서

#### 1단계: C++20 기본 기능 익히기

**예제 04: C++20 새 기능들**
```bash
./build/examples/04_cpp20_features
```

**학습 내용:**
- Designated Initializers (지정 초기화)
- Spaceship Operator (삼방향 비교 연산자)
- `consteval`, `constinit`
- `std::span`
- `contains()`, `starts_with()`, `ends_with()`

**코드 분석:** `src/examples/04_cpp20_features.cpp` 파일을 열어 각 기능의 예제를 확인하세요.

---

#### 2단계: Concepts 이해하기

**예제 01: Concepts**
```bash
./build/examples/01_concepts
```

**학습 내용:**
- Concept이란?
- `requires` 표현식
- 타입 제약 (type constraints)
- 표준 라이브러리 concepts

**실습:**
1. `src/examples/01_concepts.cpp` 열기
2. 새로운 concept 정의해보기
3. 컴파일 에러를 의도적으로 발생시켜 제약 확인

---

#### 3단계: Ranges 라이브러리

**예제 02: Ranges**
```bash
./build/examples/02_ranges
```

**학습 내용:**
- Views와 파이프라인
- `filter`, `transform`, `take`, `drop`
- 지연 평가 (lazy evaluation)
- 범위 알고리즘

**실습:**
1. `src/examples/02_ranges.cpp` 열기
2. 여러 view를 조합한 파이프라인 만들기
3. 자신만의 데이터로 테스트

---

#### 4단계: 코루틴 (Coroutines)

**예제 03: Coroutine**
```bash
./build/examples/03_coroutine
```

**학습 내용:**
- 코루틴의 개념
- `co_yield`, `co_await`, `co_return`
- Promise type
- 제너레이터 패턴

**실습:**
1. `src/examples/03_coroutine.cpp` 열기
2. Generator의 동작 원리 이해
3. 새로운 제너레이터 함수 작성

---

#### 5단계: 네트워크 프로그래밍

**서버-클라이언트 통신**

**학습 순서:**

1. **서버 코드 분석** (`src/server/main.cpp`)
   - Boost.Asio의 `io_context`
   - `tcp::acceptor`로 연결 대기
   - 코루틴을 사용한 비동기 처리

2. **클라이언트 코드 분석** (`src/client/main.cpp`)
   - 서버 연결
   - 메시지 전송/수신
   - `co_await`로 비동기 대기

3. **공통 헤더 분석**
   - `include/logger.hpp`: 로깅 시스템
   - `include/message.hpp`: 메시지 프로토콜

**실습:**
```bash
# 터미널 1: 서버 실행
make run-server

# 터미널 2: 클라이언트 실행
make run-client
```

---

### 중급 학습자를 위한 과제

#### 과제 1: 브로드캐스트 채팅 서버
- 여러 클라이언트 동시 접속
- 한 클라이언트의 메시지를 모든 클라이언트에게 전송

#### 과제 2: 커스텀 프로토콜
- 바이너리 메시지 포맷
- 헤더 + 페이로드 구조

#### 과제 3: Ranges 활용
- 접속된 클라이언트 목록을 Ranges로 필터링/변환

---

## 새 코드 추가하기

> **상세 가이드**: [BUILD.md](BUILD.md)에서 더 자세한 예제와 설명을 확인하세요.

### 1. 새 예제 추가 (자동 빌드)

`src/examples/` 디렉토리에 새 `.cpp` 파일을 추가하면 **자동으로 빌드**됩니다.

**예시: 스마트 포인터 예제 추가**

```bash
# 파일 생성
vim src/examples/05_smart_pointers.cpp

# 또는 명령어로 생성
cat > src/examples/05_my_example.cpp << 'EOF'
#include <iostream>
#include <memory>

int main() {
    auto ptr = std::make_unique<int>(42);
    std::cout << "값: " << *ptr << std::endl;
    return 0;
}
EOF

# 빌드 (자동으로 새 파일 감지)
make examples

# 실행
./build/examples/05_my_example
```

**자동 빌드 원리:** Makefile이 `wildcard` 패턴으로 파일을 자동 탐색합니다.
```makefile
EXAMPLE_SOURCES := $(wildcard $(SRC_DIR)/examples/*.cpp)
```

### 2. 공통 유틸리티 추가

여러 파일에서 사용할 코드는 `src/common/`과 `include/`에 추가합니다.

**간단한 예시**:

```bash
# 1. 헤더 파일 생성
cat > include/my_utility.hpp << 'EOF'
#ifndef MY_UTILITY_HPP
#define MY_UTILITY_HPP

#include <string>

namespace util {
    std::string format_time();  // 선언
}

#endif
EOF

# 2. 구현 파일 생성 (선택사항)
cat > src/common/my_utility.cpp << 'EOF'
#include "my_utility.hpp"
#include <chrono>
#include <sstream>

namespace util {
    std::string format_time() {
        // 구현...
        return "2024-02-07";
    }
}
EOF

# 3. 다른 파일에서 사용
# src/examples/에서 #include "my_utility.hpp"

# 4. 빌드 (자동으로 링크됨)
make
```

> **더 많은 예제**: [BUILD.md의 시나리오 2-4](BUILD.md#시나리오-2-공통-유틸리티-추가) 참조

### 3. 서버/클라이언트 기능 확장

기존 파일을 수정하여 기능을 추가할 수 있습니다.

```bash
# src/server/main.cpp 또는 src/client/main.cpp 수정
vim src/server/main.cpp

# 재빌드
make server
```

> **더 자세한 예제**: [BUILD.md의 시나리오 3](BUILD.md#시나리오-3-서버에-새-기능-추가) 참조

---

## 라이브러리 추가하기

> **상세 가이드**: [BUILD.md의 라이브러리 추가 섹션](BUILD.md#라이브러리-추가하기) 참조

### 빠른 예제: Boost 모듈 추가

```makefile
# Makefile의 LDFLAGS 수정
LDFLAGS := -lboost_system -lboost_thread -lboost_filesystem -lpthread
```

```bash
# 재빌드
make clean && make
```

### 외부 라이브러리 추가 (3단계)

1. **라이브러리 설치**
```bash
brew install 라이브러리명  # macOS
sudo apt install lib라이브러리명-dev  # Linux
```

2. **Makefile 수정**
```makefile
# Include 경로 추가
CXXFLAGS += -I/path/to/headers

# 링크 플래그 추가
LDFLAGS += -l라이브러리명
```

3. **재빌드**
```bash
make clean && make
```

### 지원되는 라이브러리 예제

[BUILD.md](BUILD.md)에서 다음 라이브러리 추가 예제를 확인할 수 있습니다:
- ✅ Boost.Filesystem
- ✅ nlohmann-json (헤더 온리)
- ✅ SQLite3 (링킹 필요)
- ✅ 기타 Boost 모듈들

---

## 문제 해결

### 자주 발생하는 문제

#### 1. "command not found: g++"

**원인**: g++ 컴파일러가 설치되지 않았습니다.

**해결**:
```bash
# macOS
brew install gcc

# Linux
sudo apt install g++-10
```

---

#### 2. "fatal error: boost/asio.hpp: No such file or directory"

**원인**: Boost 라이브러리가 설치되지 않았거나 경로가 잘못되었습니다.

**해결**:
```bash
# macOS
brew install boost

# Linux
sudo apt install libboost-all-dev

# Boost 경로 확인
brew --prefix boost  # macOS
dpkg -L libboost-dev | grep include  # Linux
```

---

#### 3. 빌드는 되지만 실행 시 "dyld: Library not loaded"

**원인**: Boost 공유 라이브러리를 찾을 수 없습니다.

**해결** (macOS):
```bash
# 라이브러리 경로 확인
brew --prefix boost

# 환경 변수 설정
export DYLD_LIBRARY_PATH=$(brew --prefix boost)/lib:$DYLD_LIBRARY_PATH

# .zshrc 또는 .bashrc에 추가
echo 'export DYLD_LIBRARY_PATH=$(brew --prefix boost)/lib:$DYLD_LIBRARY_PATH' >> ~/.zshrc
```

---

#### 4. 코루틴 관련 컴파일 에러

**원인**: 컴파일러가 C++20 코루틴을 완전히 지원하지 않습니다.

**해결**:
```bash
# g++ 버전 확인 (10 이상 필요)
g++ --version

# 버전이 낮다면 업그레이드
brew upgrade gcc  # macOS
sudo apt install g++-11  # Linux
```

---

#### 5. "Address already in use" (서버 실행 시)

**원인**: 포트 8080이 이미 사용 중입니다.

**해결**:
```bash
# 포트 사용 프로세스 확인
lsof -i :8080

# 프로세스 종료 (PID는 위 명령어에서 확인)
kill -9 <PID>

# 또는 서버 코드에서 다른 포트 사용
# src/server/main.cpp의 포트 번호 수정
```

---

### 디버깅 팁

#### 1. 컴파일 에러 상세 보기

```bash
# Makefile의 CXXFLAGS에 -v 추가 (임시)
make CXXFLAGS="-std=c++20 -Wall -Wextra -v" server
```

#### 2. 실행 파일 정보 확인

```bash
# 의존 라이브러리 확인
otool -L build/server  # macOS
ldd build/server       # Linux

# 심볼 테이블 확인
nm build/server | grep main
```

#### 3. Makefile 디버깅

```bash
# Make가 실행하는 명령어 출력
make -n server

# 변수 값 확인
make -p | grep LDFLAGS
```

---

## 추가 학습 자료

### C++20 공식 문서
- [C++ Reference - C++20](https://en.cppreference.com/w/cpp/20)
- [Concepts](https://en.cppreference.com/w/cpp/language/constraints)
- [Ranges](https://en.cppreference.com/w/cpp/ranges)
- [Coroutines](https://en.cppreference.com/w/cpp/language/coroutines)

### Boost 문서
- [Boost.Asio 공식 문서](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
- [Boost.Asio 예제](https://www.boost.org/doc/libs/release/doc/html/boost_asio/examples.html)

### 추천 책
- "C++20 - The Complete Guide" by Nicolai M. Josuttis
- "Professional C++, 5th Edition" by Marc Gregoire

---

## 프로젝트 확장 아이디어

1. **HTTP 서버 구현**: Boost.Beast 사용
2. **WebSocket 채팅**: 실시간 채팅 서버
3. **데이터베이스 연동**: SQLite 또는 PostgreSQL
4. **로그 파일 출력**: 로그를 파일로 저장
5. **설정 파일 로딩**: JSON/YAML 설정 파일 파싱
6. **멀티스레드 서버**: 스레드 풀 활용
7. **암호화 통신**: SSL/TLS 적용

---

## 기여하기

이 프로젝트는 학습용이므로 자유롭게 수정하고 확장하세요!

### 개선 제안
- 새로운 예제 추가
- 더 나은 에러 처리
- 주석 및 문서 개선
- 성능 최적화

---

## 라이선스

이 프로젝트는 교육 목적으로 제공됩니다. 자유롭게 사용, 수정, 배포하실 수 있습니다.

---

## 문의

학습 중 궁금한 점이 있으시면 이슈를 등록하거나 코드를 직접 수정해보세요!

**Happy Coding! 🚀**
