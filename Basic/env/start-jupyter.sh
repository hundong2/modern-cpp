#!/bin/sh
set -eu

env_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
basic_dir=$(CDPATH= cd -- "$env_dir/.." && pwd)
notebook_path=${1:-"$basic_dir/01-start/lesson.ipynb"}
if [ "$#" -gt 0 ]; then shift; fi
export JUPYTER_PATH="$env_dir/.jupyter${JUPYTER_PATH:+:$JUPYTER_PATH}"
export JUPYTER_RUNTIME_DIR="$env_dir/.jupyter/runtime"
export IPYTHONDIR="$env_dir/.jupyter/ipython"
mkdir -p "$JUPYTER_RUNTIME_DIR" "$IPYTHONDIR"

exec "$env_dir/.venv/bin/jupyter" lab "$notebook_path" \
  --ServerApp.root_dir="$basic_dir" "$@"
