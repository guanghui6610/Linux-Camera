/*
    本文件用来解码png图片，并调用fb.c中的显示接口来显示图片
*/

#include <stdio.h>
#include <string.h>

#include <config.h>

#include <png.h>
#include <pngstruct.h>
#include <pnginfo.h>
#include <fb.h>

#define PNG_BYTES_TO_CHECK  8

//函数功能：判断一个图片文件是不是png图片
//函数参数：path是图片文件的pathname
//返回值：如果是png则返回0，不是则返回1 ，错误则返回-1
int is_png(const char *path)
{
    FILE *fd = NULL;
    unsigned char buf[PNG_BYTES_TO_CHECK] = {0};

    if ((fd = fopen(path, "rb")) == NULL)
    {
        return -1;
    }   
    if (fread(buf, 1, PNG_BYTES_TO_CHECK, fd) != PNG_BYTES_TO_CHECK)
    {
        return -1;
    }
    return(png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK));
}


//函数功能：解码png图片，并将解码出来的数据存储
//函数参数：pPic，记录原png图片解码出来的图片宽高、图片数据缓冲区等信息
//成功返回0，失败返回-1
static int png_analyze(struct pic_info *pPic)
{
    FILE *fd = NULL;
    png_structp png_ptr;//解码操作指针
    png_infop info_ptr;//图片信息结构体指针
    int color_type;
    png_bytep* row_pointers;//指向每一行像素数据的指针数组
    //unsigned int len = 0;
    int pos = 0;
    int x = 0 , y = 0;

    //1.打开文件
    if((fd = fopen(pPic->pathname , "rb")) == NULL)
    {
        printf("can't open %s\r\n" , pPic->pathname);
        return -1;
    }

    //2.相关数据结构实例化
    //这个结构体是libpng进行所有的读取操作，（libpng库的版本号，自定义的错误数据结构的指针，自定义的错误处理函数的指针，自定义的警告处理函数的指针）
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING , 0 , 0 , 0);
    if(png_ptr == 0)
    {
        fclose(fd);
        return -1;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == 0)
    {   
        png_destroy_read_struct(&png_ptr , 0 , 0);
        fclose(fd);
        return -1;
    }    

    //3.设置错误跳转
    if(setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr , &info_ptr , 0);
        fclose(fd);
        return -1;
    }

    //4.将要解码的png图片的文件指针和png解码器绑定起来
    png_init_io(png_ptr , fd);

    //5.读取png图片信息
    //(主控制结构体指针 ， 信息结构体指针 ， 对图像数据进行一系列转换 ， 用于某些特定转换的附加参数)
    /*
        第三个参数解析：
        读取这个png图片，如果是一个调色板图像，把它转换成RGB图。
        如果它的位深度小于8，把它变成8位
        如果图像还带有Alpha透明通道，扔掉它
    */
    png_read_png(png_ptr , info_ptr , PNG_TRANSFORM_EXPAND|PNG_TRANSFORM_STRIP_ALPHA , 0);   

    //6.相关图片信息打印出来看一下
    color_type = info_ptr->color_type;
    //debug("color_type = %d\r\n" , color_type);
    pPic->width = info_ptr->width;
    pPic->height = info_ptr->height;
    pPic->bpp = info_ptr->pixel_depth;
    //len = info_ptr->width * info_ptr->height * info_ptr->pixel_depth / 8;
    debug("picture resolution: %u * %u\r\n" ,pPic->width , pPic->height);
    debug("bpp: %u\r\n" , pPic->bpp);
    //debug("len = %lu\r\n" , len);

    //7.获取每一行像素数据的指针
    row_pointers = png_get_rows(png_ptr , info_ptr);

    //只处理RGB24位真彩色图片，其他格式的图片不管
    //8.图像数据移动到pPic中的pDate中
    if(color_type == PNG_COLOR_TYPE_RGB)
    {
        //memcpy(pPic->pData , row_pointers , len);
        for(y=0 ; y<pPic->height ; y++)
        {
            for(x=0 ; x<3*pPic->width ; x+=3)
            {
                pPic->pData[pos++] = row_pointers[y][x+0];
                pPic->pData[pos++] = row_pointers[y][x+1];
                pPic->pData[pos++] = row_pointers[y][x+2];
            }
        }
    }

    //9.收尾处理，释放内存，关闭文件
    png_destroy_read_struct(&png_ptr , &info_ptr , 0);
    fclose(fd);
    return 0;
}

//封装一个对外使用的png显示函数
//本函数对外只需要一个png图片的pathname即可，复杂的数据结构都是png显示模块内部处理的
//正确显示图片显示0。显示过程出错，返回-1
int display_png(const char *path)
{
    int ret = -1;
    struct pic_info picture;//要显示的图片

    //1.检查图片是不是png
    ret = is_png(path);
    if(ret != 0)
    {
        return -1;
    }

    //2.解析png图片
    picture.pathname = (char *)path;
    picture.pData = rgb_buffer;
    png_analyze(&picture);

    //3.显示png图片
    //fb_draw2(&picture);
    fb_draw_image_adapt(&picture);

    return 0;
}
