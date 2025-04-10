# CMake最低版本
cmake_minimum_required(VERSION 3.10.0)
project(mqtt-lib)

# 设置版本信息
set(PROJECT_MAJOR_VERSION 1) # 架版本信息
set(PROJECT_MINOR_VERSION 2) # 主版本信息
set(PROJECT_PATCH_VERSION 0) # 次版本信息
set(PROJECT_VERSION ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_VERSION})

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_ROOT_DIR}/build/bin)   # 设置可执行文件的输出目录

set(LIB_OUTPUT_DIR ${PROJECT_ROOT_DIR}/build/lib)                   # 定义统一的lib输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})               # 静态库输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})               # 动态库输出目录

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})              # 确保bin目录存在
file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})                              # 确保lib目录存在

#设置交叉编译的环境
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

#指定交叉编译的gcc  g++。如果没有配置到环境变量，此处写绝对路径
# set(CMAKE_C_COMPILER "arm-linux-gnueabi-gcc")
# set(CMAKE_CXX_COMPILER "arm-linux-gnueabi-g++")
set(CMAKE_C_COMPILER "/home/dingzc/linux/tools/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "/home/dingzc/linux/tools/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++")

############## 以下几条可以不设置(实测可以)，官方文档上步骤 ############
#指定交叉编译环境的目录（包含目标系统的库和头文件）
set(CMAKE_FIND_ROOT_PATH "/home/dingzc/linux/tools/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf")
## 不在目标系统中查找可执行文件(交叉编译)下查找工具程序。(编译时利用的是宿主的工具)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#只在指定目录(交叉编译)下查找库文件
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#只在指定目录(交叉编译)下查找头文件
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#只在指定的目录(交叉编译)下查找依赖包
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
###################################################################

#设置C++ 编译的参数(根据需要自己设置)
# GPT编写的
# set(CMAKE_C_FLAGS "-march=armv7-a -mfpu=neon -mfloat-abi=hard")
# 设置与 CMAKE_C_FLAGS 相同
# set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
# 博主编写的
# set(CMAKE_C_FLAGS  "-std=c99 -Wall -O0 -g3 -Wall -fmessage-length=0")
# set(CMAKE_CXX_FLAGS "-std=c++11 -DTEST -pthread -Wall -O0 -g3 -Wall  -fmessage-length=0 -fPIC")
# -Wall       :  编译时显示警告信息
# -O          ： 选择编译器优化的级别，-O0 不优化  -O1 -O2 -O3 不同程度的优化
# -g          :  生成调试信息,debug会用到。调试信息的级别。-g3
#			    尽可能的生成gdb的可以使用的调试信息。默认为-g2。信息越多，编译出的程序越大。
# -std=c++11  ： 编译所按照C++的标准
# -fmessage-length=0 ：输出信息会根据控制台的宽度自动换行，这样就能看全输出信息了
# -fPIC       ： 作用于编译阶段，告诉编译器产生与位置无关代码
#			    (Position-Independent Code)。加载到任意位置都能执行
# -DTEST	 ： 设置TEST的宏定义，另外一种设置宏定义的方式可以直接使用 add_definitions(-D TEST) 添加

set(CMAKE_C_FLAGS "-Wall -lpthread -fmessage-length=0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -DNDEBUG")

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 -lpthread -Wall -fmessage-length=0")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
endif(CMAKE_COMPILER_IS_GNUCXX)

#设置编译的版本为debug版本。如果要编译release版本，直接写release即可
set(CMAKE_BUILD_TYPE "debug")

# 不使用add_subdirectory()就可以不加载子模块的cmake
# 这里不加载子模块,都在外层实现功能
# set(SUBDIRS  "common" "mqtt" "platform" "network" "mqttclient")
# foreach(subdir ${SUBDIRS})
#     add_subdirectory(${PROJECT_ROOT_DIR}/${subdir})
# endforeach()

# 程序所引用的头文件库目录。
include_directories(
    ${PROJECT_ROOT_DIR}/common
    ${PROJECT_ROOT_DIR}/common/log
    ${PROJECT_ROOT_DIR}/config
    ${PROJECT_ROOT_DIR}/mqtt
    ${PROJECT_ROOT_DIR}/mqttclient
    ${PROJECT_ROOT_DIR}/network
    ${PROJECT_ROOT_DIR}/network/mbedtls/include
    ${PROJECT_ROOT_DIR}/network/mbedtls/include/mbedtls
    ${PROJECT_ROOT_DIR}/network/mbedtls/wrapper
    ${PROJECT_ROOT_DIR}/platform/linux
)

# # 查找要需要编译的源文件。
# # file函数将递归查文件夹及子文件夹下的所有.cpp 和 .c文件放到SRC中
# # list函数排除build下面的.cpp和.c文件(在make过程中,会在bulid下面产生测试的cpp和c文件，避免冲突)
# # 这样我们将所需要编译的源文件放到SRC中

# # 查找 src 目录下的所有 .cpp 文件
# file(GLOB SRC_FILES "src/*.cpp")
# # 递归查找 src 目录及其子目录下的所有 .cpp 文件
# file(GLOB_RECURSE SRC_FILES "src/*.cpp")

# file(GLOB_RECURSE SRC *.cpp *.c)
# file(GLOB_RECURSE SRC_EXPECT build/*.cpp build/*.c)
# list(REMOVE_ITEM SRC ${SRC_EXPECT})

# # 查找 src 目录下的所有源文件,多次调用
# aux_source_directory(src SRC_FILES)

file(GLOB SRC_FILES
    ${PROJECT_ROOT_DIR}/common/*.c
    ${PROJECT_ROOT_DIR}/common/log/*.c
    ${PROJECT_ROOT_DIR}/common/log/arch/linux/*.c
    ${PROJECT_ROOT_DIR}/network/mbedtls/library/*.c
    ${PROJECT_ROOT_DIR}/network/mbedtls/wrapper/*.c
    ${PROJECT_ROOT_DIR}/mqtt/*.c
    ${PROJECT_ROOT_DIR}/mqttclient/*.c
    ${PROJECT_ROOT_DIR}/network/*.c
    ${PROJECT_ROOT_DIR}/platform/linux/*.c
)

##################### 生成共享库 ########################
# 编译生成共享库
add_library(${PROJECT_NAME} SHARED  ${SRC_FILES})
# ${PROJECT_NAME} : 为库的名字
# SHARED ：表示生成的是动态库 .so 。如果要编译成静态库 .a 替换成 STATIC 参数
# ${SRC_FILES} ：需要编译的源文件

### 注意： 如果不是生成可执行文件，则不需要进行后续的连接动态库的操作。
###       在生成可执行可执行文件时，一块进行连接
###########################################################


#################### 生成可执行文件#####################
# # 设置生成可执型文件的名字
# set(SRC_NAME "test")
# # 编译生成可执型文件
# add_executable("${SRC_NAME}" ${SRC})
# # ${SRC_NAME} : 生成可执型文件
# # ${SRC} ：需要编译的源文件

# # 将可执行文件连接动态库，连接动态库的方式有很多，可以自行百度。
# target_link_libraries("${SRC_NAME}"
#   	xxx.so			# 例如要连接xxx.so动态库
#     )
###注意, 连接的多个动态库库如果有相互依赖关系，注意顺序，被依赖的库，
### 一定要放到前面。否则编译通过，放到设备上无法运行。
### 如果顺序有问题，实测会报BUSERROR或者一些其他的错误，很难定位
#############################################################
