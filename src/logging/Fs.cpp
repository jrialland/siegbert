#include "Fs.hpp"

#include <cstring>
#include <libgen.h>
#include <sys/stat.h>

namespace logging {

string Fs::getDir(const string &filename) {
  char *buff = new char[filename.length() + 1];
  buff[filename.length()] = 0;
  strncpy(buff, filename.c_str(), filename.length());
  string dir(dirname(buff));
  delete[] buff;
  return dir;
}

string Fs::getBasename(const string &filename) {
  char *buff = new char[filename.length() + 1];
  buff[filename.length()] = 0;
  strncpy(buff, filename.c_str(), filename.length());
  string base(basename(buff));
  delete[] buff;
  return base;
}

string Fs::dirSep() {
#if defined(WIN32) || defined(_WIN32) ||                                       \
    defined(__WIN32) && !defined(__CYGWIN__)
  return "\\";
#else
  return "/";
#endif
}

void Fs::mkdir(const string &dirname) {
  ::mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

} // namespace logging
