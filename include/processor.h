#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "refresh.h"
#include "utilization.h"

class Processor : private UtilizationInterface, private RefreshInterface
{
public:
  float Utilization() const override;
  void Refresh() override;
  unsigned long long ActiveJiffiesDelta();

private:
  float utilization_{0.0};
  unsigned long actvJiffiesPrev_{0U};
  unsigned long idleJiffiesPrev_{0U};
  unsigned long actvJiffiesDelta_{0U};
};

#endif
