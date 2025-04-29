// drm_driver.hh
#pragma once
#include <opencv2/opencv.hpp>

class VideoPipeline
{
public:
    VideoPipeline();
    ~VideoPipeline();

    // 唯一可调用接口
    void load_frame(cv::Mat &frame);

private:
    class Impl;
    Impl *impl;

    VideoPipeline(const VideoPipeline &) = delete;
    VideoPipeline &operator=(const VideoPipeline &) = delete;
};
