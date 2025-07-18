find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(Threads REQUIRED)

# add_executable(spdlog-demo demo.cpp logger.hpp)
# target_link_libraries(spdlog-demo PRIVATE spdlog::spdlog)
if(Catch2_FOUND AND BUILD_TESTS)
  message(STATUS "Building tests with Catch2")

  add_executable(clogger_test
    test/logger_test.cpp
  )

  target_link_libraries(clogger_test
    PRIVATE
    Catch2::Catch2WithMain
    Threads::Threads
    spdlog::spdlog_header_only
  )
  target_include_directories(clogger_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

  # 自动发现测试用例
  include(Catch)

  # 自动发现测试用例（使用 Catch2 的 CMake 模块）
  include(Catch)
  catch_discover_tests(clogger_test)
endif()

add_executable(logger_main main.cpp logger.hpp)
target_link_libraries(logger_main spdlog::spdlog_header_only)
