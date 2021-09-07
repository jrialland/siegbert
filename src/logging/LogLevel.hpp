#ifndef LogLevel_HPP
#define LogLevel_HPP

#include <iostream>
#include <string>
using namespace std;

#define ID_OF_SILENT 0
#define ID_OF_TRACE 1
#define ID_OF_DEBUG 2
#define ID_OF_INFO 3
#define ID_OF_WARN 4
#define ID_OF_ERROR 5
#define ID_OF_FATAL 6


#define DECL_LOGLEVELS()\
  DECL_LOGLEVEL(ID_OF_SILENT, Silent)\
  DECL_LOGLEVEL(ID_OF_TRACE, Trace)\
  DECL_LOGLEVEL(ID_OF_DEBUG, Debug)\
  DECL_LOGLEVEL(ID_OF_INFO, Info)\
  DECL_LOGLEVEL(ID_OF_WARN, Warn)\
  DECL_LOGLEVEL(ID_OF_ERROR, Error)\
  DECL_LOGLEVEL(ID_OF_FATAL, Fatal)


namespace logging {

class LogLevel {

private:
  int ordinal_;

  string name_;

public:
  LogLevel(int ordinal, const string &name);

#define DECL_LOGLEVEL(n, name) static const LogLevel name;
  DECL_LOGLEVELS()
#undef DECL_LOGLEVEL

  string name() const { return name_; }

  int ordinal() const { return ordinal_; }

  static const LogLevel &forName(const string &name);

  static const LogLevel &forOrdinal(int ordinal);

  bool operator==(const LogLevel &other) const;

  bool operator!=(const LogLevel &other) const;

  friend ostream &operator<<(ostream &os, const LogLevel &level);
  
};

} // namespace logging
#endif
