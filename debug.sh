#!/bin/sh
SD_BIN=$(ls | grep .bin)

qemu-system-arm -M vexpress-a9 -cpu cortex-a9 -smp cores=4 -nographic -kernel kernel.img -initrd rd.img -s -S #vexpress_u-boot -drive format=raw,if=sd,file=$SD_BIN
