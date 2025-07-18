#include <barrier>
#include <format>
#include <iostream>
#include <thread>
#include <vector>
using namespace std::chrono_literals;

// 直到0，解除阻塞且重置计数
// 一轮计数结束，调用后面的函数且重置计数
std::barrier barrier{10, [n = 1]() mutable {
                       std::cout << std::format("\t第{}轮结束\n", n++);
                     }};

void work(int start, int end) {
  for (int i = start; i <= end; ++i) {
    std::cout << std::format("{} ", i);
    barrier.arrive_and_wait();  //减少并等待，直到0，解除阻塞
  }
}

int main() {
  std::vector<std::jthread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back(work, i * 10 + 1, (i + 1) * 10);
  }
  // 11,21,31,...
  // 12,22,32,...
}
