#!/bin/bash

cd $(dirname $0)

YOCTO_DEPLOY_DIR=../linux/build/tmp-glibc/deploy/images/nokia
cp ${YOCTO_DEPLOY_DIR}/bios.bin .
cp ${YOCTO_DEPLOY_DIR}/zImage .
cp ${YOCTO_DEPLOY_DIR}/nokia-image-nokia.ext4 .
cp ${YOCTO_DEPLOY_DIR}/optee/tee-header_v2.bin .
cp ${YOCTO_DEPLOY_DIR}/optee/tee-pageable_v2.bin .
cp ${YOCTO_DEPLOY_DIR}/optee/tee-pager_v2.bin .

qemu/arm-softmmu/qemu-system-arm \
    -M virt,secure=on \
    -cpu cortex-a15 \
    -m 256 \
    -bios bios.bin \
    -monitor null \
    -serial stdio \
    -semihosting \
    -display sdl \
    -drive if=none,id=drive0,format=raw,file=nokia-image-nokia.ext4 \
    -device virtio-blk-device,drive=drive0 \
    -netdev user,id=net0,hostfwd=tcp::2222-:22 \
    -device virtio-net-device,netdev=net0 \
    -device virtio-rng-device
