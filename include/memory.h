#ifndef MEMORY_H
#define MEMORY_H

#include "refresh.h"
#include "utilization.h"

class Memory : private UtilizationInterface, private RefreshInterface
{
public:
  float Utilization() const override;
  void Refresh() override;

private:
  float utilization_{0.0};
};

#endif
