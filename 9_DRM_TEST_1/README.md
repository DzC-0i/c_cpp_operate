```bash
#命令关闭图形界面
sudo systemctl set-default multi-user.target

#开启图像界面
sudo systemctl set-default graphical.target
```


终端中打印的HDMI屏幕对应的connectors、CRTCs的ID

```bash
# 查看帮助信息
modetest -h
# 打印对应参数
modetest -M rockchip  -e
modetest -M rockchip -c

# 筛选输出各个组件的id
modetest -M rockchip | cut -f1 | grep -E ^[0-9A-Z]\|id

# 在 HDMI 上测试
# 210： HDMI connector id
# 93： 某个 VOP 的 crtc id
# 3840x2160：显示 mode;
modetest -M rockchip  -s 210@93:3840x2160
modetest -M rockchip  -s 210@93:#1          # 相同的效果
```



寻找头文件

```bash
find /usr/include -name "drm.h"
```

单独编译

```bash
gcc libdrm_test.c -o libdrm_test -I/usr/include/libdrm -ldrm

gcc libdrm_test.c -o libdrm_test  $(pkg-config --cflags --libs libdrm) -D_FILE_OFFSET_BITS=64 -Wall -O0 -g
```

需要一个个的查看过，错误的情况下不能够显示
crtc_id
conn_id
plane_ID


libdrm_plane.c
查看plane通道ID以及对应的类型

libdrm_test.c
查看通道是否正常

modeset.c
原测试程序

1_modeset-single-buffer.c
单张测试

2_modeset-double-buffer.c
双队列切换显示, 加速图像填充

3_modeset-page-flip.c
使用 drmModePageFlip() 来避免 drmModeSetCrtc() 对于某些硬件来造成撕裂

4_modeset-plane-test.c
硬件图层的缩放裁剪，必须先通过drmModeSetCrtc()初始化整个显示链路，效果是一定会有一个全图的显示

更加推荐使用 atomic
5_modeset-atomic-crtc.c
基于 atomic 的 crtc 

6_modeset-atomic-plane.c
基于 atomic 的 plane

test_pic.c
图片显示 执行后面带参数

```bash
./build/bin/DRM_pic pic/belle-nuit-testchart-1080p.png 
```
