#pragma once
#include <cstdint>

#include <leylib/LeyLine.h>
#include <leylib/NodeHandle.h>

// A node in the forest. `supply` is what you set:
//   > 0  source  (injects flow)
//   < 0  sink    (consumes up to |supply| of the flow passing through)
//   0    junction (pass-through / split point)
// `firstIncident` is the head of this node's incident-line list (see LeyLine),
// kNoLine when isolated.
struct LeyNode {
  NodeHandle handle{};
  uint32_t firstIncident{kNoLine};
  float supply{};
  bool alive{};
};
