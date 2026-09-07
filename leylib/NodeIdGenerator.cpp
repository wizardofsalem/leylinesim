#include <leylib/NodeIdGenerator.h>

uint16_t NodeIdGenerator::generateId() {
  if (!availableIds_.empty()) {
    uint16_t id = *availableIds_.begin();
    availableIds_.erase(availableIds_.begin());
    return id;
  }

  return highestId_++;
}

void NodeIdGenerator::freeId(uint16_t id) { availableIds_.insert(id); }
