#!/bin/sh
SD_BIN=$(ls | grep .bin)

# uniprocessor
#qemu-system-arm -M vexpress-a9 -cpu cortex-a9 -nographic -kernel kernel.img -initrd rd.img

# multiprocessor
qemu-system-arm -M vexpress-a9 -m 32MB -cpu cortex-a9 -smp cores=2 -nographic -kernel kernel.img -dtb test.dtb -initrd rd.img #vexpress_u-boot -drive format=raw,if=sd,file=$SD_BIN

# mp w/graphics
#qemu-system-arm -M vexpress-a9 -cpu cortex-a9 -smp cores=2 -serial stdio -kernel kernel.img -initrd rd.img
