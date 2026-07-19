#!/usr/bin/env bash
set -euo pipefail
if command -v apt-get >/dev/null; then
  sudo apt-get update
  sudo apt-get install -y build-essential clang cmake ninja-build gdb lldb
elif command -v dnf >/dev/null; then
  sudo dnf install -y gcc-c++ clang cmake ninja-build gdb lldb
elif command -v pacman >/dev/null; then
  sudo pacman -S --needed base-devel clang cmake ninja gdb lldb
else
  echo "Unsupported package manager; install CMake >= 3.25, Ninja, a C++23 compiler, and GDB/LLDB." >&2
  exit 1
fi

