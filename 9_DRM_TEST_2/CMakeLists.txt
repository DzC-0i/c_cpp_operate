cmake_minimum_required(VERSION 3.15)
project(DRM VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖包
# find_package(libdrm REQUIRED)   # 找不到
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
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


if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE -Ofast)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O3  -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3  -Wall")

# 添加项目头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${LIBDRM_INCLUDE_DIRS}    # 添加 libdrm 头文件路径
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
        )
    endforeach(src ${SOURCES_TEST})
endfunction()

function(drm_media_test)
    add_executable(${TARGET}_pic ${PROJECT_SOURCE_DIR}/src/test_pic/main.c ${PROJECT_SOURCE_DIR}/src/test_pic/drm-core.c)
    # 添加 libpng 头文件路径
    target_include_directories(${TARGET}_pic PRIVATE ${LIBPNG_INCLUDE_DIRS})
    # 链接库
    target_link_libraries(${TARGET}_pic
        ${LIBDRM_LIBRARIES}       # 链接 libdrm
        ${LIBPNG_LIBRARIES}       # 链接 libpng
    )

    add_executable(${TARGET}_opencv_pic ${PROJECT_SOURCE_DIR}/src/test_opencv/pic.cc ${PROJECT_SOURCE_DIR}/src/test_opencv/drm-core.c)
    # 添加 libpng 头文件路径
    target_include_directories(${TARGET}_opencv_pic PRIVATE ${OpenCV_INCLUDE_DIRS})
    # 链接库
    target_link_libraries(${TARGET}_opencv_pic
        ${LIBDRM_LIBRARIES}       # 链接 libdrm
        ${OpenCV_LIBRARIES}       # 链接 opencv
    )

    add_executable(${TARGET}_opencv_video ${PROJECT_SOURCE_DIR}/src/test_opencv/video.cc ${PROJECT_SOURCE_DIR}/src/test_opencv/drm-core.c)
    # 添加 libpng 头文件路径
    target_include_directories(${TARGET}_opencv_video PRIVATE ${OpenCV_INCLUDE_DIRS})
    # 链接库
    target_link_libraries(${TARGET}_opencv_video
        ${LIBDRM_LIBRARIES}       # 链接 libdrm
        ${OpenCV_LIBRARIES}       # 链接 opencv
        Threads::Threads
    )
endfunction()

drm_test()
drm_media_test()

add_executable(${TARGET}_driver ${PROJECT_SOURCE_DIR}/src/drm_driver/drm_driver.cc ${PROJECT_SOURCE_DIR}/src/drm_driver/drm-core.c)
# 添加 libpng 头文件路径
target_include_directories(${TARGET}_driver PRIVATE ${OpenCV_INCLUDE_DIRS})
# 链接库
target_link_libraries(${TARGET}_driver
    ${LIBDRM_LIBRARIES}       # 链接 libdrm
    ${OpenCV_LIBRARIES}       # 链接 opencv
    Threads::Threads
)
