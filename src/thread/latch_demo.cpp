#include <format>
#include <iostream>
#include <latch>
#include <thread>
#include <vector>
using namespace std::chrono_literals;
// latch ：只能减少计数，直到0，解除阻塞
std::latch latch{10};

void work(int id) {
  std::cout << std::format("thread {} wait\n", id);
  latch.arrive_and_wait();  //减少并等待，直到0，解除阻塞
  std::cout << std::format("thread {} end\n", id);
}

int main() {
  std::vector<std::jthread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back(work, i);
  }
}
