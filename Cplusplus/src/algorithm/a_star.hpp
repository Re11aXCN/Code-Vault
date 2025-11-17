#pragma once

#include <vector>
#include <queue>
#include <limits>
#include <type_traits>
#include <concepts>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cmath>

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template<typename Node, typename Weight = double>
    requires Arithmetic<Weight>
class AStar {
public:
    using NodeType = Node;
    using WeightType = Weight;
    using Edge = std::pair<Node, Weight>;
    using Graph = std::unordered_map<Node, std::vector<Edge>>;

    // 启发式函数类型：接受当前节点和目标节点，返回估计代价
    using Heuristic = std::function<Weight(const Node&, const Node&)>;

    // 添加边
    void addEdge(const Node& from, const Node& to, Weight weight) {
        graph[from].emplace_back(to, weight);
        graph.try_emplace(to, std::vector<Edge>{});
    }

    // 添加双向边
    void addBidirectionalEdge(const Node& from, const Node& to, Weight weight) {
        addEdge(from, to, weight);
        addEdge(to, from, weight);
    }

    // 执行A*算法
    std::pair<Weight, std::vector<Node>> shortestPath(
        const Node& start,
        const Node& target,
        Heuristic heuristic) {

        // 如果起点或终点不存在
        if (!hasNode(start) || !hasNode(target)) {
            return { std::numeric_limits<Weight>::infinity(), {} };
        }

        // 如果起点就是终点
        if (start == target) {
            return { Weight{}, {start} };
        }

        // 优先队列，按f(n) = g(n) + h(n)排序
        using QueueElement = std::tuple<Weight, Weight, Node>; // f(n), g(n), node
        auto cmp = [](const QueueElement& a, const QueueElement& b) {
            return std::get<0>(a) > std::get<0>(b); // 最小堆
            };
        std::priority_queue<QueueElement, std::vector<QueueElement>, decltype(cmp)> pq(cmp);

        // 实际代价g(n)：从起点到当前节点的实际距离
        std::unordered_map<Node, Weight> gScore;
        // 前驱节点映射
        std::unordered_map<Node, Node> cameFrom;

        // 初始化
        for (const auto& [node, _] : graph) {
            gScore[node] = std::numeric_limits<Weight>::infinity();
        }
        gScore[start] = Weight{};

        Weight startHeuristic = heuristic(start, target);
        pq.emplace(startHeuristic, Weight{}, start);

        while (!pq.empty()) {
            auto [fCurrent, gCurrent, currentNode] = pq.top();
            pq.pop();

            // 如果到达目标节点
            if (currentNode == target) {
                return { gCurrent, reconstructPath(cameFrom, currentNode) };
            }

            // 如果当前g(n)不是最优的，跳过（由于优先队列中可能有重复节点）
            if (gCurrent > gScore[currentNode]) {
                continue;
            }

            // 遍历邻居
            for (const auto& [neighbor, weight] : graph[currentNode]) {
                Weight tentativeG = gCurrent + weight;

                // 如果找到更优路径
                if (tentativeG < gScore[neighbor]) {
                    // 更新路径信息
                    cameFrom[neighbor] = currentNode;
                    gScore[neighbor] = tentativeG;
                    Weight fScore = tentativeG + heuristic(neighbor, target);
                    pq.emplace(fScore, tentativeG, neighbor);
                }
            }
        }

        // 未找到路径
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

    // 重建路径
    std::vector<Node> reconstructPath(const std::unordered_map<Node, Node>& cameFrom, Node current) {
        std::vector<Node> path;
        while (cameFrom.contains(current)) {
            path.push_back(current);
            current = cameFrom.at(current);
        }
        path.push_back(current); // 添加起点
        std::reverse(path.begin(), path.end());
        return path;
    }
};

// 常用的启发式函数
namespace Heuristics {

    // 曼哈顿距离（适用于网格，只能水平和垂直移动）
    template<typename Weight = int>
    auto manhattanDistance(int dx, int dy) {
        return [dx, dy](const std::pair<int, int>& a, const std::pair<int, int>& b) -> Weight {
            return std::abs(a.first - b.first) * dx + std::abs(a.second - b.second) * dy;
            };
    }

    // 欧几里得距离（适用于可以任意方向移动的情况）
    template<typename Weight = double>
    auto euclideanDistance() {
        return [](const std::pair<Weight, Weight>& a, const std::pair<Weight, Weight>& b) -> Weight {
            Weight dx = a.first - b.first;
            Weight dy = a.second - b.second;
            return std::sqrt(dx * dx + dy * dy);
            };
    }

    // 切比雪夫距离（适用于可以8方向移动的情况）
    template<typename Weight = int>
    auto chebyshevDistance() {
        return [](const std::pair<int, int>& a, const std::pair<int, int>& b) -> Weight {
            return std::max(std::abs(a.first - b.first), std::abs(a.second - b.second));
            };
    }

    // 零启发式（退化为Dijkstra算法）
    template<typename Node, typename Weight = double>
    auto zeroHeuristic() {
        return [](const Node&, const Node&) -> Weight {
            return Weight{};
            };
    }
}

// 便捷函数，用于创建AStar实例
template<typename Node, typename Weight = double>
auto createAStar() {
    return AStar<Node, Weight>();
}

/*
#include "dijkstra.hpp"
// 示例1：字符串节点（使用简单的字符串距离作为启发式）
int stringDistance(const std::string& a, const std::string& b) {
    // 简单的启发式：字符串长度差的绝对值
    return std::abs(static_cast<int>(a.length()) - static_cast<int>(b.length()));
}

void example1() {
    std::cout << "=== Example 1: String Nodes ===" << std::endl;

    auto astar = createAStar<std::string, int>();

    // 构建图
    astar.addEdge("A", "B", 4);
    astar.addEdge("A", "C", 2);
    astar.addEdge("B", "C", 1);
    astar.addEdge("B", "D", 5);
    astar.addEdge("C", "D", 8);
    astar.addEdge("C", "E", 10);
    astar.addEdge("D", "E", 2);
    astar.addEdge("D", "F", 6);
    astar.addEdge("E", "F", 3);

    // 自定义启发式函数
    auto heuristic = [](const std::string& current, const std::string& target) -> int {
        return stringDistance(current, target);
    };

    auto [distance, path] = astar.shortestPath("A", "F", heuristic);

    std::cout << "Distance: " << distance << std::endl;
    std::cout << "Path: ";
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << path[i];
    }
    std::cout << std::endl << std::endl;
}

// 示例2：二维坐标点
void example2() {
    std::cout << "=== Example 2: 2D Coordinates ===" << std::endl;

    using Point = std::pair<int, int>;
    auto astar = createAStar<Point, double>();

    // 构建一个简单的网格图
    for (int x = 0; x <= 5; ++x) {
        for (int y = 0; y <= 5; ++y) {
            // 添加相邻节点的边
            if (x < 5) astar.addBidirectionalEdge({x, y}, {x + 1, y}, 1.0);
            if (y < 5) astar.addBidirectionalEdge({x, y}, {x, y + 1}, 1.0);
            // 对角线
            if (x < 5 && y < 5) astar.addBidirectionalEdge({x, y}, {x + 1, y + 1}, 1.414);
        }
    }

    Point start = {0, 0};
    Point target = {5, 5};

    // 使用欧几里得距离作为启发式
    auto heuristic = Heuristics::euclideanDistance<double>();

    auto [distance, path] = astar.shortestPath(start, target, heuristic);

    std::cout << "Distance: " << distance << std::endl;
    std::cout << "Path: ";
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << "(" << path[i].first << "," << path[i].second << ")";
    }
    std::cout << std::endl << std::endl;
}

// 示例3：与Dijkstra比较
void example3() {
    std::cout << "=== Example 3: A* vs Dijkstra ===" << std::endl;

    auto astar = createAStar<std::string, int>();

    // 构建相同的图
    astar.addEdge("Start", "A", 1);
    astar.addEdge("Start", "B", 4);
    astar.addEdge("A", "C", 2);
    astar.addEdge("B", "C", 1);
    astar.addEdge("C", "D", 3);
    astar.addEdge("D", "Goal", 2);
    astar.addEdge("B", "Goal", 5);

    // A* 使用启发式
    auto heuristic = [](const std::string& current, const std::string& target) -> int {
        // 简单的启发式：如果目标就是"Goal"，给一个小的估计值
        if (target == "Goal") {
            return (current == "Goal") ? 0 : 1;
        }
        return 0;
    };

    auto [aStarDistance, aStarPath] = astar.shortestPath("Start", "Goal", heuristic);

    // 作为对比，使用零启发式（相当于Dijkstra）
    auto dijkstraResult = astar.shortestPath("Start", "Goal", Heuristics::zeroHeuristic<std::string, int>());

    std::cout << "A* Result - Distance: " << aStarDistance << ", Path length: " << aStarPath.size() << std::endl;
    std::cout << "Dijkstra Result - Distance: " << dijkstraResult.first << ", Path length: " << dijkstraResult.second.size() << std::endl;
    std::cout << "Paths are " << (aStarPath == dijkstraResult.second ? "identical" : "different") << std::endl;
}

int main() {
    example1();
    example2();
    example3();

    return 0;
}
*/