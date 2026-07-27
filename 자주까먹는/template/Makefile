.DEFAULT_GOAL := help

CMAKE ?= cmake
PROJECT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR ?= $(PROJECT_DIR)/build
BUILD_TYPE ?= Debug
WARNINGS_AS_ERRORS ?= ON
CMAKE_ARGS ?=

.PHONY: help configure build run exercise boost-build boost-run

help:
	@printf '%s\n' \
		'사용 가능한 명령:' \
		'  make build       C++23 기본 예제와 연습 예제 빌드' \
		'  make run         기본 예제를 빌드한 뒤 실행' \
		'  make exercise    연습 예제를 빌드한 뒤 실행' \
		'  make boost-build Boost 예제 빌드(Boost 1.82 이상 필요)' \
		'  make boost-run   Boost 예제를 빌드한 뒤 실행' \
		'' \
		'설정 예:' \
		'  make run BUILD_TYPE=Release' \
		'  make build WARNINGS_AS_ERRORS=OFF' \
		'  make build BUILD_DIR=/tmp/modern-cpp23-build' \
		'  make build CMAKE_ARGS="-DCMAKE_CXX_COMPILER=clang++"'

configure:
	$(CMAKE) \
		-S "$(PROJECT_DIR)" \
		-B "$(BUILD_DIR)" \
		-DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
		-DMODERN_CPP_BUILD_EXERCISES=ON \
		-DMODERN_CPP_ENABLE_BOOST=OFF \
		-DMODERN_CPP_WARNINGS_AS_ERRORS="$(WARNINGS_AS_ERRORS)" $(CMAKE_ARGS)

build: configure
	$(CMAKE) --build "$(BUILD_DIR)" --parallel

run: build
	"$(BUILD_DIR)/modern_cpp_demo"

exercise: configure
	$(CMAKE) --build "$(BUILD_DIR)" --target modern_cpp_exercise --parallel
	"$(BUILD_DIR)/modern_cpp_exercise"

boost-build:
	$(CMAKE) \
		-S "$(PROJECT_DIR)" \
		-B "$(BUILD_DIR)" \
		-DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
		-DMODERN_CPP_BUILD_EXERCISES=ON \
		-DMODERN_CPP_ENABLE_BOOST=ON \
		-DMODERN_CPP_WARNINGS_AS_ERRORS="$(WARNINGS_AS_ERRORS)" $(CMAKE_ARGS)
	$(CMAKE) --build "$(BUILD_DIR)" --target modern_cpp_boost_demo --parallel

boost-run: boost-build
	"$(BUILD_DIR)/modern_cpp_boost_demo"
