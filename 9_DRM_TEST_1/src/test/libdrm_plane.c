#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <stdio.h>
#include <unistd.h>

void print_planes(int fd)
{
    drmModePlaneRes *plane_res = drmModeGetPlaneResources(fd);
    if (!plane_res)
    {
        perror("Failed to get plane resources");
        return;
    }

    printf("Total Planes: %d\n", plane_res->count_planes);
    for (int i = 0; i < plane_res->count_planes; i++)
    {
        drmModePlane *plane = drmModeGetPlane(fd, plane_res->planes[i]);
        if (!plane)
            continue;

        // 打印 Plane ID、 Crtc ID 和 类型
        printf("Plane ID: %d, Crtc_id: %d, Type: ", plane->plane_id, plane->crtc_id);

        // 获取 Plane 属性以确定类型
        drmModeObjectProperties *props = drmModeObjectGetProperties(fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);
        for (int j = 0; j < props->count_props; j++)
        {
            drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[j]);
            if (prop && strcmp(prop->name, "type") == 0)
            {
                switch (props->prop_values[j])
                {
                case DRM_PLANE_TYPE_OVERLAY:
                    printf("Overlay\n");
                    break;
                case DRM_PLANE_TYPE_PRIMARY:
                    printf("Primary\n");
                    break;
                case DRM_PLANE_TYPE_CURSOR:
                    printf("Cursor\n");
                    break;
                default:
                    printf("Unknown\n");
                }
            }
            drmModeFreeProperty(prop);
        }
        drmModeFreeObjectProperties(props);
        drmModeFreePlane(plane);
    }
    drmModeFreePlaneResources(plane_res);
}

int main()
{
    int fd = open("/dev/dri/card0", O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open DRM device");
        return 1;
    }
    print_planes(fd);
    close(fd);
    return 0;
}
