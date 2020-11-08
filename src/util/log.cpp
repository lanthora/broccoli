#include "util/log.h"
#include <algorithm>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdarg.h>
#include <stdexcept>
#include <sys/time.h>

namespace broccoli {

LOG::LOG() : level(LOG::NONE), filename("") { buffer = new char[LOGBUFFERSIZE]; };

LOG::~LOG() {
  delete[] buffer;
  buffer = nullptr;
}

void LOG::Init(const LOG::LEVEL &level, const std::string &filename) {
  this->level = level;
  this->filename = filename;
}

void LOG::Write(const LOG::LEVEL &level, const std::string &log) {

  if (this->filename == "" || !(level & this->level)) return;

  std::ofstream logfile;
  logfile.open(this->filename, std::ios::app);
  logfile << "[" << LevelToString(level) << "]";
  logfile << "[" << CurrentTime() << "]:";
  logfile << log << std::endl;
  logfile.flush();
  logfile.close();
}

void LOG::FormatWrite(const LOG::LEVEL &level, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buffer, LOGBUFFERSIZE - 1, fmt, ap);
  va_end(ap);
  this->Write(level, buffer);
}

std::string LOG::LevelToString(const LOG::LEVEL &level) {
  switch (level) {
  case DEBUG:
    return "DEBUG";
  case INFO:
    return "INFO";
  case ERROR:
    return "ERROR";
  case NONE:
    return "NONE";
  case ALL:
    return "ALL";
  }
  return "NONE";
}

std::string LOG::CurrentTime() {
  std::stringstream ss;

  time_t rawtime;
  time(&rawtime);
  struct tm *ptm;
  ptm = localtime(&rawtime);
  ss << ptm->tm_year + 1900;
  ss << "/" << std::setw(2) << std::setfill('0') << ptm->tm_mon + 1;
  ss << "/" << std::setw(2) << std::setfill('0') << ptm->tm_mday;
  ss << " " << std::setw(2) << std::setfill('0') << ptm->tm_hour;
  ss << ":" << std::setw(2) << std::setfill('0') << ptm->tm_min;
  ss << ":" << std::setw(2) << std::setfill('0') << ptm->tm_sec;

  return ss.str();
}

LOG &LOG::GetInstance() {
  static LOG instance;
  return instance;
}

} // namespace broccoli
