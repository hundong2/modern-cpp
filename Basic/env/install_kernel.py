"""Generate a relocatable-at-install-time local Jupyter kernelspec."""

import json
import sys
from pathlib import Path

project = Path(__file__).resolve().parent
kernel_dir = project / ".jupyter" / "kernels" / "cpp23"
kernel_dir.mkdir(parents=True, exist_ok=True)
spec = {
    "argv": [sys.executable, str(project / "cpp23_kernel.py"), "-f", "{connection_file}"],
    "display_name": "C++23",
    "language": "C++23",
    "metadata": {"debugger": False},
}
(kernel_dir / "kernel.json").write_text(
    json.dumps(spec, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
)
