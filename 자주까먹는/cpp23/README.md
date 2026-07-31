# C++23

- [01. expected](#1-expected)

## 빌드 및 실행

`자주까먹는` 폴더에서 실행할 `.cpp` 경로를 `run.sh`에 전달합니다. 각 파일에는
독립적인 `main()` 함수가 있어야 합니다.

```bash
cd 자주까먹는
./run.sh cpp23/main.cpp
./run.sh cpp23/expected.cpp
```

새 `.cpp` 파일을 이 폴더에 추가하면 CMake가 파일 이름과 같은 실행 타깃을 자동으로
만듭니다. 예를 들어 `optional.cpp`는 `optional` 타깃과
`build/cpp23/optional` 실행 파일을 만듭니다.

## 1. expected
