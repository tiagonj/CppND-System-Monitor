#include <iomanip>
#include <string>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds)
{
  if (seconds < 0)
  {
    return string();
  }

  long hours = seconds / 3600;
  seconds -= (hours * 3600);

  long minutes = seconds / 60;
  seconds -= (minutes * 60);

  // If we were using C++20 we could use std::format. 
  // As we're on C++17 let's use stringstream
  std::stringstream ss;
  ss << std::setw(2) << std::setfill('0') << std::to_string(hours) << ":";
  ss << std::setw(2) << std::setfill('0') << std::to_string(minutes) << ":";
  ss << std::setw(2) << std::setfill('0') << std::to_string(seconds);
  return ss.str();
}
