### 🐧 1. Linux (Ubuntu / CentOS)

리눅스는 이 도구들의 고향(GNU Binutils)이므로 설치가 가장 쉽습니다. 보통 `gcc`나 `g++`를 설치할 때 알아서 딸려옵니다.

**설치 방법:**

* **Ubuntu / Debian 계열:**
```bash
sudo apt update
sudo apt install build-essential binutils

```


* **CentOS / RHEL / Fedora 계열:**
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install binutils

```



**확인 방법:**

```bash
nm --version
objdump --version

```

---

### 🍎 2. macOS (애플 실리콘 / 인텔 맥)

macOS는 자체적으로 LLVM/Clang 컴파일러 인프라를 사용합니다. 애플은 터미널에서 C++ 개발 도구를 쓸 수 있도록 **Xcode Command Line Tools**라는 패키지를 제공하며, 이 안에 LLVM 버전의 `nm`(`llvm-nm`)과 `objdump`(`llvm-objdump`)가 포함되어 있습니다.

**설치 방법:**
터미널(Terminal)을 열고 아래 명령어 한 줄만 입력하면 설치 팝업이 뜹니다.

```bash
xcode-select --install

```

*(참고: 무거운 Xcode 전체를 앱 스토어에서 깔 필요 없이, 위 명령어만 치면 터미널용 툴체인만 가볍게 설치됩니다.)*

**확인 방법:**
설치가 끝나면 리눅스와 완전히 똑같은 명령어로 사용할 수 있습니다. (애플이 내부적으로 `llvm-nm`으로 알아서 연결해 줍니다.)

```bash
nm --version
objdump --version

```

---

### 🪟 3. Windows

윈도우는 어떤 컴파일러 생태계를 타느냐에 따라 방법이 3가지로 나뉩니다. 실무에서는 주로 **Visual Studio 전용 도구**를 쓰거나 **WSL**을 씁니다.

#### 방법 A. Visual Studio 네이티브 환경 (가장 권장)

윈도우 네이티브(MSVC) 컴파일러로 빌드한 `.obj`나 `.exe` 파일은 리눅스의 `nm`, `objdump` 대신 마이크로소프트가 만든 전용 툴인 `dumpbin`을 사용해야 완벽하게 분석됩니다.

1. Visual Studio를 설치할 때 "C++를 사용한 데스크톱 개발" 워크로드를 체크해서 설치합니다.
2. 윈도우 시작 버튼을 누르고 **"Developer Command Prompt for VS"** (또는 개발자 명령 프롬프트)를 검색해서 실행합니다. (일반 cmd나 PowerShell에서는 경로를 못 찾습니다.)
3. 이 터미널에서 `dumpbin` 명령어를 사용합니다.
* `nm` 역할 (심볼 보기): `dumpbin /SYMBOLS file.obj`
* `objdump` 역할 (어셈블리 보기): `dumpbin /DISASM file.obj`



#### 방법 B. WSL (Windows Subsystem for Linux) 사용

윈도우 환경이지만 리눅스용 C++ 코드(GCC/Clang)를 크로스 컴파일하는 경우입니다.

1. 윈도우 터미널에서 `wsl --install`을 입력하여 Ubuntu를 설치합니다.
2. 설치된 Ubuntu 터미널을 열고 리눅스와 동일하게 `sudo apt install binutils`를 입력합니다.

#### 방법 C. MSYS2 / MinGW 환경

윈도우에서 억지로 리눅스 명령어(`gcc`, `nm`)를 쓰고 싶을 때 사용하는 환경입니다.

1. MSYS2(msys2.org)를 설치합니다.
2. MSYS2 터미널에서 `pacman -S mingw-w64-ucrt-x86_64-binutils` (또는 gcc 툴체인 전체)를 설치하면 `nm`과 `objdump`를 윈도우 커맨드라인에서 쓸 수 있습니다.

---

### 💡 실무 C++ 개발자를 위한 `nm` / `objdump` 꿀팁

C++ 코드를 컴파일하면 오버로딩(Overloading)과 템플릿 때문에 함수 이름이 `_ZNK4Base7executeEv`처럼 외계어로 꼬이게 됩니다(Name Mangling). 따라서 C++ 바이너리를 뜯어볼 때는 **반드시 디맹글링(Demangling) 옵션**을 주어야 사람이 읽을 수 있습니다.

**1. `nm`으로 vtable 위치와 함수 목록만 빠르게 볼 때**

```bash
# -C (디맹글링): 외계어 이름을 Base::execute() 로 예쁘게 풀어줌
# grep vtable: 수많은 심볼 중 가상 함수 테이블만 필터링
nm -C file.o | grep vtable

```

**2. `objdump`로 기계어 어셈블리를 뜯어볼 때**

```bash
# -d (Disassemble): 코드를 어셈블리어로 번역
# -C (디맹글링): C++ 이름 예쁘게 풀기
# -M intel: (선택) 읽기 불편한 AT&T 문법 대신 친숙한 Intel 문법(MOV EAX, 1)으로 출력
objdump -d -C -M intel file.o | less

```