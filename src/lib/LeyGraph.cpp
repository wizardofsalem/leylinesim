#include "leylib/NodeHandle.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <leylib/LeyID.h>
#include <vector>

#include <leylib/FlowRndGenerator.h>
#include <leylib/LeyGraph.h>
#include <leylib/LeyLine.h>
#include <leylib/LeyNode.h>
#include <leylib/NodeIdGenerator.h>

LeyGraph::LeyGraph(uint32_t seed) {
  FlowRndGenerator flowGenerator(1000, 250, seed);
}

LeyGraph::LeyGraph() = default;

LeyGraph::~LeyGraph() = default;

LeyID LeyGraph::addLine(LeyID nodeA, LeyID nodeB, float capacity) {
  const LeyID lineId = allocLineSlot();
  LeyLine newLine{.id = lineId,
                  .aIndex = nodeA,
                  .bIndex = nodeB,
                  .capacity = capacity,
                  .alive = true};
  graph_[nodeA].connections.push_back(lineId);
  graph_[nodeB].connections.push_back(lineId);
  edges_[lineId] = newLine;
  return lineId;
}

LeyID LeyGraph::allocLineSlot() {
  if (!freeLines_.empty()) {
    const LeyID lineId = freeLines_.back();
    freeLines_.pop_back();
    return lineId;
  }

  const LeyID lineId = static_cast<LeyID>(edges_.size());
  edges_.emplace_back();
  return lineId;
}

void LeyGraph::detachLine(LeyID nodeIndex, LeyID lineIndex) {}

void LeyGraph::removeLine(LeyID lineIndex) {}

LeyID LeyGraph::addNode(float supply) {
  auto nodeId = nodeIds_.generateId();
  auto newNode = LeyNode{.handle = NodeHandle{.id = nodeId, .gen = 0},
                         .supply = supply,
                         .alive = true};

  if (nodeId == graph_.size()) {
    graph_.push_back(newNode);
  } else {
    graph_[nodeId] = newNode;
  }
  return nodeId;
}

void LeyGraph::removeNode(LeyID nodeIndex) {}

LeyID LeyGraph::splitLine(LeyID lineIndex, float supply) { return 0; }

void LeyGraph::solveFlow() {
  std::set<LeyNode *, FlowComparator> sinkPriorityList{};
  for (auto &node : graph_) {
    if (node.supply < 0) {
      sinkPriorityList.insert(&node);
    }
  }

  for (auto sink : sinkPriorityList) {
    for (auto connection : sink->connections) {

      auto &edge = edges_[connection];
      auto other = edge.aIndex == sink->handle.id ? edge.bIndex : edge.aIndex;
      if (graph_[other].supply > 0) {
        float edgeFlow = graph_[other].supply - abs(sink->supply) > 0
                             ? abs(sink->supply)
                             : graph_[other].supply;

        edge.flow = edgeFlow;
      }
    }
  }
}
