"""ipynb 생성 헬퍼.
xeus-cling 커널(`xcpp17`)을 기본 사용하고, 필요시 bash 셀로 g++ 빌드/실행한다.
"""
import json
import os
from typing import List, Dict, Any


def md(text: str) -> Dict[str, Any]:
    return {
        "cell_type": "markdown",
        "metadata": {},
        "source": text.splitlines(keepends=True) if text else [""],
    }


def code(text: str) -> Dict[str, Any]:
    return {
        "cell_type": "code",
        "execution_count": None,
        "metadata": {},
        "outputs": [],
        "source": text.splitlines(keepends=True) if text else [""],
    }


def make_notebook(cells: List[Dict[str, Any]], kernel: str = "xcpp17") -> Dict[str, Any]:
    """kernel: 'xcpp17' (xeus-cling) | 'bash' | 'python3'."""
    if kernel == "xcpp17":
        kernelspec = {
            "display_name": "C++17",
            "language": "C++17",
            "name": "xcpp17",
        }
        language_info = {
            "codemirror_mode": "text/x-c++src",
            "file_extension": ".cpp",
            "mimetype": "text/x-c++src",
            "name": "c++",
            "version": "17",
        }
    elif kernel == "bash":
        kernelspec = {"display_name": "Bash", "language": "bash", "name": "bash"}
        language_info = {"file_extension": ".sh", "mimetype": "text/x-sh", "name": "bash"}
    else:
        kernelspec = {"display_name": "Python 3", "language": "python", "name": "python3"}
        language_info = {"name": "python", "version": "3.11"}

    return {
        "cells": cells,
        "metadata": {
            "kernelspec": kernelspec,
            "language_info": language_info,
        },
        "nbformat": 4,
        "nbformat_minor": 5,
    }


def save(nb: Dict[str, Any], path: str) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(nb, f, ensure_ascii=False, indent=1)
    print(f"  ✓ {path}")
