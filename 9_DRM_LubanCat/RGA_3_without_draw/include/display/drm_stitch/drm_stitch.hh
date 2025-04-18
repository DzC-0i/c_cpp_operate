// drm_driver.hh
#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <im2d.hpp>
#include <RgaApi.h>

#include "dma_alloc.hpp"
// #include "drm_alloc.h"
#include "drm-core.h"

class VideoWall
{
public:
    // 显示模式枚举
    enum LayoutMode
    {
        AUTOMATIC,  // 自动布局
        CUSTOM_GRID // 自定义行列
    };

    VideoWall();
    ~VideoWall();

    // 核心接口
    void set_layout(LayoutMode mode, int cell_layout_size, int fps = 25, int rows = 0, int cols = 0);
    // 注：screen_place从0开始数
    void add_frames(size_t screen_place, cv::Mat &frame);

    // // 测试接口
    // cv::Mat read_mat();

private:
    // 显示参数
    uint32_t display_width_ = 1920;
    uint32_t display_height_ = 1080;
    int frame_fps = 25;
    bool is_set_resize = false;

    struct new_frame_config_
    {
        int x_offset = 0;
        int y_offset = 0;
        int width = 1920;
        int height = 1080;
    } new_frame;

    std::mutex config_mutex_; // 保护缩放配置

    // 视频流管理
    std::vector<cv::Rect> display_rects;

    // 操作图像
    // std::mutex frame_mutex_;
    // cv::Mat stitch_frame;

    int dst_dma_fd;
    uint32_t *dst_buf, dst_buf_size;
    rga_buffer_t dst_info;
    rga_buffer_handle_t dst_handle_;

    // 布局计算
    void calculate_automatic_layout(int cell_layout_size);
    void calculate_custom_layout(int cell_layout_size, int custom_rows, int custom_cols);
    void calculate_grid_layout(int size, int rows, int cols);
};
