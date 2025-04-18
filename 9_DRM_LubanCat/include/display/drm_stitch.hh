// drm_driver.hh
#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "drm_driver.hh"
#include <thread>
#include <mutex>

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
    void add_frames(int screen_place, cv::Mat &frame);

    // 测试接口
    cv::Mat read_mat();

private:
    // 布局计算
    void calculate_automatic_layout(int cell_layout_size);
    void calculate_custom_layout(int cell_layout_size, int custom_rows, int custom_cols);
    void calculate_grid_layout(int size, int rows, int cols);
    void update_display();

    std::shared_ptr<VideoPipeline> drm_operate;

    std::thread update_thread;
    bool update_running = false;

    // 显示参数
    int display_width_ = 1920;
    int display_height_ = 1080;
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
    cv::Mat stitch_frame;
};
