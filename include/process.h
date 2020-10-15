#ifndef PROCESS_H
#define PROCESS_H

#include <string>

/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
public:
  Process(int pid);
  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();
  std::string Ram();
  int RamAsInt();
  long UpTime();
  bool HasEnded();
  void Refresh(long systemUpTime, long systemActiveJiffiesDelta);

private:
  int pid_{-1};
  std::string user_;
  std::string cmd_;
  long startTimeAfterBoot_{-1};
  int ram_{-1};
  long upTime_{-1};
  unsigned long long prevActiveJiffies_{0};
  float cpu_utilization_{-1.0};
};

#endif
