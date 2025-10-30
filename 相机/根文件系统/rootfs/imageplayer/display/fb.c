/*
    fb.c 操作framebuffer的基础代码，包含fb的打开,ioctl获取信息
    基本的测试fc显示代码
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "fb.h"
#include "config.h"


// 全局变量
unsigned int *pfb = NULL;//pfb指针指向framebuffer内存首地址
int fbfd = -1; //打开fb后得到的fd

//这个函数用来打开framebuffer
int fb_open(void)
{ 
    int ret = -1;
	
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;
	
	// 第1步：打开设备
    /*
        应用程序调用open函数->C库中的open函数->系统调用->驱动的open函数
        fd(File Descriptor):文件描述符
        FBDEVICE：宏定义，代表设备文件的路径
        O_RDWR：表示可读可写的模式打开
    */
	fbfd = open(FBDEVICE, O_RDWR);
	if (fbfd < 0)
	{
		perror("open error");
		return -1;
	}
	debug("open %s success.\n", FBDEVICE);
	
	// 第2步：获取设备的硬件信息
    /*
        ioctl参数含义
        fd:上一步获得的文件描述符
        FBIOGET_FSCREENINFO,固定屏幕信息，预定义的一个宏在<linux/fb.h>中，获取固定的屏幕信息，例如显存的物理地址和长度，显示类型等
        &finfo:获取的信息需要放到这个内存地址中
    */
	ret = ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo);
	if (ret < 0)
	{
		perror("ioctl error");
		return -1;
	}
	debug("smem_start = 0x%lx, smem_len = %u.\n", finfo.smem_start, finfo.smem_len);
	
	/*
		FBIOGET_VSCREENINFO,虚拟屏幕信息
	*/
	ret = ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);
	if (ret < 0)
	{
		perror("ioctl error");
		return -1;
	}

	debug("xres = %u, yres = %u.\n", vinfo.xres, vinfo.yres);//xres代表的是水平分辨率，yres代表的是垂直分辨率
	debug("xres_virtual = %u, yres_virtual = %u.\n", vinfo.xres_virtual, vinfo.yres_virtual);
	debug("bpp = %u.\n", vinfo.bits_per_pixel); //bpp代表像素深度
	unsigned long len = vinfo.xres_virtual * vinfo.yres_virtual * vinfo.bits_per_pixel / 8;
	debug("len = %ld\n", len);

	// 第3步：进行mmap
	pfb = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if (NULL == pfb)
	{
		perror("mmap error");
		return -1;
	}
	debug("pfb = %p.\n", pfb);
	
	return 0;
}

void fb_close(void) 
{
    close(fbfd);
}

//绘制屏幕背景色的
void fb_draw_back(unsigned int width, unsigned int height, unsigned int color)
{
	unsigned int x, y;
	
	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			*(pfb + y * WIDTH + x) = color;
		}
	}
}

//画线的测试函数
void fb_draw_line(unsigned int color)
{
	unsigned int x;
	
	for (x=50; x<600; x++)
	{
		*(pfb + 200 * WIDTH + x) = color;
	}
}

//填充fb缓冲区数据（图片从右下开始）
void fb_draw(const struct pic_info *pPic)
{
	char *pDate = pPic->pData;
	unsigned int cnt = 0;//当前像素点的编号
	unsigned int a = 0;//源图像数据数组的偏移量,这样的是从数组数据出发
	unsigned int x , y;

	if((pPic->bpp != 24) && (pPic->bpp != 32))
	{
		printf("BPP %d is not support\r\n" , pPic->bpp);
		return;
	}

	a = (pPic->height) * (pPic->width) * 3 - 3;//一共有这么多个字节,bmp图像数据是从右下开始的
	for(y=0 ; y<(pPic->height) ; y++)
	{
		for(x=0 ; x<(pPic->width) ; x++)
		{
			cnt = WIDTH * y + x; //当前像素点的编号
			//cnt *= 3; //当前像素点的数据在数组中的下标
			/*
				当前像素点对应的图像数据的RGB分别是：
				pDate[cnt+0] pDate[cnt+1] pDate[cnt+2]
			*/
			*(pfb + cnt) = ((pDate[a+0] << 0) | (pDate[a+1] << 8) | (pDate[a+2] << 16));//第一个像素点的数据
			a -= 3;
		}
	}
}

//填充fb缓冲区数据（图片从左上开始）
void fb_draw2(const struct pic_info *pPic)
{
	char *pDate = pPic->pData;
	unsigned int cnt = 0;//当前像素点的编号
	unsigned int a = 0;//源图像数据数组的偏移量,这样的是从数组数据出发
	unsigned int x , y;

	if((pPic->bpp != 24) && (pPic->bpp != 32))
	{
		printf("BPP %d is not support\r\n" , pPic->bpp);
		return;
	}

	a = 0;
	//图片分辨率是pPic->height * pPic->width
	for(y=0 ; y<(pPic->height) ; y++)
	{
		for(x=0 ; x<(pPic->width) ; x++)
		{
			cnt = WIDTH * y + x; //当前像素点的编号
			//cnt *= 3; //当前像素点的数据在数组中的下标
			/*
				当前像素点对应的图像数据的RGB分别是：
				pDate[cnt+0] pDate[cnt+1] pDate[cnt+2]
			*/
			*(pfb + cnt) = ((pDate[a+2] << 0) | (pDate[a+1] << 8) | (pDate[a+0] << 16));//第一个像素点的数据
			a += 3;
		}
	}
}

//大图缩放成与屏幕分辨率一样的图片
void scale_image(const char *src , int sw , int sh,
					char *dst , int dw , int dh)
{
	int x , y;
	for(y = 0; y<dh; y++)
	{
		int sy = y * sh / dh;
		for(x = 0; x < dw; x++)
		{
			int sx = x * sw / dw;
			int src_index = (sy * sw + sx) * 3;
			int dst_index = (y * dw + x) * 3;
			dst[dst_index + 0] = src[src_index + 0];
			dst[dst_index + 1] = src[src_index + 1];
			dst[dst_index + 2] = src[src_index + 2];
		}
	}
}
//白色清屏+居中小图+大图缩放(jpg和png)
void fb_draw_image_adapt(const struct pic_info *pPic)
{
	const char *pDate = pPic->pData;
	unsigned int cnt = 0;//当前像素点的编号
	unsigned int a = 0;//源图像数据数组的偏移量,这样的是从数组数据出发
	unsigned int cnt1 = 0;//小图源图像数据数组的偏移量，这样的是从图像像素点出发
	unsigned int x0 , y0 = 0;
	unsigned int x , y;

	/* 1.白色清屏 */
	for(y = 0; y < HEIGHT; y++)
	{
		for(x = 0; x < WIDTH; x++)
		{
			*(pfb + y * WIDTH + x) = BLUE;
		}
	}

	/* 2.判断是否需要缩放 */
	if((pPic->width > WIDTH) || (pPic->height > HEIGHT))
	{
		/* 2.1大图，缩放到屏幕大小 */
		scale_image(pDate , pPic->width , pPic->height , rgb_buffer , WIDTH , HEIGHT);

		/* 2.2直接铺满屏*/
		for(y = 0; y < HEIGHT; y++)
		{
			for(x = 0; x < WIDTH; x++)
			{
				cnt = y * WIDTH + x;
				*(pfb + cnt) = ((rgb_buffer[a + 2] << 0) | (rgb_buffer[a + 1] << 8) | (rgb_buffer[a + 0] << 16));
				a += 3;
			}
		}
	}
	else
	{
		/* 3.1小图，居中显示 */
		x0 = (WIDTH - pPic->width) / 2;
		y0 = (HEIGHT - pPic->height) / 2;
		for(y = 0; y < pPic->height; y++)
		{
			if(y0 + y >= HEIGHT) break;
			for(x = 0; x < pPic->width; x++)
			{
				if(x0 + x >= WIDTH) break;
				cnt = (y + y0) * WIDTH + (x + x0);
				cnt1 = (y * pPic->width + x) * 3;
				*(pfb + cnt) = ((pDate[cnt1+0] << 16) | (pDate[cnt1+1] << 8) | (pDate[cnt1+2] << 0));//第一个像素点的数据
			}
		}
	}
}

//白色清屏+居中小图+大图缩放（bmp）
void fb_draw_image_adapt2(const struct pic_info *pPic)
{
	const char *pDate = pPic->pData;
	unsigned int cnt = 0;//屏幕像素点的编号
	unsigned int cnt1 = 0;//图像的像素点编号
	unsigned int x0 , y0 = 0;
	unsigned int x , y;

	/* 1.白色清屏 */
	for(y = 0; y < HEIGHT; y++)
	{
		for(x = 0; x < WIDTH; x++)
		{
			*(pfb + y * WIDTH + x) = BLACK;
		}
	}
	/* 2.判断bpp是32还是24 */
	/* 是否可以优化掉（2025.10.11） */
	unsigned int bytes_per_pixel = (pPic->bpp == 32) ? 4 : 3;
	int line_bytes = ((pPic->width * bytes_per_pixel + 3) / 4) * 4;

	/* 3.判断是否需要缩放 */
	if((pPic->width > WIDTH) || (pPic->height > HEIGHT))
	{
		/* 3.1大图，缩放到屏幕大小 */
		scale_image(pDate , pPic->width , pPic->height , rgb_buffer , WIDTH , HEIGHT);

		//unsigned int a = WIDTH * HEIGHT * 3 - 3;//源图像数据数组的偏移量,这样的是从数组数据出发
		/* 3.2直接铺满屏*/
		for(y = 0; y < HEIGHT; y++)
		{
			for(x = 0; x < WIDTH; x++)
			{
				cnt = y * WIDTH + x;
				cnt1 = (pPic->height - y - 1) * line_bytes + x * bytes_per_pixel;
				*(pfb + cnt) = ((rgb_buffer[cnt1 + 0] << 0) | (rgb_buffer[cnt1 + 1] << 8) | (rgb_buffer[cnt1 + 2] << 16));
				//a -= bytes_per_pixel;
			}
		}
	}
	else
	{
		/* 4.1小图，居中显示 */
		x0 = (WIDTH - pPic->width) / 2;
		y0 = (HEIGHT - pPic->height) / 2;
		for(y = 0; y < pPic->height; y++)
		{
			if(y0 + y >= HEIGHT) break;
			for(x = 0; x < pPic->width; x++)
			{
				if(x0 + x >= WIDTH) break;
				cnt = (y + y0) * WIDTH + (x + x0);
				cnt1 = (pPic->height - y - 1) * line_bytes + x * bytes_per_pixel;
				*(pfb + cnt) = ((pDate[cnt1+2] << 16) | (pDate[cnt1+1] << 8) | (pDate[cnt1+0] << 0));//第一个像素点的数据
			}
		}
	}
}

// 居中渐变显示jpg图片
void fb_draw_jpg_fadein(const char *path, unsigned int steps, unsigned int delay_ms)
{
    struct pic_info pic;
    int ret = is_jpg(path);
    if(ret != 0)
        return;

    pic.pathname = (char *)path;
    pic.pData = rgb_buffer;
    if(jpg_analyze(&pic) != 0)
        return;

    int x0 = (WIDTH  - pic.width)  / 2;
    int y0 = (HEIGHT - pic.height) / 2;

    // 只清一次屏，清理屏幕颜色为白色
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            *(pfb + y * WIDTH + x) = 0xFFFFFF;

    for (unsigned int step = 0; step <= steps; step++) 
	{
        float alpha = (float)step / steps;
        unsigned int x, y, cnt, cnt1;
        unsigned char r, g, b;

        for (y = 0; y < pic.height; y++) 
		{
            if (y0 + y >= HEIGHT) break;
            for (x = 0; x < pic.width; x++) 
			{
                if (x0 + x >= WIDTH) break;
                cnt = (y0 + y) * WIDTH + (x0 + x);
                cnt1 = (y * pic.width + x) * 3;
                r = pic.pData[cnt1+0] * alpha + (1-alpha)*255;
                g = pic.pData[cnt1+1] * alpha + (1-alpha)*255;
                b = pic.pData[cnt1+2] * alpha + (1-alpha)*255;
				/* 换成前面的反而不合适，因为这里RGB需要对应一定的透明度实现渐变效果 */
                *(pfb + cnt) = ((r << 16) | (g << 8) | (b << 0));//这个rgb可以替换成前面的，以免弄混
            }
        }
        usleep(delay_ms * 1000 / steps);
    }
}

// 开机logo动画，居中渐变
void play_boot_logo(const char *jpg_path)
{
    fb_draw_jpg_fadein(jpg_path, 48, 500); // 24步，0.5秒渐变
    usleep(400*1000); // logo全显后停留0.4秒
}


#if 0
//测试显示1024*600分辨率的图片/*
void fb_draw_picture(void)
{
	const unsigned char *pDate = gImage_1024600;
	unsigned int cnt = 0;
	unsigned int i , j;

	for(i = 0; i < HEIGHT; i++)
	{
		for(j = 0; j < WIDTH; j++)
		{
			cnt = WIDTH * i + j; //当前像素点的编号
			cnt *= 3; //当前像素点的数据在数组中的下标
			/*
				当前像素点对应的图像数据的RGB分别是：
				pDate[cnt+0] pDate[cnt+1] pDate[cnt+2]
			*/
			*pfb = ((pDate[cnt+0] << 16) | (pDate[cnt+1] << 8) | (pDate[cnt+2] << 0));//第一个像素点的数据
			pfb++;
		}
	}
}
#endif

#if 0
//测试显示比屏幕分辨率小的图片
void fb_draw_picture2(void)
{
	const unsigned char *pDate = gImage_480319;
	unsigned int cnt = 0;//当前像素点的编号
	unsigned int a = 0;//源图像数据数组的偏移量,这样的是从数组数据出发
	unsigned int i , j;

	//图片分辨率是480*319
	for(i = 0; i < 319; i++)
	{
		for(j = 0; j < 480; j++)
		{
			cnt = WIDTH * i + j; //当前像素点的编号
			//cnt *= 3; //当前像素点的数据在数组中的下标
			/*
				当前像素点对应的图像数据的RGB分别是：
				pDate[cnt+0] pDate[cnt+1] pDate[cnt+2]
			*/
			*(pfb + cnt) = ((pDate[a+0] << 16) | (pDate[a+1] << 8) | (pDate[a+2] << 0));//第一个像素点的数据
			a += 3;
		}
	}
}
#endif

#if 0
//测试显示比屏幕分辨率小的图片（任意起点,不超出屏幕）
void fb_draw_picture3(unsigned int x0 , unsigned int y0)
{
	const unsigned char *pDate = gImage_480319;
	unsigned int cnt1 = 0;//当前像素点的编号
	unsigned int cnt2 = 0;//源图像数据数组的偏移量，这样的是从图像像素点出发
	unsigned int x , y;

	//图片分辨率是480*319
	for(y = y0; y < y0+319; y++)
	{
		for(x = x0; x < x0+480; x++)
		{
			cnt1 = WIDTH * y + x;
			cnt2 = 480 * (y - y0) + (x - x0);

			//左边考虑的是当前像素点在fb内存中的偏移量
			//右边考虑的是当前像素点在图像数据数组中的下标
			*(pfb + cnt1) = ((pDate[3*cnt2+0] << 16) | (pDate[3*cnt2+1] << 8) | (pDate[3*cnt2+2] << 0));//第一个像素点的数据
		}
	}
}
#endif

#if 0
//测试显示比屏幕分辨率小的图片（任意起点，超出屏幕）
void fb_draw_picture4(unsigned int x0 , unsigned int y0)
{
	const unsigned char *pDate = gImage_480319;
	unsigned int cnt1 = 0;//当前像素点的编号
	unsigned int cnt2 = 0;//源图像数据数组的偏移量，这样的是从图像像素点出发
	unsigned int x , y;

	//图片分辨率是480*319
	for(y = y0; y < y0+319; y++)
	{
		if(y >= HEIGHT)
		{
			break;
		}
		for(x = x0; x < x0+480; x++)
		{
			if(x >= WIDTH)
			{
				continue;
			}
			cnt1 = WIDTH * y + x;
			cnt2 = 480 * (y - y0) + (x - x0);

			//左边考虑的是当前像素点在fb内存中的偏移量
			//右边考虑的是当前像素点在图像数据数组中的下标
			*(pfb + cnt1) = ((pDate[3*cnt2+0] << 16) | (pDate[3*cnt2+1] << 8) | (pDate[3*cnt2+2] << 0));//第一个像素点的数据
		}
	}
}
#endif
