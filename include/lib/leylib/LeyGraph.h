#pragma once
#include <cstddef>
#include <cstdint>
#include <leylib/LeyID.h>
#include <vector>

#include <leylib/LeyLine.h>
#include <leylib/LeyNode.h>
#include <leylib/NodeIdGenerator.h>

class FlowRndGenerator;

class LeyGraph {
public:
  LeyGraph(uint32_t seed);
  ~LeyGraph();

  LeyID addLine(LeyID nodeA, LeyID nodeB, float capacity);
  LeyID addTerminal(LeyID node, float capacity);
  void removeLine(LeyID lineIndex);

  LeyID addNode(float supply);
  void removeNode(LeyID nodeIndex);
  LeyID splitLine(LeyID lineIndex, float supply);

  // Standard maximum flow, with no sink priority. Quantities are rounded down
  // to 0.001 units. Throws for invalid inputs or an unsuccessful solver status.
  void solveFlow();
  // Latest successful solve: source production, sink consumption, junction
  // zero.
  const std::vector<float> &allocations() const { return allocations_; }

  size_t size() const { return graph_.size(); }
  const std::vector<LeyNode> &nodes() const { return graph_; }
  const std::vector<LeyLine> &lines() const { return edges_; }

  size_t degree(LeyID nodeIndex) const;

protected:
  LeyGraph();

private:
  void generateSourceTrees(size_t treeCount, FlowRndGenerator &flowGenerator);
  LeyID allocLineSlot();
  void detachLine(LeyID nodeIndex, LeyID lineIndex);

  std::vector<LeyNode> graph_;
  std::vector<LeyLine> edges_;
  std::vector<LeyID> freeLines_; // reusable slots in edges_
  NodeIdGenerator nodeIds_;      // slot allocator / free list for graph_
  std::vector<float> allocations_;
};
