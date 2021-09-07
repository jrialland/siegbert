#ifndef FileAppender_HPP
#define FileAppender_HPP

#include "Appender.hpp"
#include "StreamAppender.hpp"

namespace logging {
class FileAppender : public Appender {

private:
  string name_;

  string extension_;

  int keptDays_;

  bool rolling_;

  unsigned long lastchecked_;

public:
  FileAppender(const string &name, const string &extension = ".log",
               bool rolling = true, int keptDays = 7);

  virtual bool append(const Logger &logger, const LogMessage &message) override;

  void cleanup();
};

} // namespace logging

#endif
