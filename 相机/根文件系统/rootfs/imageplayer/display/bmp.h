#ifndef __BMP_H__
#define __BMP_H__

//BMP的文件头
typedef struct 
{
    unsigned short      bfType;
    unsigned long       bfSize;
    unsigned short      bfReserved1;
    unsigned short      bfReserved2;
    unsigned long       bfOffBits;//图像位数据相当于文件开头的偏移量
} __attribute__((packed)) ClBitMapFileHeader;

//BMP的信息头
typedef struct
{
    unsigned long       biSize;//信息头的大小，固定为40
    long                biWidth;
    long                biHeight;
    unsigned short      biPlanes;
    unsigned short      biBitCount;
    unsigned long       biCompression;
    unsigned long       biSizeImage;
    long                biXPelsPerMeter;
    long                biYPelsPerMeter;
    unsigned long       biClrUsed;
    unsigned long       biClrImportant;
} __attribute__((packed)) ClBitMapInfoHeader;

#endif