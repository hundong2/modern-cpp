#!/bin/sh
set -eu

env_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
notebook_path=${1:-"$env_dir/../01-start/lesson.ipynb"}
export UV_CACHE_DIR="${TMPDIR:-/tmp}/cpp23-uv-cache"
export JUPYTER_RUNTIME_DIR="$env_dir/.jupyter/runtime"
export IPYTHONDIR="$env_dir/.jupyter/ipython"
mkdir -p "$JUPYTER_RUNTIME_DIR" "$IPYTHONDIR"

if command -v uv >/dev/null 2>&1; then
  uv_bin=uv
elif [ -x "$env_dir/.tools/uv" ]; then
  uv_bin="$env_dir/.tools/uv"
else
  echo "uv가 없어 프로젝트 내부에 설치합니다."
  mkdir -p "$env_dir/.tools"
  UV_INSTALL_DIR="$env_dir/.tools" UV_NO_MODIFY_PATH=1 \
    curl --proto '=https' --tlsv1.2 -LsSf https://astral.sh/uv/install.sh | sh
  uv_bin="$env_dir/.tools/uv"
fi

"$uv_bin" sync --project "$env_dir"

kernel_dir="$env_dir/.jupyter/kernels/cpp23"
mkdir -p "$kernel_dir"

"$env_dir/.venv/bin/python" "$env_dir/install_kernel.py"
"$env_dir/.venv/bin/jupyter" kernelspec install \
  "$env_dir/.jupyter/kernels/cpp23" --user --replace

JUPYTER_PATH="$env_dir/.jupyter" \
  "$env_dir/.venv/bin/jupyter" nbconvert --to notebook --execute \
  "$notebook_path" --output "$env_dir/.jupyter/verification.ipynb" \
  --ExecutePreprocessor.timeout=60 >/dev/null

echo "설정 완료"
echo "C++23 검증 완료"
echo "실행: $env_dir/start-jupyter.sh"
