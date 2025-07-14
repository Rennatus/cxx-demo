#include "spdlog/spdlog.h"

int main() {
  // 设置默认日志级别（默认为 info）
  spdlog::set_level(spdlog::level::debug);

  // 基本日志记录
  spdlog::trace("这是一条 TRACE 级别的日志");
  spdlog::debug("这是一条 DEBUG 级别的日志");
  spdlog::info("这是一条 INFO 级别的日志");
  spdlog::warn("这是一条 WARNING 级别的日志");
  spdlog::error("这是一条 ERROR 级别的日志");
  spdlog::critical("这是一条 CRITICAL 级别的日志");

  // 格式化日志
  int value = 42;
  spdlog::info("格式化示例：value = {}, 十六进制 = 0x{:X}", value, value);

  return 0;
}
