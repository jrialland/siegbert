
#include "StackTrace.hpp"

#include "execinfo.h"
#include <dlfcn.h>

#include <cxxabi.h>

std::string demangle(const char* name) {
	int status;
	char *demangled = abi::__cxa_demangle(name, NULL, 0, &status);
	if(status == 0) {
		string sdemangled(demangled);
		if(demangled != NULL) {
			free(demangled);
		}
		return sdemangled;
	} else {
		return name;
	}
}

namespace logging {
	
vector<string> getStackTrace(int maxSize) {
	vector<string> vec;
	void** buffer = new void*[maxSize];
	int items = backtrace(buffer, maxSize);
	
	if(items > 0) {
		
		char **symbols = backtrace_symbols (buffer, items);
		for(int i=0; i< items; i++) {
			Dl_info info;
			if(dladdr(buffer[i], &info) && info.dli_sname != nullptr) {
				vec.push_back(demangle(info.dli_sname));
			} else {
				vec.push_back(symbols[i]);
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
