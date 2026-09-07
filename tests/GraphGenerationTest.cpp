#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <unordered_set>

#include <leylib/LeyGraph.h>
#include <leylib/LeyLine.h>
#include <leylib/LeyNode.h>

namespace {
constexpr uint32_t kSeed = 1234;
constexpr size_t kTrees = 32;
} // namespace

// Generation builds `treeCount` minimal trees: each is one source node plus one
// terminal (drain) line sized to the source's output.

TEST(GraphGeneration, CreatesOneNodePerTree) {
  LeyGraph graph(kTrees, kSeed);
  EXPECT_EQ(graph.size(), kTrees);
}

TEST(GraphGeneration, CreatesOneTerminalLinePerTree) {
  LeyGraph graph(kTrees, kSeed);
  ASSERT_EQ(graph.lines().size(), kTrees);
  for (const LeyLine &line : graph.lines()) {
    EXPECT_TRUE(line.alive);
    EXPECT_TRUE(line.isTerminal());
    EXPECT_EQ(line.bIndex, kNoNode);
    EXPECT_EQ(line.flow, 0.0f);
  }
}

TEST(GraphGeneration, ZeroTreesProducesEmptyGraph) {
  LeyGraph graph(0, kSeed);
  EXPECT_EQ(graph.size(), 0u);
  EXPECT_TRUE(graph.nodes().empty());
  EXPECT_TRUE(graph.lines().empty());
}

TEST(GraphGeneration, AllNodesAreAliveSourcesAtGenerationZero) {
  LeyGraph graph(kTrees, kSeed);
  for (const LeyNode &node : graph.nodes()) {
    EXPECT_TRUE(node.alive);
    EXPECT_GT(node.supply, 0.0f);
    EXPECT_EQ(node.handle.gen, 0u);
  }
}

TEST(GraphGeneration, AllNodeIdsAreUnique) {
  LeyGraph graph(kTrees, kSeed);
  std::unordered_set<uint32_t> seen;
  for (const LeyNode &node : graph.nodes()) {
    EXPECT_TRUE(seen.insert(node.handle.id).second)
        << "duplicate id " << node.handle.id;
  }
}

TEST(GraphGeneration, EveryNodeHasItsOwnTerminal) {
  LeyGraph graph(kTrees, kSeed);
  const auto &lines = graph.lines();
  for (uint32_t i = 0; i < graph.size(); ++i) {
    EXPECT_EQ(graph.degree(i), 1u) << "node " << i;
    ASSERT_LT(i, lines.size());
    EXPECT_EQ(lines[i].aIndex, i);
  }
}

TEST(GraphGeneration, TerminalCapacityMatchesSourceSupply) {
  LeyGraph graph(kTrees, kSeed);
  const auto &nodes = graph.nodes();
  for (const LeyLine &line : graph.lines()) {
    EXPECT_FLOAT_EQ(line.capacity, nodes[line.aIndex].supply);
  }
}

TEST(GraphGeneration, SameSeedProducesIdenticalSupplies) {
  LeyGraph a(kTrees, kSeed);
  LeyGraph b(kTrees, kSeed);
  ASSERT_EQ(a.size(), b.size());
  for (size_t i = 0; i < a.size(); ++i) {
    EXPECT_EQ(a.nodes()[i].supply, b.nodes()[i].supply) << "index " << i;
  }
}
