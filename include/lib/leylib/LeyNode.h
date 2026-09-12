#pragma once

#include <leylib/LeyLine.h>
#include <leylib/NodeHandle.h>
#include <vector>

struct LeyNode {
  NodeHandle handle{};
  float supply{};
  bool alive{};
  std::vector<LeyID> connections{};
};
