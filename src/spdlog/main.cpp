#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include "logger.hpp"
using namespace std;

int main() {
  auto current = std::filesystem::current_path();
  std::string a = "./logs";

  auto b = std::filesystem::relative(a).append("app.log");
  auto c = current / b;
  std::cout << c << std::endl;
  std::cout << b << std::endl;
};
