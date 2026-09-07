#pragma once
#include <cstdint>

class LeyLine {
public:
  LeyLine() = delete;
  ~LeyLine();

private:
  uint16_t nodeA_;
  uint16_t nodeB_;
  float flow_;
  bool alive_;
};
