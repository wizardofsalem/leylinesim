#include <leylib/FlowRndGenerator.h>
#include <leylib/LeyGraph.h>
#include <leylib/LeyNode.h>
#include <leylib/LeySource.h>
#include <leylib/NodeIdGenerator.h>
#include <memory>

LeyGraph::LeyGraph(size_t initialSize) {
  FlowRndGenerator flowGenerator(1000, 250);
  NodeIdGenerator idGenerator;

  graph_.reserve(initialSize);
  for (size_t i = 0; i < initialSize; ++i) {
    uint16_t id = idGenerator.generateId();
    graph_.emplace_back(.gen = 0, .id = 1, flowGenerator);
  }
}

LeyGraph::~LeyGraph() = default;
