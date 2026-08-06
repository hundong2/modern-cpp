#!/usr/bin/env bash

set -euo pipefail

usage() {
  echo "사용법:" >&2
  echo "  $0 <디렉터리/러스트 파일.rs> [-- 프로그램 인수...]" >&2
  echo "  $0 <디렉터리> <러스트 파일.rs> [-- 프로그램 인수...]" >&2
  echo >&2
  echo "예:" >&2
  echo "  $0 rust/hello.rs" >&2
  echo "  $0 rust hello.rs" >&2
  echo "  $0 rust/hello.rs -- Alice 3" >&2
}

if [[ $# -lt 1 ]]; then
  usage
  exit 1
fi

if [[ "$1" == *.rs ]]; then
  source_argument="$1"
  source_directory="$(dirname -- "$source_argument")"
  source_file="$(basename -- "$source_argument")"
  shift
elif [[ $# -ge 2 && "$2" == *.rs ]]; then
  source_directory="$1"
  source_file="$(basename -- "$2")"

  if [[ "$2" != "$source_file" ]]; then
    echo "오류: 두 번째 인수에는 파일 이름만 입력하세요: $2" >&2
    usage
    exit 1
  fi

  shift 2
else
  echo "오류: .rs 파일을 지정해야 합니다." >&2
  usage
  exit 1
fi

if [[ $# -gt 0 ]]; then
  if [[ "$1" != "--" ]]; then
    echo "오류: 실행 프로그램의 인수 앞에는 --를 입력하세요." >&2
    usage
    exit 1
  fi
  shift
fi

program_argument_count=$#
program_arguments=("$@")
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
requested_source_dir="$script_dir/$source_directory"

if [[ ! -d "$requested_source_dir" ]]; then
  echo "오류: 소스 디렉터리를 찾을 수 없습니다: $requested_source_dir" >&2
  exit 1
fi

source_dir="$(cd -- "$requested_source_dir" && pwd -P)"

case "$source_dir" in
  "$script_dir"|"$script_dir"/*) ;;
  *)
    echo "오류: 자주까먹는 폴더 밖의 소스는 실행할 수 없습니다: $source_dir" >&2
    exit 1
    ;;
esac

source_path="$source_dir/$source_file"
if [[ ! -f "$source_path" ]]; then
  echo "오류: Rust 소스 파일을 찾을 수 없습니다: $source_path" >&2
  exit 1
fi

rust_compiler="${RUSTC:-rustc}"
if ! command -v "$rust_compiler" >/dev/null 2>&1; then
  echo "오류: Rust 컴파일러를 찾을 수 없습니다: $rust_compiler" >&2
  echo "설치 방법은 $script_dir/README.md 의 'Rust 설치' 절을 확인하세요." >&2
  exit 1
fi

rust_edition="${RUST_EDITION:-2024}"
rust_opt_level="${RUST_OPT_LEVEL:-2}"
relative_source_dir="${source_dir#"$script_dir"/}"
if [[ "$source_dir" == "$script_dir" ]]; then
  relative_source_dir="root"
fi

target="${source_file%.rs}"
executable_suffix=""
case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*) executable_suffix=".exe" ;;
esac

runtime_output_dir="$script_dir/build/rust/$relative_source_dir"
executable="$runtime_output_dir/$target$executable_suffix"
mkdir -p "$runtime_output_dir"

echo "== compile: $source_path =="
"$rust_compiler" \
  --edition="$rust_edition" \
  -C "opt-level=$rust_opt_level" \
  "$source_path" \
  -o "$executable"

if [[ ! -x "$executable" ]]; then
  echo "오류: 실행 파일이 생성되지 않았습니다: $executable" >&2
  exit 1
fi

echo
echo "== execute: $executable =="
echo
if (( program_argument_count > 0 )); then
  "$executable" "${program_arguments[@]}"
else
  "$executable"
fi
echo
