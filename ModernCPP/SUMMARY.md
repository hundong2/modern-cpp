# ModernCPP 프로젝트 문서 요약

## 📚 문서 구조

### 시작하기
1. **[QUICKSTART.md](QUICKSTART.md)** - ⚡ 5분 빠른 시작
   - 환경 확인
   - 첫 빌드
   - 예제 실행

2. **[INSTALL.md](INSTALL.md)** - 🔧 설치 가이드
   - macOS 설치
   - Linux 설치
   - Docker 사용
   - 문제 해결

### 메인 가이드
3. **[README.md](README.md)** - 📖 전체 학습 가이드
   - 프로젝트 소개
   - 기본 사용법
   - 학습 로드맵
   - 간단한 확장 방법

### 상세 가이드
4. **[BUILD.md](BUILD.md)** - 🔨 빌드 시스템 완전 가이드
   - Makefile 구조 이해
   - 빌드 방법 (상세)
   - 새 파일 추가 (4가지 시나리오)
   - 라이브러리 추가 (Boost, 외부 라이브러리)
   - 컴파일 옵션 커스터마이징
   - 고급 빌드 기법
   - **👉 확장 시 필수 참고 문서**

### 개념별 가이드
5. **[docs/concepts_guide.md](docs/concepts_guide.md)** - 💡 Concepts 상세
6. **[docs/coroutine_guide.md](docs/coroutine_guide.md)** - ⚙️ Coroutines 상세
7. **[docs/setup_troubleshooting.md](docs/setup_troubleshooting.md)** - 🩹 문제 해결

## 🎯 상황별 추천 문서

### "빠르게 시작하고 싶어요"
→ [QUICKSTART.md](QUICKSTART.md)

### "설치부터 천천히 하고 싶어요"
→ [INSTALL.md](INSTALL.md) → [README.md](README.md)

### "새 예제를 추가하고 싶어요"
→ [BUILD.md - 시나리오 1](BUILD.md#시나리오-1-새-예제-추가-가장-간단)

### "공통 유틸리티를 만들고 싶어요"
→ [BUILD.md - 시나리오 2](BUILD.md#시나리오-2-공통-유틸리티-추가)

### "서버에 기능을 추가하고 싶어요"
→ [BUILD.md - 시나리오 3](BUILD.md#시나리오-3-서버에-새-기능-추가)

### "외부 라이브러리를 사용하고 싶어요"
→ [BUILD.md - 라이브러리 추가](BUILD.md#라이브러리-추가하기)

### "컴파일 옵션을 바꾸고 싶어요"
→ [BUILD.md - 컴파일 옵션](BUILD.md#컴파일-옵션-커스터마이징)

### "Concepts를 깊이 이해하고 싶어요"
→ [docs/concepts_guide.md](docs/concepts_guide.md)

### "Coroutines를 깊이 이해하고 싶어요"
→ [docs/coroutine_guide.md](docs/coroutine_guide.md)

### "빌드가 안 돼요"
→ [docs/setup_troubleshooting.md](docs/setup_troubleshooting.md)

## 📊 문서별 특징

| 문서 | 길이 | 난이도 | 목적 |
|------|------|--------|------|
| QUICKSTART.md | 짧음 | ⭐ 쉬움 | 즉시 시작 |
| INSTALL.md | 중간 | ⭐⭐ 보통 | 설치 완료 |
| README.md | 김 | ⭐⭐ 보통 | 전체 이해 |
| BUILD.md | 매우 김 | ⭐⭐⭐ 고급 | 완전 활용 |
| concepts_guide.md | 중간 | ⭐⭐⭐ 고급 | 개념 마스터 |
| coroutine_guide.md | 중간 | ⭐⭐⭐⭐ 전문가 | 개념 마스터 |

## 💡 추천 학습 순서

### 초보자
1. QUICKSTART.md
2. 예제 실행해보기
3. README.md 학습 가이드 섹션
4. 간단한 예제 수정

### 중급자
1. README.md 전체
2. BUILD.md 시나리오 1-2
3. concepts_guide.md
4. 새 예제 추가 실습

### 고급자
1. BUILD.md 전체
2. coroutine_guide.md
3. 서버/클라이언트 확장
4. 외부 라이브러리 통합

## 🔍 키워드 검색

### 빌드 관련
- **자동 빌드**: BUILD.md - Makefile 구조
- **선택적 빌드**: README.md - 빌드 방법
- **디버그 빌드**: BUILD.md - 컴파일 옵션

### 파일 추가
- **예제 추가**: BUILD.md - 시나리오 1
- **유틸리티 추가**: BUILD.md - 시나리오 2
- **멀티파일 모듈**: BUILD.md - 시나리오 4

### 라이브러리
- **Boost 추가**: BUILD.md - Boost 라이브러리
- **외부 라이브러리**: BUILD.md - 외부 라이브러리 추가
- **헤더 온리**: BUILD.md - nlohmann-json 예제

### C++20 기능
- **Concepts**: README.md 학습 가이드, concepts_guide.md
- **Ranges**: README.md 학습 가이드
- **Coroutines**: README.md 학습 가이드, coroutine_guide.md

## 📞 도움 받기

1. **설치 문제**: setup_troubleshooting.md
2. **빌드 문제**: BUILD.md 마지막 섹션
3. **개념 이해**: 각 개념별 가이드
4. **기타 질문**: README.md 마지막 섹션

---

**시작하기**: [QUICKSTART.md](QUICKSTART.md)를 열어 5분 안에 프로젝트를 실행해보세요!
