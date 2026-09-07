#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

#include <leylib/LeyGraph.h>
#include <leylib/LeyLine.h>
#include <leylib/LeyNode.h>

namespace {
constexpr uint32_t kSeed = 1u; // generation seed (unused: we build by hand)

// Net signed flow into `node` (positive = flowing in).
float netInflow(const LeyGraph &g, uint32_t node) {
  float sum = 0.0f;
  for (uint32_t e = g.nodes()[node].firstIncident; e != kNoLine;
       e = g.lines()[e].nextIncident(node)) {
    sum += g.lines()[e].flowTowards(node);
  }
  return sum;
}

// 1% relative tolerance with an absolute floor - float32 accumulates error
// across a deep tree.
::testing::AssertionResult nearFlow(float actual, float expected) {
  const float tol = 0.01f * std::max({std::fabs(actual), std::fabs(expected),
                                      1.0f});
  if (std::fabs(actual - expected) <= tol) {
    return ::testing::AssertionSuccess();
  }
  return ::testing::AssertionFailure()
         << actual << " vs expected " << expected << " (tol " << tol << ")";
}
} // namespace

// --- hand-built cases: the worked examples from the design -----------------

TEST(Flow, SingleSourceDrainCarriesTheSupply) {
  LeyGraph g(0, kSeed);
  const uint32_t s = g.addNode(50.0f);
  const uint32_t term = g.addTerminal(s, 100.0f);

  g.solveFlow();

  EXPECT_TRUE(nearFlow(g.lines()[term].flow, 50.0f));
}

TEST(Flow, TwoSourcesAccumulateTowardTheDrain) {
  // A(50) --lineAB-- B(50) --terminal-->
  LeyGraph g(0, kSeed);
  const uint32_t b = g.addNode(50.0f);
  const uint32_t term = g.addTerminal(b, 100.0f);
  const uint32_t a = g.addNode(50.0f);
  const uint32_t lineAB = g.addLine(a, b, 100.0f);

  g.solveFlow();

  EXPECT_TRUE(nearFlow(g.lines()[lineAB].flowTowards(b), 50.0f));
  EXPECT_TRUE(nearFlow(g.lines()[term].flow, 100.0f));
}

TEST(Flow, SinkConsumesFlowPassingThrough) {
  // Source(50) -- Sink(-50) -- drain
  LeyGraph g(0, kSeed);
  const uint32_t s = g.addNode(50.0f);
  const uint32_t k = g.addNode(-50.0f);
  const uint32_t lineSK = g.addLine(s, k, 100.0f);
  const uint32_t term = g.addTerminal(k, 100.0f);

  g.solveFlow();

  EXPECT_TRUE(nearFlow(g.lines()[lineSK].flowTowards(k), 50.0f));
  EXPECT_TRUE(nearFlow(g.lines()[term].flow, 0.0f));
}

TEST(Flow, SinkOverflowPassesTheExcessOn) {
  LeyGraph g(0, kSeed);
  const uint32_t s = g.addNode(80.0f);
  const uint32_t k = g.addNode(-50.0f);
  const uint32_t lineSK = g.addLine(s, k, 100.0f);
  const uint32_t term = g.addTerminal(k, 100.0f);

  g.solveFlow();

  EXPECT_TRUE(nearFlow(g.lines()[lineSK].flowTowards(k), 80.0f));
  EXPECT_TRUE(nearFlow(g.lines()[term].flow, 30.0f));
}

TEST(Flow, StarvedSinkTakesWhatItCanAndDrainGetsNothing) {
  LeyGraph g(0, kSeed);
  const uint32_t s = g.addNode(30.0f);
  const uint32_t k = g.addNode(-50.0f);
  g.addLine(s, k, 100.0f);
  const uint32_t term = g.addTerminal(k, 100.0f);

  g.solveFlow();

  EXPECT_TRUE(nearFlow(netInflow(g, k), 30.0f)); // consumed all 30 available
  EXPECT_TRUE(nearFlow(g.lines()[term].flow, 0.0f));
}

TEST(Flow, JunctionSplitsByCapacity) {
  // Source(90) -- J -- { drain(cap 10), drain(cap 30) }
  LeyGraph g(0, kSeed);
  const uint32_t s = g.addNode(90.0f);
  const uint32_t j = g.addNode(0.0f);
  const uint32_t lineSJ = g.addLine(s, j, 100.0f);
  const uint32_t t1 = g.addTerminal(j, 10.0f);
  const uint32_t t2 = g.addTerminal(j, 30.0f);

  g.solveFlow();

  EXPECT_TRUE(nearFlow(g.lines()[lineSJ].flowTowards(j), 90.0f));
  EXPECT_TRUE(nearFlow(g.lines()[t1].flow, 90.0f * 10.0f / 40.0f));
  EXPECT_TRUE(nearFlow(g.lines()[t2].flow, 90.0f * 30.0f / 40.0f));
}

TEST(Flow, FlowIgnoresBranchesWithNoDrain) {
  // deadEnd(source, no drain) -- J -- drain
  LeyGraph g(0, kSeed);
  const uint32_t j = g.addNode(0.0f);
  const uint32_t term = g.addTerminal(j, 50.0f);
  const uint32_t dead = g.addNode(70.0f); // source, but its own side has no drain
  const uint32_t lineDJ = g.addLine(dead, j, 50.0f);
  const uint32_t deadBranch = g.addNode(0.0f); // junction, drainless
  const uint32_t lineJD = g.addLine(j, deadBranch, 50.0f);

  g.solveFlow();

  EXPECT_TRUE(nearFlow(g.lines()[lineDJ].flowTowards(j), 70.0f));
  EXPECT_TRUE(nearFlow(g.lines()[term].flow, 70.0f)); // all of it exits the drain
  EXPECT_TRUE(nearFlow(g.lines()[lineJD].flow, 0.0f)); // nothing down the dead end
}

TEST(Flow, DeterministicAcrossSolves) {
  LeyGraph g(0, kSeed);
  const uint32_t s = g.addNode(90.0f);
  const uint32_t j = g.addNode(0.0f);
  g.addLine(s, j, 100.0f);
  g.addTerminal(j, 10.0f);
  g.addTerminal(j, 30.0f);

  g.solveFlow();
  std::vector<float> first;
  for (const LeyLine &l : g.lines()) {
    first.push_back(l.flow);
  }
  g.solveFlow();
  for (uint32_t i = 0; i < g.lines().size(); ++i) {
    EXPECT_EQ(g.lines()[i].flow, first[i]) << "line " << i;
  }
}

// --- randomised forest: conservation must hold everywhere ----------------

TEST(Flow, RandomForestConservesFlow) {
  for (uint32_t seed = 1; seed <= 40; ++seed) {
    LeyGraph g(0, seed);
    std::mt19937 rng(seed);
    std::normal_distribution<float> supplyDist(0.0f, 250.0f);
    std::uniform_real_distribution<float> capDist(5.0f, 500.0f);

    std::vector<uint32_t> nodes;
    const uint32_t root = g.addNode(std::fabs(supplyDist(rng)));
    g.addTerminal(root, capDist(rng));
    nodes.push_back(root);

    for (int i = 0; i < 180; ++i) {
      const int roll = std::uniform_int_distribution<int>(0, 4)(rng);
      if (roll == 0) { // new tree
        const uint32_t x = g.addNode(supplyDist(rng));
        g.addTerminal(x, capDist(rng));
        nodes.push_back(x);
      } else if (roll <= 2) { // attach a leaf (with its own drain)
        const uint32_t p =
            nodes[std::uniform_int_distribution<size_t>(0, nodes.size() - 1)(rng)];
        const uint32_t x = g.addNode(supplyDist(rng));
        g.addLine(p, x, capDist(rng));
        g.addTerminal(x, capDist(rng));
        nodes.push_back(x);
      } else { // split a random live line
        std::vector<uint32_t> alive;
        for (uint32_t e = 0; e < g.lines().size(); ++e) {
          if (g.lines()[e].alive) {
            alive.push_back(e);
          }
        }
        const uint32_t e = alive[std::uniform_int_distribution<size_t>(
            0, alive.size() - 1)(rng)];
        const uint32_t mid = g.splitLine(e, supplyDist(rng));
        if (mid != kNoNode) {
          nodes.push_back(mid);
        }
      }
    }

    g.solveFlow();

    float totalTerminalOut = 0.0f;
    float totalSourceSupply = 0.0f;
    float totalSinkTaken = 0.0f;

    for (uint32_t v = 0; v < g.size(); ++v) {
      if (!g.nodes()[v].alive) {
        continue;
      }
      const float supply = g.nodes()[v].supply;
      const float in = netInflow(g, v);

      if (supply > 0.0f) {
        totalSourceSupply += supply;
        EXPECT_TRUE(nearFlow(in, -supply))
            << "seed " << seed << " source " << v;
      } else if (supply < 0.0f) {
        totalSinkTaken += in;
        EXPECT_GE(in, -0.5f) << "seed " << seed << " sink " << v;
        EXPECT_LE(in, -supply + 0.5f) << "seed " << seed << " sink " << v;
      } else {
        EXPECT_NEAR(in, 0.0f, 0.5f) << "seed " << seed << " junction " << v;
      }
    }

    for (const LeyLine &l : g.lines()) {
      if (l.alive && l.isTerminal()) {
        EXPECT_GE(l.flow, -0.5f) << "seed " << seed << " terminal flow negative";
        totalTerminalOut += l.flow;
      }
    }

    // global identity: what exits the drains == produced - consumed
    EXPECT_TRUE(nearFlow(totalTerminalOut, totalSourceSupply - totalSinkTaken))
        << "seed " << seed;
  }
}
