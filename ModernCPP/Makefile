# ModernCPP Makefile - C++20 with Boost
# 초보자도 쉽게 사용할 수 있는 Make 빌드 시스템

# ============================================
# 컴파일러 및 플래그 설정
# ============================================
# macOS에서는 clang++이 더 안정적입니다
CXX := clang++
CXXFLAGS := -std=c++20 -stdlib=libc++ -Wall -Wextra -O2 -I./include
LDFLAGS := -lboost_system -lboost_thread -lpthread

# Linux에서는 g++을 사용하려면 아래 줄의 주석을 해제하세요
# CXX := g++

# macOS SDK 경로 자동 탐지 및 C++ 표준 라이브러리 헤더 경로
MACOS_SDK := $(shell xcrun --show-sdk-path 2>/dev/null)
ifneq ($(MACOS_SDK),)
    CXXFLAGS += -isysroot $(MACOS_SDK)
    # SDK 내의 C++ 헤더 명시적으로 추가
    CXXFLAGS += -I$(MACOS_SDK)/usr/include/c++/v1
endif

# macOS에서 Boost 경로 (Homebrew 설치 기준)
# brew install boost 후 사용
BOOST_INCLUDE := $(shell brew --prefix boost 2>/dev/null)/include
BOOST_LIB := $(shell brew --prefix boost 2>/dev/null)/lib

ifneq ($(BOOST_INCLUDE),)
    CXXFLAGS += -I$(BOOST_INCLUDE)
endif
ifneq ($(BOOST_LIB),)
    LDFLAGS += -L$(BOOST_LIB)
endif

# ============================================
# 디렉토리 설정
# ============================================
BUILD_DIR := ./build
SRC_DIR := ./src
INCLUDE_DIR := ./include

# ============================================
# 소스 파일 자동 탐색
# ============================================
# 서버 소스
SERVER_SOURCES := $(wildcard $(SRC_DIR)/server/*.cpp)
SERVER_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SERVER_SOURCES))
SERVER_TARGET := $(BUILD_DIR)/server_app

# 클라이언트 소스
CLIENT_SOURCES := $(wildcard $(SRC_DIR)/client/*.cpp)
CLIENT_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CLIENT_SOURCES))
CLIENT_TARGET := $(BUILD_DIR)/client_app

# 공통 라이브러리
COMMON_SOURCES := $(wildcard $(SRC_DIR)/common/*.cpp)
COMMON_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(COMMON_SOURCES))

# 예제 파일들 (각각 독립 실행 파일)
EXAMPLE_SOURCES := $(wildcard $(SRC_DIR)/examples/*.cpp)
EXAMPLE_TARGETS := $(patsubst $(SRC_DIR)/examples/%.cpp,$(BUILD_DIR)/examples/%,$(EXAMPLE_SOURCES))

# ============================================
# 기본 타겟
# ============================================
.PHONY: all clean server client examples help

# 기본: 모든 것 빌드
all: server client examples

# 서버만 빌드
server: $(SERVER_TARGET)

# 클라이언트만 빌드
client: $(CLIENT_TARGET)

# 예제들만 빌드
examples: $(EXAMPLE_TARGETS)

# ============================================
# 빌드 규칙
# ============================================

# 서버 실행 파일
$(SERVER_TARGET): $(SERVER_OBJECTS) $(COMMON_OBJECTS) | $(BUILD_DIR)
	@echo "🔨 서버 빌드 중..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✅ 서버 빌드 완료: $@"

# 클라이언트 실행 파일
$(CLIENT_TARGET): $(CLIENT_OBJECTS) $(COMMON_OBJECTS) | $(BUILD_DIR)
	@echo "🔨 클라이언트 빌드 중..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✅ 클라이언트 빌드 완료: $@"

# 예제 실행 파일들
$(BUILD_DIR)/examples/%: $(SRC_DIR)/examples/%.cpp $(COMMON_OBJECTS) | $(BUILD_DIR)/examples
	@echo "🔨 예제 빌드 중: $*"
	$(CXX) $(CXXFLAGS) -o $@ $< $(COMMON_OBJECTS) $(LDFLAGS)
	@echo "✅ 예제 빌드 완료: $@"

# 오브젝트 파일 생성 규칙
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "📦 컴파일 중: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 빌드 디렉토리 생성
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/server
	mkdir -p $(BUILD_DIR)/client
	mkdir -p $(BUILD_DIR)/common
	mkdir -p $(BUILD_DIR)/examples

$(BUILD_DIR)/examples:
	mkdir -p $(BUILD_DIR)/examples

# ============================================
# 실행 타겟 (테스트용)
# ============================================
.PHONY: run-server run-client

run-server: server
	@echo "🚀 서버 실행 중..."
	@$(SERVER_TARGET)

run-client: client
	@echo "🚀 클라이언트 실행 중..."
	@$(CLIENT_TARGET)

# ============================================
# 청소
# ============================================
clean:
	@echo "🧹 빌드 파일 정리 중..."
	rm -rf $(BUILD_DIR)
	@echo "✅ 정리 완료"

# ============================================
# 도움말
# ============================================
help:
	@echo "================================================"
	@echo "  ModernCPP Makefile 사용법"
	@echo "================================================"
	@echo ""
	@echo "📌 주요 명령어:"
	@echo "  make              - 모든 것 빌드 (서버 + 클라이언트 + 예제)"
	@echo "  make all          - 위와 동일"
	@echo "  make server       - 서버만 빌드"
	@echo "  make client       - 클라이언트만 빌드"
	@echo "  make examples     - 예제들만 빌드"
	@echo "  make clean        - 빌드 파일 모두 삭제"
	@echo ""
	@echo "🚀 실행 명령어:"
	@echo "  make run-server   - 서버 빌드 후 실행"
	@echo "  make run-client   - 클라이언트 빌드 후 실행"
	@echo ""
	@echo "📁 새 파일 추가 방법:"
	@echo "  - src/server/     에 서버 코드 추가"
	@echo "  - src/client/     에 클라이언트 코드 추가"
	@echo "  - src/common/     에 공통 유틸리티 추가"
	@echo "  - src/examples/   에 학습 예제 추가 (자동으로 개별 빌드됨)"
	@echo ""
	@echo "🔧 새 라이브러리 추가 방법:"
	@echo "  - LDFLAGS에 -l라이브러리명 추가"
	@echo "  - 예: LDFLAGS += -lboost_filesystem"
	@echo ""
	@echo "================================================"
