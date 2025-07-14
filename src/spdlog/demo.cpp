#include "logger.hpp"

// 模拟业务逻辑类
class Database {
 public:
  bool connect(const std::string& url) {
    LOG_INFO("尝试连接数据库: {}", url);

    // 模拟连接过程
    if (url.empty()) {
      LOG_ERROR("数据库URL为空");
      return false;
    }

    // 模拟耗时操作
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    LOG_INFO("数据库连接成功");
    return true;
  }

  void query(const std::string& sql) {
    LOG_DEBUG("执行SQL查询: {}", sql);

    // 模拟查询过程
    if (sql.find("SELECT") == std::string::npos) {
      LOG_WARN("非查询语句: {}", sql);
    }

    // 模拟结果返回
    LOG_INFO("查询完成，返回100条记录");
  }
};

void workerThread(int id) {
  LOG_INFO("工作线程 {} 已启动", id);

  Database db;
  if (!db.connect("localhost:3306/mydb")) {
    LOG_CRITICAL("工作线程 {} 数据库连接失败，退出", id);
    return;
  }

  for (int i = 0; i < 3; ++i) {
    db.query("SELECT * FROM users LIMIT 10");
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  LOG_INFO("工作线程 {} 已完成任务", id);
}

int main() {
  CLogger::GetInstance().InitDefaultLogger();
#ifdef DEBUG
  CLogger::GetInstance().SetLevel(spdlog::level::debug);
#else
  CLogger::GetInstance().SetLevel(spdlog::level::info);
#endif
  LOG_INFO("应用程序启动，版本 v1.0.0");
  LOG_DEBUG("调试信息");
  LOG_WARN("警告信息");
  std::string url = "localhost:3306/mydb";
  LOG_INFO("尝试连接数据库: {}", url);
  LOG_ERROR("错误信息");
  std::vector<std::thread> threads;
  for (int i = 0; i < 2; ++i) {
    threads.emplace_back(workerThread, i);
  }
  // 主线程继续执行其他任务
  LOG_INFO("主线程继续执行其他任务...");

  // 等待所有线程完成
  for (auto& t : threads) {
    t.join();
  }

  LOG_INFO("所有工作线程已完成，应用程序正常退出");
  return 0;
}
