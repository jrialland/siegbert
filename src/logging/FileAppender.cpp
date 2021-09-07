
#include "FileAppender.hpp"
#include "StreamAppender.hpp"
#include "Fs.hpp"
#include "Time.hpp"

#include <boost/regex.hpp>
#include <fstream>
using namespace std;

#include <dirent.h>

#define FILE_CLEANUP_DELAY_SEC (10*60) //10 minutes


namespace logging {

FileAppender::FileAppender(const string& name, const string& extension, bool rolling, int keptDays) :
	name_(name),
	extension_(extension),
	keptDays_(keptDays),
	rolling_(rolling),
	lastchecked_(0) {
}

bool FileAppender::append(const Logger& logger, const LogMessage& message) {
	string filename = name_;
	if(rolling_) {
		filename += "_" + Time::currentDate();
	}
	filename += extension_;
	std::ofstream ofs;
	ofs.open (filename.c_str(), std::ofstream::out | std::ofstream::app);
	
	bool emitted = false;
	if(ofs.is_open()) {		
		StreamAppender streamAppender(&ofs, false, false);
		emitted = streamAppender.append(logger, message);
		ofs.close();
	}

	//verify if we have to do some file cleanup
	unsigned long now = Time::currentTimestamp();
	if(now - lastchecked_ > FILE_CLEANUP_DELAY_SEC) {
		try {
			cleanup();
		} catch(...) {
			//silent
		}
		lastchecked_ = now;
	}
	
	return emitted;
}

void FileAppender::cleanup() {
	string path = Fs::getDir(name_);
	string base = Fs::getBasename(name_);

	string escaped = boost::regex_replace(base + "_([0-9]{4})-([0-9]{2})-([0-9]{2})" + extension_, boost::regex("\\."), "\\\\.");
	boost::regex pattern(escaped);
		
	DIR *dir;
	struct dirent *ent;
	
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			
			if(ent->d_type == DT_REG) {
			
				string filename(ent->d_name);
				
				boost::cmatch what;
				
				if(boost::regex_match(filename.c_str(), what, pattern)) {
				
					int year = atoi(what[1].first);
					int month = atoi(what[2].first);
					int day = atoi(what[3].first);
				
					if(Time::isNDaysAgo(keptDays_, year, month, day)) {
						string toremove = path + Fs::dirSep() + filename;
						remove(toremove.c_str());
					}
				}
			}
		}
		closedir (dir);
	}
}

}//namespace logging
