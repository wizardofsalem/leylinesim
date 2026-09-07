#pragma once
#include <cstdint>

class LeyLine {
public:
  LeyLine() = delete;
  LeyLine(uint16_t nodeA, uint16_t nodeB, float flow);
  ~LeyLine() = default;

private:
  uint16_t nodeA_;
  uint16_t nodeB_;
  float flow_;
  float capacity_;
};
