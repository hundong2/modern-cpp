# 환경 설정 문제 해결 가이드

## macOS에서 C++ 컴파일 환경 설정

### 문제: "iostream" file not found

이 에러는 Xcode Command Line Tools가 제대로 설치되지 않았을 때 발생합니다.

### 해결 방법

#### 1단계: Command Line Tools 제거 및 재설치

```bash
# 기존 Command Line Tools 제거
sudo rm -rf /Library/Developer/CommandLineTools

# 최신 버전 설치
xcode-select --install
```

팝업 창이 뜨면 "Install"을 클릭하고 설치가 완료될 때까지 기다립니다 (약 5-10분).

#### 2단계: 설치 확인

```bash
# Command Line Tools 경로 확인
xcode-select -p

# 출력 예시: /Library/Developer/CommandLineTools

# 컴파일러 테스트
cat > /tmp/test.cpp << 'EOF'
#include <iostream>
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
EOF

clang++ -std=c++20 /tmp/test.cpp -o /tmp/test
/tmp/test
```

"Hello, World!"가 출력되면 성공입니다!

#### 3단계: 프로젝트 빌드

```bash
cd /path/to/ModernCPP
make clean
make
```

---

## 대안: Docker 사용

Command Line Tools 설치가 어렵다면 Docker를 사용할 수 있습니다.

### Dockerfile 생성

```bash
cd ModernCPP
cat > Dockerfile << 'EOF'
FROM gcc:latest

# 필수 패키지 설치
RUN apt-get update && apt-get install -y \
    libboost-all-dev \
    make \
    && rm -rf /var/lib/apt/lists/*

# 작업 디렉토리 설정
WORKDIR /app

# 소스 코드 복사
COPY . .

# 빌드
RUN make clean && make

# 기본 명령어
CMD ["./build/examples/01_concepts"]
EOF
```

### Docker 빌드 및 실행

```bash
# 이미지 빌드
docker build -t moderncpp .

# 예제 실행
docker run --rm moderncpp ./build/examples/01_concepts
docker run --rm moderncpp ./build/examples/02_ranges

# 대화형 셸
docker run -it --rm moderncpp /bin/bash
```

---

## Linux 사용자

Linux에서는 대부분 문제없이 동작합니다:

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y g++-10 g++ make libboost-all-dev

# Fedora/CentOS
sudo dnf install -y gcc-c++ make boost-devel

# 빌드
make
```

---

## 간단한 테스트 (Boost 없이)

Boost 설치 없이 먼저 C++20 기능만 테스트해보세요:

```bash
# 간단한 C++20 예제 생성
cat > /tmp/cpp20_test.cpp << 'EOF'
#include <iostream>
#include <vector>
#include <ranges>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    
    // C++20 Ranges
    auto even = v | std::views::filter([](int n) { return n % 2 == 0; });
    
    std::cout << "짝수: ";
    for (auto n : even) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
EOF

# 컴파일 (macOS)
clang++ -std=c++20 -stdlib=libc++ /tmp/cpp20_test.cpp -o /tmp/cpp20_test

# 또는 (Linux)
g++ -std=c++20 /tmp/cpp20_test.cpp -o /tmp/cpp20_test

# 실행
/tmp/cpp20_test
```

---

## 여전히 문제가 있나요?

1. **Homebrew로 LLVM 설치** (macOS):
```bash
brew install llvm
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
```

2. **환경 확인**:
```bash
# 시스템 정보
uname -a

# 컴파일러 버전
clang++ --version
g++ --version

# Boost 버전
brew info boost  # macOS
dpkg -l | grep boost  # Linux
```

3. **이슈 리포트**:
   - 위 명령어들의 출력을 저장
   - 에러 메시지 전체를 복사
   - GitHub에 이슈 등록

---

## 추천: 온라인 컴파일러로 먼저 학습

설치가 어렵다면 온라인에서 먼저 C++20을 학습하세요:

- [Compiler Explorer (godbolt.org)](https://godbolt.org/)
- [Wandbox](https://wandbox.org/)
- [Repl.it](https://replit.com/languages/cpp)

예제 코드를 복사해서 실행해보고, 익숙해지면 로컬 환경을 설정하세요!
