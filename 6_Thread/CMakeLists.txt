cmake_minimum_required(VERSION 3.10.0)
# 可以直接写在这边，也可以单独写
# project(Thread VERSION 0.1.0)
project(Thread)
# 设置版本信息
set(PROJECT_MAJOR_VERSION 0) # 架版本信息
set(PROJECT_MINOR_VERSION 2) # 主版本信息
set(PROJECT_PATCH_VERSION 0) # 次版本信息
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_VERSION})

# 设置变量
set(TARGET ${CMAKE_PROJECT_NAME})

# 编译选项
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -O3 -pthread)

# 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set(LIB_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})              # 确保bin目录存在
file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})                              # 确保lib目录存在

# include_directories(${CMAKE_SOURCE_DIR}/include)

# 用于查找单个文件夹下的所有 .cpp 文件
# file(GLOB SRC_LIST ${CMAKE_SOURCE_DIR}/src/*.cpp)
# 用于递归查找文件夹下的所有 .c 文件
# file(GLOB_RECURSE SRC_LIST ${CMAKE_SOURCE_DIR}/src/*.c)

# # 学习cmake，正式不用这么写
# foreach(src ${SRC_LIST})
#     # message( [STATUS|WARNING|AUTHOR_WARNING|FATAL_ERROR|SEND_ERROR] "message to display" ...)
#     # (无) = 重要消息；
#     # STATUS = 非重要消息；
#     # WARNING = CMake 警告, 会继续执行；
#     # AUTHOR_WARNING = CMake 警告 (dev), 会继续执行；
#     # SEND_ERROR = CMake 错误, 继续执行，但是会跳过生成的步骤；
#     # FATAL_ERROR = CMake 错误, 终止所有处理过程；
#     # message(STATUS "The executable source in: ${src}")
#     message("\nThe executable source in: ${src}\n")

#     # 使用 get_filename_component 获取目录名称
#     get_filename_component(dir_name ${src} DIRECTORY)
#     # 打印目录名称
#     message(STATUS "Directory name: ${dir_name}")

#     # 获取当前文件所在的目录名称(文件名),参数NAME带后缀名，NAME_WE不带后缀名
#     get_filename_component(current_dir ${src} NAME)

#     # 打印当前目录名称
#     message(STATUS "Current name: ${current_dir}")

#     add_executable(${TARGET} ${src})
# endforeach(src ${SRC_LIST})

add_executable(${TARGET}_create ${CMAKE_SOURCE_DIR}/src/thread_create.cpp)
add_executable(${TARGET}_message ${CMAKE_SOURCE_DIR}/src/thread_message.cpp)
add_executable(${TARGET}_pool ${CMAKE_SOURCE_DIR}/src/thread_pool.cpp)
add_executable(${TARGET}_queue ${CMAKE_SOURCE_DIR}/src/thread_queue.cpp)

# 固定的目标指定包含头文件路径，可以多条
target_include_directories(${TARGET}_pool PRIVATE ${CMAKE_SOURCE_DIR}/include)
