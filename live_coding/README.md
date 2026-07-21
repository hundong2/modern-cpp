# Live Coding C++ Algorithm Pack

라이브 코딩 테스트에서 자주 나오는 C++ 기초, 알고리즘 20개 챕터, 실무 코드 리딩 기초 3개 챕터를 정리한 C++17 예제 모음입니다.
각 `.cpp` 파일은 독립 실행 가능하며, 상단 요약뿐 아니라 핵심 코드 라인 옆에도 설명 주석을 배치했습니다.

## 사용법

```powershell
cd D:\workspace\modern-cpp
g++ -std=c++17 -O2 -Wall -Wextra live_coding\01_fast_io_and_complexity.cpp -o live_coding\chapter01.exe
.\live_coding\chapter01.exe
```

스레드 예제는 Linux/macOS에서 `-pthread`가 필요할 수 있습니다.

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -pthread live_coding\23_thread_optimization.cpp -o live_coding\chapter23.exe
```

소켓 예제는 C++ 표준 라이브러리가 아니라 OS API를 사용합니다. Windows MinGW에서는 `-lws2_32` 링크 옵션이 필요할 수 있습니다.

## 챕터

0. `00_cpp_syntax_basics.cpp` - C++ 라이브 코딩 문법, STL, 실수 방지 체크리스트
1. `01_fast_io_and_complexity.cpp` - 빠른 입출력, 복잡도 감각, 기본 루프 패턴
2. `02_prefix_sum_difference.cpp` - 1D/2D 누적합, 차분 배열, prefix hash
3. `03_two_pointers_sliding_window.cpp` - 투 포인터, 슬라이딩 윈도우
4. `04_sorting_binary_search.cpp` - 정렬, lower/upper bound, 매개변수 탐색
5. `05_hashing_and_counting.cpp` - 해시맵, 카운팅, 좌표 압축
6. `06_stack_queue_heap.cpp` - 스택, 큐, 덱, 힙
7. `07_recursion_backtracking.cpp` - 재귀, 백트래킹, pruning
8. `08_graph_bfs_dfs.cpp` - 그래프/격자 BFS, DFS
9. `09_topological_sort.cpp` - 위상 정렬, DAG DP
10. `10_shortest_path.cpp` - Dijkstra, Bellman-Ford, Floyd, 0-1 BFS
11. `11_union_find_mst.cpp` - Union-Find, Kruskal MST
12. `12_dp_basics.cpp` - 1차원 DP 기본 문제
13. `13_dp_grid_and_knapsack.cpp` - 격자 DP, 0/1 배낭, 무한 배낭
14. `14_dp_lis_bitmask.cpp` - LIS, Edit Distance, Bitmask DP
15. `15_greedy.cpp` - 회의실, 점프 게임, 허프만식 greedy
16. `16_tree_lca.cpp` - 트리 순회, 지름, LCA
17. `17_fenwick_segment_tree.cpp` - Fenwick Tree, Segment Tree, Lazy Propagation
18. `18_string_kmp_trie.cpp` - KMP, Trie, 문자열 탐색
19. `19_math_number_theory.cpp` - GCD, 소수, 모듈러 연산, 조합
20. `20_geometry_sweep_line.cpp` - CCW, 선분 교차, 볼록 껍질, 스위프 라인
21. `21_memory_layout_and_optimization.cpp` - 메모리 배치, 캐시, 어셈블리 관점 최적화 기초
22. `22_socket_basics.cpp` - TCP 소켓, partial send/recv, RAII 기초
23. `23_thread_optimization.cpp` - thread, mutex, atomic, false sharing, 병렬 합산 기초

## 학습 순서

초반 1-6장은 구현 속도와 기본 자료구조 감각을 만드는 구간입니다.
7-11장은 그래프 문제의 핵심 골격이고, 12-15장은 DP/greedy 판단력을 훈련합니다.
16-20장은 난도가 올라가는 라이브 코딩에서 차이를 만드는 주제입니다.
21-23장은 실무 C++ 코드를 읽을 때 자주 보이는 성능/동시성/네트워크 문제를 찾는 기초 관점입니다.
