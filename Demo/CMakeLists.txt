cmake_minimum_required(VERSION 3.14)

project(demo)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

option(DEBUG "Debug Mode" OFF)
option(ENABLE_COVERAGE "Enable Code Coverage" OFF)
option(ARCH "Architecture" OFF)

if(ARCH)
  if(ARCH STREQUAL "aarch64")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    set(CMAKE_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++")
  else()
    message(FATAL_ERROR "Unknown Architecture: ${ARCH}")
  endif()
endif()

add_compile_options(-std=c++17 -Wall)

message("System: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message("Compiler is Clang, enable clang-tidy")
  set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
endif()

if(ENABLE_COVERAGE)
  message("code coverage")
  set(DEBUG ON)
endif()

if(NOT DEBUG)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
  add_compile_options(-O3)
else()
  message("debug mode")
  SET(CMAKE_BUILD_TYPE "Debug")
  add_compile_options(-O0 -g)
endif()

if(NOT ARCH)
  # when using CPM missing dependencies are downloaded at configure time. To avoid redundant downloads, 
  # it's highly recommended to set a CPM.cmake cache directory, e.g.: export CPM_SOURCE_CACHE=$HOME/.cache/CPM
  include(cmake/CPM.cmake)
  # tools such as AddressSanitizer and include-what-you-use, "tools.cmake" depends on "CPM.cmake"
  include(cmake/tools.cmake)
endif()

aux_source_directory(src ALL_SRCS)
add_library(${PROJECT_NAME} SHARED ${ALL_SRCS})

target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PUBLIC ../Common)
target_include_directories(${PROJECT_NAME} PUBLIC ../Common/third_party)

add_executable(test_demo test_demo.cpp)
target_link_libraries(test_demo PRIVATE pthread ${PROJECT_NAME})

if(NOT ARCH)
  CPMAddPackage(https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz)

  target_include_directories(test_demo PRIVATE ../Common/third_party/crashpad/include)
  target_include_directories(test_demo PRIVATE ../Common/third_party/crashpad/include/third_party/mini_chromium/mini_chromium)
  target_link_directories(test_demo PRIVATE ../Common/third_party/crashpad/lib/client
                                            ../Common/third_party/crashpad/lib/util
                                            ../Common/third_party/crashpad/lib/third_party/mini_chromium/mini_chromium/base)
  target_link_libraries(test_demo PRIVATE common client util base)

  add_executable(demo_unittest demo_unittest.cpp)
  target_link_libraries(demo_unittest PRIVATE gtest ${PROJECT_NAME})

  # 在CMake中配置gcov对C++项目代码进行分析: https://blog.csdn.net/yjj546542806/article/details/111568642
  if(ENABLE_COVERAGE)
    target_compile_options(${PROJECT_NAME} PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
    target_link_options(${PROJECT_NAME} PUBLIC -fprofile-arcs -ftest-coverage)

    if(CMAKE_COMPILER_IS_GNUCXX)
      include(cmake/CodeCoverage.cmake)
      setup_target_for_coverage_lcov(NAME ${PROJECT_NAME}_coverage EXECUTABLE test_demo DEPENDENCIES ${PROJECT_NAME})
    endif()
  endif()
endif()
