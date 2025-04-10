# 只是实现部分的，不需要全部都加

# CMake最低版本
cmake_minimum_required(VERSION 3.10.0)
# 项目名称
project(3_cmakelist VERSION 0.1.0 LANGUAGES C CXX)
# 添加编译选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -O2)

# 头文件路径,为当前CMakeLists.txt中的项目设定头文件搜索路径设置
# ${PROJECT_BINARY_DIR} 是构建目录（通常是 build/），${PROJECT_SOURCE_DIR} 是源码目录
# cpp文件 include时有选择路径的，所以之包含 include
# include_directories(
#     ${PROJECT_SOURCE_DIR}/include
#     ${PROJECT_SOURCE_DIR}/include/nlohmann
# )
# include_directories(${PROJECT_BINARY_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/include)
# 源文件(需要一个一个加,不能多个目录一起)
aux_source_directory(${PROJECT_BINARY_DIR}/../src SRC_LIST)

# 设置可执行文件生成位置(旧) VERSION 2.x
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# 设置可执行文件生成位置(新) VERSION 3.x
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# 编译可执行文件
# add_executable(${PROJECT_NAME} ${SRC_LIST})
#
# 编译 params_ini.cpp 为可执行文件
add_executable(params_ini ${PROJECT_SOURCE_DIR}/src/params_ini.cpp)
# 编译 params.cpp 为可执行文件
add_executable(params ${PROJECT_SOURCE_DIR}/src/params.cpp)

# 在此测试当中不一定有意义
# 生成动态库和静态库

# 指定编译成库的文件
# 不需要指定也可以用之前的 SRC_LIST，看情况
set(LIB_SOURCES
    ${PROJECT_BINARY_DIR}/../src/params_ini.cpp
    ${PROJECT_BINARY_DIR}/../src/params.cpp
)
# 设置库文件的输出目录
set(LIB_OUTPUT_DIR ${CMAKE_BINARY_DIR}/lib)  # 将库文件输出到 build/lib 目录
file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})       # 确保目录存在
# 设置库文件的输出路径（属性）
# set_target_properties(MyLibrary PROPERTIES
#     ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}  # 静态库的输出目录
#     LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}  # 动态库的输出目录
# )
# 静态库的输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
# 动态库的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
# 静态库STATIC,生成.a文件
# 动态库SHARED,生成.so文件
add_library(MyLibrary STATIC ${LIB_SOURCES})
# 设置生成的库名称（属性）,生成的库名称前面自动加lib，要和使用库的时候前面的lib可以不写
set_target_properties(MyLibrary PROPERTIES OUTPUT_NAME "tset")

# 查找库文件, 可以有多个查找路径
find_library(FUNC_LIB tset ${PROJECT_BINARY_DIR}/lib)
# 编译可执行文件
add_executable(main ${PROJECT_SOURCE_DIR}/src/main.cpp)
# 固定的目标指定包含头文件路径，可以多条
# target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/include)
# 把库链接到可执行文件
# target_link_libraries(main ${FUNC_LIB})
