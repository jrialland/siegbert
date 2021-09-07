

#include "LogLevel.hpp"
#include <map>
#include <sstream>

#include <boost/algorithm/string.hpp>

namespace logging {
	
	LogLevel::LogLevel(int ordinal, const string &name) :
		ordinal_(ordinal),
		name_(name) {
	}

	// declare a static variable for each log level
	#define DECL_LOGLEVEL(n, name) const LogLevel LogLevel::name(n, #name);
	DECL_LOGLEVELS()
	#undef DECL_LOGLEVEL


	const LogLevel& LogLevel::forName(const string &lvlName) {
		
		#define DECL_LOGLEVEL(n, name) if(boost::iequals(lvlName, #name)) { return LogLevel::name; } else 
		DECL_LOGLEVELS()
		#undef DECL_LOGLEVEL
		
		{
			ostringstream ss;
			ss << '\'' << lvlName << "\' : No such log level";
			throw runtime_error(ss.str());
		}
	}

	const LogLevel& LogLevel::forOrdinal(int ord) {		
		switch(ord) {
			
			#define DECL_LOGLEVEL(n, name) case n: return LogLevel::name;
			DECL_LOGLEVELS()
			#undef DECL_LOGLEVEL
			
			default:
				ostringstream ss;
				ss << ord << " : No such log level";
				throw runtime_error(ss.str());
		}
	}
	
	
	bool LogLevel::operator==(const LogLevel &other) const {
		return ordinal_ == other.ordinal_;
	}
	  
	bool LogLevel::operator!=(const LogLevel &other) const {
		return ordinal_ != other.ordinal_;
	}

	ostream& operator<<(ostream& os, const LogLevel& level) {
		os << level.name() << '(' << level.ordinal() << ')';
		return os;
	}
}
