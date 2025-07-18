

#include <catch2/catch_message.hpp>
#define CATCH_CONFIG_MAIN  // Catch2 主函数定义
#include <spdlog/sinks/ostream_sink.h>
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <filesystem>
#include <string>
#include "logger.hpp"

using namespace Logging;

struct TestLoggerEnv {
  std::ostringstream oss;
  std::shared_ptr<spdlog::sinks::ostream_sink_mt> oss_sink;
  std::shared_ptr<spdlog::logger> logger;
  std::string name;

  TestLoggerEnv(const std::string& base_name = "default") {
    // 生成唯一的logger名称
    static int counter = 0;
    name = base_name + "_" + std::to_string(++counter);

    // 创建sink和logger
    oss_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);

    // 先清理可能存在的同名logger
    spdlog::drop(name);

    // 创建新的logger并配置
    logger = LoggerManager::instance().GetOrCreate(name);
    logger->sinks().clear();  // 清除默认sink
    logger->sinks().push_back(oss_sink);
  }

  ~TestLoggerEnv() {
    // 清理logger
    spdlog::drop(name);
  }
};

TEST_CASE("default logger 控制台输出", "[logger][console]") {
  const std::string test_msg = "default logger 基本控制台输出";
  TestLoggerEnv env("default");

  LOG_INFO(Logger(env.name), test_msg);

  std::string current_file = std::filesystem::path(__FILE__).filename().string();
  INFO("日志名称：" << env.name);
  INFO("日志输出：" << env.oss.str());
  INFO("当前文件: " << current_file);
  REQUIRE_THAT(env.name, Catch::Matchers::StartsWith("default_"));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring(test_msg));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring(env.name));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring(current_file));
}

TEST_CASE("命名logger控制台输出", "[logger][console]") {
  const std::string test_msg = "test logger 控制台输出";
  TestLoggerEnv env("test");

  LOG_INFO(Logger(env.name), test_msg);

  std::string current_file = std::filesystem::path(__FILE__).filename().string();
  INFO("日志名称：" << env.name);
  INFO("日志输出：" << env.oss.str());
  INFO("当前文件: " << current_file);
  REQUIRE_THAT(env.name, Catch::Matchers::StartsWith("test_"));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring(test_msg));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring(env.name));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring(current_file));
}

TEST_CASE("logger 格式化输出", "[logger][format]") {
  const std::string test_msg = "格式化输出：{} + {} = {}";
  int a = 2, b = 3;
  int sum = a + b;
  TestLoggerEnv env("format");

  LOG_INFO(Logger(env.name), test_msg, a, b, sum);
  std::string current_file = std::filesystem::path(__FILE__).filename().string();
  INFO("日志输出：" << env.oss.str());
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring("格式化输出"));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring("2 + 3 = 5"));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring(current_file));
}

TEST_CASE("不同日志级别输出", "[logger][level]") {
  TestLoggerEnv env("level");

  //LOG_TRACE(Logger(env.name), "trace级别日志");
  //LOG_DEBUG(Logger(env.name), "debug级别日志");
  LOG_WARN(Logger(env.name), "warn级别日志");
  LOG_ERROR(Logger(env.name), "error级别日志");
  LOG_CRITICAL(Logger(env.name), "critical级别日志");

  INFO("日志输出：" << env.oss.str());

  //REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring("trace级别日志"));
  //REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring("debug级别日志"));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring("warn级别日志"));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring("error级别日志"));
  REQUIRE_THAT(env.oss.str(), Catch::Matchers::ContainsSubstring("critical级别日志"));
}

TEST_CASE("文件日志输出", "[logger][file]") {
  std::string logger_name = "test_file";
  Logging::Config config;
  config.console = false;
  config.log_dir = ".";
  config.file = true;
  ConfigureLogger(logger_name, config);

  LOG_INFO(Logger(logger_name), "测试文件日志输出");
  auto current = std::filesystem::current_path();
  current.append(logger_name + ".log");
  INFO("当前文件名: " << current);

  REQUIRE(std::filesystem::exists(current) == 1);
  std::filesystem::remove(logger_name);
}

TEST_CASE("异步日志配置", "[logger][async]") {
  Logging::Config config;
  config.async = true;
  config.queue_size = 1024;
  config.thread_count = 2;

  ConfigureLogger("async_logger", config);
  LOG_INFO(Logger("async_logger"), "测试异步日志输出");
  REQUIRE_NOTHROW(Logger("async_logger"));
}