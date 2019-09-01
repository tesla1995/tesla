#include "log/logging.h"
#include "log/asynclogging.h"

#include <unistd.h>

using namespace tesla::log;

int main(int argc, char** argv)
{
  //Logger::set_loglevel(tesla::log::Logger::LogLevel::TRACE);
  //AsyncLogging::Init(argv[0], Logger::LogLevel::TRACE);  
  
  LOG_TRACE << "test";   
  LOG_DEBUG << "test";   
  LOG_INFO  << "test";   
  LOG_ERROR << "test";   
  LOG_WARN  << "test";   

  //CHECK(true) << "test true";
  //CHECK(false) << "test false";

  sleep(3);
  return 0;
}
