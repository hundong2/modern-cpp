#!/bin/bash

set -e

if [ "$#" -ne 1 ]; then
  echo "사용법: $0 <파일명 또는 타깃명>" >&2
  echo "예: $0 05_asio_timer.cpp" >&2
  exit 1
fi

target="$(basename "$1" .cpp)"

export BOOST_ROOT="$(brew --prefix boost)"
export CMAKE_PREFIX_PATH="$BOOST_ROOT${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"

cmake -S . -B build -DCMAKE_CXX_COMPILER=/usr/bin/c++
cmake --build build --target "$target"

echo "

==execute result==

"
./build/$target


echo "

"
