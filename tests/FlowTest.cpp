#include <gtest/gtest.h>

#include <leylib/LeyGraph.h>
#include <leylib/LeyLine.h>
#include <leylib/LeyNode.h>

class TestLeyGraph : public LeyGraph {
public:
  TestLeyGraph() : LeyGraph() {}

  LeyLine getLine(LeyID lineId) { return lines()[lineId]; }
};

TEST(Flow, SingleSourceDrainCarriesTheSupply) {
  TestLeyGraph tlg{};

  auto a = tlg.addNode(1000);
  auto b = tlg.addNode(-500);

  auto lineId = tlg.addLine(a, b, 1000);

  tlg.solveFlow();

  EXPECT_FLOAT_EQ(tlg.getLine(lineId).flow, 500.0f);
}

TEST(Flow, SingleSourceDrainCarriesSupplyWithGreaterSink) {
  TestLeyGraph tlg{};

  auto a = tlg.addNode(1000);
  auto b = tlg.addNode(-1500);

  auto lineId = tlg.addLine(a, b, 1000);

  tlg.solveFlow();

  EXPECT_FLOAT_EQ(tlg.getLine(lineId).flow, 1000.0f);
}

TEST(Flow, TwoSourcesShouldNotCarrySupply) {
  TestLeyGraph tlg{};

  auto a = tlg.addNode(1000);
  auto b = tlg.addNode(500);

  auto lineId = tlg.addLine(a, b, 1000);

  tlg.solveFlow();

  EXPECT_FLOAT_EQ(tlg.getLine(lineId).flow, 0.0f);
}

TEST(Flow, TwoSinksShouldSatisy) {

  TestLeyGraph tlg{};

  auto a = tlg.addNode(1000);
  auto b = tlg.addNode(-500);
  auto c = tlg.addNode(-500);

  auto lineId1 = tlg.addLine(a, b, 1000);
  auto lineId2 = tlg.addLine(a, c, 1000);

  tlg.solveFlow();

  EXPECT_FLOAT_EQ(tlg.getLine(lineId1).flow, 500.0f);
  EXPECT_FLOAT_EQ(tlg.getLine(lineId2).flow, 500.0f);
}

TEST(Flow, TwoSinksAtFullCapacity) {
  // TODO: Under strict sink priority, the expected deliveries below should
  // be 300 to the -500 sink and 700 to the -700 sink, rather than 500 each.
  TestLeyGraph tlg{};

  auto a = tlg.addNode(1000);
  auto b = tlg.addNode(-500);
  auto c = tlg.addNode(-700);

  auto lineId1 = tlg.addLine(a, b, 1000);
  auto lineId2 = tlg.addLine(a, c, 1000);

  tlg.solveFlow();

  EXPECT_FLOAT_EQ(tlg.getLine(lineId1).flow, 500.0f);
  EXPECT_FLOAT_EQ(tlg.getLine(lineId2).flow, 500.0f);
}

// Test cases to implement.
TEST(Flow, EdgeCapacityLimitsDeliveryBelowSinkDemand) {
  // Setup: source +1000 -> sink -800, with edge capacity 300.
  // Solve once. Expect edge flow 300, leaving 500 of demand unmet.
  // Checks that connection capacity limits delivery even with spare supply.
}

TEST(Flow, FlowPassesThroughZeroSupplyJunction) {
  // Setup: source +1000 -> junction 0 -> sink -600.
  // Give both edges capacity 1000 and solve.
  // Expect flow 600 on BOTH edges. The junction neither creates nor consumes flow.
}

TEST(Flow, UnreachableSinkReceivesNoFlow) {
  // Setup: source +1000 -> junction 0, plus a disconnected sink -500.
  // Give the edge capacity 1000 and solve.
  // Expect zero flow on the edge: the dead-end junction cannot absorb supply.
  // The disconnected sink receives nothing.
}

TEST(Flow, FlowCannotTravelAgainstEdgeDirection) {
  // Setup: source +1000 and sink -500, but direct the edge sink -> source.
  // Give it capacity 1000 and solve.
  // Expect zero flow. Connectivity alone does not permit reverse delivery.
  // Treat addLine(a, b, capacity) as allowing actual flow only from a to b.
}

TEST(Flow, MultiplePathsCombineToSupplyOneSink) {
  // Setup: source +1000, two zero-supply junctions A/B, and sink -700.
  // Paths: source -> A -> sink (both capacities 300),
  // and source -> B -> sink (both capacities 400).
  // Solve. Expect 300 on each A-path edge and 400 on each B-path edge.
  // Neither path alone suffices; together they satisfy the sink.
}

TEST(Flow, MultipleSourcesCombineToSupplyOneSink) {
  // Setup: sources +300 and +400, each with an edge to one sink -700.
  // Give each edge capacity 1000 and solve.
  // Expect flows 300 and 400 respectively, delivering 700 in total.
}

TEST(Flow, LargestDemandSinkHasPriorityWhenSupplyIsInsufficient) {
  // Setup: source +1000 with separate edges to sinks -500 and -700.
  // Give both edges capacity 1000. Create the smaller sink first.
  // Solve. Expect 700 delivered to the larger sink and 300 to the smaller.
  // Priority follows original demand, not insertion order or equal sharing.
}

TEST(Flow, CapacityLimitedPrioritySinkLeavesSupplyForSmallerSinks) {
  // Setup: source +1000 -> sink -700 with capacity 200,
  // and source -> sink -500 with capacity 1000.
  // Solve. Expect 200 to the larger sink and 500 to the smaller sink.
  // The remaining 300 supply stays unused. Do not stop just because the
  // highest-priority sink cannot be fully satisfied.
}

TEST(Flow, ReroutingPreservesHigherPrioritySinkAllocation) {
  // Setup: source +1000 -> junction A (capacity 600),
  // and source -> junction B (capacity 600).
  // Add A -> high-priority sink -600 (capacity 600),
  // B -> high-priority sink (capacity 600),
  // and A -> low-priority sink -400 (capacity 400).
  // Create the A route first to encourage an initial allocation through A.
  // Solve. Expect high-priority delivery 600 AND low-priority delivery 400.
  // If the first allocation used all of A for the higher sink, the solver
  // must reroute at least 400 of that delivery through B to free A.
  // Assert sink totals and capacity/conservation, not one exact route split:
  // several final routings are valid. This checks the final outcome; an
  // implementation that chooses B first need not reroute on this instance.
}

TEST(Flow, RepeatedSolveDoesNotAccumulateFlow) {
  // Setup: source +1000 -> sink -500, edge capacity 1000.
  // Solve, record the flow, then solve again without changing the graph.
  // Expect flow 500 after BOTH solves, not 1000 after the second.
  // Also check that configured source supply and sink demand stay unchanged.
}

TEST(Flow, JunctionsConserveFlow) {
  // Setup: sources +300/+400 feed junction A; A feeds junction B;
  // B feeds sinks -200/-500. Give all edges capacity 1000.
  // Solve. For EACH junction, sum incoming edge flows and outgoing edge flows.
  // Expect incoming == outgoing (within floating-point tolerance), with 700
  // flowing from A to B and sink deliveries 200/500.
  // Junctions must not create, consume, or accumulate flow.
}

TEST(Flow, EdgeFlowsStayWithinCapacity) {
  // Setup: source +1000 and sink -1000 connected through three junction paths.
  // For each path, use the same capacity on both edges: 0, 100, and 250.
  // Solve. Inspect EVERY edge: expect 0 <= flow <= capacity, within tolerance.
  // Expect total sink delivery 350 so an all-zero implementation cannot pass.
  // The zero-capacity path must carry no flow.
}

TEST(Flow, SourcesNeverExceedAvailableSupply) {
  // Setup: source +500 feeding two sinks -400/-300 through capacity-1000 edges.
  // Solve. Sum outgoing flows from the source; expect total 500, never above it.
  // Expect deliveries 400 and 100 according to sink priority.
  // This catches accidentally giving the full source supply to each edge.
}

TEST(Flow, SinksNeverReceiveMoreThanDemand) {
  // Setup: two sources +500/+500 feeding one sink -300.
  // Give both incoming edges capacity 1000 and solve.
  // Sum incoming sink flows; expect total 300, never above its demand.
  // Do not require a particular split between the sources.
  // This catches satisfying the full sink demand independently on each edge.
}
