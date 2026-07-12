/*
Chapter 17. Fenwick Tree, Segment Tree, Lazy Propagation

문제 1: 점 업데이트와 구간 합 질의를 처리하라.
접근: Fenwick Tree. index에 lowbit만큼 더하며 올라가고, prefix sum은 내려간다.
복잡도: 업데이트/질의 O(log N)

문제 2: 구간 최솟값 또는 합을 처리하라.
접근: Segment Tree. 노드는 담당 구간의 정보를 저장한다.

문제 3: 구간에 값을 더하고 구간 합을 구하라.
접근: Lazy Propagation. 자식에게 아직 전달하지 않은 값을 lazy 배열에 보관한다.

실전 선택:
- 합/빈도처럼 prefix가 필요한 경우 Fenwick이 짧고 빠르다.
- min/max/gcd 또는 range update가 섞이면 Segment Tree가 일반적이다.
*/

#include <iostream>
#include <vector>
using namespace std;

class Fenwick {
public:
    explicit Fenwick(int n) : bit(n + 1, 0) {}

    void add(int index, long long delta) {
        for (++index; index < (int)bit.size(); index += index & -index) {
            bit[index] += delta;
        }
    }

    long long sumPrefix(int index) const {
        long long result = 0;
        for (++index; index > 0; index -= index & -index) {
            result += bit[index];
        }
        return result;
    }

    long long sumRange(int left, int right) const {
        if (left > right) return 0;
        return sumPrefix(right) - (left == 0 ? 0 : sumPrefix(left - 1));
    }

private:
    vector<long long> bit;
};

class SegmentTree {
public:
    explicit SegmentTree(const vector<int>& a) : n((int)a.size()), tree(4 * n, 0) {
        build(a, 1, 0, n - 1);
    }

    void update(int index, int value) {
        update(1, 0, n - 1, index, value);
    }

    long long query(int left, int right) const {
        return query(1, 0, n - 1, left, right);
    }

private:
    int n;
    vector<long long> tree;

    void build(const vector<int>& a, int node, int start, int end) {
        if (start == end) {
            tree[node] = a[start];
            return;
        }
        int mid = (start + end) / 2;
        build(a, node * 2, start, mid);
        build(a, node * 2 + 1, mid + 1, end);
        tree[node] = tree[node * 2] + tree[node * 2 + 1];
    }

    void update(int node, int start, int end, int index, int value) {
        if (start == end) {
            tree[node] = value;
            return;
        }
        int mid = (start + end) / 2;
        if (index <= mid) update(node * 2, start, mid, index, value);
        else update(node * 2 + 1, mid + 1, end, index, value);
        tree[node] = tree[node * 2] + tree[node * 2 + 1];
    }

    long long query(int node, int start, int end, int left, int right) const {
        if (right < start || end < left) return 0;
        if (left <= start && end <= right) return tree[node];
        int mid = (start + end) / 2;
        return query(node * 2, start, mid, left, right) + query(node * 2 + 1, mid + 1, end, left, right);
    }
};

class LazySegmentTree {
public:
    explicit LazySegmentTree(const vector<int>& a) : n((int)a.size()), tree(4 * n, 0), lazy(4 * n, 0) {
        build(a, 1, 0, n - 1);
    }

    void addRange(int left, int right, long long value) {
        addRange(1, 0, n - 1, left, right, value);
    }

    long long sumRange(int left, int right) {
        return sumRange(1, 0, n - 1, left, right);
    }

private:
    int n;
    vector<long long> tree;
    vector<long long> lazy;

    void build(const vector<int>& a, int node, int start, int end) {
        if (start == end) {
            tree[node] = a[start];
            return;
        }
        int mid = (start + end) / 2;
        build(a, node * 2, start, mid);
        build(a, node * 2 + 1, mid + 1, end);
        tree[node] = tree[node * 2] + tree[node * 2 + 1];
    }

    void push(int node, int start, int end) {
        if (lazy[node] == 0) return;
        tree[node] += lazy[node] * (end - start + 1);
        if (start != end) {
            lazy[node * 2] += lazy[node];
            lazy[node * 2 + 1] += lazy[node];
        }
        lazy[node] = 0;
    }

    void addRange(int node, int start, int end, int left, int right, long long value) {
        push(node, start, end);
        if (right < start || end < left) return;
        if (left <= start && end <= right) {
            lazy[node] += value;
            push(node, start, end);
            return;
        }

        int mid = (start + end) / 2;
        addRange(node * 2, start, mid, left, right, value);
        addRange(node * 2 + 1, mid + 1, end, left, right, value);
        tree[node] = tree[node * 2] + tree[node * 2 + 1];
    }

    long long sumRange(int node, int start, int end, int left, int right) {
        push(node, start, end);
        if (right < start || end < left) return 0;
        if (left <= start && end <= right) return tree[node];
        int mid = (start + end) / 2;
        return sumRange(node * 2, start, mid, left, right) + sumRange(node * 2 + 1, mid + 1, end, left, right);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<int> a = {1, 2, 3, 4, 5};
    Fenwick fw((int)a.size());
    for (int i = 0; i < (int)a.size(); ++i) fw.add(i, a[i]);
    cout << "[fenwick 1..3] " << fw.sumRange(1, 3) << '\n';
    fw.add(2, 10);
    cout << "[fenwick after add] " << fw.sumRange(1, 3) << '\n';

    SegmentTree seg(a);
    cout << "[segment 0..4] " << seg.query(0, 4) << '\n';
    seg.update(0, 10);
    cout << "[segment after update] " << seg.query(0, 2) << '\n';

    LazySegmentTree lazy(a);
    lazy.addRange(1, 3, 5);
    cout << "[lazy 0..4] " << lazy.sumRange(0, 4) << '\n';
    return 0;
}
