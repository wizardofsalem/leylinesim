#include <leylib/FlowRndGenerator.h>

FlowRndGenerator::FlowRndGenerator(double mean, double stddev)
    : engine_(std::random_device{}()), dist_(mean, stddev) {}

FlowRndGenerator::FlowRndGenerator(double mean, double stddev, uint32_t seed)
    : engine_(seed), dist_(mean, stddev) {}

float FlowRndGenerator::next() { return dist_(engine_); }

double FlowRndGenerator::mean() const { return dist_.mean(); }

double FlowRndGenerator::stddev() const { return dist_.stddev(); }

void FlowRndGenerator::setParams(double mean, double stddev) {
  dist_.param(std::normal_distribution<double>::param_type{mean, stddev});
}

void FlowRndGenerator::seed(uint32_t seed) { engine_.seed(seed); }
