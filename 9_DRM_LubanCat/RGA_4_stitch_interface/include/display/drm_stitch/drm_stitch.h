// drm_stitch.h
#ifndef _DRM_STITCH_H_
#define _DRM_STITCH_H_

#include <opencv2/opencv.hpp>

#ifdef __cplusplus
extern "C"
{
#endif

#define ENABLE_TEST 1 // 1=启用，0=禁用

    class VideoStitch
    {
    public:
        // 布局模式枚举
        enum LayoutMode
        {
            AUTOMATIC,  // 自动布局
            CUSTOM_GRID // 自定义行列
        };
        // 显示类型枚举
        enum DisplayType
        {
            Contain, // 显示一：保持比例，最大尺寸适应屏幕，黑边填充
            Cover,   // 显示二：保持比例，填满屏幕，可能裁剪
            Stretch  // 显示三：拉伸变形，强制填满屏幕
        };

        VideoStitch();
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
        Impl *impl;
        VideoStitch(const VideoStitch &) = delete;
        VideoStitch &operator=(const VideoStitch &) = delete;
    };

#if defined(__cplusplus)
}
#endif

#endif
