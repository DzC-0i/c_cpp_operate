// drm_driver.hh
#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include "drm_driver.hh"

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
    void set_layout(LayoutMode mode, int fps, int layout_size, int rows = 0, int cols = 0);
    void add_frames(int screen_place, cv::Mat frame);

private:
    struct VideoCell
    {
        cv::Rect display_rect;
        cv::Mat scaled_frame;
    };

    // 布局计算
    void calculate_layout();
    void calculate_automatic_layout();
    void calculate_custom_layout();
    void update_display();

    // 显示参数
    int display_width_;
    int display_height_;
    LayoutMode layout_mode_ = AUTOMATIC;
    int custom_rows_ = 0;
    int custom_cols_ = 0;

    // 视频流管理
    std::vector<VideoCell> streams_;
    cv::Mat composite_frame_;

    // 双缓冲
    cv::Mat front_buffer_;
    cv::Mat back_buffer_;
    std::mutex frame_mutex_;
    std::atomic<bool> update_ready_{false};
};
