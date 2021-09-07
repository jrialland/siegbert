#ifndef UdpAppender_HPP
#define UdpAppender_HPP

#include "Appender.hpp"

namespace logging {

/**
 * Produces logs as udp datagrams containing log4j:event xml, can be viewed with
 * tools like 'logbert' (https://github.com/couchcoding/Logbert)
 */
class UdpAppender : public Appender {

private:
  string host_;

  int port_;

public:
  UdpAppender(const string &host = "localhost", int port = 9000);

  ~UdpAppender();

  virtual bool append(const Logger &logger, const LogMessage &message) override;
};

} // namespace logging

#endif
