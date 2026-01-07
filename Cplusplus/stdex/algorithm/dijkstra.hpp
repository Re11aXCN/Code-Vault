#pragma once
#include <algorithm>
#include <concepts>
#include <functional>
#include <limits>
#include <queue>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace stdex {
template<typename Node, typename Weight = double>
requires std::is_arithmetic_v<Weight>
class Dijkstra
{
public:

  using NodeType   = Node;
  using WeightType = Weight;
  using Edge       = std::pair<Node, Weight>;
  using GraphType  = std::unordered_map<Node, std::vector<Edge>>;
  using ResultType =
      std::unordered_map<Node, std::pair<Weight, std::vector<Node>>>;

  // Add edge to graph
  void add_edge(const Node& from, const Node& to, Weight weight)
  {
    m_Graph [from].emplace_back(to, weight);
    // Ensure all nodes exist in graph, even if they only have outgoing or
    // incoming edges
    m_Graph.try_emplace(to, std::vector<Edge> {});
  }

  // Add bidirectional edge
  void add_bidirectional_edge(const Node& from, const Node& to, Weight weight)
  {
    add_edge(from, to, weight);
    add_edge(to, from, weight);
  }

  // Execute Dijkstra's algorithm
  ResultType shortest_path(const Node& start)
  {
    // Distance map storing shortest distance to each node
    std::unordered_map<Node, Weight> distances;
    // Previous node map for path reconstruction
    std::unordered_map<Node, Node> previous;
    // Priority queue ordered by distance
    using QueueElement = std::pair<Weight, Node>;
    std::priority_queue<QueueElement, std::vector<QueueElement>,
                        std::greater<QueueElement>>
        priority_queue;

    // Initialize distances
    for (const auto& [node, _] : m_Graph) {
      distances [node] = std::numeric_limits<Weight>::infinity();
    }
    distances [start] = Weight {};

    priority_queue.emplace(Weight {}, start);

    while (!priority_queue.empty()) {
      auto [current_dist, current_node] = priority_queue.top();
      priority_queue.pop();

      // Skip if already processed with shorter distance
      if (current_dist > distances [current_node]) { continue; }

      // Traverse all neighbors of current node
      for (const auto& [neighbor, edge_weight] : m_Graph [current_node]) {
        Weight new_dist = current_dist + edge_weight;

        // If shorter path found
        if (new_dist < distances [neighbor]) {
          distances [neighbor] = new_dist;
          previous [neighbor]  = current_node;
          priority_queue.emplace(new_dist, neighbor);
        }
      }
    }

    // Build result
    return build_result(start, distances, previous);
  }

  // Get shortest path to specific target node
  std::pair<Weight, std::vector<Node>> shortest_path_to(const Node& start,
                                                        const Node& target)
  {
    auto result = shortest_path(start);
    auto it     = result.find(target);
    if (it != result.end()) { return it->second; }
    return { std::numeric_limits<Weight>::infinity(), {} };
  }

  // Check if node exists
  bool has_node(const Node& node) const { return m_Graph.contains(node); }

  // Get all nodes in graph
  std::vector<Node> get_nodes() const
  {
    std::vector<Node> nodes;
    nodes.reserve(m_Graph.size());
    for (const auto& [node, _] : m_Graph) {
      nodes.push_back(node);
    }
    return nodes;
  }

private:

  GraphType m_Graph;

  ResultType build_result(const Node&                             start,
                          const std::unordered_map<Node, Weight>& distances,
                          const std::unordered_map<Node, Node>&   previous)
  {
    ResultType result;

    for (const auto& [node, dist] : distances) {
      std::vector<Node> path;

      if (dist < std::numeric_limits<Weight>::infinity()) {
        // Reconstruct path
        Node current = node;
        while (current != start) {
          path.push_back(current);
          auto it = previous.find(current);
          if (it == previous.end()) { break; }
          current = it->second;
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
      }

      result [node] = { dist, std::move(path) };
    }

    return result;
  }
};

// Convenience function for creating Dijkstra instance
template<typename Node, typename Weight = double>
auto create_dijkstra()
{
  return Dijkstra<Node, Weight>();
}
}  // namespace stdex

/*
// ========== Usage Examples ==========

int main() {
    // Use strings as nodes
    auto dijkstra = create_dijkstra<std::string, int>();

    // Add edges
    dijkstra.add_edge("A", "B", 4);
    dijkstra.add_edge("A", "C", 2);
    dijkstra.add_edge("B", "C", 1);
    dijkstra.add_edge("B", "D", 5);
    dijkstra.add_edge("C", "D", 8);
    dijkstra.add_edge("C", "E", 10);
    dijkstra.add_edge("D", "E", 2);

    // Calculate shortest path from A to all nodes
    auto result = dijkstra.shortest_path("A");

    // Output results
    for (const auto& [node, path_info] : result) {
        auto [distance, path] = path_info;
        std::cout << "To " << node << ": distance = " << distance << ", path =
"; for (size_t i = 0; i < path.size(); ++i) { if (i > 0) std::cout << " -> ";
            std::cout << path[i];
        }
        std::cout << std::endl;
    }

    // Get path to specific node
    auto [dist, path] = dijkstra.shortest_path_to("A", "E");
    std::cout << "\nShortest path from A to E: " << dist << std::endl;

    return 0;
}
*/
