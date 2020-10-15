#include <cassert>
#include <cmath>
#include <dirent.h>
#include <experimental/filesystem>
#include <string>
#include <vector>
#include <unistd.h>

#include "linux_parser.h"

using std::ifstream;
using std::istringstream;
using std::stof;
using std::stoi;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return value for one line of meminfo file
unsigned long LinuxParser::MemoryUtilizationEntry(std::ifstream& f, std::string expectedEntryName)
{
  // Get line and replace ':' with ' ' (eases checking below)
  string line;
  std::getline(f, line);
  std::replace(line.begin(), line.end(), ':', ' ');

  // Extract entry name and value from line
  istringstream linestream(line);
  string entryName, value;

  if (linestream >> entryName >> value)
  {
    assert(entryName == expectedEntryName);
    return std::stoul(value);
  }
  
  return 0U;
}

// Read and return system memory utilization information
float LinuxParser::MemoryUtilization()
{
  ifstream filestream(kProcDirectory + kMeminfoFilename);
  float utilization{0.0};

  if (filestream.is_open())
  {
    // Could have read whole file into a dictionary but decided to keep this simple
    unsigned long MemTotal = MemoryUtilizationEntry(filestream, "MemTotal");
    unsigned long MemFree = MemoryUtilizationEntry(filestream, "MemFree");

    // Simple calculation
    unsigned long memUsed = MemTotal - MemFree;
    utilization = ((float)memUsed) / MemTotal;
  }

  return utilization;
}

// Read and return the system uptime
long LinuxParser::UpTime()
{
  ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open())
  {
    float upTimeValueFloat;
    if (filestream >> upTimeValueFloat)
    {
      return std::roundf(upTimeValueFloat);
    }
  }

  return 0;
}

// Read and return CPU utilization
vector<unsigned long long> LinuxParser::CpuUtilization()
{
  string path = kProcDirectory + kStatFilename;
  string cpuLine = GetRestOfLineAfterToken(path, "cpu");

  vector<string> raw;
  string raw_time;
  istringstream linestream(cpuLine);
  while (linestream >> raw_time)
  {
    raw.push_back(raw_time);
  }
  assert(raw.size() == CPUStates::kNumCpuStates_);

  // NOTE: The idle/non-idle categorisation below is based on this
  // StackOverflow answer: https://stackoverflow.com/a/23376195
  // In particular note the fact that 'guest' and 'guestnice' are
  // said to already be included in 'usertime' and 'usernice',
  // hence we've not bothered extracting those values from 'raw'.

  unsigned long long user = stoull(raw[CPUStates::kUser_]);
  unsigned long long nice = stoull(raw[CPUStates::kNice_]);
  unsigned long long syst = stoull(raw[CPUStates::kSystem_]);
  unsigned long long idle = stoull(raw[CPUStates::kIdle_]);
  unsigned long long iowt = stoull(raw[CPUStates::kIOwait_]);
  unsigned long long nirq = stoull(raw[CPUStates::kIRQ_]);
  unsigned long long sirq = stoull(raw[CPUStates::kSoftIRQ_]);
  unsigned long long stel = stoull(raw[CPUStates::kSteal_]);
  //raw_time_t gest = stoull(raw[kGuest_]);     // normal guest
  //raw_time_t gstn = stoull(raw[kGuestNice_]); // niced guest

  unsigned long long all_actv = user + nice + syst + nirq + sirq + stel;
  unsigned long long all_idle = idle + iowt;
  unsigned long long all_time = all_actv + all_idle;

  return {all_time, all_actv, all_idle};
}

// Read and return the number of jiffies for the system
unsigned long long LinuxParser::Jiffies()
{
  return LinuxParser::CpuUtilization()[0];
}

// Read and return the number of active jiffies for the system
unsigned long long LinuxParser::ActiveJiffies()
{
  return LinuxParser::CpuUtilization()[1];
}

// Read and return the number of idle jiffies for the system
unsigned long long LinuxParser::IdleJiffies()
{
  return LinuxParser::CpuUtilization()[2];
}

// Read and return the number of active jiffies for a PID
unsigned long long LinuxParser::ActiveJiffies(int pid)
{
  string path = kProcDirectory + kSep + to_string(pid) + kSep + kStatFilename;
  ifstream filestream(path);

  if (filestream.is_open())
  {
    unsigned long utime, stime;

    // See https://man7.org/linux/man-pages/man5/proc.5.html
    // and https://stackoverflow.com/a/16736599
    LinuxParser::SkipNTokens(filestream, 13);
    filestream >> utime;              // 14
    filestream >> stime;              // 15
    return utime + stime;
  }

  return 0;
}

int LinuxParser::GetProcessesEntry(std::string entryName)
{
  string path = kProcDirectory + kStatFilename;
  string line = LinuxParser::GetRestOfLineAfterToken(path, entryName);

  int value{0};
  istringstream linestream(line);
  linestream >> value;
  return value;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses()
{
  // NOTE: this is actually the total number of forks since boot,
  // e.g. see: https://man7.org/linux/man-pages/man5/proc.5.html
  return GetProcessesEntry("processes");
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses()
{
  return GetProcessesEntry("procs_running");
}

// Return path
string LinuxParser::ProcessFolderPath(int pid)
{
  return kProcDirectory + kSep + to_string(pid) + kSep;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid)
{
  string cmd;
  string path = LinuxParser::ProcessFolderPath(pid) + kSep + kCmdlineFilename;
  ifstream filestream(path);
  if (filestream.is_open())
  {
    std::getline(filestream, cmd);
  }
  return cmd;
}

// Read and return the memory used by a process (in MB)
string LinuxParser::Ram(int pid)
{
  string path = LinuxParser::ProcessFolderPath(pid) + kStatusFilename;
  string line = LinuxParser::GetRestOfLineAfterToken(path, "VmSize:");

  int ramInKb;
  istringstream linestream(line);
  if (linestream >> ramInKb)
  {
    return to_string((int)std::round(ramInKb / 1000.0)); // KB to MB
  }
  return string("0");
}

// Read and return the user ID associated with a process
int LinuxParser::Uid(int pid)
{
  string path = LinuxParser::ProcessFolderPath(pid) + kStatusFilename;
  string line = LinuxParser::GetRestOfLineAfterToken(path, "Uid:");

  int uid{-1};
  istringstream linestream(line);
  linestream >> uid;
  return uid;
}

// Read and return the start time (after boot) of a process
long LinuxParser::StartTimeAfterBoot(int pid)
{
  string path = LinuxParser::ProcessFolderPath(pid) + kStatFilename;
  ifstream filestream(path);

  long startTime{0};

  if (filestream.is_open())
  {
    LinuxParser::SkipNTokens(filestream, 21);
    filestream >> startTime;
    startTime /= sysconf(_SC_CLK_TCK);
  }

  return startTime;
}

bool LinuxParser::ProcessHasEnded(int pid)
{
  (void)pid;
  //std::error_code ec;
  //return !fs::is_directory(LinuxParser::ProcessFolderPath(pid), ec);
  return false;																									// TODO
}

// Get user name from user ID
std::string LinuxParser::UserFromUid(int uid)
{
  string line;
  string name, pwd, userId;

  ifstream filestream(kPasswordPath);
  if (filestream.is_open())
  {
    while (std::getline(filestream, line))
    {
      // Replace ':' with ' ' to ease parsing below
      std::replace(line.begin(), line.end(), ':', ' ');

      istringstream linestream(line);
      if (linestream >> name >> pwd >> userId)
      {
        if (std::stoi(userId) == uid)
        {
          return name;
        }
      }
    }
  }

  return name;  // Not found (empty string)
}

string LinuxParser::GetRestOfLineAfterToken(const string filepath, const std::string token)
{
  string result;
  ifstream filestream(filepath);

  if (filestream.is_open())
  {
    string line;
    while (std::getline(filestream, line))
    {
      string first;
      istringstream linestream(line);
      if ((linestream >> first) && (first == token))
      {
        std::getline(linestream, result);
      }
    }
  }

  return result;
}

void LinuxParser::SkipNTokens(std::ifstream& filestream, const int n)
{
  if (n > 0)
  {
    string dummy;
    for (int ii = 0; ii < n; ++ii)
    {
      filestream >> dummy;
    }
  }
}
