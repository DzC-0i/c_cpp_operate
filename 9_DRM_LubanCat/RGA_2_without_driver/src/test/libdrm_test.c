#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    int fd = open("/dev/dri/card0", O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open DRM device");
        return -1;
    }

    drmModeRes *resources = drmModeGetResources(fd);
    if (!resources)
    {
        perror("Failed to get DRM resources");
        close(fd);
        return -1;
    }

    printf("Found %d connectors\n", resources->count_connectors);

    drmModeFreeResources(resources);
    close(fd);
    return 0;
}
