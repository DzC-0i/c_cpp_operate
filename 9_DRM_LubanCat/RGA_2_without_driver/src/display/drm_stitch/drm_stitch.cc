#include "drm_stitch.hh"
#include <algorithm>
#include <cmath>

// 测试接口
#include <random>
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-1000, 1000);
auto rnd = std::bind(dist, mt);
void simulate_hard_computation()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rnd()));
}
cv::Mat VideoWall::read_mat()
{
    return stitch_frame;
}
void test_thread(size_t x, VideoWall *test)
{
    simulate_hard_computation();
    cv::VideoCapture vc("media/video/SampleVideo_1280x720_5mb.mp4");
    if (!vc.isOpened())
    {
        fprintf(stderr, "failed to open\n");
        return;
    }

    double fps = vc.get(cv::CAP_PROP_FPS);
    // std::cout << "fps" << fps << std::endl;

    // printf("video load OK!\n");

    double target_frame_time = 1.0 / fps; // 目标帧间隔(秒)

    cv::Mat frame;

    while (true)
    {
        auto frame_start = std::chrono::steady_clock::now();
        vc >> frame; // 采集帧

        if (!frame.empty())
        {
            // 更新帧缓冲
            test->add_frames(x, frame);
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

VideoWall::VideoWall()
{
    int ret;
    // 初始化DRM
    ret = drm_init();

    if (ret < 0)
    {
        fprintf(stderr, "DRM initialization failed\n");
        return;
    }

    display_width_ = buf.width;
    display_height_ = buf.height;
    // printf("width: %d\theight: %d\n", buf.width, buf.height);

    // 对齐drm时不可以更改这个
    // display_width_ = 1920;
    // display_height_ = 1080;

    // 初始化RGA硬件
    if (c_RkRgaInit() != 0)
    {
        std::cerr << "RGA init failed!" << std::endl;
        return;
    }

    // 手动分配 4 通道对齐内存（XRGB8888）
    int channels = 4;
    int stride = (display_width_ * channels + 3) & ~3; // 4字节对齐
    dst_buf_size = stride * display_height_;

    // dma使用时CPU占用较普通的要高，但是RGA处理速度加快了一些，并且RGA核心占用下降
    /* Allocate dma_buf, return dma_fd and virtual address. */
    // ret = dma_buf_alloc(DMA_HEAP_UNCACHE_PATH, dst_buf_size, &dst_dma_fd, (void **)&dst_buf);
    // if (ret < 0)
    // {
    //     printf("alloc src dma_heap buffer failed!\n");
    //     return;
    // }

    dst_buf = aligned_alloc(4, dst_buf_size);
    memset(dst_buf, 0, dst_buf_size);

    // /* clear CPU cache */
    // dma_sync_cpu_to_device(dst_dma_fd);

    // 创建目标缓冲区（屏幕尺寸）
    stitch_frame = cv::Mat(display_height_, display_width_, CV_8UC4, dst_buf, stride);

    // stitch_frame = cv::Mat(display_height_, display_width_, CV_8UC4, cv::Scalar(0, 0, 0, 0));

    // 创建RGA源缓冲区
    dst_handle_ = importbuffer_virtualaddr(
        stitch_frame.data,
        display_width_, display_height_,
        RK_FORMAT_RGBA_8888);
    // dma内存对齐过，可能有不同，不可直接使用上面一个
    // dst_handle_ = importbuffer_fd(
    //     dst_dma_fd,
    //     display_width_, display_height_,
    //     RK_FORMAT_RGBA_8888);
    // dst_handle_ = importbuffer_fd(dst_dma_fd, dst_buf_size);

    if (dst_handle_ == 0)
    {
        printf("import dst_handle_ error!\n");
        ret = -1;
        return;
    }

    dst_info = wrapbuffer_handle(dst_handle_, display_width_, display_height_, RK_FORMAT_RGBA_8888);

    // printf("width: %d \theight: %d \tchannels: %d\n", stitch_frame.cols, stitch_frame.rows, stitch_frame.channels());
    update_running = true;
    update_thread = std::thread(&VideoWall::update_display, this);
}

VideoWall::~VideoWall()
{
    update_running = false;
    is_set_resize = false;

    if (update_thread.joinable())
        update_thread.join();

    releasebuffer_handle(dst_handle_);

    if (dst_buf)
    {
        free(dst_buf); // aligned_alloc 需用 free
        dst_buf = nullptr;
    }

    // dma释放
    // dma_buf_free(dst_buf_size, &dst_dma_fd, dst_buf);

    display_rects.clear();

    c_RkRgaDeInit();
    drm_exit();
}

void VideoWall::set_layout(LayoutMode mode, int cell_layout_size, int fps, int rows, int cols)
{
    is_set_resize = false;
    display_rects.clear();
    stitch_frame.setTo(cv::Scalar(0, 0, 0));
    frame_fps = fps;
    // std::lock_guard<std::mutex> lock(frame_mutex_);

    switch (mode)
    {
    case AUTOMATIC:
        calculate_automatic_layout(cell_layout_size);
        break;
    case CUSTOM_GRID:
        calculate_custom_layout(cell_layout_size, rows, cols);
        break;
    }
}

void VideoWall::calculate_automatic_layout(int cell_layout_size)
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

void VideoWall::calculate_custom_layout(int cell_layout_size, int custom_rows, int custom_cols)
{
    if (custom_rows * custom_cols < cell_layout_size)
    {
        std::cerr << "Invalid custom layout\n";
        return;
    }

    calculate_grid_layout(cell_layout_size, custom_rows, custom_cols);
}

void VideoWall::calculate_grid_layout(int size, int rows, int cols)
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

void VideoWall::add_frames(size_t screen_place, cv::Mat &frame)
{
    // 边界检查
    if (screen_place < 0 || screen_place >= display_rects.size())
    {
        std::cerr << "Invalid screen position: " << screen_place << std::endl;
        return;
    }

    // 获取目标区域
    const cv::Rect &target_rect = display_rects[screen_place];

    // thread_local static cv::Mat resized_frame;

    // 双重检查锁定初始化缩放配置
    if (!is_set_resize)
    {
        std::lock_guard<std::mutex> config_lock(config_mutex_);
        if (!is_set_resize)
        {
            // 计算最佳缩放比例
            double width_ratio = static_cast<double>(target_rect.width) / frame.cols;
            double height_ratio = static_cast<double>(target_rect.height) / frame.rows;
            double resize_ratio = std::min(width_ratio, height_ratio);

            new_frame.x_offset = (target_rect.width - frame.cols * resize_ratio) / 2;
            new_frame.y_offset = (target_rect.height - frame.rows * resize_ratio) / 2;
            new_frame.width = frame.cols * resize_ratio;
            new_frame.height = frame.rows * resize_ratio;
            // 预分配缩放缓冲区
            // resized_frame.create(cv::Size(new_frame.width, new_frame.height), frame.type());
            is_set_resize = true;
        }
    }

    int ret;
    rga_buffer_handle_t src_handle_;
    rga_buffer_t src_info;
    im_rect src_rect, dst_rect;

    // int src_dma_fd;
    // int src_buf_size;
    // char *src_buf;

    // src_buf_size = frame.cols * frame.rows * get_bpp_from_format(RK_FORMAT_RGB_888);

    // dma_buf_alloc(DMA_HEAP_DMA32_UNCACHE_PATCH, src_buf_size, &src_dma_fd, (void **)&src_buf);
    // memcpy(src_buf, frame.data, src_buf_size);

    // 创建RGA源缓冲区
    src_handle_ = importbuffer_virtualaddr(
        frame.data,
        frame.cols, frame.rows,
        RK_FORMAT_RGB_888);
    // src_handle_ = importbuffer_fd(src_dma_fd, src_buf_size);

    if (src_handle_ == 0)
    {
        printf("importbuffer failed!\n");
        goto release_buffer;
    }

    // 配置缩放参数
    src_info = wrapbuffer_handle(src_handle_, frame.cols, frame.rows, RK_FORMAT_RGB_888);
    // src_info.wstride = frame.cols;
    // src_info.hstride = frame.rows;

    src_rect = {0, 0, frame.cols, frame.rows};
    dst_rect = {
        target_rect.x + new_frame.x_offset,
        target_rect.y + new_frame.y_offset,
        new_frame.width,
        new_frame.height};

    // 执行硬件加速缩放+合成
    ret = imcheck(src_info, dst_info, src_rect, dst_rect, 1);

    /* Configure the current thread to use only RGA3_core0 or RGA3_core1. */
    /* 提交到哪个核心处理 , 运行速度区别不大，CPU占用区别不大，GPU使用双核指定的占比单核百分比多*/
    // imconfig(IM_CONFIG_SCHEDULER_CORE, IM_SCHEDULER_RGA3_CORE0 | IM_SCHEDULER_RGA3_CORE1);
    // imconfig(IM_CONFIG_SCHEDULER_CORE, (screen_place % 2) ? IM_SCHEDULER_RGA3_CORE1 : IM_SCHEDULER_RGA3_CORE0);

    if (ret == IM_STATUS_NOERROR)
    {
        improcess(src_info, dst_info, {},
                  src_rect, dst_rect, {},
                  IM_SYNC);
    }
    else
    {
        std::cerr << "RGA error: " << imStrError(ret) << std::endl;
        goto release_buffer;
    }

release_buffer:
    // 释放临时资源
    releasebuffer_handle(src_handle_);
    // dma_buf_free(src_buf_size, &src_dma_fd, src_buf);

    return;
}

void VideoWall::update_display()
{
    int ret;
    bool last_ver = false;

    // 计算显示位置
    int x = buf.width / 2 - display_width_ / 2;
    int y = buf.height / 2 - display_height_ / 2;

    // 边界安全检查
    auto calculate_bounds = [&]()
    {
        return std::make_tuple(
            std::max(0, x),
            std::max(0, y),
            std::min(buf.width, x + display_width_),
            std::min(buf.height, y + display_height_));
    };
    auto [start_x, start_y, end_x, end_y] = calculate_bounds();

    int last_x = 0, last_y = 0;
    uint32_t last_width = 0, last_height = 0;

    // std::cout << "\nResolution: " << metadata.width << "x" << metadata.height
    //           << "\nDisplay Area: [" << start_x << "," << start_y << "] - ["
    //           << end_x << "," << end_y << "]\n";

    // auto last_frame_time = std::chrono::steady_clock::now();
    double target_frame_time = 1.0 / frame_fps; // 目标帧间隔(秒)

    // printf("Show img load OK!\n");
    double elapsed = 0.0;
    double sleep_time = 0.0;
    auto frame_start = std::chrono::steady_clock::now();
    auto frame_end = std::chrono::steady_clock::now();
    bool copy_all_ok = stitch_frame.isContinuous() && (stitch_frame.cols * 4 == buf.pitch);

    while (update_running)
    {
        frame_start = std::chrono::steady_clock::now();

        // /* invalid CPU cache */
        // dma_sync_device_to_cpu(dst_dma_fd);

        // 检查元数据更新
        if (is_set_resize != last_ver)
        {
            x = buf.width / 2 - display_width_ / 2;
            y = buf.height / 2 - display_height_ / 2;

            std::tie(start_x, start_y, end_x, end_y) = calculate_bounds();

            if (x != last_x || y != last_y || display_width_ != last_width || display_height_ != last_height)
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
                ps5.src_w = display_width_;
                ps5.src_h = display_height_;
                ret = drm_set_plane(fd, &ps5);
                if (ret < 0)
                {
                    fprintf(stderr, "drm_set_plane fail\n");
                    // stop();
                }
                last_x = x;
                last_y = y;
                last_width = display_width_;
                last_height = display_height_;
            }

            target_frame_time = 1.0 / frame_fps;

            last_ver = is_set_resize;

            // std::cout << "\n=== 数据更新 ==="
            //           << "\nResolution: " << metadata.width << "x" << metadata.height << "\n"
            //           << "\nDisplay Area: [" << start_x << "," << start_y << "] - ["
            //           << end_x << "," << end_y << "]"
            //           << "Target FPS: " << metadata.fps << " (Frame time: "
            //           << std::fixed << std::setprecision(2) << target_frame_time * 1000 << "ms)"
            //           << std::endl;
            // last_frame_time = std::chrono::steady_clock::now(); // 重置计时
        }

        if (!stitch_frame.empty())
        {
            if (buf.vaddr == nullptr)
            {
                fprintf(stderr, "DRM buffer not initialized\n");
                continue;
            }

            if (copy_all_ok)
            {
                // 单次全帧拷贝, 要求是前面内存对齐
                std::memcpy(buf.vaddr, stitch_frame.data, dst_buf_size);
            }
            else
            {
                auto src = stitch_frame.data;
                for (uint32_t row = start_y; row < end_y; ++row)
                {
                    uint32_t *dst = reinterpret_cast<uint32_t *>(buf.vaddr) + row * buf.width + start_x;
                    std::memcpy(dst, src, stitch_frame.cols * stitch_frame.elemSize());

                    src += stitch_frame.cols * stitch_frame.elemSize();
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

int main(int argc, char **argv)
{
    // if (argc < 2)
    // {
    //     printf("Usage: %s <image-file>\n", argv[0]);
    //     return -1;
    // }

    VideoWall test;
    // cv::namedWindow("pic", cv::WINDOW_AUTOSIZE);
    // cv::namedWindow("img", cv::WINDOW_AUTOSIZE);
    // cv::Mat frame = cv::imread(argv[1], 1);
    cv::Mat frame1 = cv::imread("media/picture/2024-07-11-13_04_21.png", 1);
    cv::Mat frame2 = cv::imread("media/picture/2024-07-11-13_04_22.jpg", 1);
    cv::Mat frame3 = cv::imread("media/picture/belle-nuit-testchart-1080p.png", 1);
    // cv::imshow("pic", frame);

    test.set_layout(VideoWall::AUTOMATIC, 6);
    auto now = std::chrono::high_resolution_clock::now();
    test.add_frames(0, frame1);
    test.add_frames(1, frame1);
    test.add_frames(2, frame1);
    test.add_frames(3, frame1);
    test.add_frames(4, frame1);
    test.add_frames(5, frame1);

    auto now2 = std::chrono::high_resolution_clock::now();
    std::cout << "运行时长：" << std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now).count() << " ms" << std::endl;

    // cv::imshow("img", test.read_mat());

    // cv::waitKey(0);

    // cv::Mat mat = test.read_mat();

    // test.drm_operate->load_frame(mat);

    getchar(); // 等待按键

    test.set_layout(VideoWall::AUTOMATIC, 4);
    now = std::chrono::high_resolution_clock::now();
    test.add_frames(0, frame2);
    test.add_frames(1, frame2);
    test.add_frames(2, frame2);
    now2 = std::chrono::high_resolution_clock::now();
    std::cout << "运行时长：" << std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now).count() << " ms" << std::endl;

    getchar(); // 等待按键

    test.set_layout(VideoWall::AUTOMATIC, 9);

    std::thread thr[9];

    for (size_t i = 0; i < 9; i++)
    {
        thr[i] = std::thread(test_thread, i, &test);
    }

    for (size_t i = 0; i < 9; i++)
    {
        thr[i].join();
    }

    getchar(); // 等待按键

    // cv::destroyAllWindows();

    return 0;
}
