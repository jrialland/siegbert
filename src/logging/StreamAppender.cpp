
#include "StreamAppender.hpp"
#include "Time.hpp"

#include <algorithm>
#include <cstdio>
#include <regex>
#include <time.h>

namespace ansiseq {

const char *reset = "\x1b[0m";
const char *red = "\x1b[91m";
const char *green = "\x1b[92m";
const char *yellow = "\x1b[93m";
const char *blue = "\x1b[94m";
const char *gray = "\x1b[90m";
const char *white = "\x1b[97m";

const char *getColorForLevel(const logging::LogLevel &level) {
  switch (level.ordinal()) {
  case ID_OF_TRACE:
    return gray;
  case ID_OF_DEBUG:
    return green;
  case ID_OF_INFO:
    return blue;
  case ID_OF_WARN:
    return yellow;
  case ID_OF_ERROR:
    return red;
  case ID_OF_FATAL:
    return "\x1b[30;101m"; // black on bright red bg
  default:
    return reset;
  }
}

} // namespace ansiseq

namespace logging {

StreamAppender::StreamAppender(ostream *stream, bool tty, bool deleteAfter)
    : stream_(stream), tty_(tty), deleteAfter_(deleteAfter) {}

StreamAppender::~StreamAppender() {
  if (deleteAfter_) {
    delete stream_;
  }
}

bool StreamAppender::append(const Logger &logger, const LogMessage &message) {

  if (!logger.isLevelEnabled(message.level)) {
    return false;
  }

  if (tty_) {
    *(stream_) << ansiseq::yellow;
  }

  *(stream_) << '[';

  if (tty_) {
    *(stream_) << ansiseq::gray;
  }

  *(stream_) << Time::toDatetime(message.tstamp);

  if (tty_) {
    *(stream_) << ansiseq::yellow;
  }

  *(stream_) << ']';

  if (tty_) {
    *(stream_) << ansiseq::reset;
  }

  *(stream_) << '[' << logger.name() << ']';

  *(stream_) << '[';

  if (tty_) {
    *(stream_) << ansiseq::getColorForLevel(message.level);
  }

  *(stream_) << message.level.name();

  if (tty_) {
    *(stream_) << ansiseq::reset;
  }

  *(stream_) << ']';

#if defined(WIN32) || defined(_WIN32) ||                                       \
    defined(__WIN32) && !defined(__CYGWIN__)
  string nl = "\r\n";
#else
  string nl = "\n";
#endif
  string repl = tty_ ? (nl + "\t" + ansiseq::gray + "|\t" + ansiseq::white)
                     : (nl + "\t|\t");
  string sanitizedmsg =
      std::regex_replace(message.message, std::regex("\r?\n"), repl);

  if (tty_) {
    *(stream_) << ansiseq::white;
  }
  if (sanitizedmsg.compare(message.message)) {
    *(stream_) << repl << sanitizedmsg << nl;
  } else {
    *(stream_) << "\t" << sanitizedmsg << nl;
  }

  if (tty_) {
    *(stream_) << ansiseq::reset;
  }

  stream_->flush();
  return stream_->good();
}

} // namespace logging
