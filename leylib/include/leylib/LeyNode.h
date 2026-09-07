#pragma once
#include <cstdint>
#include <vector>

#include <leylib/NodeHandle.h>

struct LeyNode {
  NodeHandle handle{};
  std::vector<uint32_t> adjacentNodes{};
  float flow{};
  bool alive{};
};
