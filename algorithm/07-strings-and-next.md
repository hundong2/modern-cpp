# 07. 문자열 알고리즘과 심화 지도

## 문자열 기초

문자열 문제에서는 먼저 문자 인코딩과 인덱스 단위를 확인한다. C++ `std::string`과 Python UTF-8 바이트열, C# `string`의 UTF-16 코드 단위는 “사용자가 보는 문자”와 항상 일치하지 않는다. ASCII만 전제하는 코드를 유니코드 입력에 그대로 적용하지 않는다.

## 패턴 검색

| 방법 | 시간 | 언제 쓰나 |
|---|---:|---|
| 단순 비교 | 최악 `O(nm)` | 입력이 작고 구현 단순성이 중요 |
| KMP | `O(n+m)` | 한 패턴을 최악 시간 보장으로 검색 |
| Z 알고리즘 | `O(n)` | 접두사 일치 길이를 모두 계산 |
| Rabin–Karp | 평균 선형 | 해시로 여러 후보를 빠르게 거름; 충돌 확인 필요 |
| Trie | 삽입/검색 `O(L)` | 많은 단어의 접두사 질의 |
| Aho–Corasick | 텍스트+일치 수에 비례 | 여러 패턴을 동시에 검색 |

KMP(Knuth–Morris–Pratt)는 패턴의 접두사이면서 접미사인 길이를 저장해 불일치 때 텍스트 인덱스를 되돌리지 않는다. 상세 구현은 [저장소 KMP 노트](../dailystudy/exercise/algorithm/knuth-morris-pratt.md), Trie는 [접두사 Trie 노트](../dailystudy/exercise/algorithm/prefix-trie.md)를 참고한다.

## 빠뜨리지 말아야 할 다음 주제

기초 예제를 모두 설명할 수 있게 된 뒤 아래 순서로 확장한다.

1. **구간 질의**: Fenwick tree → segment tree → lazy propagation.
2. **트리 질의**: LCA, binary lifting, Euler tour.
3. **그래프 심화**: SCC, 이분 매칭, 최대 유량.
4. **고급 DP**: bitmask DP, digit DP, tree DP, 최적화.
5. **수학**: 유클리드 호제법, 소수 체, 빠른 거듭제곱, 모듈러 연산, 조합론.
6. **기하**: 방향 판정(CCW), 선분 교차, convex hull, sweep line.
7. **문자열 심화**: rolling hash, Aho–Corasick, suffix array/tree/automaton.

저장소에는 [Fenwick tree](../dailystudy/exercise/algorithm/fenwick-tree.md), [lazy segment tree](../dailystudy/exercise/algorithm/lazy-segment-tree.md), [SCC](../dailystudy/exercise/algorithm/strongly-connected-components-kosaraju.md), [최대 유량](../dailystudy/exercise/algorithm/maximum-flow-edmonds-karp.md), [LCA](../dailystudy/exercise/algorithm/binary-lifting-lca.md) 학습 노트가 있다.

## 최종 자기 점검

- BFS와 DFS의 결과가 달라지는 이유를 큐와 스택 관점에서 말할 수 있는가?
- 정렬된 배열에서 첫 번째 `target` 위치를 경계 오류 없이 찾을 수 있는가?
- 문제를 보고 상태와 전이를 정의해 DP인지 판별할 수 있는가?
- 다익스트라를 음수 간선에 쓰면 안 되는 이유를 반례로 설명할 수 있는가?
- MST와 두 정점 사이 최단 경로의 목적 차이를 말할 수 있는가?
- 각 선택의 시간·공간 복잡도와 실패 조건을 함께 말할 수 있는가?
