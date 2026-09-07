#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>

#include <leylib/LeyGraph.h>
#include <leylib/LeyLine.h>
#include <leylib/LeyNode.h>

namespace {

// Grow `g` to roughly `targetNodes` live nodes: random mix of new trees,
// attached leaves (each with its own drain), and line splits.
std::vector<uint32_t> growForest(LeyGraph &g, std::mt19937 &rng,
                                 std::size_t targetNodes) {
  std::normal_distribution<float> supply(0.0f, 300.0f);
  std::uniform_real_distribution<float> cap(5.0f, 500.0f);

  std::vector<uint32_t> live;
  for (uint32_t i = 0; i < g.size(); ++i) {
    if (g.nodes()[i].alive) {
      live.push_back(i);
    }
  }

  while (live.size() < targetNodes) {
    const int roll = std::uniform_int_distribution<int>(0, 3)(rng);
    if (roll == 0 || live.empty()) {
      const uint32_t n = g.addNode(supply(rng));
      g.addTerminal(n, cap(rng));
      live.push_back(n);
    } else if (roll <= 2) {
      const uint32_t p =
          live[std::uniform_int_distribution<size_t>(0, live.size() - 1)(rng)];
      const uint32_t n = g.addNode(supply(rng));
      g.addLine(p, n, cap(rng));
      g.addTerminal(n, cap(rng));
      live.push_back(n);
    } else {
      const auto &lines = g.lines();
      for (int tries = 0; tries < 8 && !lines.empty(); ++tries) {
        const uint32_t e =
            std::uniform_int_distribution<uint32_t>(0, lines.size() - 1)(rng);
        if (lines[e].alive) {
          const uint32_t mid = g.splitLine(e, supply(rng));
          if (mid != kNoNode) {
            live.push_back(mid);
          }
          break;
        }
      }
    }
  }
  return live;
}

} // namespace

// Construction only.
static void BM_LeyGraphConstruction(benchmark::State &state) {
  const auto treeCount = static_cast<std::size_t>(state.range(0));
  const std::uint32_t seed = 0xC0FFEEu;

  for (auto _ : state) {
    LeyGraph graph(treeCount, seed);
    benchmark::DoNotOptimize(graph.nodes().data());
  }
}
BENCHMARK(BM_LeyGraphConstruction)
    ->RangeMultiplier(10)
    ->Range(10, 100'000)
    ->Unit(benchmark::kMicrosecond);

// solveFlow() alone, on a pre-built forest of the given size.
static void BM_SolveFlow(benchmark::State &state) {
  const auto nodeCount = static_cast<std::size_t>(state.range(0));
  std::mt19937 rng(0xBEEFu);
  LeyGraph graph(0, 0xBEEFu);
  growForest(graph, rng, nodeCount);

  for (auto _ : state) {
    graph.solveFlow();
    benchmark::DoNotOptimize(graph.lines().data());
  }
}
BENCHMARK(BM_SolveFlow)
    ->RangeMultiplier(10)
    ->Range(100, 100'000)
    ->Unit(benchmark::kMicrosecond);

// One random mutation (add or remove a node) + a full solveFlow(), each tick.
static void BM_MutateAndSolve(benchmark::State &state) {
  const auto nodeCount = static_cast<std::size_t>(state.range(0));
  std::mt19937 rng(0xBEEFu);
  LeyGraph graph(0, 0xBEEFu);
  std::vector<uint32_t> live = growForest(graph, rng, nodeCount);

  std::normal_distribution<float> supply(0.0f, 300.0f);
  std::uniform_real_distribution<float> cap(5.0f, 500.0f);

  for (auto _ : state) {
    const bool add =
        live.size() < nodeCount ||
        (live.size() < nodeCount * 2 &&
         std::uniform_int_distribution<int>(0, 1)(rng) == 0);

    if (add) {
      const uint32_t p =
          live[std::uniform_int_distribution<size_t>(0, live.size() - 1)(rng)];
      const uint32_t n = graph.addNode(supply(rng));
      graph.addLine(p, n, cap(rng));
      graph.addTerminal(n, cap(rng));
      live.push_back(n);
    } else {
      const size_t k =
          std::uniform_int_distribution<size_t>(0, live.size() - 1)(rng);
      graph.removeNode(live[k]);
      live[k] = live.back();
      live.pop_back();
    }

    graph.solveFlow();
    benchmark::DoNotOptimize(graph.lines().data());
  }
}
BENCHMARK(BM_MutateAndSolve)
    ->RangeMultiplier(10)
    ->Range(100, 100'000)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
