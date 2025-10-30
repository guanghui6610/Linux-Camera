#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <config.h>

#include "fb.h"

extern unsigned int image_index;

//下面这个函数实现通过触摸屏来对图片翻页显示
int ts_updown(void)
{
    //第一步：触摸屏的触摸操作检测
    int fd = -1 , ret = -1;
    struct input_event ev;
    int i = 1;//用来记录当前显示的是第几个图片

    fd = open(DEVICE_TOUCHSCREEN , O_RDONLY);
    if(fd < 0)
    {
        perror("open");
        return -1;
    }

    while(1)
    {
        memset(&ev , 0 , sizeof(struct input_event));
        ret = read(fd, &ev, sizeof(struct input_event));
        if (ret != sizeof(struct input_event))
		{
			perror("read");
			close(fd);
			return -1;
		}
        // 用来记录当前显示的是第几个图片触摸坐标来翻页
        if((ev.type == EV_ABS) && (ev.code == ABS_X))
        {
            if((ev.value >= 0) && (ev.value < TOUCH_WIDTH))
            {
                //上翻页
                if(i-- <= 1)
                {
                    i = image_index;
                }
            }
            else if((ev.value >= (WIDTH - TOUCH_WIDTH)) && (ev.value < WIDTH))      
            {
                //下翻页
                if(i++ >= image_index)
                {
                    i = 1;
                }
            }      
            show_image(i - 1);
        }

    }
    close(fd);

	return 0;
}