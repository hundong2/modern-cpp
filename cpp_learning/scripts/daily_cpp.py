#!/usr/bin/env python3
"""
daily_cpp.py — Gemini API 로 매일 한 편의 C++ 학습 가이드를 생성한다.

환경 변수
---------
GEMINI_API_KEY   (필수)  Google AI Studio 에서 발급
GEMINI_MODEL     (선택)  기본값 'gemini-2.5-flash'. 'gemini-2.5-pro' 등 자유 선택
DAILY_TOPIC_HINT (선택)  특정 주제 강제. 비우면 자동 로테이션
DAILY_OUT_DIR    (선택)  기본값 './daily'

출력
----
{DAILY_OUT_DIR}/YYYY_MM_DD.md  (이미 존재하면 그대로 두고 종료)

의존성
------
표준 라이브러리만 사용 (urllib + json). pip install 없음.
"""

from __future__ import annotations

import json
import os
import random
import sys
import urllib.error
import urllib.request
from datetime import datetime, timezone, timedelta
from pathlib import Path
from typing import Any


# ──────────────────────────────────────────────────────────────────────
# 주제 풀 — 매일 다른 학습 자료가 나오게 한다.
# DAILY_TOPIC_HINT 가 비어 있을 때 KST 의 요일 + 일자 해시로 결정적으로 선택.
# ──────────────────────────────────────────────────────────────────────
TOPIC_BUCKETS: list[dict[str, Any]] = [
    {
        "category": "최신 C++ 기술 동향",
        "examples": [
            "C++26 표준 위원회의 최신 제안서 중 흥미로운 것 하나를 골라 해체",
            "C++23 std::expected 의 모나딕 인터페이스 실전 활용",
            "C++26 reflection (P2996) 의 컴파일타임 메타프로그래밍 가능성",
            "C++26 std::execution (P2300) 비동기 실행 모델",
            "Contracts (C++26 도입 예정) 의 동작과 활용",
            "Pattern matching 제안서 (P2688) 의 현재 상태와 사용법",
            "C++23 deducing this 가 CRTP 를 어떻게 대체하는가",
            "std::generator (C++23) 와 코루틴 기반 lazy 시퀀스",
            "std::flat_map / std::flat_set (C++23) 의 캐시 친화 설계",
            "std::mdspan (C++23) 다차원 뷰의 stride 연산",
        ],
    },
    {
        "category": "cppreference.com 예제 해체",
        "examples": [
            "std::ranges::views::join 예제를 한 줄씩 분해",
            "std::shared_ptr 의 aliasing constructor 예제 분석",
            "std::variant + std::visit 의 표준 예제 해부",
            "std::async / std::future 의 동작 모델을 예제로",
            "std::unique_lock 과 std::condition_variable 의 wait/notify 예제",
            "std::atomic_ref (C++20) 예제로 보는 외부 메모리에 atomic 적용",
            "std::ranges::views::chunk_by (C++23) 사용 예",
            "std::format 의 사용자 정의 formatter 작성",
            "std::source_location 으로 로깅 매크로 대체하기",
            "std::span 으로 함수 시그니처 단순화",
            "std::stop_token 과 std::jthread 의 협력적 취소",
            "std::barrier / std::latch (C++20) 의 사용 사례",
        ],
    },
    {
        "category": "표준 라이브러리 깊이 파기",
        "examples": [
            "std::vector 의 small buffer optimization 가능성과 표준 보장",
            "std::string SSO (Short String Optimization) 의 구현 분석",
            "std::unordered_map 의 버킷 구조와 충돌 해결",
            "std::sort 가 introsort 인 이유 — 최악 보장",
            "std::move_iterator 가 정확히 무엇을 하는가",
            "std::reference_wrapper 의 존재 이유와 사용처",
            "std::pmr (polymorphic memory resource) 의 활용",
            "std::weak_ptr 의 lock() 이 thread-safe 한 이유",
        ],
    },
    {
        "category": "성능과 시스템",
        "examples": [
            "false sharing 측정과 alignas(64) 효과 — 직접 벤치마크",
            "캐시 라인 패딩이 atomic counter 에 미치는 영향",
            "std::vector vs std::deque vs std::list 의 캐시 동작 비교",
            "branch prediction 과 likely/unlikely (C++20 [[likely]])",
            "RVO/NRVO 가 막히는 패턴과 그 회피",
            "constexpr 알고리즘으로 룩업 테이블 컴파일타임 생성",
            "SIMD 자동 벡터화가 일어나는 조건과 -fopt-info 로 확인",
            "PGO (Profile-Guided Optimization) 적용 워크플로",
        ],
    },
]


def pick_topic(seed_date: datetime) -> tuple[str, str]:
    """KST 일자를 시드로 카테고리/주제를 결정적으로 선택."""
    rng = random.Random(seed_date.strftime("%Y%m%d"))
    bucket = rng.choice(TOPIC_BUCKETS)
    return bucket["category"], rng.choice(bucket["examples"])


# ──────────────────────────────────────────────────────────────────────
# 프롬프트 빌더
# ──────────────────────────────────────────────────────────────────────
SYSTEM_PROMPT = """\
당신은 C++ 교육 자료를 만드는 시니어 C++ 엔지니어다. 다음 원칙을 지켜라.

1. **한국어로 작성**한다. 평어체("~다", "~라"). 친근하지만 전문가의 톤.
2. **출력은 순수한 마크다운 본문만**. 머리에 ```markdown 같은 펜스를 붙이지 않는다. 끝에도 펜스 없음.
3. 모든 코드는 ```cpp 블록. **컴파일 가능한 완결된 예제**여야 한다 (필요한 헤더, main 포함).
4. 단순 나열이 아니라 *왜 이렇게 됐는지* 의 동기와 trade-off 를 설명한다.
5. 헷갈리는 함정 (UB, 표준 미보장, 컴파일러별 차이)을 적극적으로 짚는다.
6. 사실 관계가 불확실한 부분은 "표준에 명시되어 있지 않다" 같이 솔직하게 표시.
7. 사용자가 직접 손으로 따라 칠 수 있도록 짧은 실습 문제를 끝에 1–2개 둔다.
8. 길이는 800–1500 단어. 코드는 그 안에 포함된 분량.
"""

USER_PROMPT_TEMPLATE = """\
오늘의 주제 (카테고리: {category}):

> {topic}

이 주제로 학습 가이드 마크다운을 작성하라. 다음 구조를 권장하지만 주제에 맞게 자연스럽게 조정해도 된다.

# {topic}

> 한 문단의 도입 — 이 글이 풀려는 문제와 독자가 얻을 것.

## 1. 배경 — 왜 이 기능/개념이 필요한가
(이 기능이 없던 시절의 불편함, 표준화 동기)

## 2. 핵심 개념
(짧고 명확한 설명. 필요하면 비교표.)

## 3. 코드로 보기
```cpp
// 컴파일 가능한 완결 예제 1
```
(예제에 대한 설명)

```cpp
// 변형 또는 확장 예제
```

## 4. 함정과 주의사항
(UB, 컴파일러별 차이, 잘못 쓰는 패턴)

## 5. 정리
(짧게 한 문단.)

## 실습
(독자가 손으로 따라 할 짧은 문제 1–2개)

---

작성 시 주의:
- **출력의 첫 글자는 `#` 이어야 한다 (마크다운 헤딩).**
- 코드는 `g++ -std=c++20 file.cpp` 정도로 빌드되어야 한다 (특별한 라이브러리 의존 없이).
- 표준이 모호한 부분은 추측하지 말고 그렇다고 명시하라.
"""


def build_prompt(category: str, topic: str) -> tuple[str, str]:
    return SYSTEM_PROMPT, USER_PROMPT_TEMPLATE.format(category=category, topic=topic)


# ──────────────────────────────────────────────────────────────────────
# Gemini API 호출
# ──────────────────────────────────────────────────────────────────────
GEMINI_BASE = "https://generativelanguage.googleapis.com/v1beta/models"


def call_gemini(model: str, api_key: str, system: str, user: str,
                timeout_sec: int = 120) -> str:
    """Gemini generateContent 호출. 응답에서 텍스트만 뽑아 반환."""
    url = f"{GEMINI_BASE}/{model}:generateContent"
    payload = {
        "systemInstruction": {"parts": [{"text": system}]},
        "contents": [
            {"role": "user", "parts": [{"text": user}]}
        ],
        "generationConfig": {
            "temperature": 0.7,
            "topP": 0.95,
            "maxOutputTokens": 8192,
        },
    }
    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(
        url,
        data=data,
        method="POST",
        headers={
            "Content-Type": "application/json",
            "x-goog-api-key": api_key,
        },
    )
    try:
        with urllib.request.urlopen(req, timeout=timeout_sec) as resp:
            body = resp.read().decode("utf-8")
    except urllib.error.HTTPError as e:
        err_body = e.read().decode("utf-8", errors="replace")
        raise SystemExit(
            f"[ERROR] Gemini API HTTP {e.code}: {e.reason}\n{err_body}"
        ) from e
    except urllib.error.URLError as e:
        raise SystemExit(f"[ERROR] Gemini API 네트워크 실패: {e.reason}") from e

    obj = json.loads(body)
    candidates = obj.get("candidates") or []
    if not candidates:
        raise SystemExit(f"[ERROR] 응답에 candidates 가 없다.\n{json.dumps(obj, ensure_ascii=False, indent=2)[:2000]}")

    cand = candidates[0]
    finish_reason = cand.get("finishReason", "UNKNOWN")
    parts = (cand.get("content") or {}).get("parts") or []
    texts = [p.get("text", "") for p in parts if "text" in p]
    text = "".join(texts).strip()
    if not text:
        raise SystemExit(
            f"[ERROR] 응답 텍스트가 비어 있다. finishReason={finish_reason}\n"
            f"{json.dumps(obj, ensure_ascii=False, indent=2)[:2000]}"
        )
    if finish_reason not in ("STOP", "MAX_TOKENS"):
        # MAX_TOKENS 도 허용 — 문서가 잘릴 수 있지만 사용 가능
        print(f"[WARN] finishReason={finish_reason}", file=sys.stderr)
    return text


# ──────────────────────────────────────────────────────────────────────
# 후처리 — 모델이 펜스로 감싸는 경우 제거
# ──────────────────────────────────────────────────────────────────────
def strip_outer_fence(text: str) -> str:
    t = text.strip()
    if t.startswith("```"):
        # 첫 줄 제거
        first_nl = t.find("\n")
        if first_nl >= 0:
            t = t[first_nl + 1:]
        if t.rstrip().endswith("```"):
            t = t.rstrip()[: -3].rstrip()
    return t


# ──────────────────────────────────────────────────────────────────────
# main
# ──────────────────────────────────────────────────────────────────────
def main() -> int:
    api_key = os.environ.get("GEMINI_API_KEY", "").strip()
    if not api_key:
        print("[ERROR] GEMINI_API_KEY 환경 변수가 비어 있다.", file=sys.stderr)
        return 2

    model = os.environ.get("GEMINI_MODEL", "gemini-2.5-flash").strip() or "gemini-2.5-flash"
    out_dir = Path(os.environ.get("DAILY_OUT_DIR", "daily")).resolve()
    topic_hint = os.environ.get("DAILY_TOPIC_HINT", "").strip()

    # KST 기준 오늘 날짜
    kst = timezone(timedelta(hours=9))
    today = datetime.now(kst)
    fname = today.strftime("%Y_%m_%d") + ".md"
    out_path = out_dir / fname

    out_dir.mkdir(parents=True, exist_ok=True)

    if out_path.exists():
        print(f"[SKIP] 이미 존재: {out_path}")
        return 0

    if topic_hint:
        category = "사용자 지정"
        topic = topic_hint
    else:
        category, topic = pick_topic(today)

    print(f"[INFO] model    = {model}")
    print(f"[INFO] date     = {today.strftime('%Y-%m-%d %H:%M %Z')}")
    print(f"[INFO] category = {category}")
    print(f"[INFO] topic    = {topic}")
    print(f"[INFO] out_path = {out_path}")

    system, user = build_prompt(category, topic)
    text = call_gemini(model, api_key, system, user)
    text = strip_outer_fence(text)

    # 메타데이터 헤더(YAML front matter) — 정적 사이트에서도 활용 가능
    front_matter = (
        "---\n"
        f"date: {today.strftime('%Y-%m-%d')}\n"
        f"category: {category}\n"
        f"topic: {json.dumps(topic, ensure_ascii=False)}\n"
        f"model: {model}\n"
        f"generated_at: {today.isoformat(timespec='seconds')}\n"
        "---\n\n"
    )

    out_path.write_text(front_matter + text + "\n", encoding="utf-8")
    print(f"[OK] {out_path}  ({len(text):,} chars)")

    # GitHub Actions 의 후속 step 에서 쓸 수 있도록 출력 변수 설정
    gh_out = os.environ.get("GITHUB_OUTPUT")
    if gh_out:
        with open(gh_out, "a", encoding="utf-8") as f:
            f.write(f"file={out_path}\n")
            f.write(f"topic={topic}\n")
            f.write(f"category={category}\n")

    return 0


if __name__ == "__main__":
    sys.exit(main())
