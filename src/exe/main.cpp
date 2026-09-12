#include <random>

#include <leylib/LeyGraph.h>

int main() {
  constexpr std::size_t kNodeCount = 10000;

  LeyGraph leyGraph(kNodeCount, std::random_device{}());

  return 0;
}
