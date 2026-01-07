#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace stdex {
template<typename Node, typename Weight = double>
requires std::is_arithmetic_v<Weight>
class AStar
{
public:

  using NodeType   = Node;
  using WeightType = Weight;
  using Edge       = std::pair<Node, Weight>;
  using GraphType  = std::unordered_map<Node, std::vector<Edge>>;

  // Heuristic function type: accepts current node and target node,
  // returns estimated cost
  using Heuristic = std::function<WeightType(const NodeType&, const NodeType&)>;

  void add_edge(const NodeType& from, const NodeType& to, WeightType weight)
  {
    m_Graph [from].emplace_back(to, weight);
    m_Graph.try_emplace(to, std::vector<Edge> {});
  }

  void add_bidirectional_edge(const NodeType& from, const NodeType& to,
                              WeightType weight)
  {
    add_edge(from, to, weight);
    add_edge(to, from, weight);
  }

  std::pair<WeightType, std::vector<NodeType>>
  shortest_path(const NodeType& start, const NodeType& target,
                Heuristic heuristic_func)
  {
    if (!has_node(start) || !has_node(target)) {
      return { std::numeric_limits<WeightType>::infinity(), {} };
    }

    // If start is the same as target
    if (start == target) { return { WeightType {}, { start } }; }

    // Priority queue, sorted by f(n) = g(n) + h(n)
    using QueueElement =
        std::tuple<WeightType, WeightType, NodeType>;  // f(n), g(n), node
    auto cmp = [](const QueueElement& a, const QueueElement& b) {
      return std::get<0>(a) > std::get<0>(b);  // min-heap
    };
    std::priority_queue<QueueElement, std::vector<QueueElement>, decltype(cmp)>
        open_set(cmp);

    // Actual cost g(n): actual distance from start to current node
    std::unordered_map<NodeType, WeightType> g_score;
    // Predecessor node mapping
    std::unordered_map<NodeType, NodeType> came_from;
    for (const auto& [node, _] : m_Graph) {
      g_score [node] = std::numeric_limits<WeightType>::infinity();
    }
    g_score [start] = WeightType {};

    WeightType start_heuristic = heuristic_func(start, target);
    open_set.emplace(start_heuristic, WeightType {}, start);

    while (!open_set.empty()) {
      auto [f_current, g_current, current_node] = open_set.top();
      open_set.pop();

      // If target node is reached
      if (current_node == target) {
        return { g_current, reconstruct_path(came_from, current_node) };
      }

      // If current g(n) is not optimal, skip
      // (due to possible duplicate nodes in priority queue)
      if (g_current > g_score [current_node]) { continue; }

      // Traverse neighbors
      for (const auto& [neighbor, edge_weight] : m_Graph [current_node]) {
        WeightType tentative_g = g_current + edge_weight;

        // If a better path is found
        if (tentative_g < g_score [neighbor]) {
          // Update path information
          came_from [neighbor] = current_node;
          g_score [neighbor]   = tentative_g;
          WeightType f_score   = tentative_g + heuristic_func(neighbor, target);
          open_set.emplace(f_score, tentative_g, neighbor);
        }
      }
    }

    return { std::numeric_limits<WeightType>::infinity(), {} };
  }

  bool has_node(const NodeType& node) const { return m_Graph.contains(node); }

  std::vector<NodeType> get_nodes() const
  {
    std::vector<NodeType> nodes;
    nodes.reserve(m_Graph.size());
    for (const auto& [node, _] : m_Graph) {
      nodes.push_back(node);
    }
    return nodes;
  }

private:

  GraphType m_Graph;

  std::vector<NodeType>
  reconstruct_path(const std::unordered_map<NodeType, NodeType>& came_from,
                   NodeType                                      current)
  {
    std::vector<NodeType> path;
    while (came_from.contains(current)) {
      path.push_back(current);
      current = came_from.at(current);
    }
    path.push_back(current);  // Add start node
    std::reverse(path.begin(), path.end());
    return path;
  }
};

// Common heuristic functions
namespace heuristics {
// Manhattan distance (for grids with only horizontal and vertical movement)
template<typename Weight = int>
auto manhattan_distance(int dx, int dy)
{
  return [dx, dy](const std::pair<int, int>& a,
                  const std::pair<int, int>& b) -> Weight {
    return std::abs(a.first - b.first) * dx +
        std::abs(a.second - b.second) * dy;
  };
}

// Euclidean distance (for arbitrary direction movement)
template<typename Weight = double>
auto euclidean_distance()
{
  return [](const std::pair<Weight, Weight>& a,
            const std::pair<Weight, Weight>& b) -> Weight {
    Weight dx = a.first - b.first;
    Weight dy = a.second - b.second;
    return std::sqrt(dx * dx + dy * dy);
  };
}

// Chebyshev distance (for 8-directional movement)
template<typename Weight = int>
auto chebyshev_distance()
{
  return
      [](const std::pair<int, int>& a, const std::pair<int, int>& b) -> Weight {
    return std::max(std::abs(a.first - b.first), std::abs(a.second - b.second));
  };
}

// Zero heuristic (degrades to Dijkstra's algorithm)
template<typename Node, typename Weight = double>
auto zero_heuristic()
{
  return [](const Node&, const Node&) -> Weight { return Weight {}; };
}
}  // namespace heuristics

// Convenience function for creating AStar instance
template<typename Node, typename Weight = double>
auto create_a_star()
{
  return AStar<Node, Weight>();
}
}  // namespace stdex

/*
// ========== Usage Examples ==========

// Example 1: String nodes (using simple string distance as heuristic)
int string_distance(const std::string& a, const std::string& b) {
    // Simple heuristic: absolute difference of string lengths
    return std::abs(static_cast<int>(a.length()) -
                    static_cast<int>(b.length()));
}

void example1() {
    std::cout << "=== Example 1: String Nodes ===" << std::endl;

    auto astar = create_a_star<std::string, int>();

    // Build graph
    astar.add_edge("A", "B", 4);
    astar.add_edge("A", "C", 2);
    astar.add_edge("B", "C", 1);
    astar.add_edge("B", "D", 5);
    astar.add_edge("C", "D", 8);
    astar.add_edge("C", "E", 10);
    astar.add_edge("D", "E", 2);
    astar.add_edge("D", "F", 6);
    astar.add_edge("E", "F", 3);

    // Custom heuristic function
    auto heuristic_func = [](const std::string& current,
                             const std::string& target) -> int {
        return string_distance(current, target);
    };

    auto [distance, path] = astar.shortest_path("A", "F", heuristic_func);

    std::cout << "Distance: " << distance << std::endl;
    std::cout << "Path: ";
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << path[i];
    }
    std::cout << std::endl << std::endl;
}

// Example 2: 2D coordinate points
void example2() {
    std::cout << "=== Example 2: 2D Coordinates ===" << std::endl;

    using Point = std::pair<int, int>;
    auto astar = create_a_star<Point, double>();

    // Build a simple grid graph
    for (int x = 0; x <= 5; ++x) {
        for (int y = 0; y <= 5; ++y) {
            // Add adjacent node edges
            if (x < 5) astar.add_bidirectional_edge({x, y}, {x + 1, y}, 1.0);
            if (y < 5) astar.add_bidirectional_edge({x, y}, {x, y + 1}, 1.0);
            // Diagonal
            if (x < 5 && y < 5)
                astar.add_bidirectional_edge({x, y}, {x + 1, y + 1}, 1.414);
        }
    }

    Point start  = {0, 0};
    Point target = {5, 5};

    // Use Euclidean distance as heuristic
    auto heuristic_func = heuristics::euclidean_distance<double>();

    auto [distance, path] = astar.shortest_path(start, target, heuristic_func);

    std::cout << "Distance: " << distance << std::endl;
    std::cout << "Path: ";
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << "(" << path[i].first << "," << path[i].second << ")";
    }
    std::cout << std::endl << std::endl;
}

// Example 3: Comparison with Dijkstra
void example3() {
    std::cout << "=== Example 3: A* vs Dijkstra ===" << std::endl;

    auto astar = create_a_star<std::string, int>();

    // Build same graph
    astar.add_edge("Start", "A", 1);
    astar.add_edge("Start", "B", 4);
    astar.add_edge("A", "C", 2);
    astar.add_edge("B", "C", 1);
    astar.add_edge("C", "D", 3);
    astar.add_edge("D", "Goal", 2);
    astar.add_edge("B", "Goal", 5);

    // A* with heuristic
    auto heuristic_func = [](const std::string& current,
                             const std::string& target) -> int {
        // Simple heuristic: if target is "Goal", give a small estimate
        if (target == "Goal") {
            return (current == "Goal") ? 0 : 1;
        }
        return 0;
    };

    auto [a_star_distance, a_star_path] =
        astar.shortest_path("Start", "Goal", heuristic_func);

    // For comparison, use zero heuristic (equivalent to Dijkstra)
    auto dijkstra_result = astar.shortest_path(
        "Start", "Goal", heuristics::zero_heuristic<std::string, int>());

    std::cout << "A* Result - Distance: " << a_star_distance
              << ", Path length: " << a_star_path.size() << std::endl;
    std::cout << "Dijkstra Result - Distance: " << dijkstra_result.first
              << ", Path length: " << dijkstra_result.second.size() <<
std::endl; std::cout << "Paths are "
              << (a_star_path == dijkstra_result.second ? "identical" :
"different")
              << std::endl;
}

int main() {
    example1();
    example2();
    example3();

    return 0;
}
*/
