cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DEBUG) # 定义 DEBUG 宏
  add_compile_definitions(_GLIBCXX_DEBUG) # 启用 libstdc++ 的调试模式
else()
  add_compile_definitions(NDEBUG) # 定义 NDEBUG 以禁用 assert()
  add_compile_options(-O3) # 添加优化选项
endif()

project(cxx-demo)
add_subdirectory(src/spdlog)
add_subdirectory(src/thread)