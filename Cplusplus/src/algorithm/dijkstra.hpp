#pragma once
#include <vector>
#include <queue>
#include <limits>
#include <type_traits>
#include <concepts>
#include <unordered_map>
#include <functional>
#include <algorithm>

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template<typename Node, typename Weight = double>
    requires Arithmetic<Weight>
class Dijkstra {
public:
    using NodeType = Node;
    using WeightType = Weight;
    using Edge = std::pair<Node, Weight>;
    using Graph = std::unordered_map<Node, std::vector<Edge>>;
    using Result = std::unordered_map<Node, std::pair<Weight, std::vector<Node>>>;

    // 添加边
    void addEdge(const Node& from, const Node& to, Weight weight) {
        graph[from].emplace_back(to, weight);
        // 确保所有节点都在图中，即使它们只有出边或只有入边
        graph.try_emplace(to, std::vector<Edge>{});
    }

    // 添加双向边
    void addBidirectionalEdge(const Node& from, const Node& to, Weight weight) {
        addEdge(from, to, weight);
        addEdge(to, from, weight);
    }

    // 执行Dijkstra算法
    Result shortestPath(const Node& start) {
        // 距离映射，存储到每个节点的最短距离
        std::unordered_map<Node, Weight> distances;
        // 前驱节点映射，用于重建路径
        std::unordered_map<Node, Node> previous;
        // 优先队列，按距离排序
        using QueueElement = std::pair<Weight, Node>;
        std::priority_queue<QueueElement, std::vector<QueueElement>,
            std::greater<QueueElement>> pq;

        // 初始化距离
        for (const auto& [node, _] : graph) {
            distances[node] = std::numeric_limits<Weight>::infinity();
        }
        distances[start] = Weight{};

        pq.emplace(Weight{}, start);

        while (!pq.empty()) {
            auto [currentDist, currentNode] = pq.top();
            pq.pop();

            // 如果找到更短的路径已经处理过该节点，跳过
            if (currentDist > distances[currentNode]) {
                continue;
            }

            // 遍历当前节点的所有邻居
            for (const auto& [neighbor, weight] : graph[currentNode]) {
                Weight newDist = currentDist + weight;

                // 如果找到更短的路径
                if (newDist < distances[neighbor]) {
                    distances[neighbor] = newDist;
                    previous[neighbor] = currentNode;
                    pq.emplace(newDist, neighbor);
                }
            }
        }

        // 构建结果
        return buildResult(start, distances, previous);
    }

    // 获取到特定目标节点的最短路径
    std::pair<Weight, std::vector<Node>> shortestPathTo(const Node& start, const Node& target) {
        auto result = shortestPath(start);
        auto it = result.find(target);
        if (it != result.end()) {
            return it->second;
        }
        return { std::numeric_limits<Weight>::infinity(), {} };
    }

    // 检查节点是否存在
    bool hasNode(const Node& node) const {
        return graph.contains(node);
    }

    // 获取图中的所有节点
    std::vector<Node> getNodes() const {
        std::vector<Node> nodes;
        nodes.reserve(graph.size());
        for (const auto& [node, _] : graph) {
            nodes.push_back(node);
        }
        return nodes;
    }

private:
    Graph graph;

    Result buildResult(const Node& start,
        const std::unordered_map<Node, Weight>& distances,
        const std::unordered_map<Node, Node>& previous) {
        Result result;

        for (const auto& [node, dist] : distances) {
            std::vector<Node> path;

            if (dist < std::numeric_limits<Weight>::infinity()) {
                // 重建路径
                Node current = node;
                while (current != start) {
                    path.push_back(current);
                    auto it = previous.find(current);
                    if (it == previous.end()) {
                        break;
                    }
                    current = it->second;
                }
                path.push_back(start);
                std::reverse(path.begin(), path.end());
            }

            result[node] = { dist, std::move(path) };
        }

        return result;
    }
};

// 便捷函数，用于创建Dijkstra实例
template<typename Node, typename Weight = double>
auto createDijkstra() {
    return Dijkstra<Node, Weight>();
}

/*
int main() {
    // 使用字符串作为节点
    auto dijkstra = createDijkstra<std::string, int>();

    // 添加边
    dijkstra.addEdge("A", "B", 4);
    dijkstra.addEdge("A", "C", 2);
    dijkstra.addEdge("B", "C", 1);
    dijkstra.addEdge("B", "D", 5);
    dijkstra.addEdge("C", "D", 8);
    dijkstra.addEdge("C", "E", 10);
    dijkstra.addEdge("D", "E", 2);

    // 计算从A到所有节点的最短路径
    auto result = dijkstra.shortestPath("A");

    // 输出结果
    for (const auto& [node, pathInfo] : result) {
        auto [distance, path] = pathInfo;
        std::cout << "To " << node << ": distance = " << distance << ", path = ";
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << path[i];
        }
        std::cout << std::endl;
    }

    // 获取到特定节点的路径
    auto [dist, path] = dijkstra.shortestPathTo("A", "E");
    std::cout << "\nShortest path from A to E: " << dist << std::endl;

    return 0;
}
*/