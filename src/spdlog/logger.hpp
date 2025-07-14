#ifndef CLOGGER_HPP
#define CLOGGER_HPP

#include <fmt/core.h>
#include <spdlog/common.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cstddef>
#include <filesystem>
#include <source_location>
#include <string_view>

class CLogger final {
 public:
  /**
   * 获取 Logger 单例实例
   */
  static CLogger& GetInstance() {
    static CLogger instance;
    return instance;
  }

  /**
   * 初始化默认 logger，默认名称为 "default"
   * @param log_dir 日志目录
   * @param file_name 日志文件名（不含路径）
   * @param file_size 单个日志文件最大大小
   * @param file_count 最大日志文件数量
   */
  void InitDefaultLogger(std::string_view log_dir = "./logs/", std::string_view file_name = "app",
                         size_t file_size = 10485760, size_t file_count = 3) {
    CreateLogger("default", log_dir, file_name, file_size, file_count);
  }

  /**
   * 创建或获取 logger，logger 名称与日志文件名绑定
   * @param name logger 名称
   * @param log_dir 日志目录
   * @param file_name 日志文件名（不含路径）
   * @param file_size 单个日志文件最大大小
   * @param file_count 最大日志文件数量
   * @return 新建或已有的 logger 指针
   */
  std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name,
                                               std::string_view log_dir = "./logs/",
                                               std::string_view file_name = "default",
                                               size_t file_size = 10485760, size_t file_count = 3) {
    std::lock_guard lock(mutex_);
    auto it = loggers_.find(name);
    if (it != loggers_.end()) {
      return it->second;
    }

    std::vector<spdlog::sink_ptr> sinks;
    // 控制台 sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sinks.push_back(console_sink);

    // 文件 sink，文件名与 logger 名称无关，使用传入的 file_name
    std::string file_path = std::string(log_dir) + "/" + std::string(file_name) + ".log";
    auto file_sink =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file_path, file_size, file_count);
    sinks.push_back(file_sink);

    auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());

#ifdef DEBUG
    logger->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%l] [%n] %v %$");
    logger->set_level(spdlog::level::debug);
#else
    logger->set_pattern("%^[%Y-%m-%d %H:%M:%S] [thread %t] [%l] [%n] %v %$");
    logger->set_level(spdlog::level::info);
#endif

    spdlog::register_logger(logger);
    loggers_[name] = logger;
    logger_files_[name] = std::string(file_name);
    return logger;
  }

  /**
   * 设置指定 logger 的日志级别，若 logger 为空则设置所有 logger
   * @param level 日志级别
   * @param logger 指定 logger，默认 nullptr 表示全部
   */
  void SetLevel(spdlog::level::level_enum level, std::shared_ptr<spdlog::logger> logger = nullptr) {
    std::lock_guard lock(mutex_);
    if (logger) {
      logger->set_level(level);
    } else {
      for (auto& [_, lg] : loggers_) {
        lg->set_level(level);
      }
    }
  }

  /**
   * 设置指定 logger 的日志格式，若 logger 为空则设置所有 logger
   * @param pattern spdlog 格式字符串
   * @param logger 指定 logger，默认 nullptr 表示全部
   */
  void SetPattern(const std::string& pattern, std::shared_ptr<spdlog::logger> logger = nullptr) {
    std::lock_guard lock(mutex_);
    if (logger) {
      logger->set_pattern(pattern);
    } else {
      for (auto& [_, lg] : loggers_) {
        lg->set_pattern(pattern);
      }
    }
  }

  /**
   * 删除指定名称的 logger，返回是否成功
   * @param name logger 名称
   */
  bool RemoveLogger(const std::string& name) {
    std::lock_guard lock(mutex_);
    auto it = loggers_.find(name);
    if (it != loggers_.end()) {
      spdlog::drop(name);
      loggers_.erase(it);
      logger_files_.erase(name);
      return true;
    }
    return false;
  }

  /**
   * 判断指定名称的 logger 是否存在
   * @param name logger 名称
   */
  bool HasLogger(const std::string& name) {
    std::lock_guard lock(mutex_);
    return loggers_.find(name) != loggers_.end();
  }

  /**
   * 获取指定名称的 logger，若不存在返回 nullptr
   * @param name logger 名称
   */
  std::shared_ptr<spdlog::logger> GetLogger(const std::string& name) {
    std::lock_guard lock(mutex_);
    auto it = loggers_.find(name);
    if (it != loggers_.end()) {
      return it->second;
    }
    return nullptr;
  }

  /**
   * 获取 logger 对应的日志文件名，若不存在返回空字符串
   * @param name logger 名称
   */
  std::string GetLoggerFileName(const std::string& name) {
    std::lock_guard lock(mutex_);
    auto it = logger_files_.find(name);
    if (it != logger_files_.end()) {
      return it->second;
    }
    return "";
  }

  // 调试版本重载
  template <typename... Args>
  void Log(const std::string& name, spdlog::level::level_enum level,
           const std::source_location& location, fmt::format_string<Args...> msg, Args&&... args) {
    std::string prefix = " [" +
                         std::string(std::filesystem::path(location.file_name()).filename()) + ":" +
                         std::to_string(location.line()) + "] [" + location.function_name() + "] ";
    std::string formatted_msg = prefix + fmt::format(msg, std::forward<Args>(args)...);

    LogImpl(name, level, formatted_msg);
  }

  // 发布版本重载
  template <typename... Args>
  void Log(const std::string& name, spdlog::level::level_enum level,
           fmt::format_string<Args...> msg, Args&&... args) {
    std::string formatted_msg = fmt::format(msg, std::forward<Args>(args)...);
    LogImpl(name, level, formatted_msg);
  }

  CLogger(const CLogger&) = delete;
  CLogger& operator=(const CLogger&) = delete;
  CLogger(CLogger&&) = delete;

 private:
  /**
   * 记录指定 logger 的日志，日志中带 logger 名称
   * @tparam Args 格式化参数模板
   * @param name logger 名称
   * @param level 日志级别
   * @param msg 格式化字符串
   * @param args 格式化参数
   * @param location 调用点信息，默认自动获取
   */
  template <typename... Args>
  void LogImpl(const std::string& name, spdlog::level::level_enum level,
               const std::string& message) {
    std::lock_guard lock(mutex_);
    auto it = loggers_.find(name);
    if (it == loggers_.end()) {
      return;
    }
    it->second->log(level, message);
  }

  CLogger() = default;
  ~CLogger() = default;

  std::mutex mutex_;
  std::unordered_map<std::string, std::string> logger_files_;  // logger 名称 -> 文件名映射
  std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers_;
};

#ifdef DEBUG
#define LOG_TRACE(...)                                                                         \
  CLogger::GetInstance().Log("default", spdlog::level::trace, std::source_location::current(), \
                             __VA_ARGS__)
#define LOG_DEBUG(...)                                                                         \
  CLogger::GetInstance().Log("default", spdlog::level::debug, std::source_location::current(), \
                             __VA_ARGS__)
#define LOG_INFO(...)                                                                         \
  CLogger::GetInstance().Log("default", spdlog::level::info, std::source_location::current(), \
                             __VA_ARGS__)
#define LOG_WARN(...)                                                                         \
  CLogger::GetInstance().Log("default", spdlog::level::warn, std::source_location::current(), \
                             __VA_ARGS__)
#define LOG_ERROR(...)                                                                       \
  CLogger::GetInstance().Log("default", spdlog::level::err, std::source_location::current(), \
                             __VA_ARGS__)
#define LOG_CRITICAL(...)                                                                         \
  CLogger::GetInstance().Log("default", spdlog::level::critical, std::source_location::current(), \
                             __VA_ARGS__)
#else

#define LOG_TRACE_L(name, ...) CLogger::GetInstance().Log(name, spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG_L(name, ...) CLogger::GetInstance().Log(name, spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO_L(name, ...) CLogger::GetInstance().Log(name, spdlog::level::info, __VA_ARGS__)
#define LOG_WARN_L(name, ...) CLogger::GetInstance().Log(name, spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR_L(name, ...) CLogger::GetInstance().Log(name, spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL_L(name, ...) \
  CLogger::GetInstance().Log(name, spdlog::level::critical(), __VA_ARGS__)
#endif

#endif