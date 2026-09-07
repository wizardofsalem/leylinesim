#pragma once
#include <cstdint>
#include <random>

class FlowRndGenerator {
public:
  FlowRndGenerator(double mean, double stddev);

  FlowRndGenerator(double mean, double stddev, uint32_t seed);

  ~FlowRndGenerator() = default;

  float next();

  double mean() const;
  double stddev() const;
  void setParams(double mean, double stddev);

  void seed(uint32_t seed);

private:
  std::mt19937 engine_;
  std::normal_distribution<double> dist_;
};
