# Hungarian 알고리즘: 최소 비용 할당 문제

## 정의

**할당 문제(assignment problem)**는 왼쪽 집합의 직원과 오른쪽 집합의 작업을 일대일로 연결하는 완전
이분 매칭 중 간선 비용 합이 최소인 것을 찾는 문제다. 정사각 비용 행렬 `c[i][j]`에서 순열 `pi`를
골라 다음 값을 최소화하는 것과 같다.

```text
minimize  c[1][pi(1)] + c[2][pi(2)] + ... + c[n][pi(n)]
```

Hungarian 알고리즘은 매칭만 직접 고치는 대신 직원 잠재값 `u[i]`, 작업 잠재값 `v[j]`라는 쌍대
변수를 함께 관리한다. 감소 비용(reduced cost)

```text
r[i][j] = c[i][j] - u[i] - v[j]
```

이 0인 **tight 간선**만으로 증대 경로를 만들어, 한 번의 단계마다 매칭 크기를 정확히 1 늘린다.
밀집한 `n x n` 행렬에서는 `O(n^3)` 시간에 최소 비용 완전 매칭과 실제 배정을 함께 복원한다.

## 적용 조건

- 직원과 작업을 각각 정확히 한 번 선택하는 최소 비용 일대일 배정에 적용한다.
- 모든 `n^2` 비용이 주어지는 밀집 완전 이분 그래프에 특히 적합하다.
- 최대 이익 배정은 `cost = -profit`으로 바꾸거나 `max_profit - profit`으로 비음수 비용을 만든다.
- 직사각 `rows x columns` 문제는 작은 쪽이 큰 쪽을 넘지 않도록 방향을 정하거나, 0비용 더미
  행·열을 붙여 정사각 행렬로 만든다. “배정하지 않음”의 실제 의미에 따라 더미 비용을 정해야 한다.
- 금지된 배정은 충분히 큰 비용으로 나타낼 수 있지만, 그 값이 실제 답과 구별되고 산술 overflow가
  나지 않아야 한다. 희소 그래프라면 최소 비용 최대 유량이 더 자연스러울 수 있다.
- 아래의 쌍대 타당성 설명은 CSES 2129처럼 비용이 비음수일 때 `u=v=0`에서 바로 성립한다. 음수
  비용도 표준 구현이 처리할 수 있지만, 같은 증명을 그대로 쓰려면 모든 비용에 공통 상수를 더해
  비음수로 만든 뒤 최종값에서 `n * 상수`를 빼는 방법이 가장 명료하다.

## 표현과 용어

1-based 구현에 실제 작업 `1..n`과 더미 작업 `0`을 둔다.

- `p[j]`: 현재 작업 `j`를 맡은 직원. `p[j] == 0`이면 아직 빈 실제 작업이다.
- `p[0]`: 이번에 매칭에 추가할 새 직원을 잠시 담는다.
- `u[i]`: 직원 `i`의 잠재값.
- `v[j]`: 작업 `j`의 잠재값. `v[0]`은 답을 누적하는 더미 값으로도 해석할 수 있다.
- `used[j]`: 현재 증대 탐색의 alternating tree에 작업 `j`가 들어왔는가.
- `minv[j]`: tree 안 직원에서 tree 밖 작업 `j`로 나가는 간선의 최소 감소 비용, 즉 slack.
- `way[j]`: `minv[j]`를 만든 경로에서 바로 앞 작업. 빈 작업을 찾은 뒤 증대 경로를 복원한다.

`p`의 방향이 “작업 -> 직원”이라는 점이 중요하다. 출력이 “직원 -> 작업”이면 마지막에
`assignment[p[j]] = j`로 역방향 배열을 만든다.

## 핵심 아이디어와 불변식

### 1. 매칭 불변식

바깥 반복에서 직원 `i`를 처리하기 직전에는 직원 `1..i-1`이 서로 다른 실제 작업에 배정되어 있다.
한 빈 작업까지의 alternating path를 뒤집으면 기존 직원의 중복 배정 없이 직원 `i`만 새로 들어간다.
따라서 반복이 끝날 때 매칭 크기는 정확히 1 증가한다.

### 2. 쌍대 타당성

모든 실제 간선에 대해 다음 제약을 유지한다.

```text
u[i] + v[j] <= c[i][j]
동치: r[i][j] = c[i][j] - u[i] - v[j] >= 0
```

비용이 비음수이면 `u=v=0`은 처음부터 타당하다. 현재 tree의 방문 작업 집합을 `T`, 그 작업에
매칭된 직원과 새 직원을 합친 집합을 `S`라 하자. 바깥 작업 `j`에 대해

```text
minv[j] = min(r[i][j] : i in S)
delta   = min(minv[j] : j not in T)
```

를 구하고 다음처럼 갱신한다.

```text
i in S 이면 u[i] += delta
j in T 이면 v[j] -= delta
j not in T 이면 minv[j] -= delta
```

간선을 네 종류로 나누면 타당성이 보인다.

- `i in S, j in T`: `u`의 증가와 `v`의 감소가 상쇄되어 감소 비용이 그대로다.
- `i in S, j not in T`: 감소 비용이 `delta` 줄지만, `delta <= minv[j] <= r[i][j]`라 음수가 아니다.
- `i not in S, j in T`: `v[j]`가 감소하므로 감소 비용은 오히려 커진다.
- 둘 다 바깥: 아무 잠재값도 바뀌지 않는다.

### 3. tight 매칭 불변식

현재 매칭에 들어간 모든 실제 간선은 감소 비용 0이다. 방문 작업 `j`의 매칭 직원 `p[j]`도 `S`에
있으므로 그 매칭 간선에는 `u[p[j]] += delta`와 `v[j] -= delta`가 함께 적용된다. 따라서 기존 tight
간선은 계속 tight하다. tree 바깥의 매칭 간선에는 둘 다 적용되지 않아 역시 그대로다.

### 4. slack과 진행 불변식

`minv[j]`는 항상 현재 `S`에서 미방문 작업 `j`로 가는 최소 감소 비용이다. 새 직원을 `S`에 넣을
때 그 직원의 모든 바깥 간선으로 `minv`를 완화한다. 잠재값 갱신에서는 모든 바깥 slack을 동일하게
`delta`만큼 줄인다.

`delta`를 바깥 작업의 최소 slack으로 골랐으므로 적어도 한 작업의 새 slack이 0이 된다. 그 작업을
tree에 넣는다. 이미 매칭된 작업이면 그 직원이 `S`에 추가되고, 빈 작업이면 증대 경로가 완성된다.
작업은 한 증대 탐색에서 최대 한 번 방문되므로 탐색은 유한하게 끝난다.

### 5. predecessor 불변식

`way[j]`에는 현재 `minv[j]`를 실현한 직원으로 오기 직전의 작업을 저장한다. 실제로 그 직원은
`p[way[j]]`다. 빈 작업에 도착한 뒤

```text
p[j] = p[way[j]]
j = way[j]
```

를 더미 0까지 반복하면 tight 간선으로만 이루어진 alternating path가 정확히 뒤집힌다.

## 단계별 절차

1. `u`, `v`, `p`, `way`를 0으로 초기화한다.
2. 새 직원 `i`를 더미 작업에 연결하여 `p[0] = i`로 둔다.
3. 이번 탐색 전용 `minv`를 무한대, `used`를 false로 초기화한다.
4. 현재 작업 `j0`을 방문하고 직원 `i0 = p[j0]`를 alternating tree에 넣는다.
5. 모든 미방문 작업 `j`에 대해 `c[i0][j]-u[i0]-v[j]`로 `minv[j]`와 `way[j]`를 완화한다.
6. 미방문 작업 중 최소 slack `delta`와 그 작업 `j1`을 고른다.
7. 방문 집합의 잠재값과 미방문 slack을 `delta`만큼 갱신한다.
8. `j1`이 이미 매칭되었다면 4번으로 돌아가고, 비어 있다면 `way`를 거슬러 경로를 뒤집는다.
9. 모든 직원에 대해 2~8을 반복한다.
10. `p[task] = employee`를 `assignment[employee] = task`로 뒤집고 비용 합을 계산한다.

## 의사코드

```text
u[0..n] = 0, v[0..n] = 0
p[0..n] = 0, way[0..n] = 0

for employee = 1..n:
    p[0] = employee
    minv[0..n] = INF
    used[0..n] = false
    j0 = 0

    do:
        used[j0] = true
        i0 = p[j0]
        delta = INF
        j1 = 0

        for task = 1..n:
            if not used[task]:
                reduced = cost[i0][task] - u[i0] - v[task]
                if reduced < minv[task]:
                    minv[task] = reduced
                    way[task] = j0
                if minv[task] < delta:
                    delta = minv[task]
                    j1 = task

        for task = 0..n:
            if used[task]:
                u[p[task]] += delta
                v[task] -= delta
            else if task != 0:
                minv[task] -= delta

        j0 = j1
    while p[j0] != 0

    do:
        j1 = way[j0]
        p[j0] = p[j1]
        j0 = j1
    while j0 != 0

for task = 1..n:
    assignment[p[task]] = task
```

## 컴파일 가능한 C++ 뼈대

아래 코드는 `n`, 비용 행렬을 읽고 최소 비용과 배정 쌍을 출력한다. 실전 제출에서는 입력 제한에
맞춰 `long long`의 범위와 `INF`를 다시 검토한다.

```cpp
// <iostream>은 표준 입력·출력 스트림을 선언한다.
#include <iostream>
// <vector>는 크기를 실행 중 정하는 연속 동적 배열을 선언한다.
#include <vector>

// 비용과 잠재값은 누적 여유가 있는 64-bit signed 정수로 계산한다.
using Cost = long long;
// 바깥 vector가 비용 행들을, 각 안쪽 vector가 한 직원의 작업별 비용을 소유한다.
using Matrix = std::vector<std::vector<Cost>>;

// cost는 복사하지 않는 const 참조, assignment는 호출자 결과를 쓰는 non-const 참조다.
Cost hungarian(const Matrix& cost, int n, std::vector<int>& assignment) {
    // sentinel 산술이 signed 정수 범위를 넘지 않도록 최댓값보다 여유 있게 작은 값을 쓴다.
    constexpr Cost inf{Cost{1} << 60};
    // u/v는 쌍대 잠재값, p는 작업->직원 매칭, way는 경로 복원 선행 작업이다.
    std::vector<Cost> u(n + 1, 0);
    std::vector<Cost> v(n + 1, 0);
    std::vector<int> p(n + 1, 0);
    std::vector<int> way(n + 1, 0);

    // 직원 한 명을 현재 매칭에 추가할 때마다 tight 증대 경로를 하나 찾는다.
    for (int employee{1}; employee <= n; ++employee) {
        // 더미 작업 0이 이번 새 직원을 가리키게 한다.
        p[0] = employee;
        // minv와 used는 이 직원의 증대 탐색마다 새로 초기화한다.
        std::vector<Cost> minv(n + 1, inf);
        std::vector<bool> used(n + 1, false);
        int j0{0};

        do {
            // j0을 tree에 넣고, 거기에 매칭된 직원을 통해 바깥 작업을 완화한다.
            used[j0] = true;
            const int i0{p[j0]};
            Cost delta{inf};
            int j1{0};

            for (int task{1}; task <= n; ++task) {
                if (!used[task]) {
                    const Cost reduced{cost[i0][task] - u[i0] - v[task]};
                    if (reduced < minv[task]) {
                        minv[task] = reduced;
                        way[task] = j0;
                    }
                    if (minv[task] < delta) {
                        delta = minv[task];
                        j1 = task;
                    }
                }
            }

            // 최소 slack만큼 potential을 바꾸면 dual feasibility를 보존하면서 j1이 tight해진다.
            for (int task{0}; task <= n; ++task) {
                if (used[task]) {
                    u[p[task]] += delta;
                    v[task] -= delta;
                } else if (task != 0) {
                    minv[task] -= delta;
                }
            }
            j0 = j1;
        } while (p[j0] != 0);

        // 빈 작업부터 더미 0까지 predecessor를 따라가며 alternating path를 뒤집는다.
        do {
            const int previous{way[j0]};
            p[j0] = p[previous];
            j0 = previous;
        } while (j0 != 0);
    }

    // 작업->직원 p를 직원->작업 assignment로 역변환하고 원본 비용으로 답을 검산한다.
    Cost answer{0};
    for (int task{1}; task <= n; ++task) {
        assignment[p[task]] = task;
    }
    for (int employee{1}; employee <= n; ++employee) {
        answer += cost[employee][assignment[employee]];
    }
    return answer;
}

int main() {
    // 공식 입력처럼 C stdio와 섞지 않을 때 불필요한 동기화와 자동 flush를 끈다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // {}는 기본 타입 int를 0으로 값 초기화한다.
    int n{};
    if (!(std::cin >> n)) {
        return 0;
    }
    // 인덱스 0을 더미로 남겨 두므로 각 차원 크기는 n+1이다.
    Matrix cost(n + 1, std::vector<Cost>(n + 1, 0));
    for (int employee{1}; employee <= n; ++employee) {
        for (int task{1}; task <= n; ++task) {
            std::cin >> cost[employee][task];
        }
    }

    // assignment도 1-based로 만들고 함수가 채운 최소 비용과 실제 쌍을 출력한다.
    std::vector<int> assignment(n + 1, 0);
    const Cost answer{hungarian(cost, n, assignment)};
    std::cout << answer << '\n';
    for (int employee{1}; employee <= n; ++employee) {
        std::cout << employee << ' ' << assignment[employee] << '\n';
    }
}
```

## 정확성 근거

### 보조정리 1: 잠재값 갱신 뒤에도 쌍대 타당성이 유지된다

초기 비용이 비음수이므로 `u=v=0`에서 모든 감소 비용은 비음수다. 한 번의 갱신에서 위의 네 간선
종류를 살펴보면, 감소 비용은 그대로이거나 커지거나 `delta`만큼만 작아진다. 마지막 경우에도
`delta`는 모든 바깥 작업의 최소 slack 이하이므로 결과가 음수가 될 수 없다. 귀납적으로 모든 실제
간선에 `u[i]+v[j] <= c[i][j]`가 유지된다.

### 보조정리 2: 현재 매칭의 모든 간선은 tight하다

처음에는 매칭이 없어 자명하다. 기존 매칭 간선은 잠재값 갱신에서 양 끝 잠재값의 변화가 상쇄되거나
두 끝 모두 변하지 않으므로 감소 비용 0을 유지한다. `delta`를 최소 slack으로 고르면 다음 작업으로
들어가는 감소 비용 0인 간선이 적어도 하나 생긴다. 증대 경로는 `way`가 기록한 이런 tight 간선과
기존 tight 매칭 간선만 번갈아 사용한다. 경로를 뒤집은 뒤 새 매칭 간선도 모두 tight하다.

### 보조정리 3: 한 바깥 반복은 매칭 크기를 정확히 1 늘린다

한 탐색에서 작업은 `used`가 false에서 true로 최대 한 번 바뀐다. 완전 이분 그래프이므로 tree 밖
작업이 있는 동안 유한한 slack이 존재하고, 매 갱신은 새 작업 하나 이상을 tree에 넣는다. 현재 매칭
크기는 직원 수보다 하나 작으므로 실제 작업 중 적어도 하나는 비어 있다. 따라서 탐색은 빈 작업에
도착한다. 빈 작업부터 더미 0까지 alternating path를 뒤집으면 경로 내부 작업은 계속 한 직원만
가지고, 끝의 빈 작업에 새 직원이 들어가므로 매칭 크기만 1 증가한다.

### 정리: 알고리즘이 출력하는 완전 매칭은 최소 비용이다

보조정리 3을 직원 n명에 적용하면 완전 매칭을 얻는다. 보조정리 2에 따라 선택된 각 `(i,j)`에는
`c[i][j]=u[i]+v[j]`가 성립한다. 완전 매칭은 모든 직원과 실제 작업을 정확히 한 번 쓰므로 선택 간선
비용 합은 쌍대 목적값 `sum(u)+sum(v)`와 같다. 보조정리 1의 쌍대 타당성과 약한 쌍대성에 따르면
어떤 배정도 이 쌍대 값보다 작을 수 없다. 현재 배정이 그 값에 도달했으므로 최소 비용이다.

## 시간·공간 복잡도

- 직원 한 명을 추가하는 탐색은 작업을 최대 `n`개 tree에 넣는다.
- 작업 하나를 넣을 때 모든 작업을 한 번 완화하고 한 번 갱신하므로 `O(n)`이다.
- 직원 `n`명 전체 시간은 `O(n^3)`이다. CSES의 `n <= 200`이면 약 수백만 번의 단순 연산 규모다.
- 비용 행렬은 `O(n^2)`, 잠재값·매칭·slack·선행자 배열은 `O(n)` 추가 공간이다.
- CSES 최대 총비용은 `200 * 1000 = 200000`이지만 재사용성을 위해 `long long`으로 비용과 잠재값을
  둔다. 일반 문제에서는 `INF`, `cost-u-v`, 잠재값 누적, 총합 모두의 범위를 함께 계산해야 한다.
- `INF`인 slack에 무조건 `delta`를 빼는 구현은 sentinel 산술을 반복할 수 있다. 완전 행렬에서는 첫
  완화 뒤 실제 바깥 작업 slack이 모두 유한하지만, 희소/금지 간선 변형에서는 유한 여부를 검사한다.

## 흔한 실수

- `p[j]`를 직원의 작업으로 오해한다. 이 구현에서 `p`는 작업에서 직원으로 가는 배열이다.
- 직원마다 `minv`와 `used`를 새로 초기화하지 않는다. 이 둘은 한 번의 증대 탐색에만 유효하다.
- 더미 작업 0을 방문 처리하거나 `u[p[0]]`, `v[0]`을 갱신하지 않는다.
- `way[j] = current_task` 대신 직원 번호를 저장하여 복원 경로의 타입을 섞는다.
- 감소 비용에서 잠재값을 더하거나 `v` 갱신 부호를 반대로 쓴다.
- 잠재값 갱신 전에 `next_task`로 이동하여 slack 0 불변식을 깨뜨린다.
- `<=` 동률 갱신을 무심코 섞어 predecessor를 예상과 다르게 덮어쓴다. 어느 최적해든 허용되지만
  결정적 출력과 디버깅을 원하면 동률 규칙을 일관되게 정한다.
- 최소 비용만 `-v[0]`으로 얻고 실제 배정을 복원하지 않거나, 반대로 배정 비용 합을 검산하지 않는다.
- 일반 큰 제약에서 `int`를 사용해 `n * max_cost` 또는 잠재값 산술을 overflow시킨다. signed overflow는
  C++에서 미정의 동작이다.
- 최대화 문제에서 비용 부호를 바꾼 뒤 원래 목적값으로 되돌리는 과정을 빠뜨린다.

## 변형

- **직사각 행렬**: `workers <= tasks`라면 같은 구조로 모든 직원을 서로 다른 작업에 배정할 수 있다.
  반대면 행·열을 전치하거나 더미 작업을 추가한다.
- **최대 가중치 완전 매칭**: `C-cost` 또는 `-cost`로 최소화 문제로 바꾼다. 상수 이동은 모든 완전
  매칭에 같은 `n*C`를 더하므로 최적 순서를 바꾸지 않는다.
- **부분 매칭/배정 거부**: 거부를 나타내는 더미 정점을 추가하고 실제 벌점 비용을 준다.
- **금지 간선**: 큰 비용을 넣되, 가능한 완전 매칭 존재 여부와 overflow를 별도 검사한다.
- **희소 이분 그래프**: 간선 수가 `n^2`보다 훨씬 작거나 용량/추가 제약이 있으면 최소 비용 최대
  유량, successive shortest augmenting path, Jonker-Volgenant 계열을 비교한다.
- **온라인 검산**: 출력 배정이 순열인지 확인하고 실제 `sum(cost[i][assignment[i]])`가 알고리즘이
  계산한 값과 같은지 디버그 빌드에서 검사한다.

## 오늘 문제와의 연결

### 2026-10-21 — CSES 2129 Task Assignment

오늘의 [`icpc_problem.cpp`](../2026-10-21/icpc_problem.cpp)는 정사각 완전 비용 행렬이라는 조건을
그대로 이용한다. `matched_employee[task]`가 위 설명의 `p`, `previous_task`가 `way`다.
`employee_potential`, `task_potential`, `minimum_slack`, `used_task`는 각각 `u`, `v`, `minv`, `used`에
대응한다. 최종 비용은 `-v[0]`에 의존하지 않고 복원한 배정을 원래 행렬에 다시 대입해 합산하므로,
출력할 쌍과 최소 비용이 같은 배정에서 나온다는 점을 코드 수준에서도 확인한다.

CSES 제약에서는 `c[i][j] >= 1`이므로 0 잠재값이 바로 쌍대 타당하다. 최대 총비용은 200000이지만
구현은 잠재값과 sentinel 산술의 여유를 위해 `long long`을 사용한다. `n=200`의 밀집 행렬에서는
`O(n^3)` Hungarian이 제한에 충분하고, 최소 비용 최대 유량보다 구현 상수와 메모리 사용이 작다.

## 직접 해보기와 초보자 검증

1. `n=1`, 비용이 `7`일 때 `p[0]`, `minv[1]`, `delta`, `u[1]`, `v[0]`, `p[1]`의 변화를 적는다.
2. 비용 행렬 `[[1,100],[2,3]]`에서 첫 직원 매칭 뒤 두 번째 직원이 만드는 alternating path를 추적한다.
3. `p`와 `assignment`의 방향을 각각 말하고 `assignment[p[j]]=j`가 왜 안전한지 설명한다.
4. 잠재값 갱신의 네 간선 종류를 표로 만들고 감소 비용이 음수가 되지 않음을 직접 증명한다.
5. 모든 비용이 같은 `3 x 3` 행렬에서 여러 최적 배정이 가능함을 확인하고 현재 `<` 동률 규칙이
   어떤 배정을 고르는지 실행해 본다.
6. `minimum_slack`이나 `used_task`를 직원 루프 바깥에서 한 번만 초기화하면 깨지는 작은 반례를 찾는다.
7. 최대 이익 행렬을 `C-profit`으로 변환했을 때 출력 최소 비용에서 원래 최대 이익을 복원하는 식을 쓴다.
8. 출력된 작업 번호가 `1..n`의 순열인지, 합산 비용이 첫 줄과 같은지 검사하는 코드를 추가한다.
