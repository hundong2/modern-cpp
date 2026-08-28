static class Verify
{
    public static void Check(bool condition, string name)
    {
        if (!condition) throw new InvalidOperationException($"check failed: {name}");
        Console.WriteLine($"[PASS] {name}");
    }
}

sealed class TreeNode(int value)
{
    public int Value { get; } = value;
    public TreeNode? Left { get; set; }
    public TreeNode? Right { get; set; }
}

sealed class DisjointSet
{
    private readonly int[] parent;
    private readonly int[] sizes;

    public DisjointSet(int size)
    {
        parent = Enumerable.Range(0, size).ToArray();
        sizes = Enumerable.Repeat(1, size).ToArray();
    }

    public int Find(int value)
    {
        if (parent[value] != value) parent[value] = Find(parent[value]); // 경로 압축
        return parent[value];
    }

    public bool Unite(int a, int b)
    {
        a = Find(a);
        b = Find(b);
        if (a == b) return false;
        if (sizes[a] < sizes[b]) (a, b) = (b, a);
        parent[b] = a;
        sizes[a] += sizes[b];
        return true;
    }
}

static class Algorithms
{
    public static int LinearSearch(IReadOnlyList<int> values, int target)
    {
        for (var i = 0; i < values.Count; ++i)
            if (values[i] == target) return i;
        return -1;
    }

    // 반열린 구간 [left, right)에서 target 이상인 첫 위치를 찾는다.
    public static int LowerBound(IReadOnlyList<int> sorted, int target)
    {
        var left = 0;
        var right = sorted.Count;
        while (left < right)
        {
            var middle = left + (right - left) / 2;
            if (sorted[middle] < target) left = middle + 1;
            else right = middle;
        }
        return left;
    }

    public static void MergeSort(int[] values)
    {
        var buffer = new int[values.Length];
        SortRange(0, values.Length);
        return;

        void SortRange(int begin, int end)
        {
            if (end - begin <= 1) return;
            var middle = begin + (end - begin) / 2;
            SortRange(begin, middle);
            SortRange(middle, end);
            var left = begin;
            var right = middle;
            var output = begin;
            while (left < middle && right < end)
                buffer[output++] = values[left] <= values[right] ? values[left++] : values[right++];
            while (left < middle) buffer[output++] = values[left++];
            while (right < end) buffer[output++] = values[right++];
            Array.Copy(buffer, begin, values, begin, end - begin);
        }
    }

    public static void Preorder(TreeNode? node, List<int> result)
    {
        if (node is null) return;
        result.Add(node.Value);
        Preorder(node.Left, result);
        Preorder(node.Right, result);
    }

    public static void Inorder(TreeNode? node, List<int> result)
    {
        if (node is null) return;
        Inorder(node.Left, result);
        result.Add(node.Value);
        Inorder(node.Right, result);
    }

    public static void Postorder(TreeNode? node, List<int> result)
    {
        if (node is null) return;
        Postorder(node.Left, result);
        Postorder(node.Right, result);
        result.Add(node.Value);
    }

    public static List<int> LevelOrder(TreeNode? root)
    {
        if (root is null) return [];
        var result = new List<int>();
        var pending = new Queue<TreeNode>();
        pending.Enqueue(root);
        while (pending.Count > 0)
        {
            var current = pending.Dequeue();
            result.Add(current.Value);
            if (current.Left is not null) pending.Enqueue(current.Left);
            if (current.Right is not null) pending.Enqueue(current.Right);
        }
        return result;
    }

    public static List<int> Bfs(IReadOnlyList<int[]> graph, int start)
    {
        var visited = new bool[graph.Count];
        var order = new List<int>();
        var pending = new Queue<int>();
        visited[start] = true;
        pending.Enqueue(start);
        while (pending.Count > 0)
        {
            var current = pending.Dequeue();
            order.Add(current);
            foreach (var next in graph[current])
            {
                if (visited[next]) continue;
                visited[next] = true;
                pending.Enqueue(next);
            }
        }
        return order;
    }

    public static List<int> Dfs(IReadOnlyList<int[]> graph, int start)
    {
        var visited = new bool[graph.Count];
        var order = new List<int>();
        Visit(start);
        return order;

        void Visit(int current)
        {
            visited[current] = true;
            order.Add(current);
            foreach (var next in graph[current])
                if (!visited[next]) Visit(next);
        }
    }

    public static long[] Dijkstra(IReadOnlyList<(int Next, int Weight)[]> graph, int start)
    {
        var distance = Enumerable.Repeat(long.MaxValue / 4, graph.Count).ToArray();
        var heap = new PriorityQueue<int, long>();
        distance[start] = 0;
        heap.Enqueue(start, 0);
        while (heap.TryDequeue(out var current, out var poppedDistance))
        {
            if (poppedDistance != distance[current]) continue;
            foreach (var (next, weight) in graph[current])
            {
                var candidate = poppedDistance + weight;
                if (candidate >= distance[next]) continue;
                distance[next] = candidate;
                heap.Enqueue(next, candidate);
            }
        }
        return distance;
    }

    public static List<int> TopologicalSort(IReadOnlyList<int[]> graph)
    {
        var indegree = new int[graph.Count];
        foreach (var edges in graph)
            foreach (var next in edges) ++indegree[next];
        var ready = new Queue<int>();
        for (var vertex = 0; vertex < graph.Count; ++vertex)
            if (indegree[vertex] == 0) ready.Enqueue(vertex);
        var order = new List<int>();
        while (ready.Count > 0)
        {
            var current = ready.Dequeue();
            order.Add(current);
            foreach (var next in graph[current])
                if (--indegree[next] == 0) ready.Enqueue(next);
        }
        return order;
    }

    public static long[] PrefixSums(IReadOnlyList<int> values)
    {
        var prefix = new long[values.Count + 1];
        for (var i = 0; i < values.Count; ++i) prefix[i + 1] = prefix[i] + values[i];
        return prefix;
    }

    public static long MaximumWindowSum(IReadOnlyList<int> values, int width)
    {
        if (width <= 0 || width > values.Count) throw new ArgumentOutOfRangeException(nameof(width));
        long window = 0;
        for (var i = 0; i < width; ++i) window += values[i];
        var best = window;
        for (var right = width; right < values.Count; ++right)
        {
            window += values[right] - values[right - width];
            best = Math.Max(best, window);
        }
        return best;
    }

    public static long ClimbStairs(int steps)
    {
        if (steps < 0) return 0;
        long previous = 1;
        long current = 1;
        for (var i = 2; i <= steps; ++i) (previous, current) = (current, previous + current);
        return current;
    }
}

public static class Program
{
public static void Main()
{
var values = new[] { 7, 2, 9, 2, 5 };
Verify.Check(Algorithms.LinearSearch(values, 9) == 2, "linear search");

var sorted = (int[])values.Clone();
Algorithms.MergeSort(sorted);
Verify.Check(sorted.SequenceEqual([2, 2, 5, 7, 9]), "merge sort");
Verify.Check(Algorithms.LowerBound(sorted, 2) == 0 && Algorithms.LowerBound(sorted, 6) == 3,
    "binary lower bound");

var root = new TreeNode(1)
{
    Left = new TreeNode(2) { Left = new TreeNode(4), Right = new TreeNode(5) },
    Right = new TreeNode(3)
};
var preorder = new List<int>();
var inorder = new List<int>();
var postorder = new List<int>();
Algorithms.Preorder(root, preorder);
Algorithms.Inorder(root, inorder);
Algorithms.Postorder(root, postorder);
Verify.Check(preorder.SequenceEqual([1, 2, 4, 5, 3]), "tree preorder DFS");
Verify.Check(inorder.SequenceEqual([4, 2, 5, 1, 3]), "tree inorder DFS");
Verify.Check(postorder.SequenceEqual([4, 5, 2, 3, 1]), "tree postorder DFS");
Verify.Check(Algorithms.LevelOrder(root).SequenceEqual([1, 2, 3, 4, 5]), "tree level-order BFS");

int[][] graph = [[1, 2], [0, 3, 4], [0, 4], [1], [1, 2]];
Verify.Check(Algorithms.Bfs(graph, 0).SequenceEqual([0, 1, 2, 3, 4]), "graph BFS");
Verify.Check(Algorithms.Dfs(graph, 0).SequenceEqual([0, 1, 3, 4, 2]), "graph DFS");

(int Next, int Weight)[][] weighted = [[(1, 4), (2, 1)], [(3, 1)], [(1, 2), (3, 5)], []];
Verify.Check(Algorithms.Dijkstra(weighted, 0).SequenceEqual([0L, 3L, 1L, 4L]),
    "Dijkstra shortest path");

int[][] dag = [[1, 2], [3], [3], []];
var topo = Algorithms.TopologicalSort(dag);
Verify.Check(topo.Count == dag.Length && topo[0] == 0 && topo[^1] == 3, "topological sort");

var sets = new DisjointSet(5);
sets.Unite(0, 1);
sets.Unite(1, 2);
Verify.Check(sets.Find(0) == sets.Find(2) && sets.Find(0) != sets.Find(3), "disjoint set union");

var prefix = Algorithms.PrefixSums(values);
Verify.Check(prefix[4] - prefix[1] == 13, "prefix sum [1, 4)");
Verify.Check(Algorithms.MaximumWindowSum(values, 3) == 18, "sliding window maximum sum");
Verify.Check(Algorithms.ClimbStairs(5) == 8, "dynamic programming");
Console.WriteLine("ALL CHECKS PASSED");
}
}
