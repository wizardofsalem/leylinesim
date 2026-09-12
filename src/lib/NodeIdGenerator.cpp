#include <leylib/NodeIdGenerator.h>

uint32_t NodeIdGenerator::generateId() {
  if (!availableIds_.empty()) {
    uint32_t id = *availableIds_.begin();
    availableIds_.erase(availableIds_.begin());
    return id;
  }

  return highestId_++;
}

void NodeIdGenerator::freeId(uint32_t id) { availableIds_.insert(id); }
