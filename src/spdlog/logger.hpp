#pragma once

#include <filesystem>
#include <source_location>

#include <spdlog/async.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <source_location>
#include <string>
#if __cplusplus < 202002L
#error "C++20 required"
#endif

namespace Logging {
struct Config {
  bool console = true;
  bool file = false;
  std::string log_dir = "./logs";
  size_t file_size = 1024 * 1024 * 5;
  size_t file_count = 3;
  bool async = false;
  size_t queue_size = 8192;
  size_t thread_count = 1;
};

class LoggerManager {
 public:
  static LoggerManager& instance() {
    static LoggerManager manager;
    return manager;
  }

  std::shared_ptr<spdlog::logger> GetOrCreate(const std::string& name = "default",
                                              const Config& config = {}) {
    // 检查日志器是否已存在
    if (auto logger = get_logger(name)) {
      return logger;
    }
    return create_logger(name, config);
  }

 private:
  mutable std::mutex mutex_;

  LoggerManager() {
    // 创建默认日志器
    create_logger("default");
  }

  ~LoggerManager() { shutdown(); }

  LoggerManager(const LoggerManager&) = delete;
  LoggerManager& operator=(const LoggerManager&) = delete;

  void shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    spdlog::shutdown();
  }

  std::shared_ptr<spdlog::logger> get_logger(const std::string& name) {
    std::unique_lock<std::mutex> lock(mutex_);
    return spdlog::get(name);
  }

  std::shared_ptr<spdlog::logger> create_logger(const std::string& name,
                                                const Config& config = {}) {
    std::unique_lock<std::mutex> lock(mutex_);
    try {
      std::vector<spdlog::sink_ptr> sinks;
      if (config.console) {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sinks.push_back(console_sink);
      }

      if (config.file) {
        std::string current_dir = std::filesystem::current_path();

        auto path = (std::filesystem::relative(config.log_dir).append(name + ".log"));
        auto file_path = current_dir / path;
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            file_path, config.file_size, config.file_count);
        sinks.push_back(file_sink);
      }
      std::shared_ptr<spdlog::logger> logger;
      if (config.async) {
        // 异步日志配置
        spdlog::init_thread_pool(config.queue_size, config.thread_count);
        logger = std::make_shared<spdlog::async_logger>(name, sinks.begin(), sinks.end(),
                                                        spdlog::thread_pool(),
                                                        spdlog::async_overflow_policy::block);
      } else {
        // 同步日志
        logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
      }
      // 设置日志格式
      logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%s:%#] [thread:%t] %v");
      spdlog::register_logger(logger);
      return logger;
    } catch (const spdlog::spdlog_ex& ex) {
      if (auto default_logger = spdlog::get("default")) {
        default_logger->error("Failed to initialize logger '{}': {}", name, ex.what());
      }
      return nullptr;
    }
  }
};

class LoggerBuilder {

 public:
  LoggerBuilder(const std::string& name) : name_(name) {}

  template <typename... Args>
  void trace_with_loc(const std::source_location& loc, const std::string_view& msg,
                      Args&&... args) const {
    log(spdlog::level::trace, loc, msg, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void debug_with_loc(const std::source_location& loc, const std::string_view& msg,
                      Args&&... args) const {
    log(spdlog::level::debug, loc, msg, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void info_with_loc(const std::source_location& loc, const std::string_view& msg,
                     Args&&... args) const {
    log(spdlog::level::info, loc, msg, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void warn_with_loc(const std::source_location& loc, const std::string_view& msg,
                     Args&&... args) const {
    log(spdlog::level::warn, loc, msg, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error_with_loc(const std::source_location& loc, const std::string_view& msg,
                      Args&&... args) const {
    log(spdlog::level::err, loc, msg, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void critical_with_loc(const std::source_location& loc, const std::string_view& msg,
                         Args&&... args) const {
    log(spdlog::level::critical, loc, msg, std::forward<Args>(args)...);
  }

 private:
  std::string name_;

  // 获取或创建日志器
  std::shared_ptr<spdlog::logger> get_or_create() const {
    return LoggerManager::instance().GetOrCreate(name_);
  }

  // log 核心实现，通过变参模板实现格式化和参数转发
  template <typename... Args>
  void log(spdlog::level::level_enum level, const std::source_location& loc,
           const std::string_view& msg, Args&&... args) const {
    auto logger = get_or_create();
    if (logger && logger->should_log(level)) {
      spdlog::source_loc source{loc.file_name(), static_cast<int>(loc.line()), loc.function_name()};
      logger->log(source, level, fmt::runtime(msg), std::forward<Args>(args)...);
    } else if (name_ != "default") {
      if (auto default_logger = LoggerManager::instance().GetOrCreate("default")) {
        default_logger->error("Failed to log to '{}': logger initialization failed", name_);
      }
    }
  }
};

// 全局配置函数
inline void ConfigureLogger(const std::string& name = "default", const Config& config = {}) {
  LoggerManager::instance().GetOrCreate(name, config);
}

// 全局日志工厂函数 - 核心接口
inline LoggerBuilder Logger(const std::string& name = "default") {
  return LoggerBuilder(name);
}
};  // namespace Logging

// 日志宏，自动捕获调用点
#define LOG_TRACE(logger, ...) (logger).trace_with_loc(std::source_location::current(), __VA_ARGS__)
#define LOG_DEBUG(logger, ...) (logger).debug_with_loc(std::source_location::current(), __VA_ARGS__)
#define LOG_INFO(logger, ...) (logger).info_with_loc(std::source_location::current(), __VA_ARGS__)
#define LOG_WARN(logger, ...) (logger).warn_with_loc(std::source_location::current(), __VA_ARGS__)
#define LOG_ERROR(logger, ...) (logger).error_with_loc(std::source_location::current(), __VA_ARGS__)
#define LOG_CRITICAL(logger, ...) \
  (logger).critical_with_loc(std::source_location::current(), __VA_ARGS__)