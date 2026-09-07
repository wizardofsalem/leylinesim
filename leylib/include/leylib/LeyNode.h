#pragma once
#include "leylib/FlowRndGenerator.h"
#include <cstdint>
#include <leylib/NodeHandle.h>
#include <memory>
#include <vector>

class LeyNode {
public:
  LeyNode() = delete;

  LeyNode(NodeHandle handle, FlowRndGenerator &flowGenerator);
  LeyNode(NodeHandle handle, float flowAmount);
  virtual ~LeyNode() = default;
  void increaseGeneration();

private:
  NodeHandle handle_;
  std::vector<uint16_t> adjacentNodes_{};
  uint16_t nodeId_;
  float flow_;
};
