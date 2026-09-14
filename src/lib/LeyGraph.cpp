#include "leylib/NodeHandle.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <leylib/LeyID.h>
#include <limits>
#include <ortools/graph/max_flow.h>
#include <stdexcept>
#include <utility>
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

using Solver = operations_research::SimpleMaxFlow;
constexpr long double kFlowScale = 1000.0L;

Solver::FlowQuantity ToSolverQuantity(float value) {
  const long double scaled =
      std::floor(static_cast<long double>(value) * kFlowScale);

  return static_cast<Solver::FlowQuantity>(scaled);
}

float FromSolverQuantity(Solver::FlowQuantity value) {
  return static_cast<float>(static_cast<long double>(value) / kFlowScale);
}

void LeyGraph::solveFlow() {
  Solver solver;
  const auto superSource = static_cast<Solver::NodeIndex>(graph_.size());
  const auto superSink = superSource + 1;

  solver.AddArcWithCapacity(superSource, superSink, 0);
  std::vector<Solver::ArcIndex> edgeArcs(edges_.size(), -1);
  std::vector<Solver::ArcIndex> nodeArcs(graph_.size(), -1);
  // Calculate total supply
  // Add arcs with capacity from each node

  // We add arcs to simulate sinks/supply nodes
  // Calculate virtual edges
  for (size_t i = 0; i < graph_.size(); ++i) {
    const auto &node = graph_[i];

    if (!node.alive)
      continue;

    const auto amount = ToSolverQuantity(std::abs(node.supply));

    const auto index = static_cast<Solver::NodeIndex>(i);

    if (node.supply > 0) {
      nodeArcs[i] = solver.AddArcWithCapacity(superSource, index, amount);
    } else if (node.supply < 0) {
      nodeArcs[i] = solver.AddArcWithCapacity(index, superSink, amount);
    }
  }

  // Add the actual edges
  for (size_t i = 0; i < edges_.size(); ++i) {
    const auto &edge = edges_[i];
    if (!edge.alive)
      continue;

    if (!graph_[edge.aIndex].alive || !graph_[edge.bIndex].alive)
      continue;

    edgeArcs[i] =
        solver.AddArcWithCapacity(static_cast<Solver::NodeIndex>(edge.aIndex),
                                  static_cast<Solver::NodeIndex>(edge.bIndex),
                                  ToSolverQuantity(edge.capacity));
  }

  auto solveStatus = solver.Solve(superSource, superSink);

  if (solveStatus != Solver::OPTIMAL) {
    return;
    // TODO add some logging
  }

  allocations_.assign(graph_.size(), 0.0f);

  for (size_t i = 0; i < nodeArcs.size(); ++i)
    if (nodeArcs[i] >= 0)
      allocations_[i] = FromSolverQuantity(solver.Flow(nodeArcs[i]));
  for (size_t i = 0; i < edges_.size(); ++i)
    edges_[i].flow =
        edgeArcs[i] >= 0 ? FromSolverQuantity(solver.Flow(edgeArcs[i])) : 0.0f;
}
