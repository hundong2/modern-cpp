# C++23 Jupyter 환경

이 폴더에는 각 코드 셀을 Clang 또는 GCC의 `-std=c++23` 옵션으로 컴파일하고
실행하는 프로젝트 로컬 Jupyter 커널이 들어 있습니다. 컴파일러는 `CXX` 환경
변수를 우선 사용하고, 없으면 `clang++`, `g++`, `c++` 순으로 자동 탐색합니다.
셀끼리 상태를 공유하지
않으며, `main()`이 없는 셀은 자동으로 `main()` 안에 감싸 실행합니다. 완전한
`main()` 프로그램을 셀에 작성해도 됩니다.

## 준비 사항

- macOS: Xcode Command Line Tools의 Clang
- Linux: Clang 또는 GCC 13 이상
- Windows: LLVM Clang 또는 MinGW GCC를 `PATH`에 등록

Python, JupyterLab과 `uv`는 프로젝트 내부에 자동 설치되므로 별도 Python 환경은
필요하지 않습니다. OS의 C++ 컴파일러는 관리자 권한과 배포판별 패키지 관리자가
필요할 수 있어 미리 설치해야 합니다.

설치 스크립트는 `C++23` kernelspec을 사용자 Jupyter 영역에도 등록합니다. 따라서
VS Code의 Jupyter 확장에서 별도 서버 주소 없이 커널을 직접 선택할 수 있습니다.

## macOS / Linux 실행

최초 한 번(또는 의존성이 바뀐 뒤) 설정합니다.

```sh
./env/setup-jupyter.sh
```

그다음 JupyterLab을 실행합니다.

```sh
./env/start-jupyter.sh
```

노트북 커널은 `C++23`을 선택합니다. 다른 컴파일러를 사용하려면
JupyterLab 실행 전에 `CXX` 환경 변수를 지정할 수 있습니다.

## Windows PowerShell 실행

```powershell
.\env\setup-jupyter.ps1
.\env\start-jupyter.ps1
```

실행 정책으로 차단되면 현재 프로세스에만 허용한 뒤 실행합니다.

```powershell
Set-ExecutionPolicy -Scope Process Bypass
```

## VS Code

Microsoft의 `Jupyter` 확장을 설치한 뒤 `lesson.ipynb`를 엽니다. 우측 상단에서
`커널 선택` → `다른 커널 선택` → `Jupyter 커널` → `C++23`을 선택합니다.
목록에 바로 나타나지 않으면 `Developer: Reload Window`를 한 번 실행합니다.
