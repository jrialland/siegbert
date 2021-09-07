
#include "LogMessage.hpp"
#include "StackTrace.hpp"
#include "Time.hpp"

namespace logging {

LogMessage::LogMessage(const LogLevel &lvl)
    : level(lvl), message(""), fnct(""), file(""), line(0) {
  tstamp = Time::currentTimestamp();
  thread_id = std::this_thread::get_id();
  stackTrace = getStackTrace();
  if (stackTrace.size()) {
    stackTrace.erase(stackTrace.begin());
  }
}

} // namespace logging
