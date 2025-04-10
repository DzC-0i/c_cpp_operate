#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

struct buffer_object
{
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t handle;
    uint32_t size;
    uint8_t *vaddr;
    uint32_t fb_id;
};

struct buffer_object buf[2];

static int modeset_create_fb(int fd, struct buffer_object *bo, uint32_t color)
{
    struct drm_mode_create_dumb create = {};
    struct drm_mode_map_dumb map = {};
    uint32_t i;

    create.width = bo->width;
    create.height = bo->height;
    create.bpp = 32;
    drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);

    bo->pitch = create.pitch;
    bo->size = create.size;
    bo->handle = create.handle;
    drmModeAddFB(fd, bo->width, bo->height, 24, 32, bo->pitch,
                 bo->handle, &bo->fb_id);

    map.handle = create.handle;
    drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map);

    bo->vaddr = mmap(0, create.size, PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, map.offset);

    for (i = 0; i < (bo->size); i++)
        bo->vaddr[i] = color;

    return 0;
}

static void modeset_destroy_fb(int fd, struct buffer_object *bo)
{
    struct drm_mode_destroy_dumb destroy = {};

    drmModeRmFB(fd, bo->fb_id);

    munmap(bo->vaddr, bo->size);

    destroy.handle = bo->handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
}

/* 效果不是很正常，应该开始时全屏显示一个颜色，之后显示部分区域颜色，而且改动之后更加有问题 */
int main(int argc, char **argv)
{
    int fd;
    drmModeConnector *conn;
    drmModeRes *res;
    drmModePlaneRes *plane_res;
    uint32_t conn_id;
    uint32_t crtc_id;
    uint32_t plane_id;

    fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);

    res = drmModeGetResources(fd);
    crtc_id = res->crtcs[1];
    conn_id = res->connectors[1];

    drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    plane_res = drmModeGetPlaneResources(fd);
    plane_id = plane_res->planes[6];

    conn = drmModeGetConnector(fd, conn_id);
    buf[0].width = conn->modes[0].hdisplay;
    buf[0].height = conn->modes[0].vdisplay;
    buf[1].width = conn->modes[0].hdisplay;
    buf[1].height = conn->modes[0].vdisplay;

    modeset_create_fb(fd, &buf[0], 0xff0000);
    modeset_create_fb(fd, &buf[1], 0x0000ff);

    drmModeSetCrtc(fd, crtc_id, buf[0].fb_id,
                   0, 0, &conn_id, 1, &conn->modes[0]);

    getchar();

    /* crop the rect from framebuffer(100, 150) to crtc(50, 50) */
    drmModeSetPlane(fd, plane_id, crtc_id, buf[1].fb_id, 0,
                    50, 50, 320, 320,
                    100 << 16, 150 << 16, 320 << 16, 320 << 16);

    usleep(500000);
    usleep(500000);

    drmModeSetPlane(fd, plane_id, crtc_id, buf[1].fb_id, 0,
                    conn->modes[0].hdisplay / 2, conn->modes[0].vdisplay / 2, conn->modes[0].hdisplay / 4, conn->modes[0].vdisplay / 4,
                    100 << 16, 150 << 16, 320 << 16, 320 << 16);

    getchar();

    modeset_destroy_fb(fd, &buf[0]);
    modeset_destroy_fb(fd, &buf[1]);

    drmModeFreeConnector(conn);
    drmModeFreePlaneResources(plane_res);
    drmModeFreeResources(res);

    close(fd);

    return 0;
}
