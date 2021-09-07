
#include "Logger.hpp"
#include "Logging.hpp"
#include "Appender.hpp"

#include <iostream>

namespace logging {

	Logger::Logger(const string& name) : level_(&LogLevel::Debug), name_(name) {
	}

	void Logger::level(const LogLevel &level) {
		level_ = &level;
	}
		
	const LogLevel& Logger::level() const {
		return *level_;
	}
		
	void Logger::setAppender(const string& appenderName) {
		appenderNames_.clear();
		addAppender(appenderName);
	}
	
	void Logger::addAppender(const string& appenderName) {
		appenderNames_.insert(appenderName);
	}
		
	void Logger::removeAppender(const string& appenderName) {
		appenderNames_.erase(appenderName);
	}
		
	bool Logger::isLevelEnabled(const LogLevel& level) const {
		int ord = level.ordinal();
		return (ord > LogLevel::Silent.ordinal()) && (ord >= this->level_->ordinal());
	}
	
	void Logger::log(const LogMessage& message) {
		for(auto appenderName : appenderNames_) {
			try {
				appender(appenderName).append(*this, message);
			} catch(...) {
				//ignore appender errors
			}
		}
	}

}//namespace logging
