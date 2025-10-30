#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>

#include "image_manage.h"
#include <config.h>
#include "fb.h"


//定义存储图片信息（图片名字和类型）的数组
image_info_t images[MAX_IMAGE_CNT];
unsigned int image_index = 0;

/*
    images数组本来是空的，然后程序初始化时会去一个事先约定好的目录（image目录）下
    递归检索所有的文件及子文件夹，并且将所有的图片格式收集并填充记录到images数组中
    经过检索后，images数组中就记录了所有的图片信息
    然后显示图片函数再去images数组中拿出相应的图片显示即可
    path就是要去检索的文件夹的pathname
*/
void scan_image(const char *path)
{
    //在本函数中递归检索path文件夹，并将其中所有图片填充到images数组中去
    DIR *dir;
    struct dirent *ptr;
    char base[1000];
    struct stat sta;

    dir = opendir(path);
    if(dir == NULL)
    {
        perror("Open dir error\r\n");
        exit(1);
    }

    /*
        readdir函数每调用一次就会返回opendir打开的文件夹目录下的一个文件
        直到该目录下的所有文件都被读完之后，就会返回NULL
    */
    while((ptr = readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name , ".") == 0 || strcmp(ptr->d_name , "..") == 0)
        continue;

        //用lstat来读取文件属性并判断文件类型
        memset(base , '\0' , sizeof(base));
        strcpy(base , path);
        strcat(base , "/");
        strcat(base , ptr->d_name);
        lstat(base , &sta);

    /*
        如果是普通文件，就要在这里进行处理
        处理思路就是先判定是否属于已知的某种图片格式，如果是则放到images数组中去
    */
        if(S_ISREG(sta.st_mode))
        {
            if(is_bmp(base) == 0)
            {
                strcpy(images[image_index].pathname , base);
                images[image_index].type = IMAGE_TYPE_BMP;
            }
            if(is_jpg(base) == 0)
            {
                strcpy(images[image_index].pathname , base);
                images[image_index].type = IMAGE_TYPE_JPG;
            }
            if(is_png(base) == 0)
            {
                strcpy(images[image_index].pathname , base);
                images[image_index].type = IMAGE_TYPE_PNG;
            }
            image_index++;
        }

        //如果是文件夹，递归调用
        if(S_ISDIR(sta.st_mode))
        {
            scan_image(base);
        }
    }
}

/*
    print_images函数和show_images函数都是滚动播放图片需要的
*/
void print_images(void)
{
    int i;

    printf("images_index=%d\r\n" , image_index);
    for(i = 0 ; i < image_index ; i++)
    {
        printf("image[i].pathname=%s , type=%d\r\n" , images[i].pathname , images[i].type);
    }
}

void show_images(void)
{
    int i;
    for(i = 0 ; i < image_index ; i++)
    {
        switch(images[i].type)
        {
            case IMAGE_TYPE_BMP:
                display_bmp(images[i].pathname);
                    break;
            case IMAGE_TYPE_JPG:
                display_jpg(images[i].pathname);
                    break;
            case IMAGE_TYPE_PNG:
                display_png(images[i].pathname);
                    break;
            default:
                    break;
        }
        sleep(2);
    }
}

void show_image(int index)
{
    switch(images[index].type)
        {
            case IMAGE_TYPE_BMP:
                display_bmp(images[index].pathname);
                debug("第 %d 张\r\n" , index+1);
                    break;
            case IMAGE_TYPE_JPG:
                display_jpg(images[index].pathname);
                debug("第 %d 张\r\n" , index+1);
                    break;
            case IMAGE_TYPE_PNG:
                display_png(images[index].pathname);
                debug("第 %d 张\r\n" , index+1);
                    break;
            default:
                    break;
        }
}



