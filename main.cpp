#include <chrono>
#include <iostream>
#include <leylib/LeyGraph.h>

int main() {
  constexpr std::size_t kNodeCount = 100;

  const auto start = std::chrono::steady_clock::now();
  LeyGraph leyGraph(kNodeCount);
  const auto end = std::chrono::steady_clock::now();

  const auto ms =
      std::chrono::duration<double, std::milli>(end - start).count();
  std::cout << "LeyGraph construction: " << kNodeCount << " nodes in " << ms
            << " ms\n";

  return 0;
}
