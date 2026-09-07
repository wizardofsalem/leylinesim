#include <gtest/gtest.h>

#include <cstdint>

#include <leylib/LeyGraph.h>
#include <leylib/LeyLine.h>

namespace {
constexpr uint32_t kSeed = 99;
} // namespace

// --- LeyLine helpers (pure struct, no graph) --------------------------------

TEST(LeyLineHelpers, OtherReturnsTheOppositeEndpoint) {
  const LeyLine line{.aIndex = 3, .bIndex = 7};
  EXPECT_EQ(line.other(3), 7u);
  EXPECT_EQ(line.other(7), 3u);
}

TEST(LeyLineHelpers, DirectionIsRelativeToTheEndpoint) {
  LeyLine line{.aIndex = 0, .bIndex = 1, .capacity = 100.0f};

  line.pushTowards(1, 30.0f); // push 30 towards b
  EXPECT_FLOAT_EQ(line.flow, 30.0f);
  EXPECT_FLOAT_EQ(line.flowTowards(1), 30.0f);
  EXPECT_FLOAT_EQ(line.flowTowards(0), -30.0f);
  EXPECT_FLOAT_EQ(line.residualTowards(1), 70.0f);
  EXPECT_FLOAT_EQ(line.residualTowards(0), 130.0f); // reverse fully, then fill
}

TEST(LeyLineHelpers, PushTowardsAIsNegativeSignedFlow) {
  LeyLine line{.aIndex = 5, .bIndex = 9, .capacity = 100.0f};
  line.pushTowards(5, 40.0f);
  EXPECT_FLOAT_EQ(line.flow, -40.0f);
  EXPECT_FLOAT_EQ(line.flowTowards(5), 40.0f);
}

// --- LeyGraph edge operations ----------------------------------------------

namespace {
// Four isolated nodes, no lines.
LeyGraph fourNodes() {
  LeyGraph g(0, kSeed);
  for (int i = 0; i < 4; ++i) {
    g.addNode(0.0f);
  }
  return g;
}
} // namespace

TEST(Edges, AddLineLinksBothEndpoints) {
  LeyGraph graph = fourNodes();

  const uint32_t line = graph.addLine(0, 2, 500.0f);

  EXPECT_EQ(graph.lines().size(), 1u);
  EXPECT_EQ(graph.degree(0), 1u);
  EXPECT_EQ(graph.degree(2), 1u);

  const LeyLine &l = graph.lines()[line];
  EXPECT_EQ(l.aIndex, 0u);
  EXPECT_EQ(l.bIndex, 2u);
  EXPECT_FLOAT_EQ(l.capacity, 500.0f);
  EXPECT_TRUE(l.alive);
  EXPECT_FALSE(l.isTerminal());
}

TEST(Edges, AddTerminalLinksOneNode) {
  LeyGraph graph = fourNodes();

  const uint32_t line = graph.addTerminal(1, 42.0f);

  EXPECT_EQ(graph.degree(1), 1u);
  EXPECT_EQ(graph.degree(0), 0u);
  EXPECT_TRUE(graph.lines()[line].isTerminal());
  EXPECT_EQ(graph.lines()[line].aIndex, 1u);
  EXPECT_EQ(graph.lines()[line].bIndex, kNoNode);
}

TEST(Edges, RemoveLineUnlinksBothEndpoints) {
  LeyGraph graph = fourNodes();
  const uint32_t line = graph.addLine(0, 2, 500.0f);
  ASSERT_EQ(graph.degree(0), 1u);

  graph.removeLine(line);

  EXPECT_EQ(graph.degree(0), 0u);
  EXPECT_EQ(graph.degree(2), 0u);
  EXPECT_FALSE(graph.lines()[line].alive);
}

TEST(Edges, RemoveTerminalUnlinksItsNode) {
  LeyGraph graph = fourNodes();
  const uint32_t line = graph.addTerminal(1, 10.0f);

  graph.removeLine(line);

  EXPECT_EQ(graph.degree(1), 0u);
  EXPECT_FALSE(graph.lines()[line].alive);
}

TEST(Edges, RemoveMiddleOfIncidentListKeepsTheRest) {
  LeyGraph graph = fourNodes();
  const uint32_t a = graph.addLine(0, 1, 1.0f);
  const uint32_t b = graph.addLine(0, 2, 2.0f);
  const uint32_t c = graph.addLine(0, 3, 3.0f);
  (void)a;
  (void)c;
  ASSERT_EQ(graph.degree(0), 3u);

  graph.removeLine(b); // b is in the middle of node 0's list

  EXPECT_EQ(graph.degree(0), 2u);
  EXPECT_EQ(graph.degree(2), 0u);
  EXPECT_TRUE(graph.lines()[a].alive);
  EXPECT_TRUE(graph.lines()[c].alive);
}

TEST(Edges, RemovedLineSlotIsReused) {
  LeyGraph graph = fourNodes();
  const uint32_t first = graph.addLine(0, 2, 1.0f);
  graph.removeLine(first);

  const uint32_t second = graph.addLine(1, 3, 2.0f);
  EXPECT_EQ(second, first);
  EXPECT_TRUE(graph.lines()[second].alive);
}

TEST(Edges, RemoveLineIsIdempotent) {
  LeyGraph graph = fourNodes();
  const uint32_t line = graph.addLine(0, 1, 1.0f);
  graph.removeLine(line);
  graph.removeLine(line); // must not corrupt the lists or double-free the slot

  EXPECT_EQ(graph.degree(0), 0u);
  EXPECT_EQ(graph.degree(1), 0u);
}
