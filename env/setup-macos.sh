#!/usr/bin/env bash
set -euo pipefail
command -v xcode-select >/dev/null && xcode-select -p >/dev/null 2>&1 || xcode-select --install
command -v brew >/dev/null || { echo "Homebrew is required: https://brew.sh" >&2; exit 1; }
brew install cmake ninja llvm
echo 'Installed. Apple Clang is the default; Homebrew LLVM is also available via brew --prefix llvm.'

