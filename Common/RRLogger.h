#pragma once

#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <iostream>
#include <string>
#include <time.h>

static int NowDateToInt()
{
  time_t now;
  time(&now);

  tm p;
#ifdef _WIN32
  localtime_s(&p, &now);
#else
  localtime_r(&now, &p);
#endif

  int now_date = (1900 + p.tm_year) * 10000 + (p.tm_mon + 1) * 100 + p.tm_mday;

  return now_date;
}

static int NowTimeToInt()
{
  time_t now;
  time(&now);

  tm p;
#ifdef _WIN32
  localtime_s(&p, &now);
#else
  localtime_r(&now, &p);
#endif

  int now_int = p.tm_hour * 10000 + p.tm_min * 100 + p.tm_sec;

  return now_int;
}

class RRLogger {
public:
  static RRLogger* GetInstance(bool isConsole = true, const std::string& level = "info", const std::string& path = ".")
  {
    static RRLogger rrlogger(isConsole, level, path);
    return &rrlogger;
  }

  static void InitLog(bool isConsole, const char* level, const char* path)
  {
    RRLogger::GetInstance(isConsole, level ? std::string(level) : std::string("info"), path ? std::string(path) : std::string("."));
  }

  std::shared_ptr<spdlog::logger> GetLogger() const
  {
    return _logger;
  }

private:
  RRLogger(bool isConsole, const std::string& level, const std::string& path)
  {
    try {
      int date = NowDateToInt();
      int time = NowTimeToInt();
      const std::string logger_name = std::to_string(date) + "_" + std::to_string(time);

      if (isConsole) {
        _logger = spdlog::stdout_color_mt(logger_name);
      } else {
        // multi part log files, with every part 10M, max 100 files
        _logger = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>(
            logger_name, path + "/" + logger_name + ".log", 10 * 1024 * 1024, 100);
      }

      _logger->set_pattern(
          "[%^%L%$][%H:%M:%S.%e][T%t][%s:%#:%!] %v");  // [level][timestamp][thread_id][filename:linenumber:funcname] msg

      if (level == "debug") {
        _logger->set_level(spdlog::level::debug);
        _logger->flush_on(spdlog::level::debug);
      } else if (level == "info") {
        _logger->set_level(spdlog::level::info);
        _logger->flush_on(spdlog::level::info);
      } else if (level == "warn") {
        _logger->set_level(spdlog::level::warn);
        _logger->flush_on(spdlog::level::warn);
      } else if (level == "error") {
        _logger->set_level(spdlog::level::err);
        _logger->flush_on(spdlog::level::err);
      } else {
        _logger->set_level(spdlog::level::info);
        _logger->flush_on(spdlog::level::info);
      }
    } catch (const spdlog::spdlog_ex& ex) {
      std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
  }

  ~RRLogger()
  {
    spdlog::drop_all();  // must do this in Visual Studio
  }

  RRLogger(const RRLogger&) = delete;
  RRLogger& operator=(const RRLogger&) = delete;

private:
  std::shared_ptr<spdlog::logger> _logger;
};

// use embedded macro to support file and line number
#define RRLOG_DEBUG(...)                                                                           \
  SPDLOG_LOGGER_CALL(RRLogger::GetInstance()->GetLogger().get(), spdlog::level::debug, __VA_ARGS__)
#define RRLOG_INFO(...)                                                                            \
  SPDLOG_LOGGER_CALL(RRLogger::GetInstance()->GetLogger().get(), spdlog::level::info, __VA_ARGS__)
#define RRLOG_WARN(...)                                                                            \
  SPDLOG_LOGGER_CALL(RRLogger::GetInstance()->GetLogger().get(), spdlog::level::warn, __VA_ARGS__)
#define RRLOG_ERROR(...)                                                                           \
  SPDLOG_LOGGER_CALL(RRLogger::GetInstance()->GetLogger().get(), spdlog::level::err, __VA_ARGS__)
