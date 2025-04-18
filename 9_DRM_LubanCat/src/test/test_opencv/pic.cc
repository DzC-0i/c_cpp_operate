#include "drm-core.h"
#include <opencv2/opencv.hpp>

uint32_t color_table[6] = {RED, GREEN, BLUE, BLACK, WHITE, BLACK_BLUE};
using namespace cv;

struct image_frame
{
    cv::Mat frame;
    uint32_t width;
    uint32_t height;
    int channels;
};

int load_image(const char *filename, struct image_frame *img)
{
    // 读取图像
    img->frame = cv::imread(filename, cv::IMREAD_UNCHANGED);
    if (img->frame.empty())
    {
        fprintf(stderr, "Failed to load image: %s\n", filename);
        return -1;
    }

    // 获取图像参数
    img->width = img->frame.cols;
    img->height = img->frame.rows;
    img->channels = img->frame.channels();

    printf("width: %d \theight: %d\tchannels: %d\n", img->width, img->height, img->channels);

    return 0;
}

int show_img(struct image_frame *img, int x, int y)
{
    if (buf.vaddr == nullptr)
    {
        fprintf(stderr, "DRM buffer not initialized\n");
        return -1;
    }

    // uint32_t word;
    // 计算显示位置
    int start_x = 0 > x ? 0 : x;
    int start_y = 0 > y ? 0 : y;
    int end_x = buf.width < x + img->width ? buf.width : x + img->width;
    int end_y = buf.height < y + img->height ? buf.height : y + img->height;

    // printf("x: %d\ty: %d\nstart_x: %d\tstart_y: %d\nend_x: %d\tend_y: %d\n",
    //        x, y, start_x, start_y, end_x, end_y);

    for (int row = start_y; row < end_y; row++)
    {
        uint8_t *src = img->frame.ptr(row - y);
        uint32_t *dst = reinterpret_cast<uint32_t *>(buf.vaddr) + row * buf.width + start_x;

        for (int col = start_x; col < end_x; col++)
        {
            // word = 0;
            // // 将原图像组合为 XRGB8888 格式（0x00RRGGBB），高八位填写FF，设置不透明
            // word = (word | src[(col - x) * img->channels + 0]) |
            //        (word | src[(col - x) * img->channels + 1]) << 8 |
            //        (word | src[(col - x) * img->channels + 2]) << 16 |
            //        (word | 255) << 24;
            // buf.vaddr[row * buf.width + col] = word;

            *dst++ = (0xFF << 24) |   // Alpha通道
                     (src[2] << 16) | // R
                     (src[1] << 8) |  // G
                     src[0];          // B
            src += img->channels;
        }
    }

    return 0;
}

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

    // 加载图像
    struct image_frame img;
    ret = load_image(argv[1], &img);
    if (ret < 0)
    {
        drm_exit();
        return -1;
    }

    auto now = std::chrono::high_resolution_clock::now();
    // 显示图像
    if (show_img(&img, buf.width / 2 - img.width / 2, buf.height / 2 - img.height / 2) < 0)
    {
        fprintf(stderr, "Failed to display image\n");
    }
    auto now2 = std::chrono::high_resolution_clock::now();
    std::cout << "运行时长：" << std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now).count() << " ms" << std::endl;

    getchar(); // 等待按键

    // 清理资源
    img.frame.release();
    drm_exit();

    return 0;
}
