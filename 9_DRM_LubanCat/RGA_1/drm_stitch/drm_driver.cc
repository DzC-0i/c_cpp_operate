#include "drm-core.h"
#include "drm_driver.hh"
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>

using namespace cv;

class FrameDoubleBuffer
{
private:
    cv::Mat front_buf;
    cv::Mat back_buf;
    std::mutex swap_mtx;
    std::atomic<bool> updated{false};

public:
    // 写入新帧
    void write(const cv::Mat &new_frame)
    {
        std::lock_guard<std::mutex> lock(swap_mtx);
        new_frame.copyTo(back_buf);
        updated.store(true, std::memory_order_release);
    }

    // 读取当前帧
    cv::Mat read()
    {
        if (updated.load(std::memory_order_acquire))
        {
            std::lock_guard<std::mutex> lock(swap_mtx);
            if (updated)
            { // 二次检查避免竞态条件
                // front_buf = back_buf.clone(); // 或使用 swap 优化
                std::swap(front_buf, back_buf);
                updated.store(false, std::memory_order_release);
            }
        }
        return front_buf;
    }
};

struct StreamMetadata
{
    std::atomic<bool> version{false};
    uint32_t width = 1920;
    uint32_t height = 1080;
    uint32_t channels = 3;
    double fps = 30.0;

    // 更新
    void update(int w, int h, int c, double f)
    {
        width = w;
        height = h;
        channels = c;
        fps = f;
        version = !version;
    }
};

class VideoPipeline::Impl
{
private:
    FrameDoubleBuffer frame_buffer;
    StreamMetadata metadata;
    std::atomic<bool> running{false};
    std::thread show_thread;
    bool is_set_frame_size = false;

    void show_img();

    // 停止处理
    void stop();

public:
    Impl();
    ~Impl();

    void load_frame(cv::Mat &frame);
};

VideoPipeline::Impl::Impl()
{
    int ret;
    // 初始化DRM
    ret = drm_init();

    if (ret < 0)
    {
        fprintf(stderr, "DRM initialization failed\n");
        return;
    }

    running.store(true);

    show_thread = std::thread(&Impl::show_img, this);
}

VideoPipeline::Impl::~Impl()
{
    stop();
}

void VideoPipeline::Impl::load_frame(cv::Mat &frame)
{
    if (!is_set_frame_size)
    {
        metadata.update(frame.cols, frame.rows, frame.channels(), 25);
        is_set_frame_size = true;
    }

    // if (!running)
    //     running.store(true);

    // static int frame_count = 0;
    // static auto last_time = std::chrono::steady_clock::now();

    // 更新帧缓冲
    frame_buffer.write(frame);

    // // 每25帧更新
    // if (++frame_count % 25 == 0)
    // {
    //     // auto now = std::chrono::steady_clock::now();
    //     // double elapsed = std::chrono::duration<double>(now - last_time).count();
    //     // double fps = 25 / elapsed;
    //     // metadata.update(frame.cols, frame.rows, frame.channels(), fps);
    //     metadata.update(frame.cols, frame.rows, frame.channels(), 25);
    //     // last_time = now;
    //     frame_count = 0;
    // }
}

void VideoPipeline::Impl::show_img()
{
    int ret;
    bool last_ver = false;

    // 计算显示位置
    int x = buf.width / 2 - metadata.width / 2;
    int y = buf.height / 2 - metadata.height / 2;

    // 边界安全检查
    auto calculate_bounds = [&]()
    {
        return std::make_tuple(
            std::max(0, x),
            std::max(0, y),
            std::min(buf.width, x + metadata.width),
            std::min(buf.height, y + metadata.height));
    };
    auto [start_x, start_y, end_x, end_y] = calculate_bounds();

    uint32_t last_x, last_y, last_width, last_height;

    // std::cout << "\nResolution: " << metadata.width << "x" << metadata.height
    //           << "\nDisplay Area: [" << start_x << "," << start_y << "] - ["
    //           << end_x << "," << end_y << "]\n";

    // auto last_frame_time = std::chrono::steady_clock::now();
    double target_frame_time = 1.0 / metadata.fps; // 目标帧间隔(秒)

    // printf("Show img load OK!\n");
    double elapsed = 0.0;
    double sleep_time = 0.0;
    auto frame_start = std::chrono::steady_clock::now();
    auto frame_end = std::chrono::steady_clock::now();

    cv::Mat frame;

    while (running)
    {
        frame_start = std::chrono::steady_clock::now();

        // 检查元数据更新
        if (metadata.version != last_ver)
        {
            x = buf.width / 2 - metadata.width / 2;
            y = buf.height / 2 - metadata.height / 2;

            std::tie(start_x, start_y, end_x, end_y) = calculate_bounds();

            if (x != last_x || y != last_y || metadata.width != last_width || metadata.height != last_height)
            {
                struct planes_setting ps5;
                ps5.plane_id = plane_id[0];
                ps5.fb_id = buf.fb_id;
                ps5.crtc_x = 0;
                ps5.crtc_y = 0;
                ps5.crtc_w = buf.width;
                ps5.crtc_h = buf.height;
                ps5.src_x = x;
                ps5.src_y = y;
                ps5.src_w = metadata.width;
                ps5.src_h = metadata.height;
                ret = drm_set_plane(fd, &ps5);
                if (ret < 0)
                {
                    fprintf(stderr, "drm_set_plane fail\n");
                    stop();
                }
                last_x = x;
                last_y = y;
                last_width = metadata.width;
                last_height = metadata.height;
            }

            target_frame_time = 1 / metadata.fps;

            // std::cout << "\n=== 数据更新 ==="
            //           << "\nResolution: " << metadata.width << "x" << metadata.height << "\n"
            //           << "\nDisplay Area: [" << start_x << "," << start_y << "] - ["
            //           << end_x << "," << end_y << "]"
            //           << "Target FPS: " << metadata.fps << " (Frame time: "
            //           << std::fixed << std::setprecision(2) << target_frame_time * 1000 << "ms)"
            //           << std::endl;
            last_ver = metadata.version;

            // last_frame_time = std::chrono::steady_clock::now(); // 重置计时
        }

        // 获取当前帧
        frame = frame_buffer.read();

        if (!frame.empty())
        {
            if (buf.vaddr == nullptr)
            {
                fprintf(stderr, "DRM buffer not initialized\n");
                continue;
            }

            // uint32_t word;
            // auto src = frame.data;
            for (uint32_t row = start_y; row < end_y; ++row)
            {
                uint8_t *src = frame.ptr(row - y);
                uint32_t *dst = reinterpret_cast<uint32_t *>(buf.vaddr) + row * buf.width + start_x;
                // std::memcpy(dst, src, frame.cols * frame.elemSize());

                // src += frame.cols * frame.elemSize();

                for (uint32_t col = start_x; col < end_x; ++col)
                {
                    *dst++ = (0xFF << 24) |   // X通道
                             (src[2] << 16) | // R
                             (src[1] << 8) |  // G
                             (src[0] << 0);   // B
                    src += metadata.channels;
                }
            }
        }

        // 计算帧率控制
        frame_end = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration<double>(frame_end - frame_start).count();
        sleep_time = target_frame_time - elapsed;

        if (sleep_time > 0)
        {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(sleep_time));
        }
        else
        {
            std::cerr << "Frame drop! Processing too slow: "
                      << elapsed * 1000 << "ms > "
                      << target_frame_time * 1000 << "ms" << std::endl;
        }
    }
}

void VideoPipeline::Impl::stop()
{
    running.store(false);
    if (show_thread.joinable())
    {
        show_thread.join();
    }

    drm_exit();
}

// 外部类接口转发
VideoPipeline::VideoPipeline() : impl(new Impl()) {}
VideoPipeline::~VideoPipeline() { delete impl; }
void VideoPipeline::load_frame(cv::Mat &frame) { impl->load_frame(frame); }

// int main(int argc, char **argv)
// {
//     if (argc < 2)
//     {
//         printf("Usage: %s <image-file>\n", argv[0]);
//         return -1;
//     }

//     VideoPipeline pipeline;

//     VideoCapture vc(argv[1]);
//     if (!vc.isOpened())
//     {
//         fprintf(stderr, "failed to open %s\n", argv[1]);
//         return 0;
//     }

//     double fps = vc.get(CAP_PROP_FPS);
//     // std::cout << "fps" << fps << std::endl;

//     // printf("video load OK!\n");

//     double target_frame_time = 1.0 / fps; // 目标帧间隔(秒)

//     cv::Mat frame;

//     while (true)
//     {
//         auto frame_start = std::chrono::steady_clock::now();
//         vc >> frame; // 采集帧

//         if (!frame.empty())
//         {
//             // 更新帧缓冲
//             pipeline.load_frame(frame);
//         }
//         else
//         {
//             break;
//         }

//         // 计算帧率控制
//         auto frame_end = std::chrono::steady_clock::now();
//         double elapsed = std::chrono::duration<double>(frame_end - frame_start).count();
//         double sleep_time = target_frame_time - elapsed;

//         if (sleep_time > 0)
//         {
//             std::this_thread::sleep_for(
//                 std::chrono::duration<double>(sleep_time));
//         }
//     }

//     getchar(); // 等待按键

//     return 0;
// }
