
#include "UdpAppender.hpp"

#include <cstring>
#include "netdb.h" // hostent
#include "arpa/inet.h"
#include <unistd.h> //close()

#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>

using namespace std;

static string getIp(const string &host) {
	struct hostent *he;
	struct in_addr **addr_list;
	if((he = gethostbyname(host.c_str())) != NULL) {
		addr_list = (struct in_addr **) he->h_addr_list;	
		for(int i = 0; addr_list[i] != NULL; i++) {
			return string(inet_ntoa(*addr_list[i]));
		}
	}
	return host;
}

static bool sendDatagram(const string& host, int port, const string& msg) {
	sockaddr_in servaddr;
	int fd = socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0){
        return false;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    string ip = getIp(host);
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
    servaddr.sin_port = htons(port);
    
    bool sent = 0 == sendto(fd, msg.c_str(), msg.length()+1, 0, (sockaddr*)&servaddr, sizeof(servaddr));
	close(fd);
	return sent;
}

class XmlDumper {

private:

	ostream& ss;
	
	vector<pair<string, bool>> tags;
		
	static string encode(const string& data) {
		std::string buffer;
		buffer.reserve(data.size());
		for(size_t pos = 0; pos != data.size(); ++pos) {
			switch(data[pos]) {
				case '&':  buffer.append("&#x26;"); break;
				case '\"': buffer.append("&#x22;"); break;
				case '\'': buffer.append("&#x27;"); break;
				case '<':  buffer.append("&#x3C;"); break;
				case '>':  buffer.append("&#x3E;"); break;
				default:   buffer.append(&data[pos], 1); break;
			}
		}
		return buffer;
	}
	
public:

	XmlDumper(ostream &stream) : ss(stream) {
	}

	void startTag(const string &tagName) {
		if(!tags.empty() && ! tags.back().second) {
			ss << '>';
			tags.back().second = true;
		}
		tags.push_back(make_pair(tagName, false));
		ss << "<" << tagName;
	}
	
	void endTag() {
		pair<string, bool> tag = tags.back();
		if(!tag.second) {
			ss << "/>";
		} else {
			ss << "</" << tag.first << '>';
		}
		tags.pop_back();
	}
	
	void addAttr(const string& key, const string& value) {
		ss << ' ' << key << '=' << '"' << encode(value) << '"';
	}
	
	void addCData(const string& data) {
		if(!tags.back().second) {
			ss << '>';
			tags.back().second = true;
		}
		ss << "<![CDATA[" << data << "]]>";
	}

};

static string str_toupper(const std::string &s) {
	string u(s);
    std::transform(u.begin(), u.end(), u.begin(),
        [](unsigned char c){ return std::toupper(c); }
    );
    return u;
}

static string threadIdToStr(std::thread::id tid) {
	ostringstream ss;
	ss << "0x" << std::hex << tid;
	return ss.str();
}

namespace logging {
	
	UdpAppender::UdpAppender(const string& host, int port) : host_(host), port_(port) {
	}

	UdpAppender::~UdpAppender() {
	}

	bool UdpAppender::append(const Logger& logger, const LogMessage& m) {
		//format description : https://github.com/apache/logging-log4j2/blob/master/log4j-core/src/main/resources/Log4j-events.dtd

		ostringstream ss;
		
		XmlDumper xml(ss);
		xml.startTag("log4j:event");
		xml.addAttr("logger", logger.name());
		xml.addAttr("level", str_toupper(m.level.name()));
		xml.addAttr("timestamp", std::to_string(m.tstamp));
		xml.addAttr("thread", threadIdToStr(m.thread_id));
		
			xml.startTag("log4j:message");
			xml.addCData(m.message);
			xml.endTag();
		
			xml.startTag("log4j:locationInfo");
			xml.addAttr("class", "className");
			xml.addAttr("method", m.fnct);
			xml.addAttr("file", m.file);
			xml.addAttr("line", std::to_string(m.line));
			xml.endTag();
		
		
		xml.endTag();
		
		return sendDatagram(host_, port_, ss.str());
	}

}//namespace logging
