#ifndef USERS_H
#define USERS_H

#include <string>
#include <unordered_map>

class Users {
 public:
  static std::string LookUpUserName(int uid);

 private:
  // Note: we could also delete the copy & move
  // c'tors and assignment op's but the singleton's
  // ref is never exposed, so not strictly needed.
  Users() {}

 private:
  std::string GetNameFromUid(int uid);
  std::unordered_map<int, std::string> users_map_;
};

#endif
