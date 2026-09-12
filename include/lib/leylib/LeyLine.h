#pragma once
#include <cstdint>

// Sentinel meaning "no line" in the incident-edge linked lists.
inline constexpr uint32_t kNoLine = UINT32_MAX;
// Sentinel node index. A line with bIndex == kNoNode is a terminal (dangling)
// line: one real endpoint, the other end open. A terminal line is a drain -
// flow entering it leaves the forest.
inline constexpr uint32_t kNoNode = UINT32_MAX;

// Connects two nodes, referenced by their index in LeyGraph::nodes() (or one
// node and an open end, for a terminal line). Structurally undirected; it
// carries a signed flow where flow > 0 means aIndex -> bIndex (and for a
// terminal line, flow > 0 means draining outward through the open end).
//
// Every line is an element of the incident-edge linked list of its endpoint(s):
//   nextIncidentA - next line touching aIndex
//   nextIncidentB - next line touching bIndex (unused for a terminal line)
// LeyGraph owns the storage and each node's list head (LeyNode::firstIncident).
struct LeyLine {
  uint32_t aIndex{};
  uint32_t bIndex{};
  uint32_t nextIncidentA{kNoLine};
  uint32_t nextIncidentB{kNoLine};
  float capacity{};
  float flow{};
  bool alive{};

  bool isTerminal() const { return bIndex == kNoNode; }

  uint32_t other(uint32_t node) const {
    return node == aIndex ? bIndex : aIndex;
  }

  // Flow oriented so that "towards node" is positive.
  float flowTowards(uint32_t node) const {
    return node == bIndex ? flow : -flow;
  }

  // Extra flow that can still be pushed towards node. With symmetric capacity
  // this reaches 2*capacity when the line currently runs the other way.
  float residualTowards(uint32_t node) const {
    return capacity - flowTowards(node);
  }

  void pushTowards(uint32_t node, float amount) {
    flow += (node == bIndex) ? amount : -amount;
  }

  // The list link belonging to whichever endpoint `node` is.
  uint32_t &nextIncident(uint32_t node) {
    return node == aIndex ? nextIncidentA : nextIncidentB;
  }
  uint32_t nextIncident(uint32_t node) const {
    return node == aIndex ? nextIncidentA : nextIncidentB;
  }
};
