#include "drm-core.h"
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>

using namespace cv;

class FrameDoubleBuffer
{
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
            {                                 // 二次检查避免竞态条件
                front_buf = back_buf.clone(); // 或使用 swap 优化
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

class VideoPipeline
{
    FrameDoubleBuffer frame_buffer;
    StreamMetadata metadata;
    std::atomic<bool> running{false};

public:
    // 启动视频处理
    void load_video(const char *filename)
    {
        if (!running)
            running.store(true);

        int frame_count = 0;

        VideoCapture vc(filename);
        if (!vc.isOpened())
        {
            fprintf(stderr, "failed to open %s\n", filename);
            stop();
        }

        double fps = vc.get(CAP_PROP_FPS);
        // std::cout << "fps" << fps << std::endl;

        // printf("video load OK!\n");

        auto last_time = std::chrono::steady_clock::now();
        double target_frame_time = 1.0 / fps; // 目标帧间隔(秒)

        cv::Mat frame;
        vc >> frame; // 采集帧
        metadata.update(frame.cols, frame.rows, frame.channels(), 30);

        while (running)
        {
            auto frame_start = std::chrono::steady_clock::now();
            vc >> frame; // 采集帧

            if (!frame.empty())
            {
                // 更新帧缓冲
                frame_buffer.write(frame);

                // 每25帧更新
                if (++frame_count % 25 == 0)
                {
                    auto now = std::chrono::steady_clock::now();
                    double elapsed = std::chrono::duration<double>(now - last_time).count();
                    double fps = 25 / elapsed;
                    metadata.update(frame.cols, frame.rows, frame.channels(), fps);
                    // metadata.update(frame.cols, frame.rows, frame.channels(), 25);
                    last_time = now;
                    frame_count = 0;
                }
            }
            else
            {
                stop();
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
        }
    }

    void show_img()
    {
        uint64_t last_ver = 0;

        // 计算显示位置
        int x = buf.width / 2 - metadata.width / 2;
        int y = buf.height / 2 - metadata.height / 2;

        // int start_x = 0 > x ? 0 : x;
        // int start_y = 0 > y ? 0 : y;
        // int end_x = buf.width < x + metadata.width ? buf.width : x + metadata.width;
        // int end_y = buf.height < y + metadata.height ? buf.height : y + metadata.height;

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

        std::cout << "\nResolution: " << metadata.width << "x" << metadata.height
                  << "\nDisplay Area: [" << start_x << "," << start_y << "] - ["
                  << end_x << "," << end_y << "]\n";

        // int start_x = 0 > buf.width / 2 - metadata.width / 2 ? 0 : buf.width / 2 - metadata.width / 2;
        // int start_y = 0 > buf.height / 2 - metadata.height / 2 ? 0 : buf.height / 2 - metadata.height / 2;
        // int end_x = buf.width < buf.width / 2 + metadata.width / 2 ? buf.width : buf.width / 2 + metadata.width / 2;
        // int end_y = buf.height < buf.height / 2 + metadata.height / 2 ? buf.height : buf.height / 2 + metadata.height / 2;

        auto last_frame_time = std::chrono::steady_clock::now();
        double target_frame_time = 1.0 / metadata.fps; // 目标帧间隔(秒)

        // printf("Show img load OK!\n");

        while (running)
        {
            auto frame_start = std::chrono::steady_clock::now();

            // 获取当前帧
            cv::Mat frame = frame_buffer.read();

            if (!frame.empty())
            {
                if (buf.vaddr == nullptr)
                {
                    fprintf(stderr, "DRM buffer not initialized\n");
                    continue;
                }

                // uint32_t word;

                for (uint32_t row = start_y; row < end_y; ++row)
                {
                    uint8_t *src = frame.ptr(row - y);
                    uint32_t *dst = reinterpret_cast<uint32_t *>(buf.vaddr) + row * buf.width + start_x;

                    for (uint32_t col = start_x; col < end_x; ++col)
                    {
                        // word = 0;
                        // // 将原图像组合为 XRGB8888 格式（0x00RRGGBB），高八位填写FF，设置不透明
                        // word = (word | src[(col - x) * metadata.channels + 0]) |
                        //        (word | src[(col - x) * metadata.channels + 1]) << 8 |
                        //        (word | src[(col - x) * metadata.channels + 2]) << 16 |
                        //        (word | 255) << 24;
                        // buf.vaddr[row * buf.width + col] = word;

                        *dst++ = (0xFF << 24) |   // Alpha通道
                                 (src[2] << 16) | // R
                                 (src[1] << 8) |  // G
                                 src[0];          // B
                        src += metadata.channels;
                    }
                }
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
                std::cerr << "Frame drop! Processing too slow: "
                          << elapsed * 1000 << "ms > "
                          << target_frame_time * 1000 << "ms" << std::endl;
            }

            // 检查元数据更新
            if (metadata.version != last_ver)
            {
                x = buf.width / 2 - metadata.width / 2;
                y = buf.height / 2 - metadata.height / 2;

                // start_x = 0 > x ? 0 : x;
                // start_y = 0 > y ? 0 : y;
                // end_x = buf.width < x + metadata.width ? buf.width : x + metadata.width;
                // end_y = buf.height < y + metadata.height ? buf.height : y + metadata.height;
                std::tie(start_x, start_y, end_x, end_y) = calculate_bounds();

                target_frame_time = 1 / metadata.fps;

                std::cout << "\n=== 数据更新 ==="
                          << "\nResolution: " << metadata.width << "x" << metadata.height << "\n"
                          << "\nDisplay Area: [" << start_x << "," << start_y << "] - ["
                          << end_x << "," << end_y << "]"
                          << "Target FPS: " << metadata.fps << " (Frame time: "
                          << std::fixed << std::setprecision(2) << target_frame_time * 1000 << "ms)"
                          << std::endl;
                last_ver = metadata.version;

                last_frame_time = std::chrono::steady_clock::now(); // 重置计时
            }
        }
    }

    // 停止处理
    void stop()
    {
        running.store(false);
    }
};

int main(int argc, char **argv)
{
    int ret;
    if (argc < 2)
    {
        printf("Usage: %s <image-file>\n", argv[0]);
        return -1;
    }

    // 初始化DRM
    ret = drm_init();
    if (ret < 0)
    {
        fprintf(stderr, "DRM initialization failed\n");
        return -1;
    }
    VideoPipeline pipeline;

    // 方法1：使用Lambda（推荐）
    std::thread t1([&pipeline, argv]()
                   { pipeline.load_video(argv[1]); });

    std::thread t2([&pipeline]()
                   { pipeline.show_img(); });

    getchar(); // 等待按键

    pipeline.stop();
    // 等待线程结束
    t1.join();
    t2.join();

    drm_exit();

    return 0;
}
