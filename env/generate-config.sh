#!/usr/bin/env bash
set -euo pipefail
project="$1"
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

for file in CMakeUserPresets.json .vscode/tasks.json .vscode/launch.json .vscode/settings.json; do
  if [[ -e "$project/$file" ]]; then
    echo "kept existing $file"
    continue
  fi
  mkdir -p "$(dirname "$project/$file")"
  sed "s|@ENV_DIR@|$script_dir|g" "$script_dir/templates/$file" > "$project/$file"
  echo "created $file"
done

