#pragma once
#include <leylib/LeyID.h>
#include <unordered_set>

class NodeIdGenerator {
public:
  NodeIdGenerator() = default;
  ~NodeIdGenerator() = default;

  LeyID generateId();

  void freeId(LeyID id);

private:
  LeyID highestId_{0};
  std::unordered_set<LeyID> availableIds_{};
};
