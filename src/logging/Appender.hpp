#ifndef Appender_HPP
#define Appender_HPP

#include "LogLevel.hpp"
#include "Logger.hpp"
namespace logging {

class Appender {
public:
  virtual bool append(const Logger &logger, const LogMessage &message) = 0;

  virtual ~Appender();
};

} // namespace logging
#endif
