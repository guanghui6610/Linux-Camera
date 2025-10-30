
#CROSS_COMPILE = arm-none-linux-gnueabi-
#不使用绝对路径的话，如果系统PATH环境中没有包含交叉编译器的路径，会报错，或在PATH中包含多个交叉编译器，系统可能使用错误的版本
CROSS_COMPILE = /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-


AS			= $(CROSS_COMPILE)as
LD			= $(CROSS_COMPILE)ld
CC			= $(CROSS_COMPILE)gcc
CPP			= $(CC) -E
AR			= $(CROSS_COMPILE)ar
NM			= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

# export导出的变量是给子目录下的Makefile使用的
export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP

# 编译器在编译时的参数设置
# C编译器所使用到的编译参数 
# -Wall 启用大部分警告信息 
# -O2 优化级别2
# -g 生成调试信息
# -DDEBUG 定义预处理宏DEBUG
CFLAGS := -Wall -O2 -g -DDEBUG
# 添加头文件路径，不添加的话include目录下的头文件编译时找不到
CFLAGS += -I $(shell pwd)/include -I $(shell pwd)/include/jpg/include -I $(shell pwd)/include/png/include -I $(shell pwd)/include/png/zlib/include
CFLAGS += -std=c99
# 链接器的链接参数设置
LDFLAGS := -ljpeg -lz -lpng -L$(shell pwd)/include -L$(shell pwd)/include/jpg/lib -L$(shell pwd)/include/png/lib -L$(shell pwd)/include/png/zlib/lib

export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

# 定义将来编译生成的可执行程序的名字
TARGET := imageplayer

# 添加项目中所有用到的源文件，有顶层目录下的.c文件，和子文件夹
# 添加顶层目录下的.c文件
obj-y += main.o

# 添加顶层目录下的子文件夹（注意目录名后面加一个/）
obj-y += display/
obj-y += image_manage/
obj-y += touch/

all: 
	make -C ./ -f $(TOPDIR)/Makefile.build
	$(CC) $(LDFLAGS) -o $(TARGET) built-in.o

cp:
	cp ../imageplayer/ /home/guanghui/linux/nfs/rootfs -rf

clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET)

#从别的电脑上拷贝过来的需要make distclean一下，把.d这些依赖文件给删了
distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET)
	