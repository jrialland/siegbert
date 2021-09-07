#ifndef Fs_HPP
#define Fs_HPP

#include <string>
using namespace std;

namespace logging {

class Fs {

public:
  static string getDir(const string &filename);

  static string getBasename(const string &filename);

  static string dirSep();

  static void mkdir(const string &dirname);
};

} // namespace logging

#endif
