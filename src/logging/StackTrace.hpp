#ifndef StackTrace_HPP
#define StackTrace_HPP

#include <string>
#include <vector>
using namespace std;

namespace logging {

vector<string> getStackTrace(int maxItems = 256);

};

#endif
