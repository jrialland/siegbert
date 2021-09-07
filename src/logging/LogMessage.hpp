#ifndef LogMessage_HPP
#define LogMessage_HPP

#include "LogLevel.hpp"

#include <string>
#include <thread>
#include <vector>

using namespace std;

namespace logging {

struct LogMessage {
  unsigned long tstamp;
  const LogLevel &level;
  string message;
  string fnct;
  string file;
  int line;
  std::thread::id thread_id;
  vector<string> stackTrace;
  LogMessage(const LogLevel &level);
};

} // namespace logging
#endif
