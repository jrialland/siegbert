
#include "Time.hpp"
#include <ctime>

#if defined(WIN32) || defined(_WIN32) ||                                       \
    defined(__WIN32) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#include <stdint.h>
#include <windows.h>
typedef struct timeval {
  long tv_sec;
  long tv_usec;
} timeval;

int gettimeofday(struct timeval *tp, struct timezone *tzp) {
  static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);
  SYSTEMTIME system_time;
  FILETIME file_time;
  uint64_t time;
  GetSystemTime(&system_time);
  SystemTimeToFileTime(&system_time, &file_time);
  time = ((uint64_t)file_time.dwLowDateTime);
  time += ((uint64_t)file_time.dwHighDateTime) << 32;
  tp->tv_sec = (long)((time - EPOCH) / 10000000L);
  tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
  return 0;
}
#else
#include <sys/time.h>
#endif

namespace logging {

unsigned long Time::currentTimestamp() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

string Time::toDatetime(unsigned long tstamp) {
  time_t t = tstamp / 1000;
  int ms = tstamp - (t * 1000);
  char buffer[32];
  struct tm *tm = localtime(&t);
  sprintf(buffer, "%d-%02d-%02d %02d:%02d:%02d.%03d", tm->tm_year + 1900,
          tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, ms);
  return string(buffer);
}

string Time::currentDate() {
  string dt = toDatetime(currentTimestamp());
  return dt.substr(0, dt.length() - 4);
}

bool Time::isNDaysAgo(int nDays, int year, int month, int day) {
  struct tm date;
  date.tm_sec = date.tm_min = date.tm_hour = 0;
  date.tm_mday = day;
  date.tm_mon = month - 1;
  date.tm_year = year - 1900;
  time_t t = mktime(&date);
  time_t now = time(NULL);
  double secs = difftime(now, t);
  return secs > nDays * 24 * 60 * 60;
}

} // namespace logging
