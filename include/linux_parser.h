#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <regex>
#include <string>

namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc/"};
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};
const std::string kSep{"/"};

// System
unsigned long MemoryUtilizationEntry(std::string path, std::string entryName);
float MemoryUtilization();
long UpTime();
std::vector<int> Pids();
int GetProcessesEntry(std::string entryName);
int TotalProcesses();
int RunningProcesses();
std::string OperatingSystem();
std::string Kernel();

// CPU
enum CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_,
  kNumCpuStates_
};
std::vector<unsigned long long> CpuUtilization();
unsigned long long Jiffies();
unsigned long long ActiveJiffies();
unsigned long long IdleJiffies();
unsigned long long ActiveJiffies(int pid);

// Processes
std::string ProcessFolderPath(int pid);
std::string Command(int pid);
std::string Ram(int pid);
int Uid(int pid);
long StartTimeAfterBoot(int pid);
bool ProcessHasEnded(int pid);

// Users
std::string UserFromUid(int uid);
  
// Helpers
std::string GetRestOfLineAfterToken(const std::string filepath, const std::string token);
void SkipNTokens(std::ifstream& filestream, const int n);

};  // namespace LinuxParser

#endif