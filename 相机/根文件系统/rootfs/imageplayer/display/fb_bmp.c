//本文件用来解析BMP图片，并且显示到fb中
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "bmp.h"
#include <fb.h>
#include <config.h>

//判断一个图片文件是不是一个合法的bmp文件
//返回值：如果是则返回0，不是则返回-1
int is_bmp(const char *path)
{
    int fd = -1;
    unsigned char buf[2] = {0};
    ssize_t ret = 0;

    //1.打开一个bmp图片
    fd = open(path , O_RDONLY);
    if(fd < 0)
    {
        printf("open %s error\r\n" , path);
        return -1;
    }

    //2.读取文件头信息
    ret = read(fd , buf , 2);
    if(ret != 2)
    {
        printf("read file header error\r\n");
        return -1;
    }

    //3.解析头信息，判断是不是BMP图片
    if((buf[0] != 'B') || (buf[1] != 'M'))
    {
        ret = -1;
        goto close;
    }

    ret = 0;

close:
    close(fd);
    return ret;
}


//path:要解析的bmp图片的路径
//V1功能：该函数解析path这个bmp图片，并且将解析出来的图片数据放到bmp_buffer数组中
//返回值：错误时返回-1 ， 解析正确时候返回0
static int bmp_analyze(struct pic_info *pPic)
{
    int fd = -1;
    ClBitMapFileHeader fHeader;
    ClBitMapInfoHeader info;
    unsigned int len;

    //1.打开一个bmp图片
    fd = open(pPic->pathname , O_RDONLY);
    if(fd < 0)
    {
        printf("open %s error\r\n" , pPic->pathname);
        return -1;
    }

    //2.读取文件头信息
    read(fd , &fHeader , sizeof(fHeader));
    //debug("bfSize = %ld\r\n" , fHeader.bfSize);
    //debug("bfOffBits = %ld\r\n" , fHeader.bfOffBits);

    read(fd , &info , sizeof(info));
    debug("picture resolution: %ld * %ld\r\n" , info.biWidth , info.biHeight);
    debug("bpp: %d\r\n" , info.biBitCount);
    debug("biHeight = %d\r\n" , info.biHeight);

    //利用输出型参数
    pPic -> width = info.biWidth;
    pPic -> height = info.biHeight;
    pPic -> bpp = info.biBitCount;

    //3.读取图片的有效信息
    //先把文件指针移动到有效信息的偏移量
    //然后读出info.width * info.height * info.bibitcount / 3这么多字节即可
    lseek(fd , fHeader.bfOffBits , SEEK_SET); 
    len = info.biWidth * info.biHeight * info.biBitCount / 3;
    read(fd , rgb_buffer , len);
    pPic -> pData = rgb_buffer;

    //4.把内容丢到fb中去显示
    //fb_draw(pPic);
    fb_draw_image_adapt2(pPic);

    //关闭打开的文件
    close(fd);
    return 0;
}

//封装一个对外使用的bmp显示函数
//本函数对外只需要一个bmp图片的pathname即可，复杂的数据结构都斯jpg显示模块内部处理的
//正确显示图片显示0。显示过程出错，返回-1
int display_bmp(const char *path)
{
    int ret = -1;
    struct pic_info picture;//要显示的图片

    //1.检查图片是不是bmp
    ret = is_bmp(path);
    if(ret != 0)
    {
        return -1;
    }
    
    //2.显示bmp图片
    picture.pathname = (char *)path;
    picture.pData = rgb_buffer; //上面analyze中指定过了
    bmp_analyze(&picture);

    return 0;
}