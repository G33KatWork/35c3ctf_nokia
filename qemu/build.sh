#!/bin/bash

cd qemu
patch -p1 < ../nokia.patch
./configure --disable-werror --target-list=arm-softmmu --enable-sdl --disable-glusterfs --disable-capstone
make
cd ..
