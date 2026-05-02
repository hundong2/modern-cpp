# C++ 학습 가이드라인 (C++17 → C++20 → C++23)

C++ 입문자가 **공식 문서를 자력으로 읽을 수 있는 수준** 까지 단계적으로 끌어올리는 11강 커리큘럼이다. 모든 강의는 **JupyterLab 노트북** 으로 제공되며, **Docker 컨테이너** 안에서 실제로 컴파일·실행된다.

---

## 1. 누구를 위한 자료인가

- C++ 가 처음인 사람 (C 정도는 알면 더 좋다)
- 다른 언어는 쓰지만 C++ 의 *이상함* 을 한 번 정리하고 싶은 사람
- `unique_ptr`, `move`, 템플릿, ranges, concepts, coroutines 가 무엇이고 *왜* 필요한지 이해하고 싶은 사람
- 메모리/캐시/네트워크 같은 시스템 레벨 동작을 같이 보고 싶은 사람

문법은 처음부터 자세히 설명한다. *왜 이렇게 됐는지* 의 동기까지 같이 다룬다.

---

## 2. 디렉토리 구조

```
cpp_learning/
├── Dockerfile               # Ubuntu + GCC-12 + JupyterLab + xeus-cling
├── docker-compose.yml       # 한 방에 띄우기
├── README.md                # 이 파일
├── notebooks/               # ★ 학습 노트북 11개 (Jupyter 가 마운트)
│   ├── 01_introduction.ipynb
│   ├── 02_memory_model.ipynb
│   ├── 03_classes_raii_smartptr.ipynb
│   ├── 04_tricky_syntax.ipynb
│   ├── 05_templates.ipynb
│   ├── 06_cpp20_features.ipynb
│   ├── 07_cpp23_features.ipynb
│   ├── 08_concurrency.ipynb
│   ├── 09_networking.ipynb
│   ├── 10_optimization.ipynb
│   └── 11_capstone.ipynb
├── _build/                  # 노트북을 생성한 파이썬 빌더 (학습용으론 무시 OK)
│   ├── nb_helper.py
│   └── lesson01.py … lesson11.py
├── scripts/
│   └── daily_cpp.py         # Gemini API 로 매일 학습 노트 생성
├── daily/                   # ★ 매일 자동 생성되는 YYYY_MM_DD.md
│   └── README.md
└── .github/workflows/
    └── daily-cpp.yml        # GitHub Actions — 매일 KST 09:00 실행
```

---

## 3. 빠르게 시작하기

### 3-1. 사전 요구사항

- Docker (20.10+) 와 Docker Compose
- 사용 가능한 포트: **8888** (JupyterLab), **9000**, **9001** (네트워크/캡스톤 실습)

### 3-2. 빌드 & 실행

```bash
cd cpp_learning
docker compose up --build
```

처음 빌드는 5–10분 정도 (conda 패키지가 무겁다). 이후엔 캐시되어 즉시.

### 3-3. JupyterLab 접속

브라우저에서:

```
http://localhost:8888
```

토큰/비밀번호는 **꺼져 있다** (학습 환경 전용). `notebooks/` 디렉토리가 보일 것이다. `01_introduction.ipynb` 부터 차례로 열자.

### 3-4. 종료

다른 터미널에서:

```bash
docker compose down
```

---

## 4. 학습 순서와 예상 시간

| # | 강의 | 핵심 주제 | 분량 |
|---|------|-----------|------|
| 01 | 첫걸음 | 컴파일/링크, Hello World, 변수, 함수, auto, 범위 기반 for | ≈ 60분 |
| 02 | 메모리 모델 | Stack/Heap/BSS, 포인터/참조, new/delete, vector 재할당, ASan | ≈ 90분 |
| 03 | 클래스 / RAII / 스마트 포인터 | 생성자, Rule of Five/Zero, move 시맨틱, unique_ptr / shared_ptr / weak_ptr | ≈ 120분 |
| 04 | 헷갈리는 문법 | const 다섯 자리, 초기화 다섯 얼굴, most vexing parse, 캐스트 4종, 람다 캡처 | ≈ 90분 |
| 05 | 템플릿 / 제네릭 | 함수·클래스 템플릿, NTTP, 특수화, SFINAE, fold expression, 표준 라이브러리 시그니처 읽는 법 | ≈ 100분 |
| 06 | C++20 | concepts, ranges, three-way comparison, std::format, modules·coroutines 개요 | ≈ 90분 |
| 07 | C++23 | std::expected, deducing this, 재귀 람다, std::println, ranges::zip / enumerate, std::stacktrace | ≈ 75분 |
| 08 | 동시성 | thread / jthread, mutex, atomic, memory_order, false sharing 벤치마크, 데드락 4규칙 | ≈ 100분 |
| 09 | 네트워크 | OSI 모델, TCP 핸드셰이크, POSIX socket, echo 서버/클라이언트, tcpdump 캡처 | ≈ 110분 |
| 10 | 성능 최적화 | 캐시 메모리, 분기 예측, -O0/-O2/-O3, RVO/NRVO, constexpr, UB, 프로파일링 | ≈ 90분 |
| 11 | 종합 프로젝트 | KV 저장소 + HTTP 서버 (모든 도구 통합), curl 로 검증, 처리량 측정 | ≈ 120분 |

**합계 ≈ 18–20시간.** 강의 안의 실습 코드를 직접 손으로 따라 치면 두 배 잡으면 된다.

순서를 권장하는 이유: 각 강의는 *바로 앞* 강의의 내용을 가정한다. 캡스톤(11)은 1–10 의 모든 도구를 합친다.

---

## 5. 노트북이 동작하는 방식

### 5-1. xeus-cling (강의 01–05)

C++17 코드를 **`int main()` 없이** 인터프리터처럼 실행하는 Jupyter 커널이다. 변수와 클래스를 셀 단위로 만들고 즉시 결과를 볼 수 있어 학습에 최적이다.

```cpp
// 셀 1
int x = 42;
```
```cpp
// 셀 2
std::cout << x;          // 42
```

### 5-2. g++ 직접 호출 (강의 06–11)

xeus-cling 은 C++17 까지만 안정 지원한다. C++20 (`concepts`, `ranges`, `coroutines`) 와 C++23 (`std::expected`, deducing this) 은 셀 안에서 **코드 문자열을 g++ 로 빌드·실행** 하는 패턴을 쓴다:

```cpp
run_cpp(R"CPP(
#include <concepts>
template <std::integral T> T add(T a, T b){ return a+b; }
int main(){ std::cout << add(3,5); }
)CPP", "-O2");
```

각 강의 첫 셀에 `run_cpp` 헬퍼가 정의되어 있다. 그대로 실행하면 된다.

### 5-3. 컨테이너에 들어가 직접 빌드해 보고 싶다면

```bash
docker exec -it cpp-learning bash
```

작업 디렉토리는 `/workspace`. `g++`, `clang++`, `gdb`, `valgrind`, `tcpdump`, `nc`, `perf` 모두 설치돼 있다. `check-cpp` 명령으로 C++17/20/23 지원 상태를 한눈에 볼 수 있다.

---

## 6. 컨테이너에 포함된 도구

| 카테고리 | 도구 |
|----------|------|
| 컴파일러 | gcc-12 / g++-12, clang-14 |
| 빌드 | cmake, ninja, make |
| 디버깅 | gdb, lldb, strace, ltrace |
| 메모리 | valgrind, AddressSanitizer / UBSanitizer (컴파일러 내장) |
| 성능 | perf (linux-perf), callgrind |
| 네트워크 | tcpdump, netcat-openbsd, curl, ss, iproute2 |
| 라이브러리 | nlohmann_json (헤더 온리) |
| Jupyter | jupyterlab, xeus-cling (C++17 커널) |

---

## 7. 트러블슈팅

### "The kernel xcpp17 is not available"
컨테이너를 재빌드하라: `docker compose up --build --force-recreate`.

### `run_cpp` 셀에서 `error: 'std::expected' is not a member`
GCC 12 는 C++23 의 일부 기능만 지원한다. 강의 07 의 일부 예제는 GCC 13+ 가 필요할 수 있다. `Dockerfile` 의 `gcc-12` 를 `gcc-13` 으로 바꾸고 재빌드하면 된다 (Ubuntu 24.04 LTS 기반에선 가능).

### gdb 가 "Operation not permitted" 로 실패
docker-compose.yml 에 다음이 들어가 있어야 한다 (이미 들어 있다):
```yaml
cap_add:
  - SYS_PTRACE
security_opt:
  - seccomp:unconfined
```

### tcpdump 가 "no permission" 으로 실패
같은 이유. 위 설정으로 해결된다.

### 강의 09 의 echo 서버가 "Address already in use"
이전에 띄운 서버가 살아 있을 수 있다. 셀에서:
```cpp
std::system("pkill -f /tmp/cpplab/net/echo_server");
```

### 강의 11 의 KV 서버가 9000 포트에서 "bind: Permission denied"
포트가 이미 사용 중이거나 컨테이너 외부에서 점유. 노트북 안의 코드는 0.0.0.0:9000 으로 들어 가지만, 호스트에서도 9000 을 쓰고 있으면 docker-compose.yml 의 포트 매핑(`9000:9000`)을 바꿔라.

---

## 8. 노트북을 수정/확장하고 싶다면

빌더 스크립트가 `_build/lessonNN.py` 에 있다. 노트북을 직접 수정해도 되지만, 공유/재생성을 원한다면:

```bash
docker exec -it cpp-learning bash
cd /workspace/_build      # (호스트에서 마운트하면)
python lesson05.py        # 해당 강의만 재빌드
```

또는 호스트에서:
```bash
cd cpp_learning/_build
python lesson05.py
```

이러려면 호스트에 Python 3.10+ 만 있으면 된다 (외부 패키지 의존 없음).

---

## 9. 참고할 자료

- **cppreference.com** — 사실상의 공식 레퍼런스. 강의 05 이후로는 매번 펼치고 살게 된다.
- **C++ Core Guidelines** (Stroustrup, Sutter) — 실무 패턴. 강의 03/04 가 자주 인용한다.
- **컴파일러 익스플로러** — https://godbolt.org — 어셈블리를 보고 싶을 때.
- **Compiler Explorer + Quick C++ Benchmark** — https://quick-bench.com — 강의 10 의 측정을 더 정밀하게.

---

## 10. 매일 자동 생성되는 학습 노트 (`daily/`)

GitHub Actions 가 **매일 KST 09:00** 에 Gemini API 를 호출해 한 편의 C++ 학습 노트를 `daily/YYYY_MM_DD.md` 로 만든다. 주제는 최신 표준(C++23/26), cppreference 예제 해부, 표준 라이브러리 내부 구조, 성능 등을 일자별로 결정적으로 로테이션한다.

### 10-1. 1회만 설정하면 끝

저장소 **Settings → Secrets and variables → Actions** 에서:

#### Secrets (필수)

| Name | Value |
|------|-------|
| `GEMINI_API_KEY` | Google AI Studio 에서 발급한 API 키 |

API 키 발급: https://aistudio.google.com/app/apikey

#### Variables (선택 — 지정하지 않으면 기본값 `gemini-2.5-flash` 사용)

| Name | 예시 |
|------|------|
| `GEMINI_MODEL` | `gemini-2.5-flash`, `gemini-2.5-pro`, `gemini-2.0-flash` 등 |

> Secrets 와 Variables 의 차이 — Secrets 는 마스킹되어 로그에 노출되지 않고, Variables 는 일반 설정 값(모델명처럼 비밀이 아닌 것)에 쓴다.

### 10-2. 모델 선택 우선순위

`daily-cpp.yml` 워크플로가 모델을 결정하는 순서:

1. **`workflow_dispatch` 입력** — Actions 탭에서 수동 실행 시 직접 지정한 값 (1회용)
2. **Repository Variable `GEMINI_MODEL`** — 영구 설정
3. **기본값** — `gemini-2.5-flash`

### 10-3. 수동으로 한 편 만들기

저장소의 **Actions 탭 → Daily C++ Learning Note → Run workflow**:

- `model` 을 비워 두면 위의 우선순위대로 자동 결정.
- `topic` 에 문장을 넣으면 그 주제로 강제 생성 (예: `std::expected 의 monadic 인터페이스 실전`).

### 10-4. 로컬에서 직접 돌리기

```bash
export GEMINI_API_KEY="<key>"
export GEMINI_MODEL="gemini-2.5-pro"             # 선택
python scripts/daily_cpp.py
```

`daily/2026_04_29.md` 같은 파일이 생긴다. 같은 날 다시 만들고 싶으면 그 파일을 먼저 지워라 (스크립트는 *덮어쓰지 않는다*).

### 10-5. 트러블슈팅

| 증상 | 원인 / 해결 |
|------|-------------|
| `GEMINI_API_KEY secret 이 설정되어 있지 않다` | Settings → Secrets 에서 `GEMINI_API_KEY` 등록 |
| `HTTP 400 ... API key not valid` | 키 만료 또는 오타. AI Studio 에서 재발급 |
| `HTTP 429` (Resource exhausted) | 무료 quota 초과. 다음 날까지 기다리거나 Pro 모델 결제 |
| `HTTP 404 ... model not found` | 존재하지 않는 모델명. 위의 예시 중에서 골라라 |
| 워크플로가 push 에서 실패 | Settings → Actions → General → "Workflow permissions" 를 **Read and write** 로 |

---

## 11. 라이선스

학습 자료. 자유롭게 수정/배포하라. 코드는 모두 표준 C++ 만 사용해서 어떤 환경에든 옮겨갈 수 있다.

행운을 빈다. C++ 는 처음 한 달이 가장 가파르고, 그 뒤로는 *예상 가능한* 산이다.
