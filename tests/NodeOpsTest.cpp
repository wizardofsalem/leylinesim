#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include <leylib/LeyGraph.h>
#include <leylib/LeyLine.h>
#include <leylib/LeyNode.h>

namespace {
constexpr uint32_t kSeed = 7;

// Count live lines incident to a node by walking lines() directly - an
// independent check on LeyGraph::degree().
size_t countIncident(const LeyGraph &g, uint32_t node) {
  size_t n = 0;
  for (uint32_t e = g.nodes()[node].firstIncident; e != kNoLine;
       e = g.lines()[e].nextIncident(node)) {
    ++n;
  }
  return n;
}
} // namespace

// --- addNode -----------------------------------------------------------

TEST(NodeOps, AddNodeAppendsAnIsolatedNode) {
  LeyGraph graph(2, kSeed); // 2 source trees: nodes 0,1
  const uint32_t idx = graph.addNode(123.0f);

  EXPECT_EQ(idx, 2u);
  EXPECT_EQ(graph.size(), 3u);
  EXPECT_TRUE(graph.nodes()[idx].alive);
  EXPECT_FLOAT_EQ(graph.nodes()[idx].supply, 123.0f);
  EXPECT_EQ(graph.degree(idx), 0u);
  EXPECT_EQ(graph.nodes()[idx].handle.id, 2u);
  EXPECT_EQ(graph.nodes()[idx].handle.gen, 0u);
}

// --- removeNode -------------------------------------------------------

TEST(NodeOps, RemoveNodeKillsNodeAndItsTerminal) {
  LeyGraph graph(1, kSeed); // node 0 + terminal line 0
  ASSERT_EQ(graph.degree(0), 1u);

  graph.removeNode(0);

  EXPECT_FALSE(graph.nodes()[0].alive);
  EXPECT_FALSE(graph.lines()[0].alive);
}

TEST(NodeOps, RemoveNodeBumpsGeneration) {
  LeyGraph graph(1, kSeed);
  ASSERT_EQ(graph.nodes()[0].handle.gen, 0u);
  graph.removeNode(0);
  EXPECT_EQ(graph.nodes()[0].handle.gen, 1u);
}

TEST(NodeOps, RemoveNodeIsIdempotent) {
  LeyGraph graph(1, kSeed);
  graph.removeNode(0);
  graph.removeNode(0); // no double-free, no second gen bump
  EXPECT_EQ(graph.nodes()[0].handle.gen, 1u);
}

TEST(NodeOps, RemoveHighDegreeNodeUnlinksEveryNeighbour) {
  LeyGraph graph(0, kSeed);
  const uint32_t hub = graph.addNode(0.0f);
  const uint32_t a = graph.addNode(0.0f);
  const uint32_t b = graph.addNode(0.0f);
  const uint32_t c = graph.addNode(0.0f);
  graph.addLine(hub, a, 1.0f);
  graph.addLine(hub, b, 1.0f);
  graph.addLine(hub, c, 1.0f);
  ASSERT_EQ(graph.degree(hub), 3u);

  graph.removeNode(hub);

  EXPECT_EQ(graph.degree(a), 0u);
  EXPECT_EQ(graph.degree(b), 0u);
  EXPECT_EQ(graph.degree(c), 0u);
  for (const LeyLine &line : graph.lines()) {
    EXPECT_FALSE(line.alive);
  }
}

// --- slot recycling -------------------------------------------------

TEST(NodeOps, RemovedSlotIsRecycledWithPreservedGeneration) {
  LeyGraph graph(1, kSeed); // node 0
  graph.removeNode(0);

  const uint32_t reused = graph.addNode(50.0f);

  EXPECT_EQ(reused, 0u);
  EXPECT_EQ(graph.size(), 1u);
  EXPECT_TRUE(graph.nodes()[0].alive);
  EXPECT_EQ(graph.nodes()[0].handle.gen, 1u);
  EXPECT_FLOAT_EQ(graph.nodes()[0].supply, 50.0f);
  EXPECT_EQ(graph.degree(0), 0u);
}

// --- splitLine ----------------------------------------------------

TEST(NodeOps, SplitInternalLineInsertsNodeBetweenEndpoints) {
  LeyGraph graph(0, kSeed);
  const uint32_t a = graph.addNode(0.0f);
  const uint32_t b = graph.addNode(0.0f);
  const uint32_t line = graph.addLine(a, b, 40.0f);

  const uint32_t mid = graph.splitLine(line, 0.0f);

  EXPECT_EQ(graph.degree(a), 1u);
  EXPECT_EQ(graph.degree(b), 1u);
  EXPECT_EQ(graph.degree(mid), 2u);

  size_t live = 0;
  for (const LeyLine &l : graph.lines()) {
    if (l.alive) {
      ++live;
      EXPECT_FALSE(l.isTerminal());
      EXPECT_FLOAT_EQ(l.capacity, 40.0f);
      EXPECT_TRUE(l.aIndex == mid || l.bIndex == mid);
    }
  }
  EXPECT_EQ(live, 2u);
}

TEST(NodeOps, SplitTerminalLineKeepsTheTerminalOnTheNewNode) {
  LeyGraph graph(1, kSeed); // node 0 + terminal line 0
  const float cap = graph.lines()[0].capacity;

  const uint32_t mid = graph.splitLine(0, -5.0f); // insert a sink

  EXPECT_EQ(graph.degree(0), 1u);   // 0 -- mid
  EXPECT_EQ(graph.degree(mid), 2u); // mid -- (internal) + mid -- (terminal)

  bool sawInternal = false;
  bool sawTerminal = false;
  for (const LeyLine &l : graph.lines()) {
    if (!l.alive) {
      continue;
    }
    EXPECT_FLOAT_EQ(l.capacity, cap);
    if (l.isTerminal()) {
      sawTerminal = true;
      EXPECT_EQ(l.aIndex, mid);
    } else {
      sawInternal = true;
    }
  }
  EXPECT_TRUE(sawInternal);
  EXPECT_TRUE(sawTerminal);
  EXPECT_FLOAT_EQ(graph.nodes()[mid].supply, -5.0f);
}

TEST(NodeOps, SplitDeadLineReturnsSentinel) {
  LeyGraph graph(1, kSeed);
  graph.removeLine(0);
  EXPECT_EQ(graph.splitLine(0, 0.0f), kNoNode);
}

// --- randomised structural stress ----------------------------------

TEST(NodeOps, RandomAddRemoveKeepsStructureConsistent) {
  LeyGraph graph(4, 20240907u);
  std::mt19937 rng(123456u);

  std::vector<uint32_t> live; // node indices we believe are alive
  for (uint32_t i = 0; i < graph.size(); ++i) {
    live.push_back(i);
  }

  auto pickLive = [&](std::mt19937 &r) -> uint32_t {
    return live[std::uniform_int_distribution<size_t>(0, live.size() - 1)(r)];
  };
  std::normal_distribution<float> supplyDist(0.0f, 500.0f);
  std::uniform_real_distribution<float> capDist(1.0f, 1000.0f);

  for (int step = 0; step < 4000; ++step) {
    const int roll = std::uniform_int_distribution<int>(0, 9)(rng);

    if (roll < 4 || live.empty()) {
      // grow: attach a fresh node (with its own drain) to an existing one
      const float supply = supplyDist(rng);
      const uint32_t n = graph.addNode(supply);
      if (!live.empty()) {
        graph.addLine(pickLive(rng), n, capDist(rng));
      }
      graph.addTerminal(n, capDist(rng));
      live.push_back(n);
    } else if (roll < 7) {
      // split a random live internal or terminal line
      const auto &lines = graph.lines();
      std::vector<uint32_t> aliveLines;
      for (uint32_t e = 0; e < lines.size(); ++e) {
        if (lines[e].alive) {
          aliveLines.push_back(e);
        }
      }
      if (!aliveLines.empty()) {
        const uint32_t e = aliveLines[std::uniform_int_distribution<size_t>(
            0, aliveLines.size() - 1)(rng)];
        const uint32_t mid = graph.splitLine(e, supplyDist(rng));
        if (mid != kNoNode) {
          live.push_back(mid);
        }
      }
    } else {
      // remove a random live node
      const size_t k =
          std::uniform_int_distribution<size_t>(0, live.size() - 1)(rng);
      graph.removeNode(live[k]);
      live[k] = live.back();
      live.pop_back();
    }

    // --- invariants (cheap ones every step) ---
    // exactly `live` nodes are alive
    size_t aliveCount = 0;
    for (const LeyNode &node : graph.nodes()) {
      if (node.alive) {
        ++aliveCount;
      }
    }
    ASSERT_EQ(aliveCount, live.size()) << "step " << step;
  }

  // --- full invariant sweep at the end ---
  for (uint32_t i = 0; i < graph.size(); ++i) {
    if (!graph.nodes()[i].alive) {
      EXPECT_EQ(graph.nodes()[i].firstIncident, kNoLine)
          << "dead node " << i << " still lists lines";
      continue;
    }
    EXPECT_EQ(graph.degree(i), countIncident(graph, i)) << "node " << i;
  }
  const auto &lines = graph.lines();
  for (uint32_t e = 0; e < lines.size(); ++e) {
    if (!lines[e].alive) {
      continue;
    }
    EXPECT_TRUE(graph.nodes()[lines[e].aIndex].alive)
        << "line " << e << " aIndex points at a dead node";
    if (!lines[e].isTerminal()) {
      EXPECT_TRUE(graph.nodes()[lines[e].bIndex].alive)
          << "line " << e << " bIndex points at a dead node";
    }
  }
}
