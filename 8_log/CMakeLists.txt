cmake_minimum_required(VERSION 3.10.0)
project(Log VERSION 0.1.0)

# 设置变量
set(TARGET ${CMAKE_PROJECT_NAME})

# 编译选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -O3 -pthread)

# 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set(LIB_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})              # 确保bin目录存在
file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})                              # 确保lib目录存在

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(${TARGET}_custom ${CMAKE_SOURCE_DIR}/src/custom.cpp)
add_executable(${TARGET}_spdlog ${CMAKE_SOURCE_DIR}/src/spdlog.cpp)
add_executable(${TARGET}_spdlog_file ${CMAKE_SOURCE_DIR}/src/spdlog_file.cpp)
add_executable(${TARGET}_spdlog_rollback ${CMAKE_SOURCE_DIR}/src/spdlog_rollback.cpp)

# # 固定的目标指定包含头文件路径，可以多条
# target_include_directories(${TARGET}_spdlog PRIVATE ${CMAKE_SOURCE_DIR}/include)
# target_include_directories(${TARGET}_spdlog_file PRIVATE ${CMAKE_SOURCE_DIR}/include)
# target_include_directories(${TARGET}_spdlog_rollback PRIVATE ${CMAKE_SOURCE_DIR}/include)
