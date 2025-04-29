// drm_stitch.h
#ifndef _DRM_STITCH_H_
#define _DRM_STITCH_H_

#include <opencv2/opencv.hpp>
#include <memory>

#ifdef __cplusplus
extern "C"
{
#endif

#define ENABLE_TEST 1 // 1=启用，0=禁用

    class VideoStitch
    {
    public:
        // 布局模式枚举
        enum class LayoutMode
        {
            AUTOMATIC,  // 自动布局
            CUSTOM_GRID // 自定义行列
        };
        // 显示类型枚举
        enum class DisplayType
        {
            Contain, // 显示一：保持比例，最大尺寸适应屏幕，黑边填充
            Cover,   // 显示二：保持比例，填满屏幕，可能裁剪
            Stretch  // 显示三：拉伸变形，强制填满屏幕
        };

        // VideoStitch();
        explicit VideoStitch(
            uint32_t display_width = 1920, // 默认分辨率 1080P
            uint32_t display_height = 1080,
            DisplayType display_type = DisplayType::Contain, // 默认显示模式
            LayoutMode layout_mode = LayoutMode::AUTOMATIC,  // 默认自动布局
            int cell_layout_size = 6,                        // 默认 6 宫格
            int custom_rows = 0,                             // 自定义行数（仅 CUSTOM_GRID 生效）
            int custom_cols = 0                              // 自定义列数（仅 CUSTOM_GRID 生效）
        );
        ~VideoStitch();

        // 核心接口
        void set_layout(LayoutMode mode, int cell_layout_size, int rows = 0, int cols = 0);
        // 注：screen_place从0开始数
        void add_frames(size_t screen_place, cv::Mat &frame);

        // 备用接口
        // 可以不主动调用
        void set_displaytype(DisplayType type = DisplayType::Contain);
        // 初始化时不主动调用，但是可以手动调用，用于更改分辨率，分辨率等参数写入在config当中
        void videostitch_init();
        void videostitch_deinit();

    private:
        class Impl;
        std::unique_ptr<Impl> impl;

        VideoStitch(const VideoStitch &) = delete;
        VideoStitch &operator=(const VideoStitch &) = delete;
    };

#if defined(__cplusplus)
}
#endif

#endif
