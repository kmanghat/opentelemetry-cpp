#pragma once

// include libraries
#include <chrono>
#include <climits>
#include <string>
#include <unordered_map>
#include <vector>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace sdk
{
namespace zpages
{

/**
This enum is used to index into the Latency_Boundaries vector that is declared later on,
using this enum lets you access the Latecy_Boundary at each index without using magic numbers
**/
enum Latency_Boundary_Names
{
  ZERO_MICROSx10,
  MICROSx10_MICROSx100,
  MICROSx100_MILLIx1,
  MILLIx1_MILLIx10,
  MILLIx10_MILLIx100,
  MILLIx100_SECONDx1,
  SECONDx1_SECONDx10,
  SECONDx10_SECONDx100,
  SECONDx100_MAX
};

/**
LatencyBoundary class is used to define a single latency boundary with a upper and lower bound
**/
class LatencyBoundary
{
public:
  LatencyBoundary(long int lowerBound, long int upperBound)
  {
    latencyLowerBound = lowerBound;
    latencyUpperBound = upperBound;
  }

  long int getLatencyLowerBound() const { return latencyLowerBound; }

  long int getLatencyUpperBound() const { return latencyUpperBound; }

private:
  long int latencyLowerBound;
  long int latencyUpperBound;
};

/**
This constannt contains the 9 latency boundaries and enables them to be interated over
**/
const std::vector<LatencyBoundary> Latency_Boundaries = {
    LatencyBoundary(0, 1e4),    LatencyBoundary(1e4, 1e5),      LatencyBoundary(1e5, 1e6),
    LatencyBoundary(1e6, 1e7),  LatencyBoundary(1e7, 1e8),      LatencyBoundary(1e8, 1e9),
    LatencyBoundary(1e9, 1e10), LatencyBoundary(1e10, 11), LatencyBoundary(1e11, LONG_MAX)};

const int Latency_Boundaries_Size = 9;

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
