#include <cassert>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "users.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid)
  : pid_(pid), user_(""), cmd_("")
  , startTimeAfterBoot_(-1), ram_(-1), upTime_(-1)
  , prevActiveJiffies_(0), cpu_utilization_(-1.0)
{
  int uid = LinuxParser::Uid(pid);
  user_ = Users::LookUpUserName(uid);
  cmd_ = LinuxParser::Command(pid);
  startTimeAfterBoot_ = LinuxParser::StartTimeAfterBoot(pid);

  // Note: Refresh() method is called to populate/refresh the
  // remaining members
}

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() { return cpu_utilization_; }

// Return the command that generated this process
string Process::Command() { return cmd_; }

// Return this process's memory utilization
string Process::Ram() { return to_string(ram_); }

// Return this process's memory utilization (as int)
int Process::RamAsInt() { return ram_; }

// Return the user (name) that generated this process
string Process::User() { return user_; }

// Return the age of this process (in seconds)
long Process::UpTime() { return upTime_; }

// Returns 'true' if the process has ended
bool Process::HasEnded()
{
  return LinuxParser::ProcessHasEnded(pid_);
}

// Refresh process data
void Process::Refresh(long systemUpTime, long systemActiveJiffiesDelta)
{
  // Refresh RAM
  ram_ = std::stoi(LinuxParser::Ram(pid_));

  // Refresh uptime
  assert(systemUpTime >= startTimeAfterBoot_);
  upTime_ = systemUpTime - startTimeAfterBoot_;

  // Refresh CPU utilisation information
  unsigned long long activeJiffies = LinuxParser::ActiveJiffies(pid_);

  if (activeJiffies == 0U)
  {
    ram_ = 0;
    upTime_ = 0;
    cpu_utilization_ = 0.0;
  }
  else
  {
    assert(activeJiffies >= prevActiveJiffies_);
    unsigned long long activeJiffiesDelta = activeJiffies - prevActiveJiffies_;

    cpu_utilization_ = 
      (systemActiveJiffiesDelta == 0) ?
      0.0 : ((float)activeJiffiesDelta / systemActiveJiffiesDelta);    
  }

  prevActiveJiffies_ = activeJiffies;
}
