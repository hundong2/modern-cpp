# `daily/` — 자동 생성 학습 노트

이 디렉토리는 **GitHub Actions + Gemini API** 가 매일 한 편씩 채운다.

- 파일명 형식: `YYYY_MM_DD.md` (KST 기준)
- 워크플로 정의: `.github/workflows/daily-cpp.yml`
- 생성 스크립트: `scripts/daily_cpp.py`
- 실행 시각: 매일 KST 09:00 (UTC 00:00)

## 어떻게 동작하나

매일 자정(UTC) 워크플로가 발동하면:

1. 그날의 KST 일자를 시드로 해서 주제 풀에서 *결정적으로* 한 주제를 뽑는다 (같은 날 재실행해도 같은 주제).
2. Gemini API 의 `generateContent` 엔드포인트에 한국어 시스템 프롬프트 + 주제별 사용자 프롬프트를 보낸다.
3. 응답 마크다운을 `daily/YYYY_MM_DD.md` 로 저장하고 자동 커밋·푸시한다.
4. 같은 날짜 파일이 이미 있으면 **건너뛴다** (덮어쓰지 않음).

## 주제 카테고리

`scripts/daily_cpp.py` 의 `TOPIC_BUCKETS` 에 정의되어 있다:

- 최신 C++ 기술 동향 (C++23/26 신기능, 위원회 제안서)
- cppreference.com 예제 해체 (표준 라이브러리 예제 분석)
- 표준 라이브러리 깊이 파기 (vector SSO, sort 의 introsort 보장 등)
- 성능과 시스템 (false sharing, RVO, SIMD 등)

주제를 추가/수정하고 싶으면 `scripts/daily_cpp.py` 의 리스트에 직접 추가하면 된다.

## 수동으로 한 편 생성하기

GitHub 웹에서 **Actions 탭 → "Daily C++ Learning Note" → Run workflow**:

- `model` 입력으로 모델을 임시 변경 (예: `gemini-2.5-pro`)
- `topic` 입력으로 주제 강제 지정 (비우면 그날의 자동 로테이션 사용)

## 로컬에서 직접 돌려 보기

저장소 루트에서:

```bash
export GEMINI_API_KEY="<your-key>"
export GEMINI_MODEL="gemini-2.5-flash"        # 선택
export DAILY_TOPIC_HINT="std::span 으로 함수 시그니처 단순화"   # 선택
python scripts/daily_cpp.py
```

생성된 파일은 `daily/YYYY_MM_DD.md`. 같은 날 다시 실행하려면 그 파일을 먼저 지워야 한다.
