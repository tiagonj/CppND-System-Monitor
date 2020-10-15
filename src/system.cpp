#include "system.h"

#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::size_t;
using std::sort;
using std::string;
using std::vector;

// Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// Return the system's memory
Memory& System::MemoryInfo() { return memory_; }

// Return a container composed of the system's processes
vector<Process>& System::Processes() { return processes_; }

// Return the system's kernel identifier (string)
std::string System::Kernel() {
  if (kernel_.length() == 0U) {
    kernel_ = LinuxParser::Kernel();
  }
  return kernel_;
}

// Return the operating system name
std::string System::OperatingSystem() {
  if (os_.length() == 0U) {
    os_ = LinuxParser::OperatingSystem();
  }
  return os_;
}

// Return the number of processes actively running on the system
int System::RunningProcesses() { return proc_running_; }

// Return the total number of processes on the system
int System::TotalProcesses() { return proc_total_; }

// Return the number of seconds since the system started running
long System::UpTime() { return upTime_; }

void System::Refresh() {
  // System up time
  upTime_ = LinuxParser::UpTime();

  // Refresh cached CPU & memory data
  cpu_.Refresh();
  memory_.Refresh();

  // Refress processes data
  proc_running_ = LinuxParser::RunningProcesses();
  proc_total_ = LinuxParser::TotalProcesses();
  RefreshProcesses();
  SortProcesses();
}

void System::RefreshProcesses() {
  // Purge any processes that have ended
  processes_.erase(std::remove_if(processes_.begin(), processes_.end(),
                                  [](Process& p) { return p.HasEnded(); }),
                   processes_.end());

  // Get new PIDs and add processes for those
  PopulateNewProcesses();

  unsigned long long systemActiveJiffiesDelta = cpu_.ActiveJiffiesDelta();

  // Refresh data for all active processes
  for (auto& p : processes_) {
    p.Refresh(upTime_, systemActiveJiffiesDelta);
  }
}

void System::PopulateNewProcesses() {
  vector<int> activePids = GetSortedActiveProcessPids();
  vector<int> cachedPids = GetSortedCachedProcessPids();

  vector<int> newPids;
  std::set_difference(activePids.begin(), activePids.end(), cachedPids.begin(),
                      cachedPids.end(),
                      std::inserter(newPids, newPids.begin()));

  for (auto pid : newPids) {
    processes_.push_back(Process(pid));
  }
}

vector<int> System::GetSortedCachedProcessPids() {
  vector<int> pids;
  for (auto p : processes_) {
    pids.push_back(p.Pid());
  }
  sort(pids.begin(), pids.end());
  return pids;
}

vector<int> System::GetSortedActiveProcessPids() {
  vector<int> pids = LinuxParser::Pids();
  sort(pids.begin(), pids.end());
  return pids;
}

void System::SortProcesses() {
  if (proc_order_ == ProcessOrder::kCpuAsc_) {
    sort(processes_.begin(), processes_.end(), [](Process& a, Process& b) {
      return a.CpuUtilization() < b.CpuUtilization();
    });
  } else if (proc_order_ == ProcessOrder::kCpuDsc_) {
    sort(processes_.begin(), processes_.end(), [](Process& a, Process& b) {
      return a.CpuUtilization() > b.CpuUtilization();
    });
  } else if (proc_order_ == ProcessOrder::kMemoryAsc_) {
    sort(processes_.begin(), processes_.end(),
         [](Process& a, Process& b) { return a.RamAsInt() < b.RamAsInt(); });
  } else {
    assert(proc_order_ == ProcessOrder::kMemoryDsc_);
    sort(processes_.begin(), processes_.end(),
         [](Process& a, Process& b) { return a.RamAsInt() > b.RamAsInt(); });
  }
}

void System::ToggleProcessOrderByCpu() {
  if (proc_order_ == ProcessOrder::kCpuDsc_) {
    proc_order_ = kCpuAsc_;
  } else  // kCpuAsc_, kMemoryAsc_ or kMemoryDsc_
  {
    proc_order_ = kCpuDsc_;
  }
}

void System::ToggleProcessOrderByMemory() {
  if (proc_order_ == ProcessOrder::kMemoryDsc_) {
    proc_order_ = kMemoryAsc_;
  } else  // kMemoryAsc_, kCpuAsc_ or kCpuDsc_
  {
    proc_order_ = kMemoryDsc_;
  }
}
