#include "processor.h"

#include "linux_parser.h"

float Processor::Utilization() const { return utilization_; }

unsigned long long Processor::ActiveJiffiesDelta() { return actvJiffiesDelta_; }

void Processor::Refresh() {
  unsigned long long actvJiffies = LinuxParser::ActiveJiffies();
  unsigned long long idleJiffies = LinuxParser::IdleJiffies();

  unsigned long long idleDelta = idleJiffies - idleJiffiesPrev_;
  unsigned long long actvDelta = actvJiffies - actvJiffiesPrev_;
  unsigned long long totalDelta = idleDelta + actvDelta;

  // Compute new utilization and update internals
  utilization_ = (totalDelta == 0) ? 0.0 : (((float)actvDelta) / totalDelta);
  idleJiffiesPrev_ = idleJiffies;
  actvJiffiesPrev_ = actvJiffies;
  actvJiffiesDelta_ = actvDelta;
}
