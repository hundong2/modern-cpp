"""A small Jupyter kernel that compiles each cell as a C++23 program."""

from __future__ import annotations

import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from ipykernel.kernelapp import IPKernelApp
from ipykernel.kernelbase import Kernel


class Cpp23Kernel(Kernel):
    implementation = "cpp23-clang"
    implementation_version = "1.0"
    language = "C++23"
    language_version = "23"
    banner = "C++23 (Clang, cell-by-cell compilation)"
    language_info = {
        "name": "c++",
        "mimetype": "text/x-c++src",
        "codemirror_mode": "text/x-c++src",
        "pygments_lexer": "cpp",
        "file_extension": ".cpp",
    }

    def _program(self, code: str) -> str:
        # A complete program is compiled unchanged. For notebook-style cells,
        # preprocessor directives stay at file scope and the rest enters main().
        if re.search(r"\b(?:int|auto)\s+main\s*\(", code):
            return code

        directives: list[str] = []
        body: list[str] = []
        continued = False
        for line in code.splitlines():
            is_directive = continued or line.lstrip().startswith("#")
            (directives if is_directive else body).append(line)
            continued = is_directive and line.rstrip().endswith("\\")

        return "\n".join(
            [*directives, "", "int main()", "{", *body, "", "    return 0;", "}"]
        )

    def do_execute(
        self,
        code: str,
        silent: bool,
        store_history: bool = True,
        user_expressions: dict | None = None,
        allow_stdin: bool = False,
        *,
        cell_meta: dict | None = None,
        cell_id: str | None = None,
    ) -> dict:
        if not code.strip():
            return {"status": "ok", "execution_count": self.execution_count,
                    "payload": [], "user_expressions": {}}

        configured = os.environ.get("CXX")
        if configured:
            compiler = shlex.split(configured)
        else:
            compiler = next(
                ([found] for name in ("clang++", "g++", "c++")
                 if (found := shutil.which(name))),
                None,
            )
        if not compiler:
            message = "C++ compiler not found. Install Clang or GCC, or set CXX."
            if not silent:
                self.send_response(self.iopub_socket, "stream", {
                    "name": "stderr", "text": message + "\n",
                })
            return {"status": "error", "execution_count": self.execution_count,
                    "ename": "CompilerNotFound", "evalue": message, "traceback": []}

        with tempfile.TemporaryDirectory(prefix="cpp23-jupyter-") as tmp:
            source = Path(tmp, "cell.cpp")
            executable = Path(tmp, "cell")
            source.write_text(self._program(code), encoding="utf-8")
            command = [*compiler, "-std=c++23"]
            # This machine's Command Line Tools keep libc++ headers inside the
            # SDK, while clang does not discover that directory automatically.
            if sys.platform == "darwin" and "clang++" in Path(compiler[0]).name:
                sdk_result = subprocess.run(
                    ["xcrun", "--show-sdk-path"], text=True, capture_output=True
                )
                if sdk_result.returncode == 0:
                    sdk = Path(sdk_result.stdout.strip())
                    command.extend(["-isysroot", str(sdk)])
                    libcxx = sdk / "usr/include/c++/v1"
                    if libcxx.is_dir():
                        command.extend(["-isystem", str(libcxx)])
            command.extend(["-Wall", "-Wextra", "-pedantic",
                            str(source), "-o", str(executable)])
            compiled = subprocess.run(command, text=True, capture_output=True)

            if compiled.returncode:
                if not silent:
                    self.send_response(self.iopub_socket, "stream", {
                        "name": "stderr", "text": compiled.stderr,
                    })
                return {"status": "error", "execution_count": self.execution_count,
                        "ename": "CompilationError", "evalue": "C++23 compilation failed",
                        "traceback": []}

            ran = subprocess.run([str(executable)], text=True, capture_output=True)
            if not silent:
                if ran.stdout:
                    self.send_response(self.iopub_socket, "stream", {
                        "name": "stdout", "text": ran.stdout,
                    })
                if ran.stderr:
                    self.send_response(self.iopub_socket, "stream", {
                        "name": "stderr", "text": ran.stderr,
                    })
            if ran.returncode:
                return {"status": "error", "execution_count": self.execution_count,
                        "ename": "RuntimeError", "evalue": f"process exited with {ran.returncode}",
                        "traceback": []}

        return {"status": "ok", "execution_count": self.execution_count,
                "payload": [], "user_expressions": {}}


if __name__ == "__main__":
    IPKernelApp.launch_instance(kernel_class=Cpp23Kernel)
