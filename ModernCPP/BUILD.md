# 빌드 및 확장 가이드

이 문서는 ModernCPP 프로젝트의 빌드 시스템과 확장 방법을 상세히 설명합니다.

---

## 📋 목차

1. [빌드 시스템 이해](#빌드-시스템-이해)
2. [빌드 방법](#빌드-방법)
3. [새 파일 추가하기](#새-파일-추가하기)
4. [라이브러리 추가하기](#라이브러리-추가하기)
5. [컴파일 옵션 커스터마이징](#컴파일-옵션-커스터마이징)
6. [고급 빌드 기법](#고급-빌드-기법)

---

## 빌드 시스템 이해

### Makefile 구조

프로젝트의 `Makefile`은 다음과 같이 구성되어 있습니다:

```makefile
# 컴파일러 설정
CXX := clang++                    # 사용할 컴파일러
CXXFLAGS := -std=c++20 ...       # 컴파일 옵션
LDFLAGS := -lboost_system ...    # 링커 옵션

# 디렉토리 설정
BUILD_DIR := ./build
SRC_DIR := ./src
INCLUDE_DIR := ./include

# 소스 파일 자동 탐색
SERVER_SOURCES := $(wildcard $(SRC_DIR)/server/*.cpp)
CLIENT_SOURCES := $(wildcard $(SRC_DIR)/client/*.cpp)
EXAMPLE_SOURCES := $(wildcard $(SRC_DIR)/examples/*.cpp)
```

### 자동 빌드 원리

Makefile은 **wildcard 패턴**을 사용하여 소스 파일을 자동으로 찾습니다:

```makefile
# src/examples/ 안의 모든 .cpp 파일을 찾음
EXAMPLE_SOURCES := $(wildcard $(SRC_DIR)/examples/*.cpp)

# build/examples/01_concepts, build/examples/02_ranges 등으로 변환
EXAMPLE_TARGETS := $(patsubst $(SRC_DIR)/examples/%.cpp,$(BUILD_DIR)/examples/%,$(EXAMPLE_SOURCES))
```

**결과**: 새 파일을 `src/examples/`에 추가하면 자동으로 빌드됩니다!

---

## 빌드 방법

### 기본 빌드

```bash
# 프로젝트 디렉토리로 이동
cd ModernCPP

# 모든 것 빌드 (서버 + 클라이언트 + 예제)
make

# 또는 명시적으로
make all
```

**출력 예시**:
```
🔨 서버 빌드 중...
✅ 서버 빌드 완료: build/server_app
🔨 클라이언트 빌드 중...
✅ 클라이언트 빌드 완료: build/client_app
🔨 예제 빌드 중: 01_concepts
✅ 예제 빌드 완료: build/examples/01_concepts
```

### 선택적 빌드

```bash
# 서버만 빌드
make server
# 결과: build/server_app

# 클라이언트만 빌드
make client
# 결과: build/client_app

# 예제만 빌드
make examples
# 결과: build/examples/01_concepts, 02_ranges, ...

# 특정 예제만 빌드 (직접 지정)
make build/examples/01_concepts
```

### 클린 빌드

```bash
# 빌드 파일 모두 삭제
make clean

# 처음부터 다시 빌드
make clean && make
```

### 빌드 결과 확인

```bash
# 빌드된 파일 확인
ls -lh build/

# 예제 실행 파일 확인
ls -lh build/examples/

# 실행
./build/server_app
./build/client_app
./build/examples/01_concepts
```

---

## 새 파일 추가하기

### 시나리오 1: 새 예제 추가 (가장 간단)

#### 1단계: 파일 생성

```bash
# 방법 1: 에디터로 생성
vim src/examples/05_smart_pointers.cpp

# 방법 2: 명령어로 생성
cat > src/examples/05_smart_pointers.cpp << 'EOF'
#include <iostream>
#include <memory>

int main() {
    auto ptr = std::make_unique<int>(42);
    std::cout << "값: " << *ptr << std::endl;
    return 0;
}
EOF
```

#### 2단계: 빌드 (자동)

```bash
# 예제만 빌드
make examples

# 또는 전체 빌드
make
```

#### 3단계: 실행

```bash
./build/examples/05_smart_pointers
```

**그게 전부입니다!** Makefile이 자동으로 새 파일을 감지하고 빌드합니다.

---

### 시나리오 2: 공통 유틸리티 추가

여러 파일에서 사용할 유틸리티를 추가하는 경우입니다.

#### 예제: 타이머 유틸리티 추가

**1단계: 헤더 파일 생성**

```bash
cat > include/timer.hpp << 'EOF'
#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <iostream>
#include <string_view>

namespace common {

class Timer {
public:
    Timer(std::string_view name = "Timer") 
        : name_(name)
        , start_(std::chrono::high_resolution_clock::now()) {
        std::cout << "[" << name_ << "] 시작..." << std::endl;
    }
    
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
        std::cout << "[" << name_ << "] 완료: " << duration.count() << "ms" << std::endl;
    }
    
    void reset() {
        start_ = std::chrono::high_resolution_clock::now();
    }
    
    double elapsed_seconds() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start_).count();
    }

private:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

} // namespace common

#endif // TIMER_HPP
EOF
```

**2단계: 구현 파일 생성 (필요한 경우)**

헤더 온리로 충분하면 이 단계는 생략 가능합니다.

```bash
cat > src/common/timer.cpp << 'EOF'
#include "timer.hpp"
// 필요한 추가 구현...
EOF
```

**3단계: 사용 예제 작성**

```bash
cat > src/examples/06_timer_test.cpp << 'EOF'
#include <iostream>
#include "timer.hpp"
#include <thread>
#include <chrono>

int main() {
    using namespace std::chrono_literals;
    
    {
        common::Timer timer("작업 1");
        std::this_thread::sleep_for(500ms);
    }
    
    {
        common::Timer timer("작업 2");
        std::this_thread::sleep_for(1s);
    }
    
    return 0;
}
EOF
```

**4단계: 빌드 및 실행**

```bash
make examples
./build/examples/06_timer_test
```

**출력**:
```
[작업 1] 시작...
[작업 1] 완료: 500ms
[작업 2] 시작...
[작업 2] 완료: 1000ms
```

---

### 시나리오 3: 서버에 새 기능 추가

#### 예제: 서버에 통계 기능 추가

**1단계: 헤더 파일**

```bash
cat > include/statistics.hpp << 'EOF'
#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <atomic>
#include <iostream>

namespace common {

class Statistics {
public:
    void increment_connections() { 
        ++total_connections_; 
    }
    
    void increment_messages() { 
        ++total_messages_; 
    }
    
    void print() const {
        std::cout << "=== 서버 통계 ===" << std::endl;
        std::cout << "총 연결: " << total_connections_ << std::endl;
        std::cout << "총 메시지: " << total_messages_ << std::endl;
    }

private:
    std::atomic<uint64_t> total_connections_{0};
    std::atomic<uint64_t> total_messages_{0};
};

} // namespace common

#endif
EOF
```

**2단계: 서버 코드에 통합**

`src/server/main.cpp`를 수정:

```cpp
#include "statistics.hpp"

// 전역 통계 객체
common::Statistics stats;

class Session : public std::enable_shared_from_this<Session> {
    // ...
    void start() {
        stats.increment_connections();  // 연결 시 카운트
        // ...
    }
    
    awaitable<void> echo() {
        // 메시지 받을 때마다
        stats.increment_messages();
        // ...
    }
};

int main() {
    // ...
    // 주기적으로 통계 출력 (예: 시그널 핸들러)
    stats.print();
}
```

**3단계: 재빌드**

```bash
make clean && make server
./build/server_app
```

---

### 시나리오 4: 멀티파일 모듈 추가

복잡한 기능을 여러 파일로 나누는 경우입니다.

#### 예제: JSON 파서 모듈

**프로젝트 구조**:
```
src/common/
├── json_parser.cpp
└── json_serializer.cpp

include/
├── json_parser.hpp
└── json_serializer.hpp
```

**1단계: 헤더 파일**

```bash
cat > include/json_parser.hpp << 'EOF'
#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include <string>
#include <map>

namespace json {

class Parser {
public:
    std::map<std::string, std::string> parse(const std::string& json);
};

} // namespace json

#endif
EOF

cat > include/json_serializer.hpp << 'EOF'
#ifndef JSON_SERIALIZER_HPP
#define JSON_SERIALIZER_HPP

#include <string>
#include <map>

namespace json {

class Serializer {
public:
    std::string serialize(const std::map<std::string, std::string>& data);
};

} // namespace json

#endif
EOF
```

**2단계: 구현 파일**

```bash
cat > src/common/json_parser.cpp << 'EOF'
#include "json_parser.hpp"
#include <sstream>

namespace json {

std::map<std::string, std::string> Parser::parse(const std::string& json) {
    // 간단한 파싱 구현 (실제로는 라이브러리 사용 권장)
    std::map<std::string, std::string> result;
    // TODO: 파싱 로직
    return result;
}

} // namespace json
EOF

cat > src/common/json_serializer.cpp << 'EOF'
#include "json_serializer.hpp"
#include <sstream>

namespace json {

std::string Serializer::serialize(const std::map<std::string, std::string>& data) {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    for (const auto& [key, value] : data) {
        if (!first) oss << ",";
        oss << "\"" << key << "\":\"" << value << "\"";
        first = false;
    }
    oss << "}";
    return oss.str();
}

} // namespace json
EOF
```

**3단계: 사용 예제**

```bash
cat > src/examples/07_json_test.cpp << 'EOF'
#include <iostream>
#include "json_parser.hpp"
#include "json_serializer.hpp"

int main() {
    json::Serializer serializer;
    
    std::map<std::string, std::string> data = {
        {"name", "Alice"},
        {"age", "30"},
        {"city", "Seoul"}
    };
    
    std::string json_str = serializer.serialize(data);
    std::cout << "JSON: " << json_str << std::endl;
    
    return 0;
}
EOF
```

**4단계: 빌드**

```bash
# src/common/ 아래의 모든 .cpp 파일이 자동으로 컴파일되어
# COMMON_OBJECTS에 포함됩니다
make examples
./build/examples/07_json_test
```

**Makefile이 자동으로 처리하는 부분**:
```makefile
# src/common/ 아래 모든 .cpp 찾기
COMMON_SOURCES := $(wildcard $(SRC_DIR)/common/*.cpp)

# 오브젝트 파일로 변환
COMMON_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(COMMON_SOURCES))

# 모든 타겟에 자동 링크
$(SERVER_TARGET): $(SERVER_OBJECTS) $(COMMON_OBJECTS)
$(CLIENT_TARGET): $(CLIENT_OBJECTS) $(COMMON_OBJECTS)
$(EXAMPLE_TARGETS): $(COMMON_OBJECTS)
```

---

## 라이브러리 추가하기

### Boost 라이브러리 추가

Boost는 이미 설치되어 있으므로 추가 모듈만 링크하면 됩니다.

#### 예제: Boost.Filesystem 추가

**1단계: Makefile 수정**

```bash
vim Makefile
```

`LDFLAGS` 라인에 `-lboost_filesystem` 추가:

```makefile
# 변경 전
LDFLAGS := -lboost_system -lboost_thread -lpthread

# 변경 후
LDFLAGS := -lboost_system -lboost_thread -lboost_filesystem -lpthread
```

**2단계: 코드 작성**

```bash
cat > src/examples/08_filesystem.cpp << 'EOF'
#include <iostream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

int main() {
    fs::path current = fs::current_path();
    std::cout << "현재 경로: " << current << std::endl;
    
    std::cout << "\n파일 목록:" << std::endl;
    for (const auto& entry : fs::directory_iterator(current)) {
        if (fs::is_regular_file(entry)) {
            std::cout << "  📄 " << entry.path().filename() << std::endl;
        } else if (fs::is_directory(entry)) {
            std::cout << "  📁 " << entry.path().filename() << std::endl;
        }
    }
    
    return 0;
}
EOF
```

**3단계: 빌드 및 실행**

```bash
make clean && make examples
./build/examples/08_filesystem
```

---

### 외부 라이브러리 추가

#### 예제: nlohmann-json (헤더 온리 라이브러리)

**1단계: 라이브러리 다운로드**

```bash
# 방법 1: Homebrew로 설치
brew install nlohmann-json

# 방법 2: 직접 다운로드
curl -o include/json.hpp \
  https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
```

**2단계: Makefile에 include 경로 추가**

Homebrew로 설치한 경우:

```makefile
# Makefile에 추가
JSON_INCLUDE := $(shell brew --prefix nlohmann-json 2>/dev/null)/include
ifneq ($(JSON_INCLUDE),)
    CXXFLAGS += -I$(JSON_INCLUDE)
endif
```

**3단계: 사용 예제**

```bash
cat > src/examples/09_json_lib.cpp << 'EOF'
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    // JSON 객체 생성
    json person = {
        {"name", "Alice"},
        {"age", 30},
        {"city", "Seoul"},
        {"skills", {"C++", "Python", "JavaScript"}}
    };
    
    // 출력
    std::cout << "JSON (들여쓰기):" << std::endl;
    std::cout << person.dump(4) << std::endl;
    
    // 파싱
    std::string json_str = R"({"name":"Bob","age":25})";
    auto parsed = json::parse(json_str);
    
    std::cout << "\n파싱 결과:" << std::endl;
    std::cout << "이름: " << parsed["name"] << std::endl;
    std::cout << "나이: " << parsed["age"] << std::endl;
    
    return 0;
}
EOF
```

**4단계: 빌드**

```bash
make examples
./build/examples/09_json_lib
```

---

#### 예제: 링킹이 필요한 라이브러리 (SQLite)

**1단계: 설치**

```bash
# macOS
brew install sqlite3

# Linux
sudo apt install libsqlite3-dev
```

**2단계: Makefile 수정**

```makefile
# include 경로 추가
SQLITE_INCLUDE := $(shell brew --prefix sqlite 2>/dev/null)/include
ifneq ($(SQLITE_INCLUDE),)
    CXXFLAGS += -I$(SQLITE_INCLUDE)
endif

# 라이브러리 경로 추가
SQLITE_LIB := $(shell brew --prefix sqlite 2>/dev/null)/lib
ifneq ($(SQLITE_LIB),)
    LDFLAGS += -L$(SQLITE_LIB)
endif

# 링커 플래그에 sqlite3 추가
LDFLAGS := -lboost_system -lboost_thread -lboost_filesystem -lsqlite3 -lpthread
```

**3단계: 사용 예제**

```bash
cat > src/examples/10_sqlite.cpp << 'EOF'
#include <iostream>
#include <sqlite3.h>
#include <string>

int main() {
    sqlite3* db;
    int rc = sqlite3_open(":memory:", &db);
    
    if (rc != SQLITE_OK) {
        std::cerr << "DB 열기 실패: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }
    
    std::cout << "SQLite 버전: " << sqlite3_libversion() << std::endl;
    std::cout << "인메모리 DB 생성 성공!" << std::endl;
    
    // 테이블 생성
    const char* sql = "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT);";
    char* err_msg = nullptr;
    
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "테이블 생성 실패: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    } else {
        std::cout << "테이블 생성 성공!" << std::endl;
    }
    
    sqlite3_close(db);
    return 0;
}
EOF
```

**4단계: 빌드**

```bash
make clean && make examples
./build/examples/10_sqlite
```

---

## 컴파일 옵션 커스터마이징

### 디버그 빌드 vs 릴리스 빌드

**현재 설정 (릴리스)**:
```makefile
CXXFLAGS := -std=c++20 -stdlib=libc++ -Wall -Wextra -O2 -I./include
```

**디버그 빌드를 위한 수정**:

```makefile
# 디버그 모드 설정
DEBUG ?= 0

ifeq ($(DEBUG), 1)
    # 디버그 플래그: 최적화 off, 디버그 심볼 포함, 추가 검사
    CXXFLAGS := -std=c++20 -stdlib=libc++ -Wall -Wextra -g -O0 -DDEBUG \
                -fsanitize=address -fsanitize=undefined \
                -I./include
    LDFLAGS += -fsanitize=address -fsanitize=undefined
else
    # 릴리스 플래그: 최적화 on
    CXXFLAGS := -std=c++20 -stdlib=libc++ -Wall -Wextra -O2 -DNDEBUG -I./include
endif
```

**사용법**:

```bash
# 디버그 빌드
make clean
make DEBUG=1

# 릴리스 빌드 (기본)
make clean
make

# 또는
make DEBUG=0
```

---

### 추가 경고 활성화

더 엄격한 검사를 위해:

```makefile
CXXFLAGS += -Wpedantic -Wconversion -Wshadow -Wnon-virtual-dtor
```

### 특정 예제만 다른 플래그로 빌드

특정 예제에만 추가 플래그가 필요한 경우:

```makefile
# Makefile 맨 아래에 추가
build/examples/special_example: CXXFLAGS += -DSPECIAL_FLAG
build/examples/special_example: src/examples/special_example.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)
```

---

## 고급 빌드 기법

### 병렬 빌드

여러 파일을 동시에 컴파일:

```bash
# 4개의 작업을 병렬로 실행
make -j4

# CPU 코어 수만큼 병렬 실행
make -j$(nproc)  # Linux
make -j$(sysctl -n hw.ncpu)  # macOS
```

### 빌드 진행 상황 출력

```bash
# 실행되는 명령어 모두 출력
make VERBOSE=1

# 또는 Makefile에서 @ 제거
```

### 조건부 컴파일

**Makefile에 추가**:

```makefile
# 환경 변수로 기능 토글
ENABLE_LOGGING ?= 1

ifeq ($(ENABLE_LOGGING), 1)
    CXXFLAGS += -DENABLE_LOGGING
endif
```

**코드에서 사용**:

```cpp
#ifdef ENABLE_LOGGING
    std::cout << "로그: " << message << std::endl;
#endif
```

**빌드**:

```bash
# 로깅 활성화 (기본)
make

# 로깅 비활성화
make ENABLE_LOGGING=0
```

### 빌드 캐시 확인

어떤 파일이 재컴파일되는지 확인:

```bash
# 변경된 파일만 재컴파일
touch src/examples/01_concepts.cpp
make  # 01_concepts만 재빌드됨
```

### Makefile 디버깅

```bash
# Make가 실행할 명령어만 출력 (실행하지 않음)
make -n

# 변수 값 확인
make -p | grep CXXFLAGS

# 특정 변수 출력
make print-LDFLAGS

# Makefile에 추가:
print-%:
	@echo $* = $($*)
```

---

## 빌드 문제 해결

### 문제: "No rule to make target"

**원인**: 파일 경로가 잘못되었거나 파일이 없음

**해결**:
```bash
# 파일 존재 확인
ls src/examples/

# 빌드 디렉토리 초기화
make clean
make
```

### 문제: "undefined reference to"

**원인**: 라이브러리가 링크되지 않음

**해결**:
```makefile
# Makefile의 LDFLAGS에 라이브러리 추가
LDFLAGS += -l라이브러리명
```

### 문제: 헤더 파일을 찾을 수 없음

**원인**: include 경로가 설정되지 않음

**해결**:
```makefile
# Makefile의 CXXFLAGS에 경로 추가
CXXFLAGS += -I/path/to/headers
```

---

## 요약

### 새 예제 추가
1. `src/examples/` 에 `.cpp` 파일 생성
2. `make examples` 실행
3. `./build/examples/파일명` 실행

### 새 유틸리티 추가
1. `include/` 에 헤더 파일 생성
2. 필요시 `src/common/` 에 구현 파일 생성
3. 다른 파일에서 `#include "파일명.hpp"` 사용
4. `make` 실행

### 라이브러리 추가
1. 라이브러리 설치 (`brew install` 또는 `apt install`)
2. Makefile의 `CXXFLAGS`에 include 경로 추가
3. Makefile의 `LDFLAGS`에 링크 플래그 추가
4. `make clean && make` 실행

### 컴파일 옵션 변경
1. Makefile의 `CXXFLAGS` 또는 `LDFLAGS` 수정
2. `make clean && make` 실행

---

**다음 문서**: [README.md](README.md) - 전체 학습 가이드

**관련 문서**:
- [INSTALL.md](INSTALL.md) - 설치 가이드
- [QUICKSTART.md](QUICKSTART.md) - 빠른 시작
- [docs/setup_troubleshooting.md](docs/setup_troubleshooting.md) - 문제 해결
