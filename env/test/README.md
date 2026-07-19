# cpp-env 실행 예제

이 폴더는 `env/cpp-env`의 동작을 확인하기 위한 독립적인 최소 CMake 프로젝트입니다.
다른 프로젝트 파일을 사용하지 않으며 외부 C++ 라이브러리도 필요하지 않습니다.

아래 명령은 저장소 루트에서 실행합니다.

## 1. 도움말 확인

```bash
./env/cpp-env help
```

Windows PowerShell:

```powershell
.\env\cpp-env.ps1 help
```

## 2. C++23 Debug 빌드

```bash
./env/cpp-env configure ./env/test 23 Debug
./env/cpp-env build ./env/test 23 Debug
```

Windows PowerShell:

```powershell
.\env\cpp-env.ps1 configure .\env\test 23 Debug
.\env\cpp-env.ps1 build .\env\test 23 Debug
```

빌드 결과는 다음 폴더에 만들어집니다.

```text
env/test/build/cpp23-Debug/
```

## 3. 프로그램 실행

macOS/Linux:

```bash
./env/test/build/cpp23-Debug/cpp_env_example
```

Windows PowerShell:

```powershell
.\env\test\build\cpp23-Debug\cpp_env_example.exe
```

예상 출력:

```text
cpp-env example: C++23
2 + 3 = 5
```

## 4. 테스트 실행

`test` 명령은 configure와 build도 자동으로 수행합니다.

```bash
./env/cpp-env test ./env/test 23 Debug
```

```powershell
.\env\cpp-env.ps1 test .\env\test 23 Debug
```

정상이라면 `100% tests passed`가 표시됩니다.

## 5. 다른 표준과 빌드 설정 확인

```bash
./env/cpp-env test ./env/test 17 Debug
./env/cpp-env test ./env/test 20 Release
./env/cpp-env test ./env/test 23 RelWithDebInfo
```

각 설정은 서로 다른 빌드 폴더를 사용하므로 동시에 보관할 수 있습니다.

```text
env/test/build/
├── cpp17-Debug/
├── cpp20-Release/
└── cpp23-RelWithDebInfo/
```

## 6. IDE 설정 생성

다음 명령은 이 예제 폴더에 `CMakeUserPresets.json`과 `.vscode` 설정을 추가합니다.

```bash
./env/cpp-env init ./env/test
```

생성 후 VS Code에서 `env/test` 폴더를 열고 CMake Tools의 프리셋을 선택해 빌드하거나
디버깅할 수 있습니다. 기존 설정 파일이 있으면 `init`은 덮어쓰지 않습니다.

## 7. 빌드 결과 정리

특정 빌드만 삭제합니다.

```bash
./env/cpp-env clean ./env/test 23 Debug
```

모든 예제 빌드 결과를 정리하려면 각 표준과 설정에 대해 `clean`을 실행합니다.

