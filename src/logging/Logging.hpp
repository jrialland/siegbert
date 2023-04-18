#ifndef Logging_HPP
#define Logging_HPP

#include "Appender.hpp"
#include "LogLevel.hpp"
#include "Logger.hpp"

#include <map>
#include <string>
using namespace std;

namespace logging {

class LoggingConfig {
public:
  LoggingConfig &withUdp();
  LoggingConfig &withRollingFiles();
};

class Logging {

private:
  static Logging *instance_;

  map<string, Appender *> appenders_;

  map<string, Logger *> loggers_;

  Logging();

  ~Logging();

public:
  static Logging &instance();

  Logger *getLogger(const string &name);

  Appender *getAppender(const string &name);

  void setAppender(const string &name, Appender *appender);
};

Logger &logger(const string &name = "default");

Appender &appender(const string &name);

void appender(const string &name, Appender *appender);

#ifndef NDEBUG

LoggingConfig basicConfig(int argc, char **argv,
                          const LogLevel &level = LogLevel::Info);

#else

LoggingConfig basicConfig(int argc, char **argv,
                          const LogLevel &level = LogLevel::Debug);
#endif

} // namespace logging

#define LOG_TRACE(...)                                                         \
  logging::logger().log(logging::LogLevel::Trace, __FILE__, __LINE__,          \
                        __PRETTY_FUNCTION__, __VA_ARGS__)

#define LOG_DEBUG(...)                                                         \
  logging::logger().log(logging::LogLevel::Debug, __FILE__, __LINE__,          \
                        __PRETTY_FUNCTION__, __VA_ARGS__)

#define LOG_INFO(...)                                                          \
  logging::logger().log(logging::LogLevel::Info, __FILE__, __LINE__,           \
                        __PRETTY_FUNCTION__, __VA_ARGS__)

#define LOG_WARN(...)                                                          \
  logging::logger().log(logging::LogLevel::Warn, __FILE__, __LINE__,           \
                        __PRETTY_FUNCTION__, __VA_ARGS__)

#define LOG_ERROR(...)                                                         \
  logging::logger().log(logging::LogLevel::Error, __FILE__, __LINE__,          \
                        __PRETTY_FUNCTION__, __VA_ARGS__)

#define LOG_FATAL(...)                                                         \
  logging::logger().log(logging::LogLevel::Fatal, __FILE__, __LINE__,          \
                        __PRETTY_FUNCTION__, __VA_ARGS__)

#endif
