#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "사용법: $0 <날짜 디렉터리>" >&2
  echo "예: $0 2026-06-30" >&2
  exit 1
fi

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
date_dir="$1"
source_dir="$script_dir/$date_dir"
cmake_file="$source_dir/CMakeLists.txt"
build_root="$script_dir/build"
cmake_build_dir="$build_root/.cmake/$date_dir"

if [[ ! "$date_dir" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]; then
  echo "오류: 날짜는 YYYY-MM-DD 형식이어야 합니다: $date_dir" >&2
  exit 1
fi

if [[ ! -f "$cmake_file" ]]; then
  echo "오류: CMake 프로젝트를 찾을 수 없습니다: $cmake_file" >&2
  exit 1
fi

# main.cpp로 만들어지는 기본 실행 타깃을 선택한다.
target="$(awk '
  /add_executable/ && /main\.cpp/ {
    line = $0
    sub(/^[[:space:]]*add_executable[[:space:]]*\([[:space:]]*/, "", line)
    split(line, fields, /[[:space:])]/)
    print fields[1]
    exit
  }
' "$cmake_file")"

if [[ -z "$target" ]]; then
  echo "오류: $cmake_file 에서 main.cpp 실행 타깃을 찾지 못했습니다." >&2
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
