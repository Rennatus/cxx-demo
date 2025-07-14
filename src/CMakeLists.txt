find_package(spdlog REQUIRED)
add_executable(spdlog-demo spdlog_demo.cpp)
target_link_libraries(spdlog-demo PRIVATE spdlog::spdlog)
