#ifndef Logger_HPP
#define Logger_HPP

#include <set>
#include <sstream>
#include <string>
using namespace std;

#include "LogLevel.hpp"
#include "LogMessage.hpp"
namespace logging {

class Logger {

private:
  const LogLevel *level_;

  string name_;

  set<string> appenderNames_;

  template <class Arg0> ostream &concat(ostream &os, int count, Arg0 arg0) {
    if (count) {
      os << ' ';
    }
    return os << arg0;
  }

  template <class Arg0, class... Args>
  ostream &concat(ostream &os, int count, Arg0 arg0, Args... args) {
    if (count > 0) {
      os << ' ';
    }
    os << arg0;
    return concat(os, count + 1, args...);
  }

public:
  Logger(const string &name);

  string name() const { return name_; }

  void level(const LogLevel &level);

  const LogLevel &level() const;

  void setAppender(const string &appenderName);

  void addAppender(const string &appenderName);

  void removeAppender(const string &appenderName);

  void log(const LogMessage &message);

  bool isLevelEnabled(const LogLevel &level) const;

  inline bool isTraceEnabled() const { return isLevelEnabled(LogLevel::Trace); }

  inline bool isDebugEnabled() const { return isLevelEnabled(LogLevel::Debug); }

  inline bool isInfoEnabled() const { return isLevelEnabled(LogLevel::Info); }

  inline bool isWarnEnabled() const { return isLevelEnabled(LogLevel::Warn); }

  inline bool isErrorEnabled() const { return isLevelEnabled(LogLevel::Error); }

  inline bool isFatalEnabled() const { return isLevelEnabled(LogLevel::Fatal); }

  template <class... Args>
  void log(const LogLevel &level, const string &file, int line,
           const string &fnct, Args... args) {
    ostringstream ss;
    concat(ss, 0, args...);
    LogMessage m(level);
    m.file = file;
    m.line = line;
    m.fnct = fnct;
    m.message = ss.str();
    log(m);
  }
};

} // namespace logging
#endif
