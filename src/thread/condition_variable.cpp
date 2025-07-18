#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
using namespace std::chrono_literals;

struct Channel {
  void get_data() {
    auto tid = std::this_thread::get_id();
    std::unique_lock loc(mutex_);
    std::cout << "thread [" << tid << "] waiting data ...\n";
    // 防止虚假唤醒，设定条件
    cond_.wait(loc, [this] { return !share_data.empty(); });
    if (share_data.empty()) {
      // 虚假唤醒
      std::cout << "thread [" << tid << "] get data failed!" << std::endl;
    } else {
      std::cout << "thread [" << tid << "] get data success : " << share_data << std::endl;
    }
    share_data.clear();
  }

  void set_data() {
    static int id = 1;
    char data[80];
    snprintf(data, sizeof(data), "data %d", id);
    {
      std::unique_lock lock(mutex_);
      share_data = data;
      std::cout << "thread [" << std::this_thread::get_id() << "] set data : " << share_data
                << std::endl;
      id++;
    }
    cond_.notify_one();
    std::this_thread::sleep_for(1ms);
  }

 private:
  std::mutex mutex_;
  std::condition_variable cond_;
  std::string share_data;
};

int main() {
  Channel channel;
  std::thread write_thread(&Channel::set_data, &channel);
  std::thread read_thread(&Channel::get_data, &channel);

  read_thread.join();
  write_thread.join();
}
