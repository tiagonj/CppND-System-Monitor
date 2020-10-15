#include "users.h"
#include "linux_parser.h"

using std::string;

string Users::LookUpUserName(int uid)
{
   static Users instance_; // Singleton instance
   return instance_.GetNameFromUid(uid);
}

string Users::GetNameFromUid(int uid)
{
  string name;

  if (uid >= 0)	// Negative user IDs not allowed/recognised
  {
    if (users_map_.find(uid) == users_map_.end())
    {
      // UID not found in the map, look up using LinuxParser
      name = LinuxParser::UserFromUid(uid);

      // Only add to the map if LinuxParser look up was successful
      if (name.length() > 0U)
      {
        users_map_[uid] = name; // Add to the map
      }
    }
    else  // UID already in the map
    {
      name = users_map_[uid];
    }
  }

  return name;
}
