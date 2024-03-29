
#include "Logging.hpp"
#include "FileAppender.hpp"
#include "Fs.hpp"
#include "StreamAppender.hpp"
#include "UdpAppender.hpp"

#include <exception>
#include <regex>
using namespace std;

#include <cstdio>         // for fileno, stdin
#include <linux/limits.h> // PATH_MAX
#include <unistd.h>       // for isatty

namespace logging {

Logging *Logging::instance_ = new Logging;

Logging::Logging() {
  setAppender("stdout", new StreamAppender(&cout, isatty(fileno(stdout))));
  setAppender("stderr", new StreamAppender(&cout, isatty(fileno(stdout))));
  getLogger("default")->setAppender("stderr");
}

Logging::~Logging() {
  for (auto it : loggers_) {
    delete it.second;
  }
  for (auto it : appenders_) {
    delete it.second;
  }
}

Logging &Logging::instance() { return *instance_; }

Logger *Logging::getLogger(const string &name) {
  auto it = loggers_.find(name);
  if (it != loggers_.end()) {
    return it->second;
  } else {
    return loggers_[name] = new Logger(name);
  }
}

Appender *Logging::getAppender(const string &name) {
  auto it = appenders_.find(name);
  if (it != appenders_.end()) {
    return it->second;
  } else {
    return nullptr;
  }
}

void Logging::setAppender(const string &name, Appender *appender) {
  auto it = appenders_.find(name);
  if (it != appenders_.end()) {
    delete it->second;
  }
  appenders_[name] = appender;
}

Logger &logger(const string &name) {
  Logger *logger = Logging::instance().getLogger(name);
  return *logger;
}

Appender &appender(const string &name) {
  Appender *appender = Logging::instance().getAppender(name);
  if (appender == nullptr) {
    throw runtime_error("No such appender : '" + name + "'");
  } else {
    return *appender;
  }
}

void appender(const string &name, Appender *appender) {
  Logging::instance().setAppender(name, appender);
}

LoggingConfig basicConfig(int argc, char **argv, const LogLevel &level) {
  if (argc > 0) {
    string logdir = Fs::getDir(argv[0]) + Fs::dirSep() + "log";
    Fs::mkdir(logdir);
    string prgName = Fs::getBasename(argv[0]);
    prgName = std::regex_replace(prgName, std::regex("\\.(exe|EXE)$"), "");
    appender("logfile", new FileAppender(logdir + Fs::dirSep() + prgName));
    logger().level(level);
    logger().setAppender("logfile");
    logger().addAppender("stderr");
  }
  LoggingConfig config;
  return config;
}

LoggingConfig &LoggingConfig::withUdp() {
  appender("udp", new UdpAppender);
  logger().addAppender("udp");
  return *this;
}

LoggingConfig &LoggingConfig::withRollingFiles() {
  char buff[PATH_MAX];
  int count = readlink("/proc/self/exe", buff, PATH_MAX);
  string exename(buff);
  exename = exename.substr(0, count);
  appender("rolling_files", new FileAppender(exename, ".log", true, 7));
  logger().addAppender("rolling_files");
  return *this;
}

} // namespace logging
