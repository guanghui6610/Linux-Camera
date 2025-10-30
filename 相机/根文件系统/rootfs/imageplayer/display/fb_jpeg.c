/*
    本文件用来解码jpg图片，并调用fb.c中的显示接口来显示图片
*/

#include <stdio.h>
#include <config.h>
#include <fb.h>
#include <jpeglib.h>
#include <jerror.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>

struct my_error_mgr 
{
  struct jpeg_error_mgr pub;	
  jmp_buf setjmp_buffer;
};	
typedef struct my_error_mgr * my_error_ptr;

//函数功能：判断一个图片文件是不是jpg图片
//函数参数：path是图片文件的pathname
//返回值：如果是jpg则返回0，不是则返回1 ，错误则返回-1
int is_jpg(const char *path)
{
    FILE *fd = NULL;
    char buf[2] = {0};

    //打开文件
    fd = fopen(path , "rb");
    if(fd == NULL)
    {
        printf("fopen %s error\r\n" , path);
        return -1;
    }

    //判断前两个字节是不是0xffd8
    fread(buf , 1 , 2 , fd);
    if(!((buf[0] == 0xff) && (buf[1] == 0xd8)))
    {
        fclose(fd);
        return 1;
    }

    //是0xffd8开头，就继续
    //将文件指针移动到倒数2个字符的位置
    //判断最后两个字节是不是0xffd9
    fseek(fd , -2 , SEEK_END);
    fread(buf , 1 , 2 , fd);
    if(!((buf[0] == 0xff) && (buf[1] == 0xd9)))
    {
        fclose(fd);
        return 1;
    }

    fclose(fd);
    return 0;
}

//自己定义的错误处理函数
METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    printf("my_error_exit\r\n");
    longjmp(myerr->setjmp_buffer , 1);
}

//函数功能：解码jpg图片，并将解码出来的数据存储
//函数参数：pPic，记录原jpg图片解码出来的图片宽高、图片数据缓冲区等信息
//成功返回0，失败返回-1
int jpg_analyze(struct pic_info *pPic)
{
    struct jpeg_decompress_struct cinfo; //cinfo贯穿整个解码过程的信息记录和传递数据结构
    struct my_error_mgr jerr; //错误处理

    unsigned char *buffer = NULL; //指向解码后的行指针组成的数组
    FILE *infile; //指向fopen打开原jpg图片文件的指针，相当于fd
    int row_stride; //解码出来的一行图片信息的字节数

    //1.打开文件
    /*
        q：为什么这里使用fopen，而之前的fb.c中却使用的是open？
        a: fb.c文件涉及到需要打开framebuffer(/dev/fb0)，这是一个设备文件，open返回的是int fd类型，表示底层文件或设备文件
            而现在的文件中涉及到libjpeg，而libjpeg的jpeg_stdio_src函数需要FILE*类型的流
    */
    if((infile = fopen(pPic->pathname , "rb")) == NULL)
    {
        printf("can't open %s\r\n" , pPic->pathname);
        return -1;
    }

    //2.错误处理函数部分的绑定
    cinfo.err = jpeg_std_error(&jerr.pub);//jpeg_std_error设置一套标准的错误处理函数
    jerr.pub.error_exit = my_error_exit;
    //设置setjmp跳转点
    if(setjmp(jerr.setjmp_buffer))
    {
        printf("JPEG decode failed,cleaning up!\r\n");
        if(buffer)
        {
            free(buffer);//释放自己分配的内存
        }
        jpeg_destroy_decompress(&cinfo);//释放libjpeg内部用到的内存和结构体
        if(infile)
        {
            fclose(infile);//关闭打开的文件
        }
        return -1;
    }

    //3.初始化jpeg解码器，给解码器做必要的内存分配和数据结构的初始化
    jpeg_create_decompress(&cinfo);

    //4.将fopen打开的原jpg图片和解码器相关联
    jpeg_stdio_src(&cinfo , infile);

    //5.读原jpg文件的头
    jpeg_read_header(&cinfo , TRUE);

    //6.启动解码器
    jpeg_start_decompress(&cinfo);
    //相关图片信息
    pPic->width = cinfo.output_width;
    pPic->height = cinfo.output_height;
    pPic->bpp = 8*cinfo.output_components;
    debug("picture resolution: %u * %u\r\n" ,pPic->width , pPic->height);
    debug("bpp: %u\r\n" , pPic->bpp);
    //解码出来的一行数据的字节数
    row_stride = cinfo.output_width * cinfo.output_components;
    //malloc分配出来的是指针类型
    buffer = malloc(row_stride);
    if(buffer == NULL)
    {
        printf("cinfo.mem->alloc_sarray error\r\n");
        return -1;
    }

    //7.逐行解码，并将解码出来的数据放到事先准备好的缓冲区中
    while(cinfo.output_scanline < cinfo.output_height)
    {
        //解码一行信息，并且放到buffer中
        jpeg_read_scanlines(&cinfo , &buffer , 1);

        //将buffer中的这一行数据移走到别的地方去暂存或在使用
        //总之要腾出buffer空间，来将循环的下一次解码一行来使用
        memcpy(pPic->pData + (cinfo.output_scanline - 1) * row_stride , buffer , row_stride);
    }

    //8.解码完了，做各种清理工作
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    return 0;
}

//封装一个对外使用的jpg显示函数
//本函数对外只需要一个jpg图片的pathname即可，复杂的数据结构都斯jpg显示模块内部处理的
//正确显示图片显示0。显示过程出错，返回-1
int display_jpg(const char *path)
{
    int ret = -1;
    struct pic_info picture;//要显示的图片

    //1.检查图片是不是jpg
    ret = is_jpg(path);
    if(ret != 0)
    {
        return -1;
    }

    //2.解析jpg图片
    picture.pathname = (char *)path;
    picture.pData = rgb_buffer;
    jpg_analyze(&picture);

    //3.显示jpg图片
    //fb_draw2(&picture); 
    fb_draw_image_adapt(&picture);
    return 0;
}


