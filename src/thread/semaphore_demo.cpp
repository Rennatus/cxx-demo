
#include <format>
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>

using namespace std::chrono_literals;
// 信号量：能增加和减少，计数为 0 则阻塞
std::counting_semaphore<5> semaphore{5};

void handle_request(int request_id) {
  semaphore.acquire();  //减少计数
  std::cout << std::format("request_id : {}获取信号量\n", request_id);
  std::this_thread::sleep_for(3s);
  semaphore.release();  //增加计数
}

int main() {
  std::vector<std::jthread> threads;
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back(handle_request, i);
  }
  return 0;
}
