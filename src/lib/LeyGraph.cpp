#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <leylib/FlowRndGenerator.h>
#include <leylib/LeyGraph.h>
#include <leylib/LeyLine.h>
#include <leylib/LeyNode.h>
#include <leylib/NodeIdGenerator.h>

void LeyGraph::generateSourceTrees(size_t treeCount,
                                   FlowRndGenerator &flowGenerator) {
  graph_.reserve(treeCount);
  edges_.reserve(treeCount);

  for (size_t i = 0; i < treeCount; ++i) {
    const uint32_t source = addNode(static_cast<float>(flowGenerator.next()));
    addTerminal(source, graph_[source].supply);
  }
}

LeyGraph::LeyGraph(size_t initialSize, uint32_t seed) {
  FlowRndGenerator flowGenerator(1000, 250, seed);
  generateSourceTrees(initialSize, flowGenerator);
}

LeyGraph::~LeyGraph() = default;

uint32_t LeyGraph::allocLineSlot() {
  if (!freeLines_.empty()) {
    const uint32_t index = freeLines_.back();
    freeLines_.pop_back();
    return index;
  }
  const uint32_t index = static_cast<uint32_t>(edges_.size());
  edges_.emplace_back();
  return index;
}

uint32_t LeyGraph::addLine(uint32_t nodeA, uint32_t nodeB, float capacity) {
  const uint32_t index = allocLineSlot();
  edges_[index] = LeyLine{
      .aIndex = nodeA,
      .bIndex = nodeB,
      .nextIncidentA = graph_[nodeA].firstIncident,
      .nextIncidentB = graph_[nodeB].firstIncident,
      .capacity = capacity,
      .flow = 0.0f,
      .alive = true,
  };
  graph_[nodeA].firstIncident = index;
  graph_[nodeB].firstIncident = index;
  return index;
}

uint32_t LeyGraph::addTerminal(uint32_t node, float capacity) {
  const uint32_t index = allocLineSlot();
  edges_[index] = LeyLine{
      .aIndex = node,
      .bIndex = kNoNode,
      .nextIncidentA = graph_[node].firstIncident,
      .nextIncidentB = kNoLine,
      .capacity = capacity,
      .flow = 0.0f,
      .alive = true,
  };
  graph_[node].firstIncident = index;
  return index;
}

void LeyGraph::detachLine(uint32_t nodeIndex, uint32_t lineIndex) {
  uint32_t *link = &graph_[nodeIndex].firstIncident;
  while (*link != kNoLine) {
    if (*link == lineIndex) {
      *link = edges_[lineIndex].nextIncident(nodeIndex);
      return;
    }
    link = &edges_[*link].nextIncident(nodeIndex);
  }
}

void LeyGraph::removeLine(uint32_t lineIndex) {
  LeyLine &line = edges_[lineIndex];
  if (!line.alive) {
    return;
  }
  detachLine(line.aIndex, lineIndex);
  if (line.bIndex != kNoNode) {
    detachLine(line.bIndex, lineIndex);
  }
  line.alive = false;
  freeLines_.push_back(lineIndex);
}

size_t LeyGraph::degree(uint32_t nodeIndex) const {
  size_t count = 0;
  for (uint32_t e = graph_[nodeIndex].firstIncident; e != kNoLine;
       e = edges_[e].nextIncident(nodeIndex)) {
    ++count;
  }
  return count;
}

uint32_t LeyGraph::addNode(float supply) {
  const uint32_t index = nodeIds_.generateId();

  if (index < graph_.size()) {
    LeyNode &node = graph_[index];
    node = LeyNode{
        .handle = {.id = index, .gen = node.handle.gen},
        .supply = supply,
        .alive = true,
    };
  } else {
    graph_.push_back(LeyNode{
        .handle = {.id = index, .gen = 0},
        .supply = supply,
        .alive = true,
    });
  }
  return index;
}

void LeyGraph::removeNode(uint32_t nodeIndex) {
  LeyNode &node = graph_[nodeIndex];
  if (!node.alive) {
    return;
  }

  while (node.firstIncident != kNoLine) {
    removeLine(node.firstIncident);
  }

  node.alive = false;
  ++node.handle.gen; // stale handles now fail the generation check
  nodeIds_.freeId(nodeIndex);
}

uint32_t LeyGraph::splitLine(uint32_t lineIndex, float supply) {
  const LeyLine &line = edges_[lineIndex];
  if (!line.alive) {
    return kNoNode;
  }
  const uint32_t a = line.aIndex;
  const uint32_t b = line.bIndex; // may be kNoNode
  const float capacity = line.capacity;

  removeLine(lineIndex);
  const uint32_t mid = addNode(supply);
  addLine(a, mid, capacity);
  if (b == kNoNode) {
    addTerminal(mid, capacity);
  } else {
    addLine(mid, b, capacity);
  }
  return mid;
}

void LeyGraph::solveFlow() {
  const uint32_t n = static_cast<uint32_t>(graph_.size());
  for (LeyLine &line : edges_) {
    line.flow = 0.0f;
  }

  std::vector<uint32_t> parentNode(n, kNoNode);
  std::vector<uint32_t> order;
  order.reserve(n);
  std::vector<uint8_t> seen(n, 0);

  for (uint32_t root = 0; root < n; ++root) {
    if (!graph_[root].alive || seen[root]) {
      continue;
    }
    seen[root] = 1;
    const size_t compStart = order.size();
    order.push_back(root);
    for (size_t h = compStart; h < order.size(); ++h) {
      const uint32_t u = order[h];
      for (uint32_t e = graph_[u].firstIncident; e != kNoLine;
           e = edges_[e].nextIncident(u)) {
        if (edges_[e].isTerminal()) {
          continue;
        }
        const uint32_t v = edges_[e].other(u);
        if (!seen[v]) {
          seen[v] = 1;
          parentNode[v] = u;
          order.push_back(v);
        }
      }
    }
  }

  std::vector<uint32_t> drainsBelow(n, 0);
  for (size_t i = order.size(); i-- > 0;) {
    const uint32_t u = order[i];
    for (uint32_t e = graph_[u].firstIncident; e != kNoLine;
         e = edges_[e].nextIncident(u)) {
      if (edges_[e].isTerminal()) {
        drainsBelow[u] += 1;
      }
    }
    if (parentNode[u] != kNoNode) {
      drainsBelow[parentNode[u]] += drainsBelow[u];
    }
  }
  std::vector<uint32_t> totalDrains(n, 0);
  for (uint32_t u : order) {
    totalDrains[u] = (parentNode[u] == kNoNode) ? drainsBelow[u]
                                                : totalDrains[parentNode[u]];
  }

  auto drainsBeyond = [&](uint32_t node, uint32_t e) -> uint32_t {
    const LeyLine &line = edges_[e];
    if (line.isTerminal()) {
      return 1;
    }
    const uint32_t w = line.other(node);
    if (parentNode[node] == w) {
      return totalDrains[node] - drainsBelow[node];
    }
    return drainsBelow[w];
  };

  struct Frame {
    uint32_t node;
    float amount;
    uint32_t parent;
  };

  auto fanOut = [&](uint32_t source) {
    std::vector<Frame> stack;
    stack.push_back({source, graph_[source].supply, kNoLine});
    while (!stack.empty()) {
      const Frame f = stack.back();
      stack.pop_back();
      if (f.amount <= 0.0f) {
        continue;
      }

      float capSum = 0.0f;
      uint32_t eligible = 0;
      for (uint32_t e = graph_[f.node].firstIncident; e != kNoLine;
           e = edges_[e].nextIncident(f.node)) {
        if (e == f.parent || drainsBeyond(f.node, e) == 0) {
          continue;
        }
        capSum += edges_[e].capacity;
        ++eligible;
      }
      if (eligible == 0) {
        continue; // flow pools here (drainless subtree)
      }

      for (uint32_t e = graph_[f.node].firstIncident; e != kNoLine;
           e = edges_[e].nextIncident(f.node)) {
        if (e == f.parent || drainsBeyond(f.node, e) == 0) {
          continue;
        }
        const float weight = capSum > 0.0f
                                 ? edges_[e].capacity / capSum
                                 : 1.0f / static_cast<float>(eligible);
        const float share = f.amount * weight;
        const uint32_t far = edges_[e].other(f.node);
        edges_[e].pushTowards(far, share);
        if (!edges_[e].isTerminal()) {
          stack.push_back({far, share, e});
        }
      }
    }
  };

  for (uint32_t s = 0; s < n; ++s) {
    if (graph_[s].alive && graph_[s].supply > 0.0f) {
      fanOut(s);
    }
  }

  auto drawDown = [&](uint32_t sink) {
    float inflow = 0.0f;
    for (uint32_t e = graph_[sink].firstIncident; e != kNoLine;
         e = edges_[e].nextIncident(sink)) {
      const float towards = edges_[e].flowTowards(sink);
      if (towards > 0.0f) {
        inflow += towards;
      }
    }
    const float taken = std::min(inflow, -graph_[sink].supply);
    if (taken <= 0.0f) {
      return;
    }

    std::vector<Frame> stack;
    stack.push_back({sink, taken, kNoLine});
    while (!stack.empty()) {
      const Frame f = stack.back();
      stack.pop_back();
      if (f.amount <= 0.0f) {
        continue;
      }

      float outSum = 0.0f;
      for (uint32_t e = graph_[f.node].firstIncident; e != kNoLine;
           e = edges_[e].nextIncident(f.node)) {
        if (e == f.parent) {
          continue;
        }
        const float leaving = -edges_[e].flowTowards(f.node);
        if (leaving > 0.0f) {
          outSum += leaving;
        }
      }
      if (outSum <= 0.0f) {
        continue;
      }

      for (uint32_t e = graph_[f.node].firstIncident; e != kNoLine;
           e = edges_[e].nextIncident(f.node)) {
        if (e == f.parent) {
          continue;
        }
        const float leaving = -edges_[e].flowTowards(f.node);
        if (leaving <= 0.0f) {
          continue;
        }
        // Clamp so a share can never push an edge's flow past zero.
        const float cut = std::min(f.amount * (leaving / outSum), leaving);
        edges_[e].pushTowards(f.node, cut); // reduce the outflow
        if (!edges_[e].isTerminal()) {
          stack.push_back({edges_[e].other(f.node), cut, e});
        }
      }
    }
  };

  constexpr float kFlowEps = 1e-4f;
  std::vector<uint32_t> indeg(n, 0);
  for (uint32_t v = 0; v < n; ++v) {
    if (!graph_[v].alive) {
      continue;
    }
    for (uint32_t e = graph_[v].firstIncident; e != kNoLine;
         e = edges_[e].nextIncident(v)) {
      if (!edges_[e].isTerminal() && edges_[e].flowTowards(v) > kFlowEps) {
        ++indeg[v];
      }
    }
  }
  std::vector<uint32_t> topo;
  topo.reserve(n);
  for (uint32_t v = 0; v < n; ++v) {
    if (graph_[v].alive && indeg[v] == 0) {
      topo.push_back(v);
    }
  }
  for (size_t h = 0; h < topo.size(); ++h) {
    const uint32_t v = topo[h];
    for (uint32_t e = graph_[v].firstIncident; e != kNoLine;
         e = edges_[e].nextIncident(v)) {
      if (edges_[e].isTerminal() || edges_[e].flowTowards(v) >= -kFlowEps) {
        continue; // only follow flow leaving v
      }
      const uint32_t w = edges_[e].other(v);
      if (--indeg[w] == 0) {
        topo.push_back(w);
      }
    }
  }
  for (uint32_t v = 0; v < n; ++v) {
    if (graph_[v].alive && indeg[v] != 0) {
      topo.push_back(v);
    }
  }

  for (uint32_t k : topo) {
    if (graph_[k].alive && graph_[k].supply < 0.0f) {
      drawDown(k);
    }
  }
}
