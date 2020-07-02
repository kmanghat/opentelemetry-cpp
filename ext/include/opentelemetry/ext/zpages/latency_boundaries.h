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
Latency_Boundary_Name enum is used to index into the Latency_Boundaries vector that is declared later on,
using this enum lets you access the Latecy_Boundary at each index without using magic numbers
**/
typedef enum LatencyBoundaryName
{
  k0MicroTo10Micro = 0,
  k10MicroTo100Micro,
  k100MicroTo1Milli,
  k1MilliTo10Milli,
  k10MilliTo100Milli,
  k100MilliTo1Second,
  k1SecondTo10Second,
  k10SecondTo100Second,
  k100SecondToMax
} LatencyBoundaryName;

/**
LatencyBoundary class is used to define a single latency boundary with a upper and lower bound
**/
class LatencyBoundary
{
public:
  LatencyBoundary(std::chrono::nanoseconds lower_bound, std::chrono::nanoseconds upper_bound)
  {
    latency_lower_bound_ = lower_bound;
    latency_upper_bound_ = upper_bound;
  }

  /**
   * GetLatencyLowerBound() function gets the lower bound of the this latency boundary
   * @return the lower bound of time duration
   */
  std::chrono::nanoseconds GetLatencyLowerBound() const { return latency_lower_bound_; }

  /**
   * GetLatencyUpperBound() function gets the upper bound of the this latency boundary
   * @return the lower upper bound of time duration
   */
  std::chrono::nanoseconds GetLatencyUpperBound() const { return latency_upper_bound_; }
  
  /**
   * IsDurationInBucket is used to check if a duration is within this latency boundary
   * @param duration is the given duration to be checked
   * @returns true if it is within the boundaries
   */
  bool IsDurationInBucket(std::chrono::nanoseconds duration) const
  {
    return (duration >= latency_lower_bound_ && duration < latency_upper_bound_);
  }

private:
  std::chrono::nanoseconds latency_lower_bound_;
  std::chrono::nanoseconds latency_upper_bound_;
};

/**
 *Latency_Boundaries constant that contains the 9 latency boundaries and enables them to be iterated over
 */
const std::vector<LatencyBoundary> kLatencyBoundaries = {
    LatencyBoundary(std::chrono::nanoseconds(0), std::chrono::nanoseconds(std::chrono::microseconds(10))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::microseconds(10)), std::chrono::nanoseconds(std::chrono::microseconds(100))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::microseconds(100)), std::chrono::nanoseconds(std::chrono::milliseconds(1))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::milliseconds(1)), std::chrono::nanoseconds(std::chrono::milliseconds(10))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::milliseconds(10)), std::chrono::nanoseconds(std::chrono::milliseconds(100))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::milliseconds(100)), std::chrono::nanoseconds(std::chrono::seconds(1))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::seconds(1)), std::chrono::nanoseconds(std::chrono::seconds(10))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::seconds(10)), std::chrono::nanoseconds(std::chrono::seconds(100))),
    LatencyBoundary(std::chrono::nanoseconds(std::chrono::seconds(100)), std::chrono::nanoseconds(std::chrono::system_clock::duration::max())),
  };

/** The number of latency boundaries **/
const int kNumberOfLatencyBoundaries = 9;

}  // namespace zpages
}  // namespace ext
OPENTELEMETRY_END_NAMESPACE
