# 빌드 파이프라인

[전처리기](README.md)는 `#include`, 조건부 컴파일, 매크로를 텍스트 수준에서 처리한다. 컴파일러는 각 `.cpp` 번역 단위를 기계어 목적 파일로 바꾼다. 링커는 목적 파일의 미해결 심볼을 다른 목적 파일/라이브러리 정의와 연결한다. 헤더에 non-`inline` 함수 정의를 여러 번 넣으면 [ODR(One Definition Rule)](https://en.cppreference.com/w/cpp/language/definition)을 어길 수 있다.

`inline`의 언어적 핵심은 “호출 코드를 반드시 펼쳐라”가 아니라 여러 번 동일하게 정의할 수 있게 하는 ODR 규칙이다. 실제 인라이닝 최적화는 컴파일러 판단이며 `inline` 없이도 일어난다.
