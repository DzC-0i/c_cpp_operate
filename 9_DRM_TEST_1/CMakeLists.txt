cmake_minimum_required(VERSION 3.10)
project(DRM VERSION 0.1.0)

# set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖包
# find_package(libdrm REQUIRED)   # 找不到
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)
pkg_check_modules(LIBPNG REQUIRED libpng)

# 设置变量
set(TARGET ${CMAKE_PROJECT_NAME})

# 设置输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
set(LIB_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})              # 确保bin目录存在
file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})                              # 确保lib目录存在

# 添加项目头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${LIBDRM_INCLUDE_DIRS}    # 添加 libdrm 头文件路径
    ${LIBPNG_INCLUDE_DIRS}    # 添加 libpng 头文件路径
)

function(drm_test)
    # 查找测试源文件
    file(GLOB_RECURSE SOURCES_TEST ${PROJECT_SOURCE_DIR}/src/test/*.c)

    foreach(src ${SOURCES_TEST})
        # 获取当前文件所在的目录名称(文件名)
        get_filename_component(current_dir ${src} NAME_WE)

        # 打印当前目录名称
        message(STATUS "Current name: ${current_dir}")

        add_executable(test_${current_dir} ${src})

        # 链接库
        target_link_libraries(test_${current_dir}
            ${LIBDRM_LIBRARIES}       # 链接 libdrm
            ${LIBPNG_LIBRARIES}       # 链接 libpng
        )
    endforeach(src ${SOURCES_TEST})
endfunction()

drm_test()


add_executable(${PROJECT_NAME}_pic ${PROJECT_SOURCE_DIR}/src/test_pic/main.c ${PROJECT_SOURCE_DIR}/src/test_pic/drm-core.c)

# 链接库
target_link_libraries(${PROJECT_NAME}_pic
    ${LIBDRM_LIBRARIES}       # 链接 libdrm
    ${LIBPNG_LIBRARIES}       # 链接 libpng
)
