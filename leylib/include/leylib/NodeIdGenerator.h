#pragma once
#include <cstdint>
#include <unordered_set>

// Mints node ids, reusing ids that have been freed.
// Falls back to a monotonically increasing counter when no freed id is
// available.
class NodeIdGenerator {
public:
  NodeIdGenerator() = default;
  ~NodeIdGenerator() = default;

  // Returns a unique id (a previously freed one if available, else a new one).
  uint32_t generateId();

  // Returns an id to the pool so it can be reused.
  void freeId(uint32_t id);

private:
  uint32_t highestId_{0};
  std::unordered_set<uint32_t> availableIds_{};
};
