# 11. 모르는 항목을 발견했을 때

이 문서는 “자료에 없는 용어가 나오면 보완한다”는 반복 절차입니다. C++ 전체를 유한한 문서에
완전히 담을 수는 없으므로, 누락을 숨기지 않고 발견·연결·검증 가능하게 만드는 것이 목표입니다.

## 7단계

1. **문맥을 보존한다.** 이해 안 되는 선언/문장 한 줄과 페이지 URL, 표준 버전을 적습니다.
2. **분류한다.** 기호, 언어 문법, 라이브러리 타입, named requirement, 메모리 모델,
   구현/플랫폼 용어 중 무엇인지 정합니다.
3. **토큰으로 쪼갠다.** `::`, `<T>`, `()`, 후행 한정자처럼 이미 아는 부분을 제거합니다.
4. **내부 색인을 찾는다.** [용어·기호 색인](10-glossary.md)과 README 문서 지도를 봅니다.
5. **cppreference 상위 페이지를 읽는다.** 멤버보다 클래스, 함수보다 헤더/라이브러리 개요를
   먼저 읽습니다.
6. **최소 실험을 만든다.** 한 개념만 포함한 10~30줄 코드를 만들고 정상/실패 경우를 비교합니다.
7. **이 자료를 보완한다.** 정의, 반례, 최소 코드, 원문 링크를 넣고 색인과 역링크를 추가합니다.

## 보완 항목 템플릿

```markdown
### 용어

한 문장 정의:

아닌 것/자주 하는 오해:

최소 선언 또는 코드:

선행 개념:

수명·예외·복잡도·동시성 주의점:

cppreference:

연결 실습:
```

## 페이지 감사(audit) 체크리스트

새 cppreference 페이지를 공부한 뒤 모두 체크합니다.

- [ ] 제목의 모든 `::`, `<...>`를 설명할 수 있다.
- [ ] 모든 overload 선언을 반환/이름/인자/후행 한정자로 분리했다.
- [ ] attribute와 `(since/until/deprecated)`를 확인했다.
- [ ] 생략 가능한 인자와 기본값을 확인했다.
- [ ] template parameter와 deduction 조건을 확인했다.
- [ ] named requirement/concept의 의미론까지 확인했다.
- [ ] precondition과 UB 조건을 기록했다.
- [ ] 반환 객체의 소유권과 수명을 확인했다.
- [ ] iterator/reference invalidation을 확인했다.
- [ ] complexity를 입력 크기 N과 연결했다.
- [ ] exception guarantee를 확인했다.
- [ ] thread-safety와 atomic ordering을 확인했다.
- [ ] 구현 정의/미지정/플랫폼 의존 결과를 구분했다.
- [ ] Example의 가능한 출력과 보장된 출력을 구분했다.
- [ ] See also의 유사 API와 선택 기준을 한 줄로 적었다.
- [ ] C++17/20 중 어느 표준으로 컴파일 가능한지 확인했다.

## 이번 자료의 범위 경계

이 과정은 C++17/20 참고 문서를 읽는 데 필요한 공통 기반을 제공합니다. 다음은 이름과
진입점은 다루지만 각각 독립 과정이 필요한 심화 분야입니다.

- object representation, strict aliasing, placement new, `std::launder`
- allocator 구현, `pmr`
- lock-free 자료구조와 safe memory reclamation
- template metaprogramming/SFINAE의 모든 corner case
- coroutine promise/awaiter 직접 구현
- module build graph와 compiler별 BMI
- locale, codecvt 대체, Unicode text 처리
- networking/graphics/OS API처럼 C++ 표준 밖의 영역
- ABI, calling convention, linker script, shared library 배포

이 중 하나가 실제 읽는 페이지에 등장하면 위 템플릿으로 새 심화 문서를 추가하고
[용어 색인](10-glossary.md)과 해당 버전 가이드 양쪽에 링크합니다.
