#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>

#include "memory.h"
#include "process.h"
#include "processor.h"
#include "refresh.h"
#include "users.h"

enum ProcessOrder {
  kCpuAsc_ = 0,
  kCpuDsc_,
  kMemoryAsc_,
  kMemoryDsc_
};

class System : private RefreshInterface
{
public:
  void Refresh() override;

  Processor& Cpu();
  Memory& MemoryInfo();
  std::vector<Process>& Processes();
  long UpTime();
  int TotalProcesses();
  int RunningProcesses();
  std::string Kernel();
  std::string OperatingSystem();
  void ToggleProcessOrderByCpu();
  void ToggleProcessOrderByMemory();

private:
  void RefreshProcesses();
  void PopulateNewProcesses();
  std::vector<int> GetSortedActiveProcessPids();
  std::vector<int> GetSortedCachedProcessPids();
  void SortProcesses();

private:
  Processor cpu_ = {};                  // Refreshed
  Memory memory_ = {};                  // Refreshed
  long upTime_{0};                      // Refreshed
  int proc_running_{0};                 // Refreshed
  int proc_total_{0};                   // Refreshed
  std::vector<Process> processes_ = {}; // Refreshed
  std::string os_;                      // Read & set once (cached)
  std::string kernel_;                  // Read & set once (cached)
  ProcessOrder proc_order_{kCpuDsc_};   // Can be toggled at run time
};

#endif
