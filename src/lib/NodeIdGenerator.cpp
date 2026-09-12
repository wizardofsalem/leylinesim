#include <leylib/LeyID.h>
#include <leylib/NodeIdGenerator.h>

LeyID NodeIdGenerator::generateId() {
  if (!availableIds_.empty()) {
    LeyID id = *availableIds_.begin();
    availableIds_.erase(availableIds_.begin());
    return id;
  }

  return highestId_++;
}

void NodeIdGenerator::freeId(LeyID id) { availableIds_.insert(id); }
