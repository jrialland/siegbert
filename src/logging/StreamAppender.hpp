#ifndef StreamAppender_HPP
#define StreamAppender_HPP

#include "Appender.hpp"

#include <iostream>
using namespace std;
namespace logging {

class StreamAppender : public Appender {

private:
  ostream *stream_;

  bool tty_;

  bool deleteAfter_;

public:
  StreamAppender(ostream *stream, bool tty = false, bool deleteAfter = false);

  virtual ~StreamAppender();

  virtual bool append(const Logger &logger, const LogMessage &message) override;
};

} // namespace logging

#endif
