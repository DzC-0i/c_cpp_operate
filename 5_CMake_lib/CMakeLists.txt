# CMake最低版本
cmake_minimum_required(VERSION 3.10.0)
project(mqtt-onenet-tests)

# 设置版本信息
set(PROJECT_MAJOR_VERSION 1) # 架版本信息
set(PROJECT_MINOR_VERSION 2) # 主版本信息
set(PROJECT_PATCH_VERSION 0) # 次版本信息
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_VERSION})

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_ROOT_DIR}/build/bin)   # 设置可执行文件的输出目录
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})              # 确保bin目录存在

set(CMAKE_C_FLAGS "-Wall -lpthread -fmessage-length=0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -DNDEBUG")

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 -lpthread -Wall -fmessage-length=0")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
endif(CMAKE_COMPILER_IS_GNUCXX)

#设置编译的版本为release版本。如果要编译debug版本，直接写debug即可
set(CMAKE_BUILD_TYPE "release")

# 遍历所有子目录并添加到包含路径中
get_filename_component(HEADER_ROOT_DIR ${PROJECT_ROOT_DIR}/include ABSOLUTE)
file(GLOB_RECURSE SUB_DIRS LIST_DIRECTORIES true ${HEADER_ROOT_DIR}/*)

foreach(SUB_DIR ${SUB_DIRS})
    if(IS_DIRECTORY ${SUB_DIR})
        include_directories(${SUB_DIR})
	    # message( [STATUS|WARNING|AUTHOR_WARNING|FATAL_ERROR|SEND_ERROR] "message to display" ...)
	    # (无) = 重要消息；
	    # STATUS = 非重要消息；
	    # WARNING = CMake 警告, 会继续执行；
	    # AUTHOR_WARNING = CMake 警告 (dev), 会继续执行；
	    # SEND_ERROR = CMake 错误, 继续执行，但是会跳过生成的步骤；
	    # FATAL_ERROR = CMake 错误, 终止所有处理过程；
        message(STATUS "header found at: ${SUB_DIR}")
    endif()
endforeach()

# 递归查找 src 目录及其子目录下的所有 .cpp  .c 文件
file(GLOB_RECURSE SRC_FILES ${PROJECT_ROOT_DIR}/src/*.cpp ${PROJECT_ROOT_DIR}/src/*.c)

if (NOT SRC_FILES)
    message(FATAL_ERROR "src not found!")
else()
    foreach(src ${SRC_FILES})
        message(STATUS "src found at: ${src}")
    endforeach()
endif()

add_executable(${PROJECT_NAME} ${SRC_FILES})

# 使用库文件的时候，前面的lib不需要
find_library(MY_LIB mqttclient
    PATHS ${PROJECT_ROOT_DIR}/lib   # 指定搜索路径
    PATH_SUFFIXES lib               # 可选：指定子目录（如 lib）
)

if (NOT MY_LIB)
    message(FATAL_ERROR "mylib not found!")
else()
    message(STATUS "mylib found at: ${MY_LIB}")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${MY_LIB})
