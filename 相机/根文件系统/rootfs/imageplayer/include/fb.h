#include <config.h>

// 宏定义
#define FBDEVICE	"/dev/fb0"

// 屏幕大小
#define WIDTH		1024	
#define HEIGHT		600

#define WHITE		0xffffffff			// test ok
#define BLACK		0x00000000
#define RED			0xffff0000
#define GREEN		0xff00ff00			// test ok
#define BLUE		0xff0000ff			

// 函数声明
int fb_open(void);
void fb_close(void);
void fb_draw_back(unsigned int width, unsigned int height, unsigned int color);
void fb_draw_line(unsigned int color);

/*
void fb_draw_picture(void);
void fb_draw_picture2(void);
void fb_draw_picture3(unsigned int x0 , unsigned int y0);
void fb_draw_picture4(unsigned int x0 , unsigned int y0);
*/

//彼此差异是图片转了180度，RGB和BGR
void fb_draw(const struct pic_info *pPic);
void fb_draw2(const struct pic_info *pPic);
void scale_image(const char *src , int sw , int sh,
					char *dst , int dw , int dh);
void fb_draw_image_adapt(const struct pic_info *pPic);
void fb_draw_image_adapt2(const struct pic_info *pPic);
void fb_draw_jpg_fadein(const char *path, unsigned int steps, unsigned int delay_ms);
void play_boot_logo(const char *jpg_path);