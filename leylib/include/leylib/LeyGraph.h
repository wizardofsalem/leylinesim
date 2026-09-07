#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

class LeyNode;

class LeyGraph {
public:
  LeyGraph() = delete;
  explicit LeyGraph(size_t initialSize);
  ~LeyGraph();

private:
  std::vector<LeyNode> graph_;
};
