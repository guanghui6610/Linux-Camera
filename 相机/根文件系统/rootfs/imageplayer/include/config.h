#ifndef __CONFIG_H__
#define __CONFIG_H__

/******************  宏定义  ******************/

//#define DEBUG //打开调试信息输出的开关
#ifdef DEBUG
#define debug(...)                                          \
        {                                                   \
            fprintf(stderr , "[debug][%s:%s:%d]" ,          \
                    __FILE__ , __FUNCTION__ , __LINE__);    \
            fprintf(stderr , __VA_ARGS__);                  \
        }                                                    
#else
#define debug(...)
#endif

//规定最大支持1920*1080这么大的图片
#define BMP_MAX_RESOLUTION      (2560*1440)
#define BMP_BUF_SIZE            (BMP_MAX_RESOLUTION*3)
//我们规定最多支持100张图片
#define MAX_IMAGE_CNT           100 
//触摸屏设备的设备名
#define DEVICE_TOUCHSCREEN      "/dev/input/event1"
#define TOUCH_WIDTH             200

/***********************  结构体定义  *********************/
//结构体用来封装一个图片的各种信息
typedef struct pic_info
{
    char *pathname; //图片在文件系统中的路径名+文件名
    unsigned int width; //图片分辨率的宽
    unsigned int height; //图片分辨率的高
    unsigned int bpp; //图片bpp
    char *pData; //指向图片有效数据存储的buffer数据
} pic_info;

/*************************** 全局变量 ******************************/
char rgb_buffer[BMP_BUF_SIZE];//存储像素数据的buf

/********************  函数声明  *******************************/
int is_bmp(const char *path);
int is_jpg(const char *path);
int is_png(const char *path);

int jpg_analyze(struct pic_info *pPic);

int display_bmp(const char *path);
int display_jpg(const char *path);
int display_png(const char *path);

void scan_image(const char *path);
void print_images(void);
void show_images(void);

void show_image(int index);
int ts_updown(void);

#endif