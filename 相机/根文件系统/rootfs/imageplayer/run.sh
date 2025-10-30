#!/bin/sh
export LD_LIBRARY_PATH=/imageplayer/include/jpg/lib:$LD_LIBRARY_PATH
cd /lib/modules/4.1.15
depmod
modprobe gt9147.ko
cd /imageplayer
./imageplayer
