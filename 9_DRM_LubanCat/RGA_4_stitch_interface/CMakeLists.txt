cmake_minimum_required(VERSION 3.15)
project(DRM VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置库架构
set(LIB_ARCH "aarch64")
set(DEVICE_NAME "RK3588")

# 查找依赖包
# find_package(libdrm REQUIRED)     # 找不到
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
# find_package(LIBRGA REQUIRED)       #找不到
# find_package(YAML-CPP REQUIRED)     #部分找不到,include找不到,需要再包含时写入部分位置
# find_package(Boost REQUIRED COMPONENTS filesystem system)

pkg_check_modules(LIBDRM REQUIRED libdrm)
pkg_check_modules(LIBPNG REQUIRED libpng)
pkg_check_modules(LIBRGA REQUIRED librga)

# 设置变量
set(TARGET ${CMAKE_PROJECT_NAME})
# set(Boost_USE_STATIC_LIBS ON)  # 强制使用静态库

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
    ${LIBDRM_INCLUDE_DIRS}    # 添加 libdrm 头文件路径
)

function(drm_test)
    # 查找测试源文件
    file(GLOB SOURCES_TEST ${PROJECT_SOURCE_DIR}/src/test/*.c)

    foreach(src ${SOURCES_TEST})
        # 获取当前文件所在的目录名称(文件名)
        get_filename_component(current_dir ${src} NAME_WE)

        # 打印当前目录名称
        message(STATUS "Current name: ${current_dir}")

        add_executable(test_${current_dir} ${src})

        # 添加头文件路径
        target_include_directories(test_${current_dir} PRIVATE
            ${CMAKE_SOURCE_DIR}/include/display
            )

        # 链接库
        target_link_libraries(test_${current_dir}
            ${LIBDRM_LIBRARIES}       # 链接 libdrm
        )
    endforeach(src ${SOURCES_TEST})
endfunction()

function(drm_media_test)
    add_executable(${TARGET}_pic ${PROJECT_SOURCE_DIR}/src/test/test_pic/main.c ${PROJECT_SOURCE_DIR}/src/test/test_pic/drm-core.c)
    # 添加 libpng 头文件路径
    target_include_directories(${TARGET}_pic PRIVATE ${LIBPNG_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/display)
    # 链接库
    target_link_libraries(${TARGET}_pic
        ${LIBDRM_LIBRARIES}       # 链接 libdrm
        ${LIBPNG_LIBRARIES}       # 链接 libpng
    )

    add_executable(${TARGET}_opencv_pic ${PROJECT_SOURCE_DIR}/src/test/test_opencv/pic.cc ${PROJECT_SOURCE_DIR}/src/test/test_opencv/drm-core.c)
    # 添加 libpng 头文件路径
    target_include_directories(${TARGET}_opencv_pic PRIVATE ${OpenCV_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/display)
    # 链接库
    target_link_libraries(${TARGET}_opencv_pic
        ${LIBDRM_LIBRARIES}       # 链接 libdrm
        ${OpenCV_LIBRARIES}       # 链接 opencv
    )

    add_executable(${TARGET}_opencv_video ${PROJECT_SOURCE_DIR}/src/test/test_opencv/video.cc ${PROJECT_SOURCE_DIR}/src/test/test_opencv/drm-core.c)
    # 添加 libpng 头文件路径
    target_include_directories(${TARGET}_opencv_video PRIVATE ${OpenCV_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/display)
    # 链接库
    target_link_libraries(${TARGET}_opencv_video
        ${LIBDRM_LIBRARIES}       # 链接 libdrm
        ${OpenCV_LIBRARIES}       # 链接 opencv
        Threads::Threads
    )
endfunction()

# drm_test()
# drm_media_test()

add_executable(${TARGET}_driver ${PROJECT_SOURCE_DIR}/src/display/drm_driver/drm_driver.cc ${PROJECT_SOURCE_DIR}/src/display/drm_driver/drm-core.c)
# 添加 libpng 头文件路径
target_include_directories(${TARGET}_driver PRIVATE ${OpenCV_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include/display/drm_driver)
# 链接库
target_link_libraries(${TARGET}_driver
    ${LIBDRM_LIBRARIES}       # 链接 libdrm
    ${OpenCV_LIBRARIES}       # 链接 opencv
    Threads::Threads
)


add_executable(${TARGET}_stitch
    ${PROJECT_SOURCE_DIR}/src/display/drm_stitch/drm_stitch.cc
    ${PROJECT_SOURCE_DIR}/src/display/drm_stitch/drm-core.c)

# 添加库路径
link_directories(
    # RGA 在这边
    /usr/lib/aarch64-linux-gnu/
    # 或第三方库路径
    # ${CMAKE_SOURCE_DIR}/third_party/rga/lib
)

# 添加头文件路径
target_include_directories(${TARGET}_stitch PRIVATE
                                ${OpenCV_INCLUDE_DIRS}
                                # ${LIBRGA_INCLUDE_DIRS}    # 找不到
                                /usr/include/rga
                                ${PROJECT_SOURCE_DIR}/include/3rdparty/allocator/dma
                                ${PROJECT_SOURCE_DIR}/include/display/drm_stitch
                            )
# 链接库
target_link_libraries(${TARGET}_stitch
    ${LIBDRM_LIBRARIES}       # 链接 libdrm
    ${OpenCV_LIBRARIES}       # 链接 opencv
    ${LIBRGA_LIBRARIES}
    # ${YAML_CPP_LIBRARIES}
    # Boost::filesystem
    # Boost::system
    # dl
    Threads::Threads
)
