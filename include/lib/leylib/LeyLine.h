#pragma once
#include "leylib/LeyID.h"

struct LeyLine {
  LeyID id{};
  LeyID aIndex{};
  LeyID bIndex{};
  float capacity{};
  float flow{};
  bool alive{};
};
