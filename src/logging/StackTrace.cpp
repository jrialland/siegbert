
#include "StackTrace.hpp"

#include "execinfo.h"
#include <dlfcn.h>
#include <cxxabi.h>

#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>
#include <boost/algorithm/string.hpp>

std::string demangle(const char* name) {
	int status;
	char *demangled = abi::__cxa_demangle(name, NULL, 0, &status);
	if(status == 0) {
		string sdemangled(demangled);
		if(demangled != nullptr) {
			free(demangled);
		}
		return sdemangled;
	} else {
		return name;
	}
}

std::string get_executable_name() {
	#if defined(PLATFORM_POSIX) || defined(__linux__) //check defines for your setup

    string sp;
    ifstream("/proc/self/comm") >> sp;
	return sp;

	#elif defined(_WIN32) 
	
	char buf[MAX_PATH];
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
	return buf;

	#else

	return "";

	#endif
}

string addr2line(void *addr) {
	ostringstream ss;
	ss << "addr2line -a " << addr << " -e ./" << get_executable_name();
	std::array<char, 128> buffer;
	unique_ptr<FILE, decltype(&pclose)> pipe(popen(ss.str().c_str(), "r"), pclose);

	if(!pipe) {
		return "";
	}

	string result;

	while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	boost::trim(result);
	boost::replace_all(result, "\n", " ");
	return result;
}

namespace logging {
	
vector<string> getStackTrace(int maxSize) {
	vector<string> vec;
	void** buffer = new void*[maxSize];
	int items = backtrace(buffer, maxSize);
	
	if(items > 0) {
		
		char **symbols = backtrace_symbols (buffer, items);
		for(int i=0; i< items; i++) {

			string line;

			Dl_info info;
			if(dladdr(buffer[i], &info) && info.dli_sname != nullptr) {
				line += demangle(info.dli_sname);
				line += " [" + addr2line( (void*)((char*)info.dli_saddr - (char*)info.dli_fbase)) + "]"; 
			} else {
				line += string(symbols[i]);
			}
			if(!line.empty()) {
				vec.push_back(line);
			}
		}
		
		if(symbols != NULL) {
			free(symbols);
		}
		
		if(items == maxSize) {
			vec.push_back("[...]");
		}
	}
	
	if(vec.size() > 0) {
		vec.erase(vec.begin());
	}
	
	return vec;
}

};
