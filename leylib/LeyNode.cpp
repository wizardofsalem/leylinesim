#include <leylib/LeyNode.h>

LeyNode::LeyNode(NodeHandle handle, FlowRndGenerator &flowGenerator)
    : handle_(handle), flow_(flowGenerator.next()) {}

LeyNode::LeyNode(NodeHandle handle, float flowAmount)
    : handle_(handle), flow_(flowAmount) {}

void LeyNode::increaseGeneration() { handle_.gen++; }
