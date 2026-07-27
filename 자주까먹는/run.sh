#!/usr/bin/env bash

set -euo pipefail

usage() {
  echo "사용법:" >&2
  echo "  $0 <디렉터리> <cpp 파일>" >&2
  echo "  $0 <디렉터리/cpp 파일>" >&2
  echo >&2
  echo "예:" >&2
  echo "  $0 type dynamic_cast.cpp" >&2
  echo "  $0 type/dynamic_cast.cpp" >&2
}

if [[ $# -eq 1 ]]; then
  source_argument="$1"
  source_directory="$(dirname -- "$source_argument")"
  source_file="$(basename -- "$source_argument")"
elif [[ $# -eq 2 ]]; then
  source_directory="$1"
  source_file="$(basename -- "$2")"

  if [[ "$2" != "$source_file" ]]; then
    echo "오류: 두 번째 인수에는 파일 이름만 입력하세요: $2" >&2
    usage
    exit 1
  fi
else
  usage
  exit 1
fi

if [[ "$source_file" != *.cpp ]]; then
  echo "오류: .cpp 파일을 지정해야 합니다: $source_file" >&2
  exit 1
fi

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source_dir="$script_dir/$source_directory"
source_path="$source_dir/$source_file"
cmake_file="$source_dir/CMakeLists.txt"
target="${source_file%.cpp}"
build_root="$script_dir/build"
cmake_build_dir="$build_root/.cmake/$source_directory"

if [[ ! -f "$cmake_file" ]]; then
  echo "오류: CMake 프로젝트를 찾을 수 없습니다: $cmake_file" >&2
  exit 1
fi

if [[ ! -f "$source_path" ]]; then
  echo "오류: C++ 소스 파일을 찾을 수 없습니다: $source_path" >&2
  exit 1
fi

mkdir -p "$build_root"

cmake \
  -S "$source_dir" \
  -B "$cmake_build_dir" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$build_root"

cmake --build "$cmake_build_dir" --target "$target"

executable="$build_root/$target"
if [[ ! -x "$executable" ]]; then
  echo "오류: 실행 파일이 생성되지 않았습니다: $executable" >&2
  exit 1
fi

echo
echo "== execute: $executable =="
echo
"$executable"
echo
