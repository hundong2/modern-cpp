C++의 STL과 C#의 컬렉션은 알고리즘 문제 해결의 뼈대입니다. 가장 많이 쓰이는 자료구조들의 고정/동적 선언 방식과 필수 내장 함수, 그리고 실전 패턴을 정리했습니다.

## 1. 배열 및 동적 배열 (Array & Vector / List)

고정된 크기가 필요할 때는 일반 배열(Array)을, 크기가 변해야 할 때는 동적 배열(C++ `vector`, C# `List`)을 사용합니다.

**고정 크기 선언 및 초기화**

* **C++:**
* 배열: `int arr[10] = {0};`
* 벡터: `vector<int> v(10, 0);` (크기 10, 모든 값 0으로 초기화)


* **C#:**
* 배열: `int[] arr = new int[10];` (기본값 0으로 자동 초기화)
* 리스트: `List<int> list = Enumerable.Repeat(0, 10).ToList();`



**자주 쓰이는 핵심 함수 5선**

| 기능 | C++ (`vector<int> v`) | C# (`List<int> list`) |
| --- | --- | --- |
| **끝에 추가** | `v.push_back(val);` | `list.Add(val);` |
| **끝에 제거** | `v.pop_back();` | `list.RemoveAt(list.Count - 1);` |
| **크기 확인** | `v.size();` | `list.Count;` |
| **전체 정렬** | `sort(v.begin(), v.end());` | `list.Sort();` |
| **초기화(비우기)** | `v.clear();` | `list.Clear();` |

## 2. 스택과 큐 (Stack & Queue)

데이터를 넣고 빼는 순서가 중요할 때 사용합니다. 스택은 나중에 넣은 것을 먼저 빼고(LIFO), 큐는 먼저 넣은 것을 먼저 뺍니다(FIFO).

| 구조/기능 | C++ (`stack<int> s`, `queue<int> q`) | C# (`Stack<int> s`, `Queue<int> q`) |
| --- | --- | --- |
| **스택 데이터 넣기** | `s.push(val);` | `s.Push(val);` |
| **스택 데이터 빼기** | `s.pop();` | `s.Pop();` |
| **스택 맨 위 확인** | `s.top();` (꺼내지 않고 값만 확인) | `s.Peek();` |
| **큐 데이터 넣기** | `q.push(val);` | `q.Enqueue(val);` |
| **큐 데이터 빼기** | `q.pop();` | `q.Dequeue();` |
| **큐 맨 앞 확인** | `q.front();` | `q.Peek();` |
| **비어있는지 확인** | `s.empty();` / `q.empty();` | `s.Count == 0;` / `q.Count == 0;` |

## 3. 해시 맵과 셋 (Hash Map & Hash Set)

특정 키(Key)의 존재 여부를 $O(1)$의 속도로 찾거나, 중복을 제거할 때 사용합니다. 알고리즘에서는 정렬이 필요 없는 해시 기반 구조가 속도 면에서 유리합니다.

**선언**

* **C++:** `unordered_map<string, int> map;` / `unordered_set<int> set;`
* **C#:** `Dictionary<string, int> dict = new();` / `HashSet<int> set = new();`

**자주 쓰이는 핵심 함수 5선**

| 기능 | C++ (`unordered_map`, `unordered_set`) | C# (`Dictionary`, `HashSet`) |
| --- | --- | --- |
| **값 할당/추가** | `map[key] = val;`, `set.insert(val);` | `dict[key] = val;`, `set.Add(val);` |
| **키 존재 여부** | `map.count(key) > 0` | `dict.ContainsKey(key)` |
| **요소 삭제** | `map.erase(key);`, `set.erase(val);` | `dict.Remove(key);`, `set.Remove(val);` |
| **키로 값 찾기** | `int val = map[key];` | `int val = dict[key];` (없으면 에러) |
| **안전하게 값 찾기** | `if(map.find(key) != map.end())` | `if(dict.TryGetValue(key, out int val))` |

---

## 4. 알고리즘에서 자주 쓰이는 기본 패턴

위의 자료구조들을 조합하여 푸는 대표적인 패턴 3가지입니다.

### 패턴 1: 해시 맵을 활용한 빈도수 체크 (Frequency Map)

배열 내 원소들의 등장 횟수를 맵에 기록해두고 빠르게 조회하는 패턴입니다. (예: 아나그램 판별, 중복 원소 찾기)

* **동작:** 배열을 순회하며 `map[원소]++` (C++의 경우 자동 0 초기화, C#은 키 존재 여부를 체크한 뒤 더해줌).

### 패턴 2: 투 포인터 (Two Pointers)

정렬된 배열(Vector/List)에서 두 개의 인덱스(왼쪽 끝, 오른쪽 끝)를 조작하여 특정 합이나 조건을 찾는 패턴입니다. 이중 for문을 $O(N^2)$에서 $O(N)$으로 줄여줍니다.

* **동작:** `left = 0`, `right = arr.Length - 1`로 시작해 합이 타겟보다 크면 `right--`, 작으면 `left++`를 수행합니다.

### 패턴 3: 슬라이딩 윈도우 (Sliding Window)

배열에서 "연속된 K개의 원소"로 이루어진 부분 배열의 합이나 최대/최솟값을 구할 때 씁니다. 매번 K개를 다 더하지 않고, 창문을 밀듯이 앞쪽 값을 빼고 뒤쪽 값을 더합니다.

* **동작:** `currentSum += arr[i] - arr[i - K]` 방식으로 매 단계 $O(1)$ 연산만 수행합니다. 큐(Queue)를 활용해 창문(Window)에 들어오는 값과 나가는 값을 관리하기도 합니다.

C++에서 `std::next_permutation`은 주로 **"주어진 원소들을 나열하는 모든 경우의 수를 확인해야 하는 완전 탐색(Brute Force)"** 알고리즘 문제에서 사용됩니다.

특히 재귀 함수(DFS)를 직접 짤 필요 없이 매우 간결하고 빠르게 동작하기 때문에, 알고리즘 대회나 코딩 테스트에서 다음 3가지 상황일 때 거의 무조건 활용됩니다.

### 1. N의 크기가 작고 "순서"가 결과에 영향을 줄 때 (N ≤ 10)

원소를 나열하는 순서에 따라 결과값이 달라지는 문제에서, 가능한 모든 순서를 다 해봐야 할 때 사용합니다. $10!$(팩토리얼)은 약 360만 번의 연산이므로 1초 시간 제한 내에 충분히 탐색이 가능합니다.

* **대표 문제:**
* 외판원 순회 (TSP): N개의 도시를 어떤 순서로 방문해야 가장 비용이 적게 드는가?
* 연산자 끼워넣기: 숫자 사이에 `+, -, *, /` 연산자를 어떤 순서로 배치해야 최댓값이 나오는가?
* 줄 세우기: 조건에 맞게 학생 N명을 줄 세우는 모든 경우의 수.



### 2. 조합(Combination)을 구하고 싶을 때 (가장 유용한 실전 팁)

C++에는 순열 함수는 있지만 조합(`nCr`)을 구해주는 내장 함수는 없습니다. 이때 `next_permutation`과 **0과 1로 이루어진 배열**을 활용하면 조합을 아주 쉽게 뽑아낼 수 있습니다.

예를 들어 5개의 숫자 중 3개를 고르는 경우(5C3), 0 두 개와 1 세 개로 이루어진 보조 배열을 만들어 순열을 돌리면 됩니다.

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    vector<int> data = {10, 20, 30, 40, 50};
    
    // 5개 중 3개를 고르기 위한 마스크 배열 생성
    // 선택할 3개는 1, 선택 안 할 2개는 0으로 둡니다. (오름차순 정렬 상태로 시작)
    vector<int> mask = {0, 0, 1, 1, 1}; 

    do {
        for (int i = 0; i < 5; i++) {
            if (mask[i] == 1) { // 1이 위치한 자리의 실제 데이터를 선택
                cout << data[i] << " ";
            }
        }
        cout << "\n";
    } while (next_permutation(mask.begin(), mask.end()));

    return 0;
}

```

### 3. 중복된 원소가 포함된 순열을 찾아야 할 때

배열에 `[1, 1, 2]`처럼 중복된 숫자가 섞여 있을 때, 이를 DFS로 직접 짜서 중복 결과를 제거하려면 앞서 설명해 드렸던 `i > idx && candidates[i] == candidates[i - 1]` 같은 복잡한 분기 처리(가지치기) 로직을 짜야 합니다.

하지만 `std::next_permutation`은 배열 내의 값을 기준으로 다음 사전순 위치를 찾기 때문에, 내부적으로 중복된 결과(`1A-1B-2`와 `1B-1A-2`)를 알아서 하나로 합쳐서 반환해 줍니다.

**주의할 점:**
$N$이 11 이상이 되면 $11! \approx 3,990$만, $12! \approx 4.7$억으로 연산량이 기하급수적으로 폭발합니다. 따라서 원소의 개수가 11~12개를 넘어가는 문제라면 모든 순열을 구하는 방식이 아니라 다이나믹 프로그래밍(DP)이나 그리디(Greedy) 알고리즘으로 풀어야 하는 문제일 확률이 높습니다.