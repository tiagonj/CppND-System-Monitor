#include "memory.h"

#include "linux_parser.h"

float Memory::Utilization() const { return utilization_; }

void Memory::Refresh() {
  float newUtilization = LinuxParser::MemoryUtilization();

  if ((newUtilization >= 0.0) && (newUtilization <= 1.0)) {
    utilization_ = newUtilization;
  } else {
    utilization_ = 0.0;
  }
}
