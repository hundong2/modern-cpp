# 설치 가이드

## macOS (Homebrew)

### 1. 필수 도구 설치
```bash
# Homebrew가 없다면 설치
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Command Line Tools 설치
xcode-select --install

# 필수 패키지
brew install boost make
```

### 2. 프로젝트 빌드
```bash
cd ModernCPP
make
```

### 3. 테스트
```bash
# 예제 실행
./build/examples/01_concepts

# 서버-클라이언트 테스트
make run-server  # 터미널 1
make run-client  # 터미널 2
```

## Linux (Ubuntu/Debian)

### 1. 필수 도구 설치
```bash
sudo apt update
sudo apt install -y \
    g++-10 \
    g++ \
    make \
    libboost-all-dev

# g++를 기본 컴파일러로 설정
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100
```

### 2. Makefile 수정
```bash
# Makefile에서 CXX 변경
# CXX := clang++ 를 주석 처리하고
# CXX := g++ 의 주석 해제
```

### 3. 빌드 및 테스트
```bash
make
./build/examples/01_concepts
```

## Docker (모든 플랫폼)

### 1. Dockerfile 생성
```bash
cd ModernCPP
cat > Dockerfile << 'DOCKER_EOF'
FROM gcc:latest

RUN apt-get update && apt-get install -y \
    libboost-all-dev \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN make clean && make

CMD ["./build/examples/01_concepts"]
DOCKER_EOF
```

### 2. 빌드 및 실행
```bash
docker build -t moderncpp .
docker run --rm moderncpp ./build/examples/01_concepts
docker run --rm moderncpp ./build/examples/02_ranges
```

## 문제 해결

자세한 문제 해결은 [docs/setup_troubleshooting.md](docs/setup_troubleshooting.md)를 참조하세요.

### 일반적인 문제

**"iostream not found"**
```bash
# macOS: Command Line Tools 재설치
sudo rm -rf /Library/Developer/CommandLineTools
xcode-select --install
```

**"boost/asio.hpp not found"**
```bash
# macOS
brew install boost

# Linux
sudo apt install libboost-all-dev
```

**컴파일러 버전 낮음**
```bash
# C++20 지원 확인
clang++ --version  # 10+ 필요
g++ --version      # 10+ 필요
```

## 설치 확인

모든 것이 제대로 설치되었는지 확인:

```bash
# 컴파일러
clang++ --version || g++ --version

# Boost
brew list boost || dpkg -l | grep boost

# Make
make --version

# 빌드 테스트
cd ModernCPP
make clean
make
./build/examples/01_concepts
```

"=== C++20 Concepts 예제 ===" 가 출력되면 성공입니다!
