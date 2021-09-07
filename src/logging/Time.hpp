#ifndef Time_HPP
#define Time_HPP

#include <string>
    using namespace std;

namespace logging {

class Time {
public:
  /** @return current date (yyyy-mm-dd) */
  static string currentDate();

  /** @return current date (yyyy-MM-dd hh:mm:ss.xxx) */
  static string toDatetime(unsigned long tstamp);

  /** @return current timestamp, in milliseconds */
  static unsigned long currentTimestamp();

  /** @return true if the given date is older that nDays ago */
  static bool isNDaysAgo(int nDays, int year, int month, int day);
};
}

#endif
