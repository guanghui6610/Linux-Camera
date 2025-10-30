#include "lcd.h"
#include "jpeg_handler.h"
#include "v4l2_camera.h"

// 全局LCD设备实例
lcd_device_t g_lcd = {0};

int lcd_init(lcd_device_t *lcd) {
    struct fb_var_screeninfo var;
    struct fb_fix_screeninfo fix;
    
    // 打开framebuffer设备
    lcd->fd_fb = open("/dev/fb0", O_RDWR);
    if (lcd->fd_fb < 0) {
        perror("打开LCD失败");
        return ERROR;
    }
    
    // 获取LCD信息
    if (ioctl(lcd->fd_fb, FBIOGET_VSCREENINFO, &var) < 0) {
        perror("获取屏幕可变信息失败");
        close(lcd->fd_fb);
        return ERROR;
    }
    // 在 lcd_init 函数中，获取var后添加
    printf("LCD参数：宽=%d, 高=%d, bpp=%d（每像素位数）\n", 
                var.xres, var.yres, var.bits_per_pixel);
    
    if (ioctl(lcd->fd_fb, FBIOGET_FSCREENINFO, &fix) < 0) {
        perror("获取屏幕固定信息失败");
        close(lcd->fd_fb);
        return ERROR;
    }
    
    // 设置LCD参数
    lcd->screen_size = var.xres * var.yres * var.bits_per_pixel / 8;
    lcd->width = var.xres;
    lcd->height = var.yres;
    lcd->bpp = var.bits_per_pixel;
    lcd->line_length = fix.line_length;
    
    // 映射framebuffer到内存
    lcd->fbbase = mmap(NULL, lcd->screen_size, PROT_READ | PROT_WRITE, 
                       MAP_SHARED, lcd->fd_fb, 0);
    if (lcd->fbbase == (unsigned char *)-1) {
        perror("内存映射失败");
        close(lcd->fd_fb);
        return ERROR;
    }
    
    // 清屏为白色
    memset(lcd->fbbase, 0xFF, lcd->screen_size);
    
    return SUCCESS;
}

void lcd_cleanup(lcd_device_t *lcd) {
    if (lcd->fbbase && lcd->fbbase != (unsigned char *)-1) {
        munmap(lcd->fbbase, lcd->screen_size);
        lcd->fbbase = NULL;
    }
    
    if (lcd->fd_fb >= 0) {
        close(lcd->fd_fb);
        lcd->fd_fb = -1;
    }
}

int lcd_clear_screen(lcd_device_t *lcd, unsigned int color) {
    if (!lcd || !lcd->fbbase) {
        return ERROR;
    }
    
    unsigned int *pixel = (unsigned int *)lcd->fbbase;
    int total_pixels = lcd->width * lcd->height;

    // 32位bpp时确保Alpha通道为不透明（如果传入的color不含Alpha）
    if (lcd->bpp == 32 && (color & 0xFF000000) == 0) {
        color |= 0xFF000000; // 补充Alpha=0xFF
    }
    
    for (int i = 0; i < total_pixels; i++) {
        pixel[i] = color;
    }
    
    return SUCCESS;
}

int lcd_show_jpeg_file(lcd_device_t *lcd, const char *jpeg_path) {
    return jpeg_decode_file_to_lcd(jpeg_path, lcd);
}

int lcd_show_jpeg_data(lcd_device_t *lcd, const char *jpeg_data, int size) {
    return jpeg_decode_data_to_lcd(jpeg_data, size, lcd);
}

/**
 * 显示RGB565格式像素数据
 * @param lcd: LCD设备
 * @param rgb565_data: RGB565数据缓冲区（每个像素2字节）
 * @param data_size: 数据总大小（字节）
 */

extern v4l2_camera_t g_camera;
#if 0
int lcd_show_rgb565_data(lcd_device_t *lcd, const unsigned char *rgb565_data, int data_size) {
    if (!lcd || !lcd->fbbase || !rgb565_data || data_size <= 0) {
        return ERROR;
    }

    // 计算有效像素数（摄像头和LCD取较小分辨率）
    //int valid_width = (CAMERA_WIDTH < lcd->width) ? CAMERA_WIDTH : lcd->width;
    //int valid_height = (CAMERA_HEIGHT < lcd->height) ? CAMERA_HEIGHT : lcd->height;
    // 新代码：使用摄像头实际输出的宽高（从全局变量g_camera获取）
    int valid_width = g_camera.width;  // 摄像头实际宽度
    int valid_height = g_camera.height;  // 摄像头实际高度
    // 若LCD分辨率更小，则取LCD的大小（避免超出LCD范围）
    if (valid_width > lcd->width) valid_width = lcd->width;
    if (valid_height > lcd->height) valid_height = lcd->height;


    int total_valid_pixels = valid_width * valid_height;
    int total_valid_bytes = total_valid_pixels * 2;  // RGB565每个像素2字节


    // 确保数据大小足够
    //if (data_size < total_valid_bytes) {
     //   printf("RGB565数据大小不足\n");
      //  return ERROR;
    //}

    // 新逻辑：取实际数据能覆盖的最大像素数
    int max_possible_pixels = data_size / 2;  // 每个像素2字节
    int max_possible_width = valid_width;
    int max_possible_height = max_possible_pixels / max_possible_width;
    if (max_possible_height > valid_height) max_possible_height = valid_height;

    // 更新实际有效字节数（按实际能显示的像素计算）
    total_valid_bytes = max_possible_width * max_possible_height * 2;

    // 直接将RGB565数据写入LCD帧缓冲（假设LCD帧缓冲支持RGB565格式）
    // 若LCD为RGB888，需在此处添加RGB565转RGB888的转换逻辑
    memcpy(lcd->fbbase, rgb565_data, total_valid_bytes);

    return SUCCESS;
}
#endif

#if 0
int lcd_show_rgb565_data(lcd_device_t *lcd, const unsigned char *rgb565_data, int data_size) {
    if (!lcd || !lcd->fbbase || !rgb565_data || data_size <= 0) {
        return ERROR;
    }

    // 获取有效宽高（摄像头与LCD取较小值）
    int valid_width = g_camera.width;
    int valid_height = g_camera.height;
    if (valid_width > lcd->width) valid_width = lcd->width;
    if (valid_height > lcd->height) valid_height = lcd->height;

    // 计算实际可显示的像素数（RGB565每个像素2字节）
    int max_possible_pixels = data_size / 2;
    int max_possible_width = valid_width;
    int max_possible_height = max_possible_pixels / max_possible_width;
    if (max_possible_height > valid_height) max_possible_height = valid_height;

    // 每行有效数据字节数（RGB565）
    int line_valid_bytes = max_possible_width * 2;

    // 逐行复制数据（关键修复：按line_length偏移行地址）
    unsigned char *lcd_ptr = lcd->fbbase;  // 当前行的LCD帧缓冲地址
    const unsigned char *data_ptr = rgb565_data;  // 当前行的RGB565数据地址

    for (int y = 0; y < max_possible_height; y++) {
        // 复制当前行的有效数据
        memcpy(lcd_ptr, data_ptr, line_valid_bytes);
        // 移动到下一行（使用line_length确保对齐）
        lcd_ptr += lcd->line_length;
        data_ptr += line_valid_bytes;
    }

    return SUCCESS;
}
#endif

int lcd_show_rgb565_data(lcd_device_t *lcd, const unsigned char *rgb565_data, int data_size) {
    if (!lcd || !lcd->fbbase || !rgb565_data || data_size <= 0) {
        return ERROR;
    }

    // 获取有效宽高（摄像头与LCD取较小值）
    int valid_width = g_camera.width;
    int valid_height = g_camera.height;
    if (valid_width > lcd->width) valid_width = lcd->width;
    if (valid_height > lcd->height) valid_height = lcd->height;

    // 计算实际可显示的像素数
    int max_possible_pixels = data_size / 2;
    int max_possible_width = valid_width;
    int max_possible_height = max_possible_pixels / max_possible_width;
    if (max_possible_height > valid_height) max_possible_height = valid_height;

    // 计算居中偏移（关键：让图像在LCD中居中）
    int x_offset = (lcd->width - max_possible_width) / 2;  // X方向偏移（列）
    int y_offset = (lcd->height - max_possible_height) / 2; // Y方向偏移（行）

    // 每行有效数据字节数（RGB565）
    int line_valid_bytes = max_possible_width * 2;

    // 计算LCD帧缓冲的起始地址（考虑偏移）
    unsigned char *lcd_ptr = lcd->fbbase + y_offset * lcd->line_length + x_offset * 2;
    const unsigned char *data_ptr = rgb565_data;

    // 逐行复制数据（按line_length偏移行地址）
    for (int y = 0; y < max_possible_height; y++) {
        memcpy(lcd_ptr, data_ptr, line_valid_bytes);
        lcd_ptr += lcd->line_length;  // 下一行（按硬件行宽偏移）
        data_ptr += line_valid_bytes; // 下一行数据
    }

    return SUCCESS;
}
#if 0
int lcd_show_rgb565_data(lcd_device_t *lcd, const unsigned char *rgb565_data, int data_size) {
    if (!lcd || !lcd->fbbase || !rgb565_data || data_size <= 0) {
        return ERROR;
    }

    // 1. 固定显示宽度为左侧800像素（刚好覆盖非触摸区）
    int valid_width = 800;  // 与TOUCH_AREA_X_MIN一致，覆盖左侧白色背景
    // 若摄像头宽度小于800，以摄像头实际宽度为准（避免拉伸）
    if (g_camera.width < valid_width) {
        valid_width = g_camera.width;
    }

    // 2. 显示高度为全屏600像素（覆盖整个左侧区域）
    int valid_height = 600;  // 与LCD高度一致
    if (g_camera.height < valid_height) {
        valid_height = g_camera.height;
    }

    // 3. 计算实际可显示的像素数（RGB565每个像素2字节）
    int max_possible_pixels = data_size / 2;
    int max_possible_height = max_possible_pixels / valid_width;
    if (max_possible_height > valid_height) {
        max_possible_height = valid_height;
    }

    // 4. 从左侧X=0开始显示，Y方向顶部对齐（完全覆盖白色背景）
    int x_offset = 0;       // 左侧起点
    int y_offset = 0;       // 顶部起点（无偏移，全屏覆盖）

    // 5. 每行有效数据字节数（RGB565）
    int line_valid_bytes = valid_width * 2;

    // 6. 计算LCD帧缓冲起始地址（左侧X=0，顶部Y=0）
    unsigned char *lcd_ptr = lcd->fbbase + y_offset * lcd->line_length + x_offset * 2;
    const unsigned char *data_ptr = rgb565_data;

    // 7. 逐行复制数据（按line_length偏移，确保对齐）
    for (int y = 0; y < max_possible_height; y++) {
        memcpy(lcd_ptr, data_ptr, line_valid_bytes);
        lcd_ptr += lcd->line_length;  // 下一行（硬件行宽）
        data_ptr += line_valid_bytes; // 下一行数据
    }

    return SUCCESS;
}
#endif

// lcd.c 新增函数：从文件读取RGB565数据并显示
int lcd_show_rgb565_file(lcd_device_t *lcd, const char *rgb565_path) {
    if (!lcd || !lcd->fbbase || !rgb565_path) {
        return ERROR;
    }

    // 打开RGB565文件
    FILE *file = fopen(rgb565_path, "rb");
    if (!file) {
        perror("打开RGB565文件失败");
        return ERROR;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配缓冲区并读取数据
    unsigned char *rgb565_data = malloc(file_size);
    if (!rgb565_data) {
        fclose(file);
        return ERROR;
    }
    fread(rgb565_data, 1, file_size, file);
    fclose(file);

    // 调用现有函数显示RGB565数据
    int ret = lcd_show_rgb565_data(lcd, rgb565_data, file_size);
    free(rgb565_data); // 释放缓冲区
    return ret;
}
