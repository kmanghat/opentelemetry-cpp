#pragma once

// include libraries
#include <chrono>
#include <climits>
#include <string>
#include <unordered_map>
#include <vector>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace ext
{
namespace zpages
{

/**
This enum is used to index into the Latency_Boundaries vector that is declared later on,
using this enum lets you access the Latecy_Boundary at each index without using magic numbers
**/
typedef enum Latency_Boundary_Name
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
} Latency_Boundary_Name;

/**
LatencyBoundary class is used to define a single latency boundary with a upper and lower bound
**/
class LatencyBoundary
{
public:
  LatencyBoundary(std::chrono::nanoseconds lowerBound, std::chrono::nanoseconds upperBound)
  {
    latencyLowerBound = lowerBound;
    latencyUpperBound = upperBound;
  }

  std::chrono::nanoseconds GetLatencyLowerBound() const { return latencyLowerBound; }

  std::chrono::nanoseconds GetLatencyUpperBound() const { return latencyUpperBound; }

private:
  std::chrono::nanoseconds latencyLowerBound;
  std::chrono::nanoseconds latencyUpperBound;
};

/**
This constannt contains the 9 latency boundaries and enables them to be interated over
**/
const std::vector<LatencyBoundary> Latency_Boundaries = {
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::microseconds(0)), std::chrono::nanoseconds(std::chrono::microseconds(10))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::microseconds(10)), std::chrono::nanoseconds(std::chrono::microseconds(100))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::microseconds(100)), std::chrono::nanoseconds(std::chrono::milliseconds(1))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::milliseconds(1)), std::chrono::nanoseconds(std::chrono::milliseconds(10))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::milliseconds(10)), std::chrono::nanoseconds(std::chrono::milliseconds(100))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::milliseconds(100)), std::chrono::nanoseconds(std::chrono::seconds(1))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::seconds(1)), std::chrono::nanoseconds(std::chrono::seconds(10))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::seconds(10)), std::chrono::nanoseconds(std::chrono::seconds(100))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::seconds(100)), std::chrono::nanoseconds(std::chrono::system_clock::duration::max())),
  };

const int NUMBER_OF_LATENCY_BOUNDARIES = 9;

}  // namespace zpages
}  // namespace sdk
OPENTELEMETRY_END_NAMESPACE
