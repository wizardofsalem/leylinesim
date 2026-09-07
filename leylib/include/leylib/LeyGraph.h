#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

#include <leylib/LeyLine.h>
#include <leylib/LeyNode.h>
#include <leylib/NodeIdGenerator.h>

class FlowRndGenerator;

class LeyGraph {
public:
  LeyGraph() = delete;
  LeyGraph(size_t initialSize, uint32_t seed);
  ~LeyGraph();

  uint32_t addLine(uint32_t nodeA, uint32_t nodeB, float capacity);
  uint32_t addTerminal(uint32_t node, float capacity);
  void removeLine(uint32_t lineIndex);

  uint32_t addNode(float supply);
  void removeNode(uint32_t nodeIndex);
  uint32_t splitLine(uint32_t lineIndex, float supply);

  void solveFlow();

  size_t size() const { return graph_.size(); }
  const std::vector<LeyNode> &nodes() const { return graph_; }
  const std::vector<LeyLine> &lines() const { return edges_; }

  size_t degree(uint32_t nodeIndex) const;

private:
  void generateSourceTrees(size_t treeCount, FlowRndGenerator &flowGenerator);
  uint32_t allocLineSlot();
  void detachLine(uint32_t nodeIndex, uint32_t lineIndex);

  std::vector<LeyNode> graph_;
  std::vector<LeyLine> edges_;
  std::vector<uint32_t> freeLines_; // reusable slots in edges_
  NodeIdGenerator nodeIds_;         // slot allocator / free list for graph_
};
