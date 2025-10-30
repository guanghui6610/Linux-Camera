#include <stdio.h>
#include "fb.h"
#include <config.h>
#include "unistd.h"

int main(void) 
{
    int ret = -1;

    //printf("image decode player:");
    
    /*
        fb_open是定义在fb.c中的
        作用1.打开FrameBuffer设备文件
            2.获取屏幕信息
            3.内存映射：将framebuffer设备的物理地址映射到进程的虚拟地址空间
                这样，程序就可以像读写普通内存一样，直接向这个地址写入像素数据，从而改变屏幕显示
    */
    ret = fb_open(); 
    if(ret < 0)
    {
        return -1;
    }

    //显示bmp图片
    //display_bmp("./image/bmp/gentlman.bmp");
    //display_bmp("./image/gentlman.bmp");

    //sleep(3);
    //显示jpg图片
    //display_jpg("image/people.jpg");
    //display_png("dog.png");

    /* 开机淡入动画 */
    play_boot_logo("guanghui.jpg");

    scan_image("./image");
    //show_image(0);
    
    /*
        下面两行注释打开可以循环播放6张图片
    */
    //print_images();
    //show_images();

    ts_updown();
    fb_close();

    return 0;
}