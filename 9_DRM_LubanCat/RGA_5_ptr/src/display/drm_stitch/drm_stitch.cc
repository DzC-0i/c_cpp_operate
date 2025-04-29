#include "drm_stitch.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include <mutex>

// #include <yaml-cpp/yaml.h>
// #include <boost/filesystem.hpp>
#include <rga/im2d.hpp>
#include <rga/RgaApi.h>

#include "dma_alloc.hpp"
#include "drm-core.h"

class VideoStitch::Impl
{
private:
    struct new_frame_config_
    {
        int x_offset = 0;
        int y_offset = 0;
        int width = 1920;
        int height = 1080;
    } new_target_frame;

    // 显示参数
    uint32_t display_width_ = 1920;
    uint32_t display_height_ = 1080;
    bool is_videostitch_init = false;
    bool is_set_layout = false;
    bool is_set_resize = false;

    std::mutex config_mutex_; // 保护缩放配置

    // 视频流管理
    std::vector<cv::Rect> display_rects;

    // 操作图像
    // std::mutex frame_mutex_;
    // cv::Mat stitch_frame;

    DisplayType displaytype = DisplayType::Contain;

    int dst_dma_fd;
    uint32_t *dst_buf, dst_buf_size;
    rga_buffer_t dst_info;
    rga_buffer_handle_t dst_handle_;
    im_rect src_rect;

    // 布局计算
    void calculate_automatic_layout(int cell_layout_size);
    void calculate_custom_layout(int cell_layout_size, int custom_rows, int custom_cols);
    void calculate_grid_layout(int size, int rows, int cols);

public:
    // // 显示模式枚举
    // enum LayoutMode
    // {
    //     AUTOMATIC,  // 自动布局
    //     CUSTOM_GRID // 自定义行列
    // };
    // 显示类型枚举
    // enum DisplayType
    // {
    //     Contain, // 显示一：保持比例，最大尺寸适应屏幕，黑边填充
    //     Cover,   // 显示二：保持比例，填满屏幕，可能裁剪
    //     Stretch  // 显示三：拉伸变形，强制填满屏幕
    // };

    Impl(uint32_t display_width, uint32_t display_height, DisplayType type,
         LayoutMode layout_mode, int cell_layout_size, int custom_rows, int custom_cols);

    ~Impl();
    void set_displaytype(DisplayType type);
    void set_layout(LayoutMode mode, int cell_layout_size, int rows, int cols);
    void add_frames(size_t screen_place, cv::Mat &frame);
    void videostitch_init();
    void videostitch_deinit();
};

VideoStitch::Impl::Impl(uint32_t display_width, uint32_t display_height, DisplayType type,
                        LayoutMode layout_mode, int cell_layout_size, int custom_rows, int custom_cols)
    : display_width_(display_width), display_height_(display_height), displaytype(type)
{
    videostitch_init();
    set_layout(layout_mode, cell_layout_size, custom_rows, custom_cols);
}

VideoStitch::Impl::~Impl()
{
    videostitch_deinit();
}

void VideoStitch::Impl::videostitch_init()
{
    if (is_videostitch_init)
    {
        std::cerr << "The videos titch is already init!\n";
        return;
    }
    // 手动编写分辨率
    // display_width_ = 3840;
    // display_height_ = 2160;
    // display_width_ = 1920;
    // display_height_ = 1080;

    // 手动分配 4 通道对齐内存（XRGB8888）
    int channels = 4;
    uint32_t stride = (display_width_ * channels + 3) & ~3; // 4字节对齐
    dst_buf_size = stride * display_height_;

    // dma使用时CPU占用较普通的要高，但是RGA处理速度加快了一些，并且RGA核心占用下降
    /* Allocate dma_buf, return dma_fd and virtual address. */
    if (dma_buf_alloc(DMA_HEAP_UNCACHE_PATH, dst_buf_size, &dst_dma_fd, (void **)&dst_buf) < 0)
    {
        std::cerr << "Alloc src dma_heap buffer failed!\n";
        return;
    }

    memset(dst_buf, 0x00, dst_buf_size);

    // /* clear CPU cache */
    // dma_sync_cpu_to_device(dst_dma_fd);

    // 创建目标缓冲区（屏幕尺寸）,不需要图图片测试时不创建
    // stitch_frame = cv::Mat(display_height_, display_width_, CV_8UC4, dst_buf, stride);

    // stitch_frame = cv::Mat(display_height_, display_width_, CV_8UC4, cv::Scalar(0, 0, 0, 0));

    // 初始化DRM并传递DMA参数
    struct drm_device drm_buf = {
        .width = display_width_,
        .height = display_height_,
        .pitch = stride,
        .size = dst_buf_size,
        .vaddr = dst_buf,
        .dma_fd = dst_dma_fd};

    if (drm_init(&drm_buf) < 0)
    {
        std::cerr << "DRM initialization failed!\n";
        dma_buf_free(dst_buf_size, &dst_dma_fd, dst_buf);
        return;
    }

    // 初始化RGA硬件
    if (c_RkRgaInit() != 0)
    {
        std::cerr << "RGA init failed!\n";
        drm_exit();
        return;
    }

    // 创建RGA源缓冲区
    // dma内存对齐过，可能有不同，不可直接使用上面一个
    // dst_handle_ = importbuffer_fd(
    //     dst_dma_fd,
    //     display_width_, display_height_,
    //     RK_FORMAT_RGBA_8888);
    dst_handle_ = importbuffer_fd(dst_dma_fd, dst_buf_size);

    if (dst_handle_ == 0)
    {
        std::cerr << "Import dst_handle_ error!\n";
        c_RkRgaDeInit();
        drm_exit();
        return;
    }

    dst_info = wrapbuffer_handle(dst_handle_, display_width_, display_height_, RK_FORMAT_RGBA_8888);

    is_videostitch_init = true;
}

void VideoStitch::Impl::videostitch_deinit()
{
    if (!is_videostitch_init)
    {
        std::cerr << "The videos titch is not init!\n";
        return;
    }

    is_set_layout = false;
    is_set_resize = false;

    releasebuffer_handle(dst_handle_);

    // dma释放,drm里面有释放
    // dma_buf_free(dst_buf_size, &dst_dma_fd, dst_buf);

    display_rects.clear();

    c_RkRgaDeInit();
    drm_exit();
}

void VideoStitch::Impl::set_displaytype(DisplayType type)
{
    displaytype = type;
    is_set_resize = false;
}

void VideoStitch::Impl::set_layout(LayoutMode mode, int cell_layout_size, int rows, int cols)
{
    is_set_layout = false;
    is_set_resize = false;
    display_rects.clear();
    // stitch_frame.setTo(cv::Scalar(0, 0, 0));
    memset(dst_buf, 0, dst_buf_size);
    // std::lock_guard<std::mutex> lock(frame_mutex_);

    switch (mode)
    {
    case LayoutMode::AUTOMATIC:
        calculate_automatic_layout(cell_layout_size);
        break;
    case LayoutMode::CUSTOM_GRID:
        calculate_custom_layout(cell_layout_size, rows, cols);
        break;
    }
    is_set_layout = true;
}

void VideoStitch::Impl::calculate_automatic_layout(int cell_layout_size)
{
    const int num_streams = cell_layout_size;
    if (num_streams <= 0)
    {
        std::cerr << "Invalid number of layout\n";
        return;
    }

    float aspect_ratio = static_cast<float>(display_width_) / display_height_;
    bool is_landscape = (aspect_ratio >= 1.0f);

    int best_cols = 0;
    int best_rows = std::floor(std::sqrt(num_streams));
    int min_empty_cells = num_streams;

    for (int col = best_rows; col <= best_rows + 2; ++col)
    {
        int row = num_streams / col;
        if (num_streams % col)
        {
            row++;
        }
        int min_empty_cells_ = row * col - num_streams;
        if (row * col >= num_streams && min_empty_cells_ <= min_empty_cells && (std::abs(row - col) <= 1))
        {
            if (is_landscape)
            {
                best_rows = row;
                best_cols = col;
            }
            else
            {
                best_rows = col;
                best_cols = row;
            }
            min_empty_cells = min_empty_cells_;
        }
    }

    calculate_grid_layout(num_streams, best_rows, best_cols);
}

void VideoStitch::Impl::calculate_custom_layout(int cell_layout_size, int custom_rows, int custom_cols)
{
    if (custom_rows * custom_cols < cell_layout_size)
    {
        std::cerr << "Invalid custom layout\n";
        return;
    }

    calculate_grid_layout(cell_layout_size, custom_rows, custom_cols);
}

void VideoStitch::Impl::calculate_grid_layout(int size, int rows, int cols)
{
    // 计算单元格尺寸（考虑整除情况）
    const int cell_width = display_width_ / cols;
    const int cell_height = display_height_ / rows;

    // 处理可能的余数，将剩余像素分配到后面的单元格
    int width_remain = display_width_ % cols;
    int height_remain = display_height_ % rows;

    // std::cout << "rate: " << rows << "x" << cols << std::endl;

    for (int i = 0; i < size; ++i)
    {
        int row = i / cols;
        int col = i % cols;

        // 动态调整单元格尺寸以分配余数
        int curr_width = cell_width + (col < width_remain ? 1 : 0);
        int curr_height = cell_height + (row < height_remain ? 1 : 0);

        // 计算位置偏移
        int x = col * cell_width + std::min(col, width_remain);
        int y = row * cell_height + std::min(row, height_remain);

        // std::cout << "x: " << x
        //           << "\ty: " << y
        //           << "\twidth: " << curr_width
        //           << "\theight: " << curr_height
        //           << std::endl;

        display_rects.push_back(cv::Rect(x, y, curr_width, curr_height));
    }
}

void VideoStitch::Impl::add_frames(size_t screen_place, cv::Mat &frame)
{
    if (!is_set_layout)
    {
        std::cerr << "Layout is not set!" << std::endl;
        return;
    }
    // 边界检查
    if (screen_place >= display_rects.size())
    {
        std::cerr << "Invalid screen position: " << screen_place << std::endl;
        return;
    }

    // 获取目标区域
    const cv::Rect &target_rect = display_rects[screen_place];

    int ret;
    rga_buffer_handle_t src_handle_;
    rga_buffer_t src_info;
    im_rect dst_rect;

    // thread_local static cv::Mat resized_frame;

    // 双重检查锁定初始化缩放配置
    if (!is_set_resize)
    {
        std::lock_guard<std::mutex> config_lock(config_mutex_);
        if (!is_set_resize)
        {
            is_set_resize = true;

            switch (displaytype)
            {
            case DisplayType::Contain:
            {
                // 计算最佳缩放比例
                double width_ratio = static_cast<double>(target_rect.width) / frame.cols;
                double height_ratio = static_cast<double>(target_rect.height) / frame.rows;
                double resize_ratio = std::min(width_ratio, height_ratio);

                new_target_frame.x_offset = (target_rect.width - frame.cols * resize_ratio) / 2;
                new_target_frame.y_offset = (target_rect.height - frame.rows * resize_ratio) / 2;
                new_target_frame.width = frame.cols * resize_ratio;
                new_target_frame.height = frame.rows * resize_ratio;

                src_rect = {0, 0, frame.cols, frame.rows};

                std::cout << "In Contain:\n"
                          << "x: " << 0
                          << "\ty: " << 0
                          << "\twidth: " << frame.cols
                          << "\theight: " << frame.rows
                          << std::endl;
                break;
            }
            case DisplayType::Cover:
            {
                new_target_frame.x_offset = 0;
                new_target_frame.y_offset = 0;
                new_target_frame.width = target_rect.width;
                new_target_frame.height = target_rect.height;

                double target_ratio = static_cast<double>(target_rect.width) / target_rect.height;
                double src_ratio = static_cast<double>(frame.cols) / frame.rows;
                int crop_width, crop_height;
                if (src_ratio > target_ratio)
                {
                    crop_height = frame.rows;
                    crop_width = crop_height * target_ratio;
                }
                else
                {
                    crop_width = frame.cols;
                    crop_height = crop_width / target_ratio;
                }

                src_rect = {static_cast<int>((frame.cols - crop_width) / 2),
                            static_cast<int>((frame.rows - crop_height) / 2),
                            crop_width,
                            crop_height};

                std::cout << "In Cover:\n"
                          << "x: " << static_cast<int>((frame.cols - crop_width) / 2)
                          << "\ty: " << static_cast<int>((frame.rows - crop_height) / 2)
                          << "\twidth: " << crop_width
                          << "\theight: " << crop_height
                          << std::endl;
                break;
            }
            case DisplayType::Stretch:
            {
                new_target_frame.x_offset = 0;
                new_target_frame.y_offset = 0;
                new_target_frame.width = target_rect.width;
                new_target_frame.height = target_rect.height;

                src_rect = {0, 0, frame.cols, frame.rows};

                std::cout << "In Stretch:\n"
                          << "x: " << 0
                          << "\ty: " << 0
                          << "\twidth: " << frame.cols
                          << "\theight: " << frame.rows
                          << std::endl;
                break;
            }
            default:
            {
                std::cerr << "Unknown display type!" << std::endl;
                break;
            }
            }

            // 预分配缩放缓冲区
            // resized_frame.create(cv::Size(new_target_frame.width, new_target_frame.height), frame.type());
        }
    }

    // 创建RGA源缓冲区
    src_handle_ = importbuffer_virtualaddr(
        frame.data,
        frame.cols, frame.rows,
        RK_FORMAT_RGB_888);

    if (src_handle_ == 0)
    {
        std::cerr << "Importbuffer failed!" << std::endl;
        // goto release_buffer;
        return;
    }

    // 使用RAII确保句柄释放
    std::shared_ptr<void> src_handle_guard(nullptr, [&](void *)
                                           { if (src_handle_) releasebuffer_handle(src_handle_); });

    // 配置缩放参数
    src_info = wrapbuffer_handle(src_handle_, frame.cols, frame.rows, RK_FORMAT_RGB_888);

    dst_rect = {
        target_rect.x + new_target_frame.x_offset,
        target_rect.y + new_target_frame.y_offset,
        new_target_frame.width,
        new_target_frame.height};

    ret = imcheck(src_info, dst_info, src_rect, dst_rect, 1);
    if (ret != IM_STATUS_NOERROR)
    {
        std::cerr << "RGA check error: " << imStrError((IM_STATUS)ret) << std::endl;
        // goto release_buffer;
        return;
    }

    /* Configure the current thread to use only RGA3_core0 or RGA3_core1. */
    /* 提交到哪个核心处理 , 运行速度区别不大，CPU占用区别不大，GPU使用双核指定的占比单核百分比多*/
    // imconfig(IM_CONFIG_SCHEDULER_CORE, IM_SCHEDULER_RGA3_CORE0 | IM_SCHEDULER_RGA3_CORE1);
    // imconfig(IM_CONFIG_SCHEDULER_CORE, (screen_place % 2) ? IM_SCHEDULER_RGA3_CORE1 : IM_SCHEDULER_RGA3_CORE0);

    ret = improcess(src_info, dst_info, {},
                    src_rect, dst_rect, {},
                    IM_SYNC);

    if (ret != IM_STATUS_SUCCESS)
    {
        std::cerr << "RGA process error: " << imStrError((IM_STATUS)ret) << std::endl;
        // goto release_buffer;
        return;
    }

// release_buffer:
//     // 释放临时资源
//     if (src_handle_)
//         releasebuffer_handle(src_handle_);
//     // dma_buf_free(src_buf_size, &src_dma_fd, src_buf);

//     return;
}

// 外部类接口转发
VideoStitch::VideoStitch(uint32_t display_width, uint32_t display_height, DisplayType type,
                         LayoutMode layout_mode, int cell_layout_size, int custom_rows, int custom_cols)
    : impl(std::make_unique<Impl>(display_width, display_height, type,
                                  layout_mode, cell_layout_size, custom_rows, custom_cols)) {}
VideoStitch::~VideoStitch() = default;
void VideoStitch::set_displaytype(DisplayType type) { impl->set_displaytype(type); }
void VideoStitch::set_layout(LayoutMode mode, int cell_layout_size, int rows, int cols) { impl->set_layout(mode, cell_layout_size, rows, cols); }
void VideoStitch::add_frames(size_t screen_place, cv::Mat &frame) { impl->add_frames(screen_place, frame); }

void VideoStitch::videostitch_init() { impl->videostitch_init(); }
void VideoStitch::videostitch_deinit() { impl->videostitch_deinit(); }

#if ENABLE_TEST
// 测试接口
#include <random>
#include <thread>

// 定义全局智能指针
std::shared_ptr<VideoStitch> video;

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-1000, 1000);
auto rnd = std::bind(dist, mt);
void simulate_hard_computation()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rnd()));
}

void test_thread(size_t x)
{
    simulate_hard_computation();
    cv::VideoCapture vc("media/video/SampleVideo_1280x720_5mb.mp4");
    if (!vc.isOpened())
    {
        std::cerr << "failed to open the video: media/video/SampleVideo_1280x720_5mb.mp4\n";
        return;
    }

    double fps = vc.get(cv::CAP_PROP_FPS);
    // std::cout << "fps" << fps << std::endl;

    double target_frame_time = 1.0 / fps; // 目标帧间隔(秒)

    cv::Mat frame;

    while (true)
    {
        auto frame_start = std::chrono::steady_clock::now();
        vc >> frame; // 采集帧

        if (!frame.empty())
        {
            // 更新帧缓冲
            video->add_frames(x, frame);
        }
        else
        {
            break;
        }

        // 计算帧率控制
        auto frame_end = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(frame_end - frame_start).count();
        double sleep_time = target_frame_time - elapsed;

        if (sleep_time > 0)
        {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(sleep_time));
        }
        else
        {
            std::cerr << "\t\tIN thread Frame drop! Processing too slow: "
                      << elapsed * 1000 << "ms > "
                      << target_frame_time * 1000 << "ms" << std::endl;
        }
    }
    vc.release();
}

int main(int argc, char **argv)
{
    // if (argc < 2)
    // {
    //     std::cout << "Usage: " << argv[0] << "<image-file>\n";
    //     return -1;
    // }

    // 初始化指针
    video = std::make_shared<VideoStitch>(3840, 2160);
    // cv::namedWindow("pic", cv::WINDOW_AUTOSIZE);
    // cv::namedWindow("img", cv::WINDOW_AUTOSIZE);
    // cv::Mat frame = cv::imread(argv[1], 1);
    cv::Mat frame1 = cv::imread("media/picture/2024-07-11-13_04_21.png", 1);
    cv::Mat frame2 = cv::imread("media/picture/2024-07-11-13_04_22.jpg", 1);
    cv::Mat frame3 = cv::imread("media/picture/belle-nuit-testchart-1080p.png", 1);
    // cv::imshow("pic", frame);

    // video->set_layout(VideoStitch::LayoutMode::AUTOMATIC, 6);
    // video->set_displaytype(VideoStitch::DisplayType::Cover);
    auto now = std::chrono::high_resolution_clock::now();
    video->add_frames(0, frame1);
    video->add_frames(1, frame1);
    video->add_frames(2, frame1);
    video->add_frames(3, frame1);
    video->add_frames(4, frame1);
    video->add_frames(5, frame1);

    auto now2 = std::chrono::high_resolution_clock::now();
    std::cout << "运行时长：" << std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now).count() << " ms" << std::endl;

    // cv::imshow("img", test.read_mat());

    // cv::waitKey(0);

    // cv::Mat mat = test.read_mat();

    // test.drm_operate->load_frame(mat);

    getchar(); // 等待按键

    video->set_layout(VideoStitch::LayoutMode::AUTOMATIC, 4);
    video->set_displaytype(VideoStitch::DisplayType::Stretch);
    now = std::chrono::high_resolution_clock::now();
    video->add_frames(0, frame2);
    video->add_frames(1, frame2);
    video->add_frames(2, frame2);
    now2 = std::chrono::high_resolution_clock::now();
    std::cout << "运行时长：" << std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now).count() << " ms" << std::endl;

    getchar(); // 等待按键

    video->set_layout(VideoStitch::LayoutMode::AUTOMATIC, 16);
    video->set_displaytype(VideoStitch::DisplayType::Cover);

    std::thread thr[16];

    for (size_t i = 0; i < 16; i++)
    {
        thr[i] = std::thread(test_thread, i);
    }

    for (size_t i = 0; i < 16; i++)
    {
        thr[i].join();
    }

    getchar(); // 等待按键

    // cv::destroyAllWindows();

    return 0;
}

#endif
