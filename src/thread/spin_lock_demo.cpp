
#include <atomic>
#include <format>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class SpinLock {
 public:
  void lock() {
    while (locked.test_and_set(std::memory_order_acquire)) {
      // 自旋等待，直到成功获取锁
    }
  }

  void unlock() { locked.clear(std::memory_order_release); }

 private:
  std::atomic_flag locked = ATOMIC_FLAG_INIT;
};

SpinLock spinlock;
int g = 0;

void task() {
  for (int i = 0; i < 5; i++) {
    std::lock_guard<SpinLock> lock(spinlock);
    g++;
    std::cout << std::format("task:g={}\n", g);
  }
}

int main() {
  std::vector<std::jthread> threads;
  for (int i = 0; i < 2; ++i) {
    threads.emplace_back(task);
  }
  return 0;
}
